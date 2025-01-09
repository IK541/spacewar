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

#define PLAYERS_COUNT = 20;


// #define MAX_CLIENTS_PER_ROOM 8

std::mutex *mtx = new mutex;


int Room::free_room_id = 0;
Room rooms[3];
Player players[20];

void logic(int room_id){
    printf("room %i is rooming", room_id);

    mtx->lock();

    printf("\n room id:%i ready \n", rooms[room_id].getID());
    mtx->unlock();
}



void serveRooms(){
    cout << "creating rooms\n";
    std::thread r1(logic, 0);
    std::thread r2(logic, 1);
    std::thread r3(logic, 2);
    
    r1.join();
    r2.join();
    r3.join();

    sleep(1);

    // cout << "ready rooms:" << rooms.size() << endl;

    // na potrzeby testow
    sleep(9);
}


void handleClients(){
    cout << "preparing client handler \n";

    Serv server(PORT);


    // uruchamia po forku funkcję, która wobsługuje klienta. Przepisać na poll?
    server.serve(players, rooms, mtx);

    cout << "client handler prepared\n";

    // na potrzeby tesstow
    sleep(10);
}





int main(){

    for(int i = 0; i < 20; i++){
        players[i] = Player();
    }

    std::thread serveR(serveRooms);
    sleep(1);
    std::thread serveC(handleClients);
    
    
    serveR.join();
    serveC.join();


    return 0;
}