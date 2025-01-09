#pragma once
#include "Serv.hpp"

#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <iostream>
#include <thread>



using namespace std;




Serv::Serv(int _port){
        port = _port;
        opt = 1;
        addrlen = sizeof(address);
        buffer_size = 1024;

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket failed");
            exit(-1);
        }

        // Attach socket to the port
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("Setsockopt failed");
            exit(-1);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(-1);
        }

        std::cout << "server created successfully\n";


}


void Serv::serve(){

        // Start listening
        if (listen(server_fd, 10) < 0) {
            perror("Listen failed");
            exit(-1);
        }

        std::cout << "Server is listening on port " << port << "...\n";
        while (true) {
            int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (client_fd < 0) {
                perror("Accept failed");
                continue;
            }

            std::cout << "New client connected.\n";

            // std::thread client

            // Create a new process for the client
            pid_t pid = fork();
            if (pid == 0) { // Child process
                close(server_fd); // Close the server socket in the child process
                handle_client(client_fd);
                exit(0);
            } else if (pid < 0) {
                perror("Fork failed");
            }

            close(client_fd); // Parent process closes the client socket
        }

        close(server_fd);
        return;

    }



void Serv::handle_client(int client_fd){
    char buffer[buffer_size] = {0};

    memset(buffer, 0, buffer_size);

    // get nickname
    std::string nickname;
    std::string msg = "give nick";

    send(client_fd, msg.c_str(), strlen(msg.c_str()), 0);

    int bytes_read = read(client_fd, buffer, buffer_size);
        if (bytes_read <= 0) {
            std::cout << "nick error disconnected.\n";
        }

    nickname = std::string(buffer, bytes_read - 1);
    memset(buffer, 0, buffer_size);

    std::cout << "nick got:" << nickname << "\n";

    //give rooms
    std::string rooms = "room1: 2, 0\nroom2: 0,0";
    send(client_fd, rooms.c_str(), strlen(rooms.c_str()), 0);

    
    while (true) {
        memset(buffer, 0, buffer_size);
        int bytes_read = read(client_fd, buffer, buffer_size);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected.\n";
            break;
        }
        std::cout << "Received from client: " << buffer;
        send(client_fd, buffer, bytes_read, 0);
    }

    close(client_fd);
}