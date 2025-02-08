#ifndef UDP_H
#define UDP_H
#include "sys/socket.h"
#include "unistd.h"
#include <cstdint>
#include <sys/types.h>
#include <vector>

struct UDPPacket {
  struct PseudoHeader {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint8_t zero;
    uint8_t protocol;
    uint8_t udpLength;
  } pseudo_header;
  struct UDPHeader {
    uint16_t sourcePort;
    uint16_t destPort;
    uint16_t lenght;
    uint16_t checksum;
  } udp_header;
  std::vector<uint8_t> payload;
};

uint16_t calculate_checksum(const UDPPacket &udpPacket);

#endif // !UDP_H
