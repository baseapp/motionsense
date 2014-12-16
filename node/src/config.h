#ifndef CONFIG_H_
#define CONFIG_H_

// NODE config

#define NODEID        2    //unique for each node on same network, 2 is node, 1 is gateway
#define NETWORKID     100  //the same on all nodes that talk to each other

// nodes send to gateway
#define SEND_DATA_TO     1

//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ

#warning "TODO enc keys"
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME      30 // max # of ms to wait for an ack

#define LED           6 // Moteinos have LEDs on D9

#define SERIAL_BAUD   115200

#define BOOST_REG_PIN	7
#define PIR_POWER_PIN	8

// conected to INT1 pin
#define PIR_IRQ_NUM		1
// PD3
#define PIR_INT_PIN		3

// button PC3 PCINT11, pin17 A3
#define BUTTON_INT_PIN	17
#define BUTTON_INT_PIN_AN	A3


/* EEPROM offsets */
//4 bytes
#define _SELF_ADDRESS_OFF		(0)
#define _GATEWAY_ADDRESS_OFF	(_SELF_ADDRESS_OFF+4)
//1 byte
#define _FIXED_NODEID_OFF		(_GATEWAY_ADDRESS_OFF+4)
#define _FIXED_NETWORKID_OFF	(_FIXED_NODEID_OFF+1)
#define _SLEEP_DURATION_OFF		(_FIXED_NETWORKID_OFF+1)

#endif