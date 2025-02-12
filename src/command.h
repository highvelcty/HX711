#ifndef command_h
#define command_h

#include <Arduino.h>
#include "HX711.h"
#include "protocol/network.h"
#include "protocol/transport.h"

#define WAIT_READY_RETRIES 5
#define WAIT_READY_RETRY_DELAY_MS 100

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

enum Cmd: uint16_t {
    CMD_LOOPBACK = 0,
    CMD_SAMPLE = 1
};

enum RespType: uint16_t {
    RESP_TYPE_VOID = 0,
    RESP_TYPE_BYTE = 1,
    RESP_TYPE_LONG = 2,
    RESP_TYPE_FLOAT = 3,
    RESP_TYPE_DOUBLE = 4
};

struct RespVoid {
    PacketHdr header;
    RespVoid() {
        this->header.type = RESP_TYPE_VOID;
    };
};

struct RespByte {
    PacketHdr header;
    uint8_t data;
    RespByte() {
        this->header.type = RESP_TYPE_BYTE;
    };
};

struct RespLong {
    PacketHdr header;
    int32_t data;
    RespLong() {
        this->header.type = RESP_TYPE_LONG;
    };
};

struct RespFloat {
    PacketHdr header;
    float data;
    RespFloat() {
        this->header.type = RESP_TYPE_FLOAT;
    };
};

struct RespDouble {
    PacketHdr header;
    double data;
    RespDouble() {
        this->header.type = RESP_TYPE_DOUBLE;
    };
};

extern HX711* scale;

void execute(PacketHdr* packet_hdr);
bool sample(long& sample);

#endif /* command_h */