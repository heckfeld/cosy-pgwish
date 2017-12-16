#ifndef MIN
#define         MIN(a,b)        ((a)<(b)? (a): (b))
#endif MIN
#ifndef MAX
#define         MAX(a,b)        ((a)>(b)? (a): (b))
#endif

struct  fis {
    int reqtype,
        vtype;
    union   {   float f;
                int   i;
                unsigned char s[100];
            } value;
    };

/*
 *      Definition of value in struct fis
 */
 
#define FLOATTYPE   -1
#define INTEGERTYPE -2


/*
 *      Definition of Network-Request
 */

#define NETCTRL     1
#define NETSTAT     2

/*
 *      Definition of Request-Type NETCTRL
 */
 
#define LOG_SSAP            56
#define LOG_DSAP            55
#define LOG_CONTROL         57
#define LOG_DEST_ADDR       52
#define LOG_READ_TIMEOUT    53
#define LOG_READ_CACHE      54
#define RESET_STATISTICS   100

/*
 *      Definitonen fuer Request-Type NETSTAT
 */
 
#define FRAME_HEADER    50
#define DEVICE_STATUS   5
#define LOCAL_ADDRESS	1
#define READ_STATISTICS	200

/*
 *      Werte in arg.value.i beim Request DEVICE_STATUS
 */

#define FAILED          0
#define INACTIVE        1
#define INITIALIZING    2
#define ACTIVE          3

#define ETHERNET_ADDRESS_LENGTH		6

/*
 *      Werte fuer CONTROL einer TYPE 1 LLC
 */

#define UI_CONTROL      0x03
#define TEST_CONTROL    0xF3
#define XID_CONTROL     0xBF
#define POLL_FINAL      0x10
