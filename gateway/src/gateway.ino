// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

#include <RFM69.h>
#include <SPI.h>
#include "cylon_packet.h"
#include "config.h"

RFM69 radio;
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);

  radio.setHighPower(); //only for RFM69HW!

  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  // wait for router to boot, do not send stray commands for 20s
  // for(int i=0;i<20;i++)
  //   delay(1000);
}

byte ackCount=0;
void loop() {
  
  if (radio.receiveDone())
  {
    packet_print(radio.DATA, radio.RSSI);
    Serial.println("got packet");
    cylon_packet_t * p = (cylon_packet_t *)radio.DATA;
    if (is_ack_requested(radio.DATA))
    {
      // swap to<->from
      send_ack(radio, p->to, p->from);
    }

    // Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}