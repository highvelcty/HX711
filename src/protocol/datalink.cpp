#include "datalink.h"

uint16_t SerialRecv::buf_len() {
    return this->buf_ptr - this->buf;
}

void SerialRecv::reset() {
    this->buf_ptr = this->buf;
    this->within_escape = false;
}


bool slip_recv(byte a_byte) {
    if (a_byte == SLIP_END){
        return true;
    }
    else if (a_byte == SLIP_ESC){
        serial_recv->within_escape = true;
    }
    else{
        if (serial_recv->within_escape){
            serial_recv->within_escape = false;
            if (a_byte == SLIP_ESC_END){
                *serial_recv->buf_ptr++ = SLIP_END;
            }
            else if (a_byte == SLIP_ESC_ESC){
                *serial_recv->buf_ptr++ = SLIP_ESC;
            }
        }
        else{
            *serial_recv->buf_ptr++ = a_byte;
        }
    }
    return false;
}


void slip_send(byte* buf, unsigned int buf_len) {
    for (unsigned int idx = 0; idx < buf_len; ++idx){
        if (buf[idx] == SLIP_END){
            Serial.write(SLIP_ESC);
            Serial.write(SLIP_ESC_END);
        }
        else if (buf[idx] == SLIP_ESC){
            Serial.write(SLIP_ESC);
            Serial.write(SLIP_ESC_ESC);
        }
        else {
            Serial.write(buf[idx]);
        }
    }
}


void slip_send_end(){
    Serial.write(SLIP_END);
}

SerialRecv* serial_recv = new SerialRecv();
