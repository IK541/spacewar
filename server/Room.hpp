#pragma once

#include <cstdio>
#include <unistd.h> // for sleep
#include "Player.hpp"
#include <mutex>

using namespace std;

class Room {
public:
    static const int max_players = 6;
    static int free_room_id;
    int id;
    Player players[max_players];
    bool room_slots[max_players];
    bool playing;

    Room();

    int getID();
    int getReadyPlayers();
    int getPlayerCount();
    bool startGame();
};
