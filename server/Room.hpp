#pragma once

#include <cstdio>
#include <unistd.h> 
#include "Player.hpp"

using namespace std;

class Room {
public:

    static const int max_players = 6;
    int id;
    Player players[max_players];
    bool room_slots[max_players];
    bool playing;

    Room(int id);

    int getID();
    int getReadyPlayers();
    int getPlayerCount();
    bool startGame();
};
