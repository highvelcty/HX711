#include "HX711.h"

#define MAIN_POLLING_LOOP_INTERVAL_MS 1
#define SERIAL_RECV_BYTES 64

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

enum Slip: byte {
    SLIP_END = 0xC0,
    SLIP_ESC = 0xDB,
    SLIP_ESC_END = 0xDC,
    SLIP_ESC_ESC = 0xDD
};

enum Cmd: unsigned int {
    CMD_LOOPBACK = 0
};

struct CmdHdr{
    Cmd cmd;
};

struct CmdRtn{
    CmdHdr hdr;
    union {
        double double_value;
        long long_value;
        float float_value;
    };
};

HX711 scale;
char serial_recv_buf[SERIAL_RECV_BYTES];
char* serial_recv_ptr = serial_recv_buf;
bool within_escape = false;

void setup() {
  memset(serial_recv_buf, 0, sizeof(serial_recv_buf));
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void slip_send(byte* buf, unsigned int buf_len){
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

unsigned int slip_recv(byte a_byte){
    // Returns the number of bytes of a received packet. The return length will be zero for
    // incomplete packets.

    char outbuf[16];
    itoa(a_byte, &outbuf[0], 10);
    Serial.write("emey recv: ");
    // Serial.write(outbuf);
    Serial.print(a_byte);
    Serial.write("\n");

    unsigned int len = 0;
    if (a_byte == SLIP_END){        
        len = serial_recv_ptr - &serial_recv_buf[0];
        serial_recv_ptr = &serial_recv_buf[0];
    }
    else if (a_byte == SLIP_ESC){
        within_escape = true;
    }
    else{
        if (within_escape){
            within_escape = false;
            if (a_byte == SLIP_ESC_END){
                *serial_recv_ptr++ = SLIP_END;
            }
            else if (a_byte == SLIP_ESC_ESC){
                *serial_recv_ptr++ = SLIP_ESC;
            }
        }
        else{
            *serial_recv_ptr++ = a_byte;
        }
    }
    return len;
}



// SerialRecv* slip_decode(byte* buf){
//     unsigned int writing_idx = 0;
//     for(unsigned int reading_idx = 0; reading_idx < SERIAL_RECV_BYTES; ++reading_idx){
//         if (buf[reading_idx] == END){
//             break;
//         }
//         else if (buf[reading_idx] == ESC){
//             if (buf[reading_idx+1] == ESC_END){
//                 buf[writing_idx] = END;
//             }
//             else if (buf[reading_idx+1] == ESC_ESC){
//                 buf[writing_idx] = ESC;
//             }
//             // Increment past the escaping
//             ++reading_idx;
//         }
//         else {
//             buf[writing_idx] = buf[reading_idx];
//         }
//         ++writing_idx;
//     }
//     return (SerialRecv*)buf;
// }
//
//
// void loop() {
//     while (!Serial.available()){
//         delay(MAIN_POLLING_LOOP_INTERVAL_MS);
//     }
//     *serial_recv_ptr = Serial.read();
//     if (*serial_recv_ptr == SLIP_END){
//         *serial_recv_ptr = '\0';
//         if (!strcmp("sample", serial_recv)){
//             sample();
//         }
//         else if (!strcmp("loopback", serial_recv)) {
//             Serial.print(serial_recv);
//             Serial.write(SLIP_END);
//         }
//         else{
//             Serial.print("Unrecognized cmd: ");
//             Serial.print(serial_recv);
//             Serial.write(SLIP_END);
//         }
//         serial_recv_ptr = &serial_recv[0];
//     }
//     else{
//         ++serial_recv_ptr;
//     }
// }
//
// void sample(){
//     if (scale.wait_ready_retry(10, 100)){
//         long reading = scale.read();
//     }
// }
//
//
// void loop() {
//
//   if (scale.is_ready()) {
//     long reading = scale.read();
//     Serial.print("HX711 reading: ");
//     Serial.println(reading);
//   } else {
//     Serial.println("HX711 not found.");
//   }
//
//   delay(1000);
//
// }

void loop() {
    unsigned int packet_len = 0;
    CmdHdr* hdr;

    while (!Serial.available()){
        delay(MAIN_POLLING_LOOP_INTERVAL_MS);
    }
    packet_len = slip_recv(Serial.read());
    Serial.write("emey packet_len: ");
    Serial.print(packet_len);
    Serial.write("\n");
    if (packet_len >= sizeof(CmdHdr)){
        hdr = (CmdHdr*)serial_recv_buf;
        if (hdr->cmd == CMD_LOOPBACK){
            slip_send((byte*)hdr, sizeof(CmdHdr));
        }
    }
}
