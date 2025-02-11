#ifndef slip_h
#define slip_h

#define SERIAL_RECV_ALLOC_BYTES 64

typedef enum {
    SLIP_END = 0xC0,
    SLIP_ESC = 0xDB,
    SLIP_ESC_END = 0xDC,
    SLIP_ESC_ESC = 0xDD
}Slip;

//class SerialRecv {
//    public:
//        uint8_t buf[SERIAL_RECV_ALLOC_BYTES];
//        uint8_t* buf_ptr = buf;
//        bool within_escape = false;
//
//        uint16_t buf_len();
//        void reset();
//};
//SerialRecv serial_recv;

//
//uint8_t serial_send[SERIAL_SEND_ALLOC_BYTES];

#endif /* slip_h */