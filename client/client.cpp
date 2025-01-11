#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Change to your server's IP if running on a different machine
#define SERVER_PORT 8080      // Change to your server's port
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in server_address;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported\n";
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection to the server failed\n";
        return -1;
    }

    std::cout << "Connected to the server.\n";

    // Test messages
    const char *messages[] = {
        "A Mynick!", // Should match the 'A' case
        "B Give me rooms",   // Should match the 'B' case
        "C Join room 1",     // Should match the 'C' case
        "D Ready!", // Should go to the default case
        nullptr                 // End of messages
    };

    for (int i = 0; messages[i] != nullptr; ++i) {
        // Send message
        send(sock, messages[i], strlen(messages[i]), 0);
        std::cout << "Sent: " << messages[i] << "\n";

        // Receive response
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Server response: " << buffer << "\n";
        } else {
            std::cerr << "Failed to receive response or connection closed.\n";
            break;
        }

        // Sleep for a short time to simulate delay between messages
        sleep(1);
    }

    // Close the connection
    close(sock);
    std::cout << "Disconnected from the server.\n";

    return 0;
}
