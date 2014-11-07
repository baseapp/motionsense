// Sample RFM69 sender/node sketch, with ACK and optional encryption
// Sends periodic messages of increasing length to gateway (id=1)
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
#include <RFM69.h>
#include <SPI.h>
#include "cylon_packet.h"
#include "config.h"

int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
char payload[] = "123ABC";
char buff[20];
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;

void setup() 
{
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(true);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
}

long lastPeriod = -1;
void loop() 
{
  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print("RX: ");packet_print(radio.DATA, radio.RSSI);
    
    cylon_packet_t * p = (cylon_packet_t *)radio.DATA;

    if (is_ack_requested(p))
    {
      send_ack(radio, 0xdeadbeef, p->from);
      Serial.print(" - ACK sent");
    }
    Blink(LED,5);
    Serial.println();
  }

  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    lastPeriod=currPeriod;
    Blink(LED,3);

    char msg[10]={0};static int i;
    sprintf(msg, "%d", i++);
    send_data(radio, 0xdeadbeef, 0xabad1dea, REQUEST_ACK, strlen(msg), msg, 2);
  }

}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}