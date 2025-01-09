#pragma once

#include <vector>
#include <cstdio>
#include <unistd.h> // for sleep
using namespace std;

class Player {
public:
    bool ready;
    // team will be added
    // name will be added
    // socket will be added
};

class Room {
public:
    int id;
    vector<Player> players;
    bool playing;

    Room(int);

    int getID();
    int getReadyPlayers();
    int getPlayerCount();
    bool startGame();
};
