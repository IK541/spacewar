#pragma once
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>

// structs

struct UserInput {
    sf::Vector2i pos;
    bool lmb;
    bool rmb;
};

struct UdpSendData {
    uint32_t timestamp;
    float direction;
    uint8_t flags;
};

struct TcpSendData {};

struct SendData {
    bool udp_present;
    UdpSendData udp_data;
    bool tcp_present;
    TcpSendData tcp_data;
};

// mock definitions

class GameState {
    public:
    virtual bool is_game_running() = 0;
};
class MockGameState : public GameState {
    public:
    bool is_game_running() { return true; }
};

// objects full declarations

class InputCollector {
    private:
    sf::Window* window;
    public:
    InputCollector(sf::Window* window);
    UserInput collect();
};

class InputTranslator {
    private:
    GameState* game_state;
    sf::Window* window;
    uint32_t timer;
    public:
    InputTranslator(GameState* game_state, sf::Window* window);
    void reset_timer();
    SendData translate(UserInput input);
};

// NEW
uint8_t* UdpOutputTranslator(UdpSendData data);
