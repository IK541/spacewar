#pragma once
#include <cstring>
#include <netinet/in.h>
#include <mutex>
#include <thread>
#include "game_engine.hpp"
#include "../common.hpp"

#define SERVER_PORT 10000

#define BUFFER_SIZE 4096

namespace gamespace{

// NEW
uint8_t* UdpOutputTranslator(GameOut output);

GameIn UdpInputTranslator(uint8_t* input);


struct PlayerInput {
    uint8_t ship_id;
    sockaddr_in addr;
    GameIn input;
};
struct GameManagerInput {
    uint8_t ship_id;
    sockaddr_in addr;
};
class Players {
    private:
    std::vector<PlayerInput> players;
    std::mutex mtx;
    public:
    GameIn get(uint8_t ship_id);
    void set(sockaddr_in addr, GameIn input);
    void init(std::vector<GameManagerInput> players);
};
void game_recv(int sfd, Players* players);

class GameManager {
    std::thread recv_thread;
    Players* players;
    int sfd;

    public:
    // returns winner
    int run_game(std::vector<GameManagerInput> players);
    // NEW
    GameManager(int room_id);
    ~GameManager();
};
} // namespace gamespace