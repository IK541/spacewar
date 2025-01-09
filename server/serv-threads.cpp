#include <iostream>
#include <thread>
#include <time.h>
#include <mutex>
#include <vector>
#include <sys/wait.h>


#include "Room.hpp"
#include "Serv.hpp"

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024


// #define MAX_CLIENTS_PER_ROOM 8

std::mutex mtx;


vector<Room> rooms;
vector<Player> players;


void logic(int room_id){
    printf("room %i is rooming", room_id);

    Room r(room_id);
    mtx.lock();
    rooms.push_back(r);

    printf("\n room id:%i ready \n", r.getID());
    mtx.unlock();
}



void serveRooms(){
    cout << "creating rooms\n";
    std::thread r1(logic, 1);
    std::thread r2(logic, 2);
    std::thread r3(logic, 3);
    
    r1.join();
    r2.join();
    r3.join();

    sleep(1);

    cout << "ready rooms:" << rooms.size() << endl;

    // na potrzeby testow
    sleep(9);
}


void handleClients(){
    cout << "preparing client handler \n";
    sleep(1);

    Serv server(PORT);


    // uruchamia po forku funkcję, która wobsługuje klienta. Przepisać na poll?
    // server.serve();

    cout << "client handler prepared\n";

    // na potrzeby tesstow
    sleep(10);
}





int main(){

    std::thread serveR(serveRooms);
    std::thread serveC(handleClients);
    
    
    serveR.join();
    serveC.join();


    return 0;
}