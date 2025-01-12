#include "Room.hpp"
#include <vector>
#include <cstdio>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "Player.hpp"
#include "Serv.hpp"
#include <mutex>
#include <iostream>
#include "../single-server/game_manager.hpp"


using namespace std;



Room::Room() {
    id = free_room_id++;
    playing = false;
    free_slots = max_players;
    teams_player_number[0] = 0;
    teams_player_number[1] = 0;

}

// will be changed to get room info
int Room::get_id() {
    return id;
}

int Room::get_ready_players() {
    int ready = 0;

    for(int i = 0; i < Player::max_players; i++)
        if(Player::players[i].room == id && Player::players[i].ready)   ready++;
    
    return ready;
}

int Room::get_player_count() {
    int count = 0;
    for(int i = 0; i < Player::max_players; i++)
        if(Player::players[i].room == id)   count++;
    
    return count;
}
// DONE NOT TESTED
string Room::get_binary_room_info(){

//[0] msg type
//[1] 0 -> room
//[2] 0 -> Started
//[3] team 0 how many players
//[4] team 1 how many players
//[5] is 1st player ready
//[6-7] len of 1st player 
//[8] 1st player nick 
// [..]
//[9] is 1st player ready
//[10] len of 1st player
//[11] 1st player nick






    string info = "R ";

    info += to_string(id) + " ";
    info += to_string(playing) + " ";
    info += to_string(teams_player_number[0]) + " ";
    info += to_string(teams_player_number[1]) + " ";

    // team 1
    
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 0){
            info += to_string(Player::players[i].ready) + " ";
            info += Player::players[i].nick + " ";
        }
    }

    // team 2

    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 1){
            info += to_string(Player::players[i].ready) + " ";
            info += Player::players[i].nick + " ";
        }
    }

    info[info.size()-1] = '\n';



    return info;
}

string Room::get_room_info_human(){
    string info = to_string(id) + " room info: \n";

    info += "ID: " + to_string(id) + "\n";
    info += "PlayersNo: " + to_string(get_player_count()) + "\n";
    info += "Ready: " + to_string(get_ready_players()) + "\n";
    info += "Started: " + to_string(playing) + "\n";

    int no_of_players = 0;
    info += "Team 0: \n";

    
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 0)
        {
            info += Player::players[i].get_player_info();

            no_of_players++;
        }
    }
    if (!no_of_players){
        info+="None \n";
    }

    no_of_players= 0;

    info += "Team 1: \n";

    
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 1)
            info += Player::players[i].get_player_info();
    }

    if (!no_of_players){
        info+="None\n";
    }

    return info;}

// DONE NOT TESTED
bool Room::start_game() {
    unique_lock<mutex> lock(mtx);

    playing = true;
    printf("Game has started in room %i\n", id);

    for(int i = 0; i < Player::max_players; i++){
        Player::players[i].ready = false;
    }

    GameManager gm(id + 1);

    vector<GameManagerInput> p;

    int sh_id = BLUE_TEAM_BEGIN;

    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 0){
            GameManagerInput gmi;
            gmi.ship_id = sh_id + i;
            gmi.addr = Player::players[i].address;
            p.push_back(gmi);
        }
    }
    sh_id = RED_TEAM_BEGIN;
    for(int i = 0; i < Player::max_players; i++){
        if(Player::players[i].room == id && Player::players[i].team == 1){
            GameManagerInput gmi;
            gmi.ship_id = sh_id + i;
            gmi.addr = Player::players[i].address;
            p.push_back(gmi);
        }
    }


    string msg = "G\n";
    Serv::serv.send_to_room_members(id, msg);
    // notify lobby members
    string binary_lobby = Room::get_binary_general_room_info();

    Serv::serv.send_to_lobby_members(binary_lobby);
    int game_state = gm.run_game(p);
    if (game_state == DRAW){
        msg = "H\n";
        Serv::serv.send_to_room_members(id, msg);
    }
    else if (game_state == 1){
        msg = "I\n";
        Serv::serv.send_to_room_members(id, msg);
    }
    else if (game_state == 2){
        msg = "J\n";
        Serv::serv.send_to_room_members(id, msg);
    }

    msg = Room::get_binary_general_room_info();

    Serv::serv.send_to_lobby_members(msg);

    printf("Game has ended in room %i\n", id);
    playing = false;
    return true;
}


string Room::get_general_room_info() {
    string info = "Rooms info: ";

    for(int i = 0; i < max_rooms; i++){
        info += "ID: " + to_string(rooms[i].id) + "\n";
        // per team
        info += "Team 0: " + to_string(rooms[i].teams_player_number[0]) + "\n";
        info += "Team 1: " + to_string(rooms[i].teams_player_number[1]) + "\n";

        // info += "PlayersNo: " + to_string(rooms[i].get_player_count()) + "\n";
        // info += "Ready: " + to_string(rooms[i].get_ready_players()) + "\n";
        info += "Started: " + to_string(rooms[i].playing) + "\n";
    }


    return info;
}
// DONE NOT TESTED
string Room::get_binary_general_room_info() {
    // L + 3x<ID/blue/red/started>

    string info = "L ";
    
    for(int i = 0; i < max_rooms; i++){
        info += to_string(rooms[i].id) + " ";
        info += to_string(rooms[i].playing) + " ";
        info += to_string(rooms[i].teams_player_number[0]) + " ";
        info += to_string(rooms[i].teams_player_number[1]) + " ";
    }

    info[info.size()-1] = '\n';

    return info;   
}



string Room::join_room(int _id){
    rooms_mutex.lock();
    
    if (free_slots == 0){
        rooms_mutex.unlock();
        return "N\n room full\n";
    } 


    if (Player::players[_id].room != -1){
        // zwolnienie miejsca w pokoju i druzyny starego pokoju
        Room::rooms[Player::players[_id].room].free_slots++;
        Room::rooms[Player::players[_id].room].teams_player_number[Player::players[_id].team]--;
    }

    Player::players[_id].room = id;
    free_slots--;

    if( teams_player_number[0] < 3 && teams_player_number[0] <= teams_player_number[1]){
        printf("true\n");
        Player::players[_id].team = 0;
        teams_player_number[0]++;
    }
    else if(teams_player_number[1] < 3){
        Player::players[_id].team = 1;
        teams_player_number[1]++;

    }
    else{
        rooms_mutex.unlock();

        return "N\n this shouldn't happen\n";
    }


    rooms_mutex.unlock();
    return "Y\n";
}

string Room::leave_room(int client_id){
    rooms_mutex.lock();
    if (Player::players[client_id].room == -1){
        rooms_mutex.unlock();
        return "N\n not in room\n";
    }

    Room::rooms[Player::players[client_id].room].free_slots++;
    Room::rooms[Player::players[client_id].room].teams_player_number[Player::players[client_id].team]--;
    Player::players[client_id].room = -1;
    rooms_mutex.unlock();
    return "Y\n";
}

string Room::switch_teams(int _id){


    if (Player::players[_id].room == -1){
        return "N\n not in room\n";
    }


    rooms_mutex.lock();

    int curr_team = Player::players[_id].team;


    if (curr_team == 1){
        if(teams_player_number[0] >=3){
            rooms_mutex.unlock();
            return "N\n team full\n";
        }

        Player::players[_id].team = 0;
        teams_player_number[0]++;
        teams_player_number[1]--;

    } 

    else{
        if(teams_player_number[1] >=3){
            rooms_mutex.unlock();
            return "N\n team full\n";
        }

        Player::players[_id].team = 1;
        teams_player_number[1]++;
        teams_player_number[0]--;

    } 

    rooms_mutex.unlock();
    return "Y\n";
}


void Room::monitor(){
    std::cout << "monitoring room " << id << std::endl;
    while (true) {
        unique_lock<mutex> lock(game_mtx);
        cv.wait(lock, [this] { return !events.empty() || stop; });

        while (!events.empty()) {
            string event = events.front();
            std::cout << "detecten event: " << event << endl;
            if(event[0] == '0')
                start_game();

            events.pop();
        }

        if (stop && events.empty()) {
            break;
        }
    }
}