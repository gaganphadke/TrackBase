#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080

// 🛠️ Function to send a commit message to the server
void sendCommit(const std::string& message) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("[Client] Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {  
        perror("[Client] Invalid address/Address not supported");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("[Client] Connection Failed");
        close(sock);
        return;
    }

    std::string newMessage = message + "\n";  // Add newline to end of message
    if (send(sock, newMessage.c_str(), newMessage.size(), 0) < 0) {
        perror("[Client] Send Failed");
        close(sock);
        return;
    }

    if (read(sock, buffer, 1024) < 0) {
        perror("[Client] Read Failed");
        close(sock);
        return;
    }

    std::cout << "[Client] Server: " << buffer << "\n";
    close(sock);
}
