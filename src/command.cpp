#include "command.h"

HX711* scale = new HX711;


void execute(PacketHdr* packet_hdr) {
    if (packet_hdr->type == CMD_LOOPBACK) {
        slip_send(serial_recv->buf, serial_recv->buf_len());
        slip_send_end();
    }
    else if (packet_hdr->type == CMD_SAMPLE) {
        RespLong resp;

        if (sample(resp.data)){
            send_response((byte*)&resp, sizeof(resp));
        }
    }
    return NULL;
}

 bool sample(long& sample) {
    if (scale->wait_ready_retry(WAIT_READY_RETRIES, WAIT_READY_RETRY_DELAY_MS)){
        sample = scale->read_average(7);
        return true;
    }
    return false;
}
