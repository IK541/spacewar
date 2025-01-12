#include <iostream>
#include <thread>
#include <time.h>
#include <mutex>
#include <sys/wait.h>



#include "Room.hpp"
#include "Serv.hpp"
#include "Player.hpp"

using namespace std;

#define PORT 56789



// #define MAX_CLIENTS_PER_ROOM 8

std::mutex Room::rooms_mutex;
std::mutex Serv::mtx;
bool Serv::work = true;
bool Room::work = true;

int Room::free_room_id = 0;
Room Room::rooms[Room::max_rooms];
Player Player::players[Player::max_players];
Serv Serv::serv(PORT);


void cleanup(){
    Serv::work = false;
    Room::work = false;
    Serv::serv.cleanup();
}

void signalHandler(int signum) {

    std::cout << "\nInterrupt signal (" << signum << ") received. Gracefully exiting...\n";
    cleanup();
    exit(signum);
}

void logic(int room_id){
    printf("room %i is rooming", room_id);

    // mtx->lock();

    printf("\n room id:%i ready \n", Room::rooms[room_id].get_id());

    Room::rooms[room_id].monitor();

    // while(true){
    //     mtx->lock();
    //     cout << Room::rooms[room_id].get_room_info();
    //     mtx->unlock();
    //     sleep(30);
    // }

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


    Serv::serv.serve();
    sleep(1);
}

void monitor_changes(){

    Serv::serv.monitor();
    sleep(1);

}



int main(){



    signal(SIGINT, signalHandler); // Handle Ctrl+C (SIGINT)

    std::thread serveR(serveRooms);
    sleep(1);

    std::thread serveC(handleClients);
    sleep(1);

    std::thread serveM(monitor_changes);
    sleep(1);

    
    
    serveR.join();
    serveC.join();
    serveM.join();


    return 0;
}