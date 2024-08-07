#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// Use the standard namespace to simplify code
using namespace std;

int main(int argc, char **argv) {
    // You can use print statements as follows for debugging, they'll be visible when running tests.
    cout << "Logs from your program will appear here!\n";

    // Uncomment this block to pass the first stage

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Failed to create server socket\n";
        return 1;
    }

    // Since the tester restarts your program quite often, setting REUSE_PORT
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        cerr << "setsockopt failed\n";
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        cerr << "Failed to bind to port 6379\n";
        return 1;
    }

    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        cerr << "listen failed\n";
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    cout << "Waiting for a client to connect...\n";

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0) {
        cerr << "Failed to accept client connection\n";
        return 1;
    }
    cout << "Client connected\n";

    // Use a buffer to store received messages
    char msg[100];

    // Loop to handle client messages
    while (true) {
        ssize_t bytes_received = recv(client_fd, msg, sizeof(msg) - 1, 0);
        if (bytes_received == -1) {
            cerr << "Failed to receive message\n";
            return 1;
        }

        // Null-terminate the received data
        msg[bytes_received] = '\0';

        cout << "Received message: " << msg << endl;

        const char *response = "+PONG\r\n";
        if (send(client_fd, response, strlen(response), 0) == -1) {
            cerr << "Failed to send message\n";
            return 1;
        }
    }

    close(server_fd);
    return 0;
}
