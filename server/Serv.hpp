#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <poll.h>


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

    void handle_new_connection(pollfd pfds[], bool free_pfds[], int server_fd);

    void handle_client_input(int client_id, pollfd *pfds, bool *free_pfds);


    void handle_client(int client_fd);

    void disconnect_client(int client_id, pollfd *pfds, bool *free_pfds);

    void handle_client_output(int client_id, pollfd *pfds, bool *free_pfds);

};