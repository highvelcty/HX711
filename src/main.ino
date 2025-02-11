#include "HX711.h"
#include "slip.h"

#define PACKET_CHECKSUM_BYTES 2
#define PACKET_MIN_BYTES sizeof(PacketHdr) + PACKET_CHECKSUM_BYTES
#define MAIN_POLLING_LOOP_INTERVAL_MS 1
#define SAMPLE_RETRIES 5
#define SAMPLE_RETRY_DELAY_MS 100
// #define SERIAL_RECV_ALLOC_BYTES 64
#define SERIAL_SEND_ALLOC_BYTES 64

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

enum Cmd: uint16_t {
    CMD_LOOPBACK = 0,
    CMD_SAMPLE = 1
};

struct PacketHdr {
    uint16_t command;
};

struct BaseCommand { };

struct BaseResponse { };

struct RespSample {
    long sample;
};

class SerialRecv {
    public:
        uint8_t buf[SERIAL_RECV_ALLOC_BYTES];
        uint8_t* buf_ptr = buf;
        bool within_escape = false;

        uint16_t buf_len() {
            return this->buf_ptr - this->buf;
        }

        void reset() {
             this->buf_ptr = this->buf;
             this->within_escape = false;
        }
};
SerialRecv serial_recv;

uint8_t serial_send[SERIAL_SEND_ALLOC_BYTES];

void setup() {
    serial_recv.reset();
    Serial.begin(115200);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}


PacketHdr* deserialize_to_packet() {
    PacketHdr* packet_hdr = NULL;
    uint16_t buf_len = serial_recv.buf_len();
    uint16_t calc_checksum = 0;
    uint16_t checksum = 0;

    if (buf_len >= PACKET_MIN_BYTES) {
        packet_hdr = (PacketHdr*)&serial_recv.buf[0];
        checksum = *(uint16_t*)&serial_recv.buf[buf_len - PACKET_CHECKSUM_BYTES];
        for (uint16_t byte_idx = 0; byte_idx < buf_len - PACKET_CHECKSUM_BYTES; ++byte_idx){
            calc_checksum += serial_recv.buf[byte_idx];
        }

        if (calc_checksum == checksum) {
            return packet_hdr;
        }
        else {
            return NULL;
        }
    }
    else {
        return NULL;
    }
}


void execute(PacketHdr* packet_hdr) {
    if (packet_hdr->command == CMD_LOOPBACK) {
        slip_send(serial_recv.buf, serial_recv.buf_len());
        slip_send_end();
    }
    else if (packet_hdr->command == CMD_SAMPLE) {
        RespSample* resp = (RespSample*)&serial_send[0];

        if (sample(&resp->sample)){
            slip_send_resp((Cmd)packet_hdr->command, (byte*)resp, sizeof(RespSample));
        }
    }
    return NULL;
}


void loop() {
    PacketHdr* packet_hdr;

    while (!Serial.available()){
        delay(MAIN_POLLING_LOOP_INTERVAL_MS);
    }

    if (slip_recv(Serial.read())){
        packet_hdr = deserialize_to_packet();
        if (packet_hdr != NULL){
            execute(packet_hdr);
        }
        serial_recv.reset();
    }
}

bool sample(long* sample){
    for (int retry = 0; retry < SAMPLE_RETRIES; ++retry){
        if(retry > 0){
            delay(SAMPLE_RETRY_DELAY_MS);
        }
        if (scale.is_ready()) {
            *sample = scale.read();
            return true;
        }
    }
    return false;
}

bool slip_recv(byte a_byte){
    if (a_byte == SLIP_END){
        return true;
    }
    else if (a_byte == SLIP_ESC){
        serial_recv.within_escape = true;
    }
    else{
        if (serial_recv.within_escape){
            serial_recv.within_escape = false;
            if (a_byte == SLIP_ESC_END){
                *serial_recv.buf_ptr++ = SLIP_END;
            }
            else if (a_byte == SLIP_ESC_ESC){
                *serial_recv.buf_ptr++ = SLIP_ESC;
            }
        }
        else{
            *serial_recv.buf_ptr++ = a_byte;
        }
    }
    return false;
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

void slip_send_end(){
    Serial.write(SLIP_END);
}

void slip_send_resp(Cmd cmd, byte* resp, uint16_t resp_len) {
    PacketHdr packet_hdr;
    uint16_t checksum = cmd;

    packet_hdr.command = cmd;

    for (uint16_t byte_idx = 0; byte_idx < resp_len; ++byte_idx){
        checksum += resp[byte_idx];
    }

    slip_send((byte*)&packet_hdr, sizeof(packet_hdr));
    slip_send(resp, resp_len);
    slip_send((byte*)&checksum, sizeof(checksum));
    slip_send_end();
}