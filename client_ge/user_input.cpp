#include "user_input.hpp"
#include <cstring>

InputCollector::InputCollector(sf::Window* window):window(window){};
UserInput InputCollector::collect() {
    return UserInput {
        .pos = sf::Mouse::getPosition(*window),
        .lmb = sf::Mouse::isButtonPressed(sf::Mouse::Left),
        .rmb = sf::Mouse::isButtonPressed(sf::Mouse::Right)
    };
}

InputTranslator::InputTranslator(GameState* game_state, sf::Window* window) {
    this->game_state = game_state;
    this->window = window;
    this->timer = 0;
}

void InputTranslator::reset_timer() { this->timer = 0; }

SendData InputTranslator::translate(UserInput input) {
    SendData result;
    if (this->game_state->is_game_running()) {
        result.udp_present = true;
        sf::Vector2f mouse_position = sf::Vector2f(input.pos) - 0.5f * sf::Vector2f(this->window->getSize());
        result.udp_data = UdpSendData {
            .timestamp = this->timer++,
            .direction = atan2f(mouse_position.y, mouse_position.x),
            .flags = (uint8_t)(input.lmb << 1 | input.rmb),
        };
        // TODO: tcp
    }
    return result;
}

uint8_t* UdpOutputTranslator(UdpSendData data) {
    uint8_t* bytes = new uint8_t[sizeof(UdpSendData)];
    memcpy(bytes, &data.timestamp, 4);
    memcpy(bytes+4, &data.direction, 4);
    memcpy(bytes+8, &data.flags, 1);
    return bytes;
}
