#ifndef ROOM_H
#define ROOM_H

#include <vector>
#include <cstdio>
#include <unistd.h> // for sleep
using namespace std;

class Player {
public:
    bool ready; 
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

#endif
