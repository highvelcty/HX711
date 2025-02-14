#include "command.h"

HX711* scale = new HX711;

template <typename T>

bool check_and_respond_to_deserialization_underflow(const T& structure) {
    if (slip_buf->buf_len() >= sizeof(structure)) {
        return false;
    }
    else{
        RespError resp;
        resp.error = ERROR_CMD_DESERIALIZATION_BUFFER_UNDERFLOW;
        send_packet(resp);
        return true;
    }

}

void execute(PacketHdr* packet_hdr) {
    if (packet_hdr->type == CMD_LOOPBACK) {
        send_slip(slip_buf->buf, slip_buf->buf_len());
        send_slip_end();
    }
    else if (packet_hdr->type == CMD_SAMPLE) {
        CmdSample* cmd = (CmdSample*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespLong resp;

            if (sample(cmd->times, resp.data)){
                send_packet(resp);
            }
         }
    }
    else{
        RespError resp;
        resp.error = ERROR_UNRECOGNIZED_COMMAND;
        send_packet(resp);
    }
}

 bool sample(uint8_t times, long& sample) {
    if (scale->wait_ready_retry(WAIT_READY_RETRIES, WAIT_READY_RETRY_DELAY_MS)){
        sample = scale->read_average(times);
        return true;
    }
    return false;
}
