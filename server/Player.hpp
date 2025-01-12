#pragma once

#include <cstdio>
#include <unistd.h> // for sleep
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <mutex>
#include <queue>

using namespace std;

class Player {
public:
    bool ready;
    bool team;
    int room;
    string nick;
    sockaddr_in address;
    bool free;
    std::mutex mtx; // for msg
    std::queue<char> data;


    static const int max_players = 20;
    static Player players[max_players];



    Player();

    void take(sockaddr_in _address);

    void make_free();

    bool set_nick(string); // depricated, now debug only

    bool set_nick(string, int); 


    
    string change_ready_state();

    string get_player_info();

    string get_binary_player_info();


    static int find_free_slot_serv(){
        
        for(int i = 0; i < max_players; i++){
            if(players[i].free)
            return i;
        }
        return -1;
    }

};