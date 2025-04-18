#ifndef growbies_h
#define growbies_h

#include "HX711.h"
#include "command.h"
#include "constants.h"
#include "protocol/network.h"

class Growbies : protected HX711 {
    public:
        const int sensor_count;
        Growbies(): sensor_count(4){};
        void execute(PacketHdr* packet_hdr);
        void begin(byte channel = 0, byte gain = 128);

    private:
        byte channel = 0;

		// Reads data from the chip the requested number of times. The median is found and then all
		// samples that are within the medi  an +/- a 24 DAC threshold are averaged and returned.
		long read_median_filter_avg(byte times = 3, int threshold = 10000);

};

template <typename PacketType>
bool check_and_respond_to_deserialization_underflow(const PacketType& packet) {
    if (slip_buf->buf_len() >= sizeof(packet)) {
        return true;
    }
    else{
        RespError resp;
        resp.error = ERROR_CMD_DESERIALIZATION_BUFFER_UNDERFLOW;
        send_packet(resp);
        return false;
    }
};

template <typename PacketType>
bool validate_packet(const PacketType& packet) {
    bool result;
    result = check_and_respond_to_deserialization_underflow(packet);
    return result;
}

extern Growbies* growbies;


#endif /* growbies_h */