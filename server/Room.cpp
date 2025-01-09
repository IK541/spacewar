#include "Room.hpp"
#include <vector>


Room::Room(int room_id) {
    id = room_id;
    playing = false;
}

// will be changed to get room info
int Room::getID() {
    return id;
}

int Room::getReadyPlayers() {
    int ready = 0;
    for (unsigned i = 0; i < players.size(); i++) {
        if (players[i].ready)
            ready++;
    }
    return ready;
}

int Room::getPlayerCount() {
    return players.size();
}

bool Room::startGame() {
    playing = true;
    printf("Game has started\n");
    sleep(5);
    printf("Game has ended\n");
    playing = false;
    return true;
}
