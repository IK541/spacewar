#pragma once

#include <SFML/Window.hpp>

#include "../common.hpp"

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

// mocks

class MockGameState : public GameStateI {
    public:
    bool is_game_running() { return true; }
};

// objects

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
    // NEW
    SendData translate(UserInput input);
};

// NEW
uint8_t* UdpOutputTranslator(GameIn data);
