#ifndef network_h
#define network_h

#include <Arduino.h>
#include "datalink.h"

#define PACKET_CHECKSUM_BYTES 2
#define PACKET_MIN_BYTES sizeof(PacketHdr) + PACKET_CHECKSUM_BYTES

struct PacketHdr {
    uint16_t type;
};

PacketHdr* deserialize_to_packet();

#endif /* network_h */