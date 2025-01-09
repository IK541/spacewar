#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        return -1;
    }

    std::cout << "Connected to the server.\n";
    //              send  nick            
    std::cout << "enter nick:\n";
    std::string nick;

    std::cin.getline(buffer, BUFFER_SIZE);
    std::cout << "sending ur nick: " << nick << "\n";

    send(sock, buffer, strlen(buffer), 0);
    std::cout << "sent \n";


    memset(buffer, 0, BUFFER_SIZE);



        //              get rooms            

    std::cout << "getting room info" << "\n";
    
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_read = read(sock, buffer, BUFFER_SIZE);
    if (bytes_read <= 0) {
        std::cout << "Blad odczytu.\n";
    }
    std::cout << "Received from client: \n" << buffer;
    std::cin >> buffer;




    // Send messages
    while (true) {
        std::cout << "Enter message: ";
        std::cin.getline(buffer, BUFFER_SIZE);
        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            std::cout << "Server disconnected.\n";
            break;
        }
        std::cout << "Echo from server: " << buffer << "\n";
    }

    close(sock);
    return 0;
}
