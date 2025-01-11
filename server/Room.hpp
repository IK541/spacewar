#pragma once

#include <cstdio>
#include <unistd.h> 
#include "Player.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

class Room {
public:
    static const int max_players = 6; 
    static const int max_rooms = 3;
    static int free_room_id; 
    static Room rooms[ max_rooms ];
    int id;
    int free_slots;
    bool playing;
    int teams_player_number[2];

    static mutex rooms_mutex;
    mutex game_mtx; //TODO  semafor to implement
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::string> events;
    bool stop = false;
    static bool work;


    Room();

    int get_id();
    int get_ready_players();
    int get_player_count();
    string get_room_info_human();

    string get_room_info();

    string join_room(int id);

    string switch_teams(int id);


    bool start_game();

    static string get_general_room_info();

    void monitor();



};