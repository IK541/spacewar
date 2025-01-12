#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include "game_engine.hpp"
#include "game_manager.hpp"

#define LISTEN_QUEUE 1
#define BUFFER_SIZE 4096

#define SERVER_PORT 56789

#define SHIP_ID 1
#define ROOM_ID 0

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
    uint8_t buffer[BUFFER_SIZE];
    recvfrom(sfd, buffer, BUFFER_SIZE, 0, (sockaddr*) &caddr, &caddr_size);
    close(sfd);

    GameManager game_manager(ROOM_ID);
    std::vector<GameManagerInput> gm_input;
    gm_input.push_back(GameManagerInput{SHIP_ID, caddr});
    int result = game_manager.run_game(gm_input);
    printf("winner: %d\n", result);

    return 0;
}
