#ifndef datalink_h
#define datalink_h

#include <Arduino.h>

#define SERIAL_RECV_ALLOC_BYTES 64

enum Slip {
    SLIP_END = 0xC0,
    SLIP_ESC = 0xDB,
    SLIP_ESC_END = 0xDC,
    SLIP_ESC_ESC = 0xDD
};

class SerialRecv {
    public:
        uint8_t buf[SERIAL_RECV_ALLOC_BYTES];
        uint8_t* buf_ptr = buf;
        bool within_escape = false;

        uint16_t buf_len();
        void reset();
};

bool slip_recv(byte a_byte);
void slip_send(byte* buf, unsigned int buf_len);
void slip_send_end();

extern SerialRecv* serial_recv;

#endif /* datalink_h */