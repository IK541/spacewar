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


void Player::set_nick(char buffer[1024]){
    nick = buffer;
}


string Player::get_player_info(){
    string msg = "";
    msg += "nick: " + nick + "\n";
    msg += "ready: " + to_string(ready) + "\n";
    return msg;
}

