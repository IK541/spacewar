#include <vector>
#include "Player.hpp"
class Room{

    public:
    std::vector<Player> players;
    int get_ready_players(){
        int readys = 0;
        for(int i = 0; i < players.size(); i++){
            if(players[i].ready)
                readys++;
        }
        return readys;
    };
    int get_number_of_players(){
        return players.size();
    }

};