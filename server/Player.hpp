#include <cstdio>
#include <unistd.h> // for sleep
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

using namespace std;

class Player {
public:
    bool ready;
    bool team;
    int room;
    string nick;
    sockaddr_in address;
    int fd;
    bool free;



    Player();

    void take(sockaddr_in _address, int _fd);

    void make_free();

    static int find_free_slot_serv(Player players[]){
        
        for(int i = 0; i < 20; i++){
            if(players[i].free)
            return i;
        }
        return -1;
    }
};