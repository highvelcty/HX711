#include "transport.h"


void send_response(byte* resp, uint16_t resp_len) {
    uint16_t checksum = 0;

    for (uint16_t byte_idx = 0; byte_idx < resp_len; ++byte_idx){
        checksum += resp[byte_idx];
    }

    slip_send(resp, resp_len);
    slip_send((byte*)&checksum, sizeof(checksum));
    slip_send_end();
}
