#ifndef UDP_H
#define UDP_H
#include "sys/socket.h"
#include "unistd.h"
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

struct UDPPacket {
  struct PseudoHeader {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint8_t zeros;
    uint8_t protocol;
    uint8_t udpLength;
  } pseudo_header;
  struct UDPHeader {
    uint16_t sourcePort;
    uint16_t destPort;
    uint16_t lenght;
    uint16_t checksum;
  } udp_header;
  std::unique_ptr<std::vector<char>> payload;
};

class UDP {
private:
  int sockfd;
  struct sockaddr_in addr;
  uint16_t calculate_checksum(const UDPPacket &udpPacket);

public:
  UDP();
  void bind(const char *ip, uint16_t port);
  void sendTo(const std::unique_ptr<std::vector<char>>, const char *destIP,
              uint16_t destPort);
  size_t receiveFrom(char *buffer, size_t maxLength, char *sourceIP,
                     uint16_t *sourcePort);
};
#endif // !UDP_H
