#include "Player.hpp"
#include "Room.hpp"
#include "Serv.hpp"

Player::Player(){
    address = {};
    if(fd > 0) {
        shutdown(fd, SHUT_RDWR);
        close(fd);
    }
    fd = -1;
    ready = false;
    team = 0;
    nick = ""; //"free player";
    room = -1;
    free = true;
}

void Player::take(sockaddr_in _address, int _fd){
    address = _address;
    fd = _fd;
    ready = false;
    team = 0;
    nick = "";
    room = -1;
    free = false;
}



void Player::make_free(){
    address = {};
    fd = -1;
    ready = false;
    team = 0;
    nick = "";
    room = -1;
    free = true;
}

string Player::get_binary_player_info(){
    string msg = to_string(ready);
    if (nick.size() <= 9)
        msg += "0" + to_string(nick.size());
    else
        msg += to_string(nick.size());

    msg += nick;
    return msg;
}




bool Player::set_nick(string _nick){

    if (_nick.size() < 1 && _nick.size() >= 12) return 0;

    for(int i = 0; i < Player::max_players; i++)
        if(Player::players[i].nick == _nick) return 0;

    nick = _nick;
    return 1;
}


string Player::get_player_info(){
    string msg = "";
    msg += "nick: " + nick;
    msg += "ready: " + to_string(ready) + "\n";
    return msg;
}

string Player::change_ready_state(){
    unique_lock<mutex> lock(mtx);

    if(room == -1)
        return "N\nNot in room " + to_string(ready) + "\n";


    if(ready) ready = 0;
    else ready = 1;
    printf("Y\nready state changed to %i\n", ready);
    Serv::serv.events.push("1" + std::to_string(room));// FAILS HERE
    Serv::serv.cv.notify_one();

    if (Room::rooms[room].get_player_count() == Room::rooms[room].get_ready_players()
    &&  Room::rooms[room].teams_player_number[0] > 0
    && Room::rooms[room].teams_player_number[1] > 0){

                
                Room::rooms[room].events.push("0");
                Room::rooms[room].cv.notify_one();

    }




    return "Y\nState changed to " + to_string(ready) + "\n";
}