#include "udp.h"
#include <cstring>
#include <iostream>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
  UDP client;

  std::string message = "Hello from UDP client!";
  auto payload =
      std::make_unique<std::vector<char>>(message.begin(), message.end());

  client.sendTo(std::move(payload), SERVER_IP, SERVER_PORT);

  // Receive response
  char buffer[1024];
  char serverIP[INET_ADDRSTRLEN];
  uint16_t serverPort;

  size_t received =
      client.receiveFrom(buffer, sizeof(buffer), serverIP, &serverPort);
  if (received > 0) {
    buffer[received] = '\0'; // Null-terminate the received string
    std::cout << "Response from server: " << buffer << std::endl;
  }

  return 0;
}
