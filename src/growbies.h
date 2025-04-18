#ifndef growbies_h
#define growbies_h

#include "HX711.h"
#include "command.h"
#include "constants.h"
#include "protocol/network.h"

class Growbies : protected HX711 {
    public:
        const int sensor_count;


        Growbies(int sensor_count = 4);
        ~Growbies();

        void execute(PacketHdr* packet_hdr);
        void begin(byte channel = 0, byte gain = 128);

    private:
        MassDataPoint* mass_data_points;
        byte channel = 0;
		MassDataPoint* read_all();
		// Reads data from the chip the requested number of times. The median is found and then all
		// samples that are within the medi  an +/- a 24 DAC threshold are averaged and returned.
		MassDataPoint* read_median_filter_avg(const byte times = 3, const int threshold = 10000);
		void shiftAllIn();
		bool wait_all_ready_retry(const int retries, const unsigned long delay_ms);
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