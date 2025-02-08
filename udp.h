#ifndef UDP_H
#define UDP_H
#include "sys/socket.h"
#include "unistd.h"
#include <cstdint>
#include <sys/types.h>

struct UDP_HEADER {
  uint16_t sourcePort;
  uint16_t destPort;
  uint16_t lenght;
  uint16_t checksum;
};

uint16_t calculate_checksum(const UDP_HEADER *udpHeader, const char *data,
                            size_t dataLen);

#endif // !UDP_H
