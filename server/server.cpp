#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "game_engine.hpp"
#include "room_manager.hpp"

#define LISTEN_QUEUE 1
#define BUFFER_SIZE 4096

#define SERVER_PORT 10000

#define SHIP_ID 1

class InputState {
    private:
    GameIn input;
    std::mutex mtx;
    public:
    GameIn get_input(){std::lock_guard<std::mutex> lock(this->mtx);return this->input;}
    void set_input(GameIn i){std::lock_guard<std::mutex> lock(this->mtx);this->input=i;}
};

InputState input_state;

void receiver(int sfd) {
    int size = 0;
    uint8_t buffer[BUFFER_SIZE];
    while(1) {
        size = recvfrom(sfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        if(size < 9) continue;
        GameIn input = UdpInputTranslator((uint8_t*)buffer);
        input_state.set_input(input);
    }
}

int main() {
    int sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sfd < 0) perror("socket error");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERVER_PORT);
    if(bind(sfd, (sockaddr*) &saddr, sizeof(saddr))) perror("bind error");
    sockaddr_in caddr;
    socklen_t caddr_size = sizeof(caddr);

    GameEngine game_engine;
    game_engine.set_ship(SHIP_ID);

    uint8_t buffer[BUFFER_SIZE];
    recvfrom(sfd, buffer, BUFFER_SIZE, 0, (sockaddr*) &caddr, &caddr_size);
    std::thread recv_thread(receiver, sfd);
    recv_thread.detach();
    while (1) {
        game_engine.update_input(SHIP_ID, input_state.get_input());
        game_engine.update_physics(1.0/FPS);
        GameOut out = game_engine.get_output(SHIP_ID);
        uint8_t* bytes = UdpOutputTranslator(out);
        sendto(sfd, bytes, 18+6*out.movables_count, 0, (sockaddr*) &caddr, caddr_size);
        delete_GameOut(&out);
        delete [] bytes;
        usleep(1000000/FPS);
    }
    close(sfd);
    return 0;
}
