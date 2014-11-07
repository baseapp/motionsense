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
#ifdef IS_RFM69HW
  radio.setHighPower(); //only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  
    Serial.println("Starting");
}

byte ackCount=0;
void loop() {
  
  if (radio.receiveDone())
  {
    // Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    // if (promiscuousMode)
    // {
    //   Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    // }
    // for (byte i = 0; i < radio.DATALEN; i++)
    //   Serial.print((char)radio.DATA[i]);
    // Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    packet_print(radio.DATA, radio.RSSI);
    cylon_packet_t * p = (cylon_packet_t *)radio.DATA;
    if (is_ack_requested(radio.DATA))
    {
      // swap to<->from
      send_ack(radio, p->to, p->from);
      Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        Serial.print(" Pinging node ");
        Serial.print(p->from, HEX);
        Serial.print(" - ACK...");
        delay(6); //need this when sending right after reception .. ?
        // if(
        send_data(radio, p->to, p->from, REQUEST_ACK, 8, "ACK TEST");  // 0 = only 1 attempt, no retries
        // )
        //   Serial.print("ok!");
        // else Serial.print("nothing");
      }

    }
    Serial.println();
    Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}