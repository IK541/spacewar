#include "Room.hpp"
#include <vector>
#include <cstdio>
#include <unistd.h> // for sleep
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "Player.hpp"
#include <mutex>
using namespace std;



Room::Room() {
    id = free_room_id++;
    playing = false;
    for(int i = 0; i < Room::max_players; i++){
        room_slots[i] = false;
    }
}

// will be changed to get room info
int Room::getID() {
    return id;
}

int Room::getReadyPlayers() {
    int ready = 0;
    for (int i = 0; i < max_players ; i++) {

        if(room_slots[i] && players[i].ready)
            ready++;
    }
    return ready;
}

int Room::getPlayerCount() {
    int count = 0;
    for (int i = 0; i < max_players ; i++) {

        if(room_slots[i]){
            count++;
        }
    }
    return count;
}

bool Room::startGame() {
    playing = true;
    printf("Game has started\n");
    sleep(5);
    printf("Game has ended\n");
    playing = false;
    return true;
}


