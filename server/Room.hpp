#pragma once

#include <cstdio>
#include <unistd.h> 
#include "Player.hpp"
#include <mutex>

using namespace std;

class Room {
public:
    static const int max_players = 6; 
    static const int max_rooms = 3;
    static int free_room_id; // only to generate rooms
    static Room rooms[ max_rooms ];
    int id;
    int free_slots;
    bool playing;
    int teams_player_number[2];

    static mutex rooms_mutex;
    mutex room_mtx;
    mutex game_mtx; //TODO  semafor to implement

    Room();

    int get_id();
    int get_ready_players();
    int get_player_count();

    string get_room_info();

    string join_room(int id);

    string switch_teams(int id);


    bool start_game();

    static string get_general_room_info();



};