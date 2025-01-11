#include "Room.hpp"
#include <vector>
#include <cstdio>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "Player.hpp"
#include <mutex>
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

string Room::get_room_info(){
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
        info+="None \n";
    }

    return info;
}



bool Room::start_game() {
    playing = true;
    printf("Game has started\n");
    sleep(5);
    printf("Game has ended\n");
    playing = false;
    return true;
}


string Room::get_general_room_info() {
    string info = "Rooms info: ";

    for(int i = 0; i < max_rooms; i++){
        info += "ID: " + to_string(rooms[i].id) + "\n";
        info += "PlayersNo: " + to_string(rooms[i].get_player_count()) + "\n";
        info += "Ready: " + to_string(rooms[i].get_ready_players()) + "\n";
        info += "Started: " + to_string(rooms[i].playing) + "\n";
    }

    
    
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