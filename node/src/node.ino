// Sample RFM69 sender/node sketch, with ACK and optional encryption
// Sends periodic messages of increasing length to gateway (id=1)
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
#include <RFM69.h>
#include <SPI.h>
#include <EEPROM.h>
#include "cylon_packet.h"
#include "config.h"
#include "power_save.h"

int TRANSMITPERIOD = 500; //transmit a packet to gateway so often (in ms)
char payload[] = "123ABC";
char buff[20];
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;

//in eeprom
uint32_t self_address;
uint32_t gateway_address;
uint8_t nodeid;
uint8_t networkid;
uint8_t sleep_duration; // number of 8seconds to sleep for

//flag that PIR interrupt occurred in last $interval time
bool pir_int_flag=false;
bool wdt_int_flag=false;

//little endian
uint32_t get_eeprom_dword(int offset)
{
  uint32_t retval;
  uint8_t * p=(uint8_t *)(void *)&retval;

  for (int i = 0; i < sizeof(retval); i++)
    *p++ = EEPROM.read(offset++);

  return retval;
}

uint32_t read_vcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  uint32_t result = ((uint32_t)(high<<8)) | ((uint32_t)low);
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void pir_interrupt_handler()
{
  pir_int_flag = true;
}

// Watchdog Interrupt Service Routine. 
// Very first thing after sleep wakes with WDT Interrupt
ISR(WDT_vect) {
  wdt_int_flag = true;
}

// called when pir instead of wdt wakes up from sleep
inline void go_back_to_sleep()
{
  Serial.println("go_back_to_sleep");
  Serial.flush();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  if(wdt_int_flag == true)
  {
    // if wdt triggered meanwhile, dont sleep
    return;
  }
  sleep_enable();
  sei();
  sleep_cpu();
  cli();
  sleep_disable();
  sei();
}

inline void sleep_till_wdti()
{
  Serial.println("sleep_till_wdti");
  Serial.flush();

  wdt_enable_intonly();// 8seconds
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();
  sleep_enable();
  sei();
  sleep_cpu();
  cli();
  sleep_disable();
  sei();
}

// times is multiples of 8s to sleep for
void long_sleep(uint8_t times)
{
  Serial.println("longsleep");
  if(times == 0)
    times = 1;

  // digitalWrite(BOOST_REG_PIN, HIGH);
  if(times > 1)
  {
    // turn on boost and sleep for 1 cycle(8s)
    sleep_till_wdti();
    times--;
    cli();
    pir_int_flag = false;
    sei();
  }

  while(times != 0)
  {
    times--;

    sleep_till_wdti();

    check_again:
    Serial.println("som");

    // turn boost off only if we have to to sleep for 1 cycle(8s)
    // if((pir_int_flag == true))
    //   digitalWrite(BOOST_REG_PIN, LOW);

    // if wdt interrupt, then wake up, else go back to sleep
    if(wdt_int_flag == true)
    {
      wdt_int_flag = false;
      Serial.println("waking up");
    }
    else
    {
      Serial.println("PIR int");
      go_back_to_sleep();
      goto check_again;
    }
  }
}

void transmit_status()
{
  char msg[MAX_PACKET_SIZE];static int i;

  //send data
  sprintf(msg, "num: %d batt: %lumV state: %10s", i++, read_vcc(), pir_int_flag?"present":"absent");
  Serial.println(msg);
  Serial.println("Sending packet");
  if(send_data(radio, self_address, gateway_address, REQUEST_ACK, strlen(msg), msg, 2))
  {
    Serial.println(" - ACK rcvd");
  }
  else
  {
    Serial.println(" - nothing");
  }
  Serial.println("sent.");

  // clear PIR sensor flag
  if(pir_int_flag==true)
  {
    Serial.println("at seat");
    pir_int_flag = false;
  }
  else
  {
    Serial.println("not at seat");
  }
}

void setup() 
{
  // clock_prescale_set(clock_div_8);

  Serial.begin(SERIAL_BAUD);
  Serial.println("start");

  // enable boost regulator
  // pinMode(BOOST_REG_PIN, OUTPUT);
  // digitalWrite(BOOST_REG_PIN, HIGH);

  // enable PIR sensor
  pinMode(PIR_POWER_PIN, OUTPUT);
  digitalWrite(PIR_POWER_PIN, HIGH);

  // attach interrupt on pir output pin
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  attachInterrupt(PIR_IRQ_NUM, pir_interrupt_handler, RISING);

  // get own address and gateway address from eeprom
  self_address = get_eeprom_dword(_SELF_ADDRESS_OFF);
  gateway_address = get_eeprom_dword(_GATEWAY_ADDRESS_OFF);
  nodeid = EEPROM.read(_FIXED_NODEID_OFF);
  networkid = EEPROM.read(_FIXED_NETWORKID_OFF);
  sleep_duration = EEPROM.read(_SLEEP_DURATION_OFF);

  self_address = 0xdeadbeef;
  gateway_address = 0xabad1dea;
  nodeid = NODEID;
  networkid = NETWORKID;
  sleep_duration = 2;

  // EEPROM.write(_SELF_ADDRESS_OFF, 0xef);EEPROM.write(_SELF_ADDRESS_OFF+1, 0xbe);EEPROM.write(_SELF_ADDRESS_OFF+2, 0xad);EEPROM.write(_SELF_ADDRESS_OFF+3, 0xde);
  // EEPROM.write(_GATEWAY_ADDRESS_OFF, 0xea);EEPROM.write(_GATEWAY_ADDRESS_OFF+1, 0x1d);EEPROM.write(_GATEWAY_ADDRESS_OFF+2, 0xad);EEPROM.write(_GATEWAY_ADDRESS_OFF+3, 0xab);
  // EEPROM.write(_FIXED_NODEID_OFF, 2);
  // EEPROM.write(_FIXED_NETWORKID_OFF, 100);

  Serial.print("Self: ");Serial.println(self_address, HEX);
  Serial.print("Gateway: ");Serial.println(gateway_address, HEX);
  Serial.print("nodeid: ");Serial.println(nodeid, HEX);
  Serial.print("networkid: ");Serial.println(networkid, HEX);

  Serial.print("vcc: ");Serial.print(read_vcc());Serial.println("mV");

  radio.initialize(FREQUENCY, nodeid, NETWORKID);

  radio.setHighPower(); //uncomment only for RFM69HW!

  radio.encrypt(ENCRYPTKEY);
  radio.setPowerLevel(2);
  radio.promiscuous(true);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);

  low_power_mode();
  normal_mode();

  // radio.setHighPower(false);
  // radio.sleep();
  // low_power_mode();
  // while(1){sleep_till_wdti();};
}

void loop() 
{
  //check for any received packets
  // if (radio.receiveDone())
  // {
  //   Serial.print("RX: ");packet_print(radio.DATA, radio.RSSI);
    
  //   cylon_packet_t * p = (cylon_packet_t *)radio.DATA;

  //   if (is_ack_requested(p))
  //   {
  //     send_ack(radio, self_address, p->from);
  //     Serial.print(" - ACK sent");
  //   }
  //   Blink(LED,5);
  //   Serial.println();
  // }

  radio.setHighPower(false);
  radio.sleep();
  low_power_mode();
  long_sleep(sleep_duration);
  normal_mode();

  Serial.print("setting radio.setHighPower");
  radio.setHighPower(true);
  Serial.println('.');
  transmit_status();
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}