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

    pollfd pfds[Player::max_players + 1]{};
    bool free_pfds[Player::max_players]{};
    static std::mutex serv_mutex;


    Serv(int _port);


    void serve(Player players[], Room rooms[], std::mutex *mtx);

    void serve(std::mutex *mtx);

    void handle_new_connection();

    void handle_client_input(int client_id);

    void disconnect_client(int client_id);

    void handle_client_output(int client_id);

    void cleanup();
};