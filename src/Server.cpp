#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 128

using namespace std;

void handle(int fd) {
    char buff[BUFFER_SIZE];
    bzero(&buff, sizeof(buff));

    while (true) {
        memset(&buff, '\0', sizeof(buff));

        // Receive message from client
        int bytes_received = recv(fd, buff, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            // Client has disconnected or error occurred
            if (bytes_received == 0) {
                cout << "Client disconnected gracefully\n";
            } else {
                cerr << "Error receiving data from client\n";
            }
            close(fd);
            return;
        }

        buff[bytes_received] = '\0'; // Null-terminate the received message

        if (strcasecmp(buff, "*1\r\n$4\r\nPING\r\n") == 0) {
            // Valid Redis PING command
            if (send(fd, "+PONG\r\n", 7, 0) == -1) {
                cerr << "Error sending PONG response\n";
                close(fd);
                return;
            }
        } else {
            // Unexpected command received, ignore or handle appropriately
            cerr << "Unexpected command: " << buff << "\n";
            continue;
        }
    }
}

int main(int argc, char **argv) {
    cout << "Logs from your program will appear here!\n";

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Failed to create server socket\n";
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        cerr << "setsockopt failed\n";
        close(server_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        cerr << "Failed to bind to port 6379\n";
        close(server_fd);
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        cerr << "listen failed\n";
        close(server_fd);
        return 1;
    }

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        cout << "Waiting for a client to connect...\n";

        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            cerr << "Failed to accept client connection\n";
            continue;
        }

        cout << "Client connected\n";
        handle(client_fd);
    }

    close(server_fd);
    return 0;
}
