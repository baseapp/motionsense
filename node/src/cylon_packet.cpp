#include <Arduino.h>
#include <RFM69.h>
#include "cylon_packet.h"
#include "config.h"

// internal functions
bool send_with_retry(RFM69 & radio, const void* buffer, byte bufferSize, byte retries, byte retryWaitTime);

bool send_data(RFM69 & radio, uint32_t from, uint32_t to, uint8_t ack_byte, uint8_t length, char* message, uint8_t retries, uint8_t retryWaitTime)
{
  char buff[MAX_PACKET_SIZE];
  cylon_packet_t * packet = (cylon_packet_t *)buff;
  packet->from = from;
  packet->to = to;
  packet->ack_byte = ack_byte;
  packet->length = length;

  for(int i = sizeof(cylon_packet_t), j=0; (i<MAX_PACKET_SIZE) && (j<length); i++, j++)
  {
    buff[i] = message[j];
  }

  // Serial.print("TX: ");packet_print(packet);

  if(retries == 0)
  {
    radio.send(SEND_DATA_TO, buff, length+sizeof(cylon_packet_t));
    Serial.println();
    return true;
  }
  else
  {
    return send_with_retry(radio, buff, length+sizeof(cylon_packet_t), retries, retryWaitTime);
  }
}

bool send_ack(RFM69 & radio, uint32_t from, uint32_t to)
{
  char buff[sizeof(cylon_packet_t)];
  cylon_packet_t * packet = (cylon_packet_t *)buff;
  packet->from = from;
  packet->to = to;
  packet->ack_byte = IS_ACK;
  packet->length = 0;

  radio.send(SEND_DATA_TO, buff, sizeof(cylon_packet_t));
  return true;
}

bool is_ack_requested(volatile void * p)
{
  cylon_packet_t * packet = (cylon_packet_t *)p;
  return ((packet->ack_byte & REQUEST_ACK) == REQUEST_ACK);
}

bool is_ack(volatile void * p)
{
  cylon_packet_t * packet = (cylon_packet_t *)p;
  return ((packet->ack_byte & IS_ACK) == IS_ACK);
}

void packet_print(volatile void * p, int RSSI)
{
  cylon_packet_t * packet = (cylon_packet_t *)p;
  Serial.print('[');Serial.print(packet->from, HEX);Serial.print("] ");
  Serial.print("to [");Serial.print(packet->to, HEX);Serial.print("] ");
  if(is_ack(packet))
  {
    Serial.print("ACK ");
  }
  if(is_ack_requested(packet))
  {
    Serial.print("REQACK ");
  }
  if(RSSI != -1000)
  {
    Serial.print("   [RX_RSSI:");Serial.print(RSSI);Serial.print("]");
  }

  char *buf=(char*)packet+sizeof(cylon_packet_t);
  for (byte i = 0; i < (packet->length); i++)
    Serial.print((char)buf[i]);
  Serial.println("");
}

// send pre filled structured data. can be used for retry ack or retry data
// TODO use message_seq everywhere
bool send_with_retry(RFM69 & radio, const void* buffer, byte bufferSize, byte retries, byte retryWaitTime)
{
  long sentTime;
  uint32_t dest = ((cylon_packet_t *)buffer)->to; // save because it will be overwritten

  for (byte i=0; i<=retries; i++)
  {
    radio.send(SEND_DATA_TO, buffer, bufferSize, true);
    sentTime = millis();
    while( (millis()-sentTime) < retryWaitTime)
    {
      if (radio.receiveDone())
      {
        cylon_packet_t * p = (cylon_packet_t *)radio.DATA;
        if( (p->from == dest) && is_ack(p) )
        {
          return true;
        }
      }
    }
    // Serial.print(" RETRY#");Serial.println(i+1);
  }
  return false;
}