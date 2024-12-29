#include <string>
#include <thread>
class Player{
    public:
    std::string name;
    // std::mutex playerMutex;
    int socket;
    bool ready;
    bool in_room;

    Player(std::string _name, int _sock){
        name = _name;
        socket = _sock;

    }
};