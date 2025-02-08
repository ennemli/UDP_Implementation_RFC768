#ifndef UDP_H
#define UDP_H

#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

struct UDPPacket {
  struct PseudoHeader {
    uint32_t src_ip;    // Source IP address
    uint32_t dest_ip;   // Destination IP address
    uint8_t zeros;      // Reserved field, must be zero
    uint8_t protocol;   // Protocol number (17 for UDP)
    uint16_t udpLength; // Length of UDP header + data
  } pseudo_header;

  struct UDPHeader {
    uint16_t sourcePort; // Source port number
    uint16_t destPort;   // Destination port number
    uint16_t length;     // Length of UDP header + data
    uint16_t checksum;   // Checksum of header and data
  } udp_header;

  std::unique_ptr<std::vector<char>> payload;
};

class UDP {
private:
  int sockfd;
  struct sockaddr_in addr;

  // Calculate UDP checksum including pseudo-header
  uint16_t calculateChecksum(const UDPPacket &udpPacket);

public:
  UDP();

  // Bind socket to specific IP and port
  void bind(const char *ip, uint16_t port);

  // Send data to specified destination
  void sendTo(const std::unique_ptr<std::vector<char>> data, const char *destIP,
              uint16_t destPort);

  // Receive data and get source information
  size_t receiveFrom(char *buffer, size_t maxLength, char *sourceIP,
                     uint16_t *sourcePort);

  // Cleanup socket on destruction
  ~UDP() { close(sockfd); }
};

#endif // UDP_H
