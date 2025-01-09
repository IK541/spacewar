#pragma once

#include <sys/socket.h>
#include <netinet/in.h>



class Serv {
    int port;
    int server_fd;
    sockaddr_in address;
    int opt;
    int addrlen;
    int buffer_size;

    public:


    Serv(int _port);

    void serve();


    void handle_client(int client_fd);

};