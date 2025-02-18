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
    else if (packet_hdr->type == CMD_READ_AVERAGE) {
        CmdReadAverage* cmd = (CmdReadAverage*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespLong resp;

            if (wait_and_read_average(cmd->times, resp.data)){
                send_packet(resp);
            }
            else {
                RespError resp;
                resp.error = ERROR_HX711_NOT_READY;
                send_packet(resp);
            }
         }
    }
    else if (packet_hdr->type == CMD_SET_GAIN) {
        CmdSetGain* cmd = (CmdSetGain*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespVoid resp;
            scale->set_gain(cmd->gain);
            // The first read after setting the gain applies the gain. The value returned looks
            // off from experimentation and is discarded.
            scale->read();
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_GET_UNITS) {
        CmdGetUnits* cmd = (CmdGetUnits*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespLong resp;

            if (wait_and_get_units(cmd->times, resp.data)){
                send_packet(resp);
            }
            else{
                RespError resp;
                resp.error = ERROR_HX711_NOT_READY;
                send_packet(resp);
            }
        }
    }
    else if (packet_hdr->type == CMD_TARE) {
        CmdTare* cmd = (CmdTare*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespVoid resp;
            scale->tare(cmd->times);
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_SET_SCALE) {
        CmdSetScale* cmd = (CmdSetScale*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespVoid resp;
            scale->set_scale(cmd->scale);
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_GET_SCALE) {
        CmdGetScale* cmd = (CmdGetScale*)slip_buf->buf;
        if (!check_and_respond_to_deserialization_underflow(*cmd)){
            RespFloat resp;
            resp.data = scale->get_scale();
            send_packet(resp);
         }
    }
    else{
        RespError resp;
        resp.error = ERROR_UNRECOGNIZED_COMMAND;
        send_packet(resp);
    }
}

bool wait_and_get_units(uint8_t times, long& data) {
    if (scale->wait_ready_retry(WAIT_READY_RETRIES, WAIT_READY_RETRY_DELAY_MS)){
        data = scale->get_units(times);
        return true;
    }
    return false;
}

bool wait_and_read_average(uint8_t times, long& data) {
    if (scale->wait_ready_retry(WAIT_READY_RETRIES, WAIT_READY_RETRY_DELAY_MS)){
        data = scale->read_average(times);
        return true;
    }
    return false;
}
