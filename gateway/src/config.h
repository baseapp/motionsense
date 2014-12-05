#ifndef CONFIG_H_
#define CONFIG_H_

// GATEWAY config

#define NODEID        1    //unique for each node on same network, 2 is node, 1 is gateway
#define NETWORKID     100  //the same on all nodes that talk to each other


#define SEND_DATA_TO     2

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack

#define LED           9 // Moteinos have LEDs on D9
#define FLASH_SS      8 // and FLASH SS on D8

#define SERIAL_BAUD   115200

#endif