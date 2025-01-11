#include <cmath>
#include <unistd.h>
#include <sys/socket.h>
#include <cstdio>
#include "game_manager.hpp"
#include "game_engine.hpp"



#pragma pack(push, 1)
struct CompressedMovable {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t angle;
};
#pragma pack(pop)

uint8_t* UdpOutputTranslator(GameOut output) {
    int size = OUTPUT_HEAD_SIZE + (output.movables_count) * sizeof(CompressedMovable);
    uint8_t* bytes = new uint8_t[size];
    int pos = 0;
    memcpy(bytes+pos, &output.timestamp, sizeof(output.timestamp)); pos += sizeof(output.timestamp);
    memcpy(bytes+pos, &output.blue_hp, sizeof(output.blue_hp)); pos += sizeof(output.blue_hp);
    memcpy(bytes+pos, &output.red_hp, sizeof(output.red_hp)); pos += sizeof(output.red_hp);
    memcpy(bytes+pos, &output.ammo, sizeof(output.ammo)); pos += sizeof(output.ammo);
    memcpy(bytes+pos, &output.reload, sizeof(output.reload)); pos += sizeof(output.reload);
    memcpy(bytes+pos, &output.rearm, sizeof(output.rearm)); pos += sizeof(output.rearm);
    memcpy(bytes+pos, &output.respawn, sizeof(output.respawn)); pos += sizeof(output.respawn);
    memcpy(bytes+pos, &output.ship_id, sizeof(output.ship_id)); pos += sizeof(output.ship_id);
    memcpy(bytes+pos, &output.movables_count, sizeof(output.movables_count)); pos += sizeof(output.movables_count);
    for(SpaceObject* movable: *output.objects) {
        double angle = movable->angle;
        CompressedMovable compressed_movable = CompressedMovable{
            movable->id,
            (uint16_t)((movable->x + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            (uint16_t)((movable->y + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            (uint8_t)(fmod(angle+2*M_PI,2*M_PI)/(2*M_PI)*UINT8_MAX)
        };
        memcpy(bytes+pos, &compressed_movable, sizeof(CompressedMovable));
        pos += sizeof(CompressedMovable);
    }
    return bytes;
}

GameIn UdpInputTranslator(uint8_t* input) {
    return GameIn {
        *(uint32_t*)(input),
        *(float*)(input+4),
        (bool)(*(input+8) & 0x02),
        (bool)(*(input+8) & 0x01)
    };
}



GameIn Players::get(uint8_t ship_id) {
    std::lock_guard<std::mutex> lock(this->mtx);
    for(int i = BLUE_TEAM_BEGIN; i < BLUE_BULLETS_BEGIN; ++i) {
        if(this->players[i-1].ship_id == ship_id)
            return this->players[i-1].input;
    } return GameIn{0,0.0,false,false};
}

void Players::set(sockaddr_in addr, GameIn input) {
    std::lock_guard<std::mutex> lock(this->mtx);
    int i = 0;
    for(PlayerInput player: this->players) {
        if(player.addr.sin_addr.s_addr == addr.sin_addr.s_addr
            && player.addr.sin_port == addr.sin_port
        ) this->players[i].input = input;
        ++i;
    }
}

void Players::init(std::vector<GameManagerInput> players) {
    std::lock_guard<std::mutex> lock(this->mtx);
    this->players.clear();
    for(GameManagerInput player: players) {
        float angle = (float)(player.ship_id<RED_TEAM_BEGIN?M_PI_2:-M_PI_2);
        PlayerInput new_player = PlayerInput{player.ship_id,player.addr,GameIn{0,angle,false,false}};
        this->players.push_back(new_player);
    }
}

void game_recv(int sfd, Players* players) {
    int size = 0;
    uint8_t buffer[BUFFER_SIZE];
    sockaddr_in addr;
    socklen_t addr_size = sizeof(sockaddr_in);
    while(1) {
        size = recvfrom(sfd, buffer, BUFFER_SIZE, 0, (sockaddr*) &addr, &addr_size);
        if(size < 9) continue;
        GameIn input = UdpInputTranslator((uint8_t*)buffer);
        players->set(addr, input);
    }
}

int GameManager::run_game(std::vector<GameManagerInput> players) {
    GameEngine game_engine;
    this->players->init(players);
    for(GameManagerInput player: players)
        game_engine.set_ship(player.ship_id);

    while(true) {
        uint64_t start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        for(GameManagerInput player: players) {
            sockaddr_in addr = player.addr;
            uint8_t ship_id = player.ship_id;
            GameIn in = this->players->get(ship_id);
            game_engine.update_input(ship_id, in);
            GameOut out = game_engine.get_output(ship_id);
            uint8_t* bytes = UdpOutputTranslator(out);
            sendto(sfd, bytes, OUTPUT_HEAD_SIZE+sizeof(CompressedMovable)*out.movables_count, 0, (sockaddr*) &addr, sizeof(addr));
            delete_GameOut(&out);
            delete [] bytes;
        }
        int result = game_engine.update_physics(1.0/FPS);
        if(result != NO_WIN) return result;
        uint64_t end = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        uint32_t to_sleep = (uint32_t)(end - start > 1000000UL/FPS ? 0 : 1000000UL/FPS - end + start);
        std::usleep(to_sleep);
    }
}

GameManager::GameManager(int room_id) {
    this->players = new Players;

    this->sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sfd < 0) perror("socket error");
    int one = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(SERVER_PORT+room_id);
    if(bind(sfd, (sockaddr*) &saddr, sizeof(saddr))) perror("bind error");

    this->recv_thread = std::thread(game_recv, this->sfd, this->players);
    recv_thread.detach();
}

GameManager::~GameManager() {
    shutdown(this->sfd, SHUT_RDWR);
    close(sfd);

    delete this->players;
}
