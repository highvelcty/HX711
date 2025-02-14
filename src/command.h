#ifndef command_h
#define command_h

#include <Arduino.h>
#include "HX711.h"
#include "protocol/network.h"

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
    RESP_TYPE_DOUBLE = 4,
    RESP_TYPE_ERROR = 0xFFFF,
};

enum Error: uint32_t {
    ERROR_NONE = 0,
    ERROR_CMD_DESERIALIZATION_BUFFER_UNDERFLOW = 1,
    ERROR_UNRECOGNIZED_COMMAND = 2,
};

struct BaseCmd : PacketHdr {};
struct BaseResp : PacketHdr {
    BaseResp(RespType resp_type) : PacketHdr(resp_type) {};
};
struct BaseCmdWithTimesParam : BaseCmd {
    uint8_t times;
};

struct CmdSample : BaseCmdWithTimesParam {};

struct RespVoid : BaseResp {
    RespVoid() : BaseResp(RESP_TYPE_VOID) {};
};

struct RespByte : BaseResp {
    uint8_t data;
    RespByte() : BaseResp(RESP_TYPE_BYTE) {};
};

struct RespLong : BaseResp {
    int32_t data;
    RespLong() : BaseResp(RESP_TYPE_LONG) {};
};

struct RespFloat : BaseResp {
    float data;
    RespFloat() : BaseResp(RESP_TYPE_FLOAT) {};
};

struct RespDouble : BaseResp {
    double data;
    RespDouble() : BaseResp(RESP_TYPE_DOUBLE) {};
};

struct RespError : BaseResp {
    Error error;
    RespError() : BaseResp(RESP_TYPE_ERROR) {};
};

extern HX711* scale;

void execute(PacketHdr* packet_hdr);
bool sample(uint8_t times, long& sample);

#endif /* command_h */