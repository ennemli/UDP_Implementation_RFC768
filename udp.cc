#include "udp.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
uint16_t UDP::calculate_checksum(const UDPPacket &udpPacket) {
  size_t total_size = sizeof(UDPPacket::PseudoHeader) +
                      sizeof(UDPPacket::UDPHeader) + udpPacket.payload.size();
  if (total_size % 2)
    total_size++;

  std::vector<uint8_t> buffer(total_size, 0);
  size_t offset = 0;

  // Copy Pseudo Header
  memcpy(buffer.data(), &udpPacket.pseudo_header,
         sizeof(UDPPacket::PseudoHeader));
  offset += sizeof(UDPPacket::PseudoHeader);

  // Copy UDP Header
  // and set checksum set to 0
  UDPPacket::UDPHeader udpHeader = udpPacket.udp_header;
  udpHeader.checksum = 0;
  memcpy(buffer.data() + offset, &udpHeader, sizeof(UDPPacket::UDPHeader));
  offset += sizeof(UDPPacket::UDPHeader);

  // Copy Data
  memcpy(buffer.data() + offset, udpPacket.payload.data(),
         udpPacket.payload.size());

  // calculate ChceckSum
  uint32_t sum = 0;
  uint16_t *ptr = reinterpret_cast<uint16_t *>(buffer.data());

  for (size_t i = 0; i < total_size / 2; ++i) {
    sum += ntohs(ptr[i]);
    // Handle carry
    if (sum >> 16) {
      sum += (sum & 0xFFFF) + (sum >> 16);
    }
  }
  return ~sum & 0xFFFF;
}

UDP::UDP() {
  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (sockfd) {
    throw std::runtime_error("Failed to created soocket");
  }
}
void UDP::bind(const char *ip, uint16_t port) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    throw std::runtime_error("Failed to bind socket");
  }
}
