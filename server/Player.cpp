#include "Player.hpp"

Player::Player(){
    free = true;
    address = {};
    fd = -1;
    ready = false;
    team = 0;
    nick = "free player";
    room = -1;
}

void Player::take(sockaddr_in _address, int _fd){
    address = _address;
    fd = _fd;
    ready = false;
    team = 0;
    nick = "default player";
    room = -1;
    free = false;
}



void Player::make_free(){
    address = {};
    fd = -1;
    ready = false;
    team = 0;
    nick = "free player";
    room = -1;
    free = true;
}


string Player::set_nick(string _nick){

    for(int i = 0; i < Player::max_players; i++)
        if(Player::players[i].nick == _nick) return "N\nNick taken\n";

    nick = _nick;
    return "Y\nNick set properly\n";
}


string Player::get_player_info(){
    string msg = "";
    msg += "nick: " + nick;
    msg += "ready: " + to_string(ready) + "\n";
    return msg;
}

string Player::change_ready_state(){
    if(room == -1)
        return "N\nNot in room " + to_string(ready) + "\n";


    if(ready) ready = 0;
    else ready = 1;
    return "Y\nState changed to " + to_string(ready) + "\n";
}