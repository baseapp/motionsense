#define REQUEST_ACK 0x80
#define IS_ACK      0x01

// on payload
typedef struct __attribute__((packed))
{
  uint32_t from;		// from address
  uint32_t to;			// to address
  uint32_t message_seq; // sequence number to prevent repeats and such
  uint8_t ack_byte;		// REQUEST_ACK or IS_ACK
  uint8_t length;		// LENGTH of message after this
}cylon_packet_t;

bool send_data(RFM69 & radio, uint32_t from, uint32_t to, uint8_t ack_byte, uint8_t length, char* message, uint8_t retries=0, uint8_t retryWaitTime=40);
bool send_ack(RFM69 & radio, uint32_t from, uint32_t to);
bool is_ack_requested(volatile void * packet);
bool is_ack(volatile void * packet);
void packet_print(volatile void * packet, int RSSI=-1000);