#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <poll.h>

#include <condition_variable>
#include <queue>
#include <string>


#include "Room.hpp"

#define BUFFER_SIZE 1024


class Serv {
    int port; // accept port
    int server_fd; // accept socket file descriptor
    sockaddr_in address; // accept address
    int addrlen; // length of address

    public:
    static bool work;

    pollfd pfds[Player::max_players + 1]{};
    bool free_pfds[Player::max_players]{};
    static std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::string> events;
    bool stop = false;

    static Serv serv;


    Serv(int _port);



    void serve();

    void handle_new_connection();

    void handle_client_input(int client_id);

    void disconnect_client(int client_id);

    void handle_client_output(int client_id);

    void monitor();

    void send_to_room_members(int room_id, string msg);

    void send_to_room_members(int room_id, char* msg, int len);


    void send_to_lobby_members(string msg);

    void send_to_lobby_members(char* msg, int len);


    void send_to_player(int player_id, string msg);

    void send_to_player(int player_id, char* msg, int len);



    

    void cleanup();
};