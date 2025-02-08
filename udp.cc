#include "udp.h"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <ios>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <vector>
uint16_t UDP::calculateChecksum(const UDPPacket &udpPacket) {
  size_t total_size = sizeof(UDPPacket::PseudoHeader) +
                      sizeof(UDPPacket::UDPHeader) + udpPacket.payload->size();
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
  memcpy(buffer.data() + offset, udpPacket.payload->data(),
         udpPacket.payload->size());

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
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Failed to create socket");
  }
}
void UDP::bind(const char *ip, uint16_t port) {
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);
  if (::bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    throw std::runtime_error("Failed to bind socket");
  }
}

void UDP::sendTo(const std::unique_ptr<std::vector<char>> data,
                 const char *destIP, uint16_t destPort) {
  struct sockaddr_in destAddr;
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(destPort);
  destAddr.sin_addr.s_addr = inet_addr(destIP);

  size_t packetSize = sizeof(UDPPacket::UDPHeader) + data->size();
  UDPPacket udpPacket;
  udpPacket.udp_header.sourcePort = addr.sin_port;
  udpPacket.udp_header.destPort = destAddr.sin_port;
  udpPacket.udp_header.length = htons(packetSize);
  udpPacket.udp_header.checksum = 0;

  udpPacket.pseudo_header.src_ip = addr.sin_addr.s_addr;
  udpPacket.pseudo_header.dest_ip = destAddr.sin_addr.s_addr;
  udpPacket.pseudo_header.zeros = 0;
  udpPacket.pseudo_header.protocol = 17;
  udpPacket.pseudo_header.udpLength = udpPacket.udp_header.length;
  udpPacket.payload = std::make_unique<std::vector<char>>(*data);
  // calculate CheckSum
  udpPacket.udp_header.checksum = calculateChecksum(udpPacket);

  // Combine header and data in one packet
  char packet[65535];
  memcpy(packet, &udpPacket.udp_header, sizeof(UDPPacket::UDPHeader));
  memcpy(packet + sizeof(UDPPacket::UDPHeader), data->data(), data->size());

  if (sendto(sockfd, packet, packetSize, 0, (struct sockaddr *)&destAddr,
             sizeof(destAddr)) < 0) {
    throw std::runtime_error("Failed to send data");
  }
}

size_t UDP::receiveFrom(char *buffer, size_t maxLength, char *sourceIP,
                        uint16_t *sourcePort) {

  struct sockaddr_in sourceAddr;
  socklen_t addrLen = sizeof(sourceAddr);

  // Receive packet
  char packet[65535];
  ssize_t received = recvfrom(sockfd, packet, sizeof(packet), 0,

                              (struct sockaddr *)&sourceAddr, &addrLen);

  if (received < 0) {
    throw std::runtime_error("Failed to receive data");
  }
  UDPPacket::UDPHeader header;
  memcpy(&header, packet, sizeof(UDPPacket::UDPHeader));

  // Create UDP packet for checksum verification
  UDPPacket udpPacket;
  // Extract UDP Packet

  udpPacket.udp_header = header;
  udpPacket.pseudo_header.src_ip = sourceAddr.sin_addr.s_addr;
  udpPacket.pseudo_header.dest_ip = addr.sin_addr.s_addr;
  udpPacket.pseudo_header.zeros = 0;
  udpPacket.pseudo_header.protocol = 17;
  udpPacket.pseudo_header.udpLength = header.length;
  size_t dataLength = received - sizeof(UDPPacket::UDPHeader);
  udpPacket.payload = std::make_unique<std::vector<char>>(
      packet + sizeof(UDPPacket::UDPHeader),
      packet + sizeof(UDPPacket::UDPHeader) + dataLength);

  // Verify checksum
  uint16_t receivedChecksum = udpPacket.udp_header.checksum;
  udpPacket.udp_header.checksum = 0;
  uint16_t calculatedChecksum = calculateChecksum(udpPacket);
  if (receivedChecksum != calculatedChecksum) {
    throw std::runtime_error("Checksum verification failed");
  }

  // Copy data to buffer
  if (dataLength > maxLength) {
    throw std::runtime_error("Buffer too small");
  }

  memcpy(buffer, packet + sizeof(UDPPacket::UDPHeader), dataLength);

  // Set source information
  strcpy(sourceIP, inet_ntoa(sourceAddr.sin_addr));
  *sourcePort = ntohs(sourceAddr.sin_port);

  return dataLength;
}
