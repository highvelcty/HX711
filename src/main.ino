#include "command.h"
#include "protocol/datalink.h"
#include "protocol/network.h"
#include "protocol/transport.h"


#define MAIN_POLLING_LOOP_INTERVAL_MS 1


void setup() {
    serial_recv->reset();
    Serial.begin(115200);
    scale->begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
    PacketHdr* packet_hdr;

    while (!Serial.available()){
        delay(MAIN_POLLING_LOOP_INTERVAL_MS);
    }

    if (slip_recv(Serial.read())){
        packet_hdr = deserialize_to_packet();
        if (packet_hdr != NULL){
            execute(packet_hdr);
        }
        serial_recv->reset();
    }
}


