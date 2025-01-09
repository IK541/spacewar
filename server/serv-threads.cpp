#include <iostream>
#include <thread>
#include <time.h>
#include <mutex>
#include <sys/wait.h>


#include "Room.hpp"
#include "Serv.hpp"
#include "Player.hpp"

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024



// #define MAX_CLIENTS_PER_ROOM 8

std::mutex *mtx = new mutex;
std::mutex Serv::serv_mutex;


int Room::free_room_id = 0;
Room Room::rooms[Room::max_rooms];
Player Player::players[Player::max_players];

void logic(int room_id){
    printf("room %i is rooming", room_id);

    // mtx->lock();

    printf("\n room id:%i ready \n", Room::rooms[room_id].getID());
    // mtx->unlock();
}



void serveRooms(){
    cout << "creating rooms\n";
    std::thread r1(logic, 0);
    std::thread r2(logic, 1);
    std::thread r3(logic, 2);
    
    r1.join();
    r2.join();
    r3.join();

    cout << "successfully joined all rooms";
}


void handleClients(){
    cout << "preparing client handler \n";

    Serv server(PORT);


    // uruchamia po forku funkcję, która wobsługuje klienta. Przepisać na poll?
    // cout << "test_begins\n";
    // server.test();
    server.serve(mtx);
}





int main(){

    for(int i = 0; i < Player::max_players; i++){
        cout << Player::players[i].free << endl;
    }

    std::thread serveR(serveRooms);
    sleep(1);

    std::thread serveC(handleClients);
    
    
    serveR.join();
    serveC.join();


    return 0;
}