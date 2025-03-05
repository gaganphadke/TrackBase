#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "/Users/gaganphadke/Versioning/versioned-db/include/Database.h"
#include "/Users/gaganphadke/Versioning/versioned-db/include/VersionControl.h"

#define PORT 8080

int main() {
    Database db("data/mydb.json");
    db.load();
    VersionControl vc(db, "RemoteUser");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");;
    address.sin_port = htons(PORT);

    // Bind socket to port
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        char buffer[1024] = {0};
        read(new_socket, buffer, 1024);

        std::string data(buffer);
        std::string command = data.substr(0, data.find(' '));
        std::string message = data.substr(data.find(' ') + 1);

        if (command == "commit") {
            vc.commit(message);
            std::cout << "[Server] Commit received: " << message << std::endl;  // Log received commit
            send(new_socket, "Commit received", strlen("Commit received"), 0);
        } else {
            send(new_socket, "Invalid command", strlen("Invalid command"), 0);
        }
        close(new_socket);
    }
    return 0;
}
