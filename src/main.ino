#include "command.h"
#include "protocol/datalink.h"
#include "protocol/network.h"


#define MAIN_POLLING_LOOP_INTERVAL_MS 1


void setup() {
    slip_buf->reset();
    Serial.begin(115200);
    scale->begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
    PacketHdr* packet_hdr;

    while (!Serial.available()){
        delay(MAIN_POLLING_LOOP_INTERVAL_MS);
    }

    if (recv_slip(Serial.read())){
        packet_hdr = recv_packet();
        if (packet_hdr != NULL) {
            execute(packet_hdr);
        }
        slip_buf->reset();
    }
}


