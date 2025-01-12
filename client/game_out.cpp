#include "game_out.hpp"

#include <cstring>

InputCollector::InputCollector(sf::Window* window):window(window){};
UserInput InputCollector::collect() {
    return UserInput {
        sf::Mouse::getPosition(*window),
        sf::Mouse::isButtonPressed(sf::Mouse::Left),
        sf::Mouse::isButtonPressed(sf::Mouse::Right)
    };
}

InputTranslator::InputTranslator(sf::Window* window) {
    this->window = window;
    this->timer = 0;
}

void InputTranslator::reset_timer() { this->timer = 0; }

GameIn InputTranslator::translate(UserInput input) {
    sf::Vector2f mouse_position = sf::Vector2f(input.pos) - 0.5f * sf::Vector2f(this->window->getSize());
    return GameIn {
        this->timer++,
        atan2f(-mouse_position.y, mouse_position.x),
        input.lmb,
        input.rmb
    };
}

uint8_t* UdpOutputTranslator(GameIn data) {
    uint8_t* bytes = new uint8_t[10];
    uint8_t flags = data.shoot << 1 | data.engine_on;
    memcpy(bytes, &data.timestamp, 4);
    memcpy(bytes+4, &data.angle, 4);
    memcpy(bytes+8, &flags, 1);
    return bytes;
}
