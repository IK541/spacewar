#pragma once
#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include "draw.hpp"

// structs

struct UserInput {
    sf::Vector2i pos;
    bool lmb;
    bool rmb;
};

struct TcpSendData {};

struct SendData {
    bool udp;
    void* data;
};

// mock definitions

class MockGameState : public GameStateI {
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
    GameStateI* game_state;
    sf::Window* window;
    uint32_t timer;
    public:
    InputTranslator(GameStateI* game_state, sf::Window* window);
    void reset_timer();
    SendData translate(UserInput input);
};

// NEW
uint8_t* UdpOutputTranslator(GameIn data);
