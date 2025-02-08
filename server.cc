#include "udp.h"
#include <cstring>
#include <iostream>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main() {
  UDP server;
  server.bind(SERVER_IP, SERVER_PORT);

  char buffer[1024];
  char clientIP[INET_ADDRSTRLEN];
  uint16_t clientPort;

  std::cout << "UDP Server listening on " << SERVER_IP << ":" << SERVER_PORT
            << std::endl;

  while (true) {
    size_t received =
        server.receiveFrom(buffer, sizeof(buffer), clientIP, &clientPort);
    if (received > 0) {
      buffer[received] = '\0'; // Null-terminate the received string
      std::cout << "Received from " << clientIP << ":" << clientPort << " -> "
                << buffer << std::endl;

      // Send response
      std::string response = "Message received!";
      auto responsePayload =
          std::make_unique<std::vector<char>>(response.begin(), response.end());
      server.sendTo(std::move(responsePayload), clientIP, clientPort);
    }
  }

  return 0;
}
