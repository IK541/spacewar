#pragma once

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024


class Serv {
    int port; // accept port
    int server_fd; // accept socket file descriptor
    sockaddr_in address; // accept address
    int opt; // 1 => SO_REUSE_ADDR
    int addrlen; // length of address

    public:


    Serv(int _port);

    void serve();


    void handle_client(int client_fd);

};