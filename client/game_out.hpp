#pragma once

#include <SFML/Window.hpp>

#include "../common.hpp"

// structs

struct UserInput {
    sf::Vector2i pos;
    bool lmb;
    bool rmb;
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
    sf::Window* window;
    uint32_t timer;
    public:
    InputTranslator(sf::Window* window);
    void reset_timer();
    // NEW
    GameIn translate(UserInput input);
};

// NEW
uint8_t* UdpOutputTranslator(GameIn data);
