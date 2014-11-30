/* Protocol defines */
#define MAXMISS 3	// Number of retry attempts on read thread.
#define MAXSENT 5   // Number of packets successfully sent

/* define protocol control chars */
#define SYN1 18
#define SYN2 19
#define RVI  17
#define ETB  23
#define EOT  04
#define ETB  23
#define ETX  03
#define ENQ  05
#define ACK  06
#define NAK  21