#include <util/atomic.h>

#include "growbies.h"
#include "utils/sort.h"


Growbies* growbies = new Growbies();

Growbies::Growbies(int sensor_count) : sensor_count(sensor_count) {
    this->mass_data_points = (MassDataPoint*)malloc(sizeof(MassDataPoint) * this->sensor_count);
}

Growbies::~Growbies() {
    free(this->mass_data_points);
}

void Growbies::begin(byte channel, byte gain){
    pinMode(ARDUINO_HX711_SCK, OUTPUT);
    for(int sensor = 0; sensor < this->sensor_count; ++sensor) {
        pinMode(get_HX711_dout_pin(sensor), INPUT_PULLUP);
    }
}

void Growbies::execute(PacketHdr* packet_hdr) {
    if (packet_hdr->type == CMD_LOOPBACK) {
        send_slip(slip_buf->buf, slip_buf->buf_len());
        send_slip_end();
    }
    else if (packet_hdr->type == CMD_READ_MEDIAN_FILTER_AVG) {
        CmdReadMedianFilterAvg* cmd = (CmdReadMedianFilterAvg*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            send_packet(*this->read_median_filter_avg(cmd->times),
                sizeof(MassDataPoint) * this->sensor_count);
         }
    }
    else if (packet_hdr->type == CMD_SET_GAIN) {
        CmdSetGain* cmd = (CmdSetGain*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->set_gain(cmd->gain);
            // The first read after setting the gain applies the gain. The value returned looks
            // off from experimentation and is discarded.
            this->read();
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_GET_UNITS) {
        CmdGetUnits* cmd = (CmdGetUnits*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespLong resp;
            resp.data = this->get_units(cmd->times);
            if (resp.data == ERROR_HX711_NOT_READY){
                RespError error_response;
                error_response.error = (Error)resp.data;
                send_packet(error_response);
            }
            else {
                send_packet(resp);
            }
        }
    }
    else if (packet_hdr->type == CMD_TARE) {
        CmdTare* cmd = (CmdTare*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->tare(cmd->times);
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_SET_SCALE) {
        CmdSetScale* cmd = (CmdSetScale*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->set_scale(cmd->scale);
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_GET_SCALE) {
        CmdGetScale* cmd = (CmdGetScale*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespFloat resp;
            resp.data = this->get_scale();
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_POWER_UP) {
        CmdPowerUp* cmd = (CmdPowerUp*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->power_up();
            send_packet(resp);
        }
    }
    else if (packet_hdr->type == CMD_POWER_DOWN) {
        CmdPowerDown* cmd = (CmdPowerDown*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->power_down();
            send_packet(resp);
        }
    }
    else if (packet_hdr->type == CMD_SET_CHANNEL) {
        CmdSetChannel* cmd = (CmdSetChannel*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespVoid resp;
            this->channel = cmd->channel;
            this->begin();
            send_packet(resp);
         }
    }
    else if (packet_hdr->type == CMD_GET_CHANNEL) {
        CmdGetChannel* cmd = (CmdGetChannel*)slip_buf->buf;
        if (validate_packet(*cmd)) {
            RespByte resp;
            resp.data = this->channel;
            send_packet(resp);
        }
    }
    else{
        RespError resp;
        resp.error = ERROR_UNRECOGNIZED_COMMAND;
        send_packet(resp);
    }
}

MassDataPoint* Growbies::read_all(){
    if (!wait_all_ready_retry(WAIT_READY_RETRIES, WAIT_READY_RETRY_DELAY_MS)){
        return this->mass_data_points;
    }

    this->read_all();
    return this->mass_data_points;
}

MassDataPoint* Growbies::read_median_filter_avg(const byte times, const int threshold) {
    // This method filters serial bit errors often caused by timing.
    long median;
    byte middle;
    long sample;
    int sensor;
    byte sensor_sample;

    bool ready = true;
    long sensor_samples[this->sensor_count][times] = {0};
    long sum = 0;
    int sum_count = 0;

	// Read samples
	for (sample = 0; sample < times; ++sample) {
        this->read_all();
        for (sensor_sample = 0; sensor_sample < this->sensor_count; ++sensor_sample){
            sensor_samples[sensor][sensor_sample] = this->mass_data_points[sensor_sample].data;
            ready &= this->mass_data_points[sensor].ready;
        }
	}
	if (!ready){
	    return this->mass_data_points;
	}

    for (sensor = 0; sensor < this->sensor_count; ++sensor){
        // Sort
        insertion_sort(sensor_samples[sensor], times);

        // Find median
        middle = times / 2;
        if (times % 2) {
            // Odd - simply take the middle number
            median = sensor_samples[sensor][middle];
        }
        else {
            // Even - average the middle two numbers
            median = (sensor_samples[sensor][middle - 1] + sensor_samples[sensor][middle]) / 2;
        }

        // Average and return samples that fall within a threshold
        for (sensor_sample = 0; sensor_sample < times; ++sensor_sample) {
            sample = sensor_samples[sensor][sensor_sample];
            if (abs(median - sample) <= threshold) {
                sum += sample;
                ++sum_count;
            }
            else {
                ++this->mass_data_points[sensor].error_count;
            }
        }
        this->mass_data_points[sensor].data = sum / sum_count;
    }

    return this->mass_data_points;
}

void Growbies::shiftAllIn() {
    int sensor;
    uint8_t data_in = 0;

    // Initialize output data
    for (sensor = 0; sensor < this->sensor_count; ++ sensor){
        this->mass_data_points[sensor].data = 0;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // For each bit, toggle the serial clock line to high, read all sensor data pins, toggle the
        // serial clock line to low.
        for(int ii = 0; ii < HX711_DAC_BITS; ++ii) {
            digitalWrite(ARDUINO_HX711_SCK, HIGH);
            delayMicroseconds(SCK_TOGGLE_DELAY_MICROSECONDS);
            // Read pins 8-13
            data_in = PINB;
            digitalWrite(ARDUINO_HX711_SCK, LOW);
            delayMicroseconds(SCK_TOGGLE_DELAY_MICROSECONDS);

            for (sensor = 0; sensor < this->sensor_count; ++sensor) {
                this->mass_data_points[sensor].data |= ((data_in & (1 << sensor)) << ii);
            }
        }

        // Set the channel and the gain factor for the next reading using the clock pin.
	    for (unsigned int i = 0; i < GAIN; i++) {
	    	digitalWrite(ARDUINO_HX711_SCK, HIGH);
	    	delayMicroseconds(SCK_TOGGLE_DELAY_MICROSECONDS);
	    	digitalWrite(ARDUINO_HX711_SCK, LOW);
	    	delayMicroseconds(SCK_TOGGLE_DELAY_MICROSECONDS);
	    }
    }

    // Pad with 1's to retain negativity when converting from a 24-bit sign int to a 32-bit
    // signed int
    for (int sensor = 0; sensor < this->sensor_count; ++sensor){
        if (this->mass_data_points[sensor].data & (HX711_DAC_BITS - 1)){
            this->mass_data_points[sensor].data |= ((long)0xFF << HX711_DAC_BITS);
        }
    }
}

bool Growbies::wait_all_ready_retry(const int retries, const unsigned long delay_ms)
{
	bool all_ready;
	int sensor;
	byte ready_pins = 0;
	int retry_count = 0;

	do {
        // Check for readiness from all sensors
        // Read pins 8-13
        ready_pins = PINB;
        all_ready = true;
	    for (sensor = 0; sensor < this->sensor_count; ++sensor) {
	        // The sensor is not ready when the data line is high.
	        this->mass_data_points[sensor].ready = (ready_pins & (1 << sensor));
	        all_ready &= this->mass_data_points[sensor].ready;
        }

        if (!all_ready){
            ++retry_count;
            delay(delay_ms);
        }

	} while ((retry_count <= retries) && (!all_ready));

	return all_ready;
}