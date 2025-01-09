#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>


#include "Room.hpp"

#define BUFFER_SIZE 1024


class Serv {
    int port; // accept port
    int server_fd; // accept socket file descriptor
    sockaddr_in address; // accept address
    int addrlen; // length of address

    public:


    Serv(int _port);

    void serve(Player players[], Room rooms[], std::mutex *mtx);


    void handle_client(int client_fd);

};