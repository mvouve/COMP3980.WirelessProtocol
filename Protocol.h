/* Protocol defines */
#define MAXMISS 3	// Number of retry attempts on read thread.
#define MAXSENT 5   // Number of packets successfully sent

/* define protocol control chars */
#define SYN1 0x18
#define SYN2 0x19
#define RVI  0x17
#define ETB  0x23
#define EOT  0x04
#define ETB  0x23
#define ETX  0x03
#define ENQ  0x05
#define ACK  0x06
#define NAK  0x21