#pragma once

#include <cstdio>
#include <unistd.h> // for sleep
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

using namespace std;

class Player {
public:
    bool ready;
    bool team;
    int room;
    string nick;
    sockaddr_in address;
    int fd;
    bool free;

    static const int max_players = 6;
    static Player players[max_players];



    Player();

    void take(sockaddr_in _address, int _fd);

    void make_free();

    void setNick(char buffer[1024]);
    static int find_free_slot_serv(){
        
        for(int i = 0; i < max_players; i++){
            if(players[i].free)
            return i;
        }
        return -1;
    }
};