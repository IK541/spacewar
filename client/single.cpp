#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>

#include "user_input.hpp"
#include "draw.hpp"
#include "../server/game_engine.hpp"

#define WINDOW_SIZE 800
// #define SIGHT_LIMIT 30

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 10000

#define TEST_SIZE 1.f

#define PLAYER_ID 1
#define SHIP_ID 1

void resize_window(sf::RenderWindow* window, sf::Event* event, float sight_limit);

// HELPERS:

Input send_to_input(SendData send_data);
UdpRecvData output_to_recv(Output output);

int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Spacewar");
    window.setFramerateLimit(FPS);

    // Drawing
    GameState game_state;
    Drawer drawer;
    Asteroid asteroid(ASTEROIDS_BEGIN,vec2{0.0,0.0},vec2{5.0,5.0});
    drawer.add(&asteroid);

    // Network
    GameEngine game_engine;
    game_engine.set_player(SHIP_ID, PLAYER_ID);

    // input
    MockGameState mock_game_state;
    InputCollector input_collector(&window);
    InputTranslator input_translator(&mock_game_state, &window);

    while (window.isOpen()) {
        // init phase
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) resize_window(&window, &event, SIGHT_LIMIT);
        }

        // server phase
        game_engine.update_physics(1.0/FPS);
        // FIX why is output neighours count 0? It should be at least 1 (player's ship itself)
        Output out = game_engine.get_output(PLAYER_ID);
        UdpRecvData server_output = output_to_recv(out);
        game_state.setMovables(server_output.timestamp, server_output.movables);

        // draw phase
        drawer.addAll(&game_state);
        window.clear();
        drawer.draw(&window);
        window.display();
        drawer.clear();

        // user phase
        UserInput user_input = input_collector.collect();
        SendData out_data = input_translator.translate(user_input);
        game_engine.update_input(PLAYER_ID, send_to_input(out_data));
    }

    return 0;
}

void resize_window(sf::RenderWindow* window, sf::Event* event, float sight_limit) {
    sf::Vector2f window_size = sf::Vector2f(sf::Vector2u(event->size.width, event->size.height));
    float vx = SIGHT_LIMIT/sqrtf(1+(window_size.y*window_size.y)/(window_size.x*window_size.x));
    float vy = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.x)/(window_size.y*window_size.y));
    window->setView(sf::View(sf::FloatRect(-vx, -vy, 2*vx, 2*vy)));
}

Input send_to_input(SendData send_data) {
    return Input {
        .direction = send_data.udp_data.direction,
        .shoot = (bool)(send_data.udp_data.flags & 2),
        .engine_on = (bool)(send_data.udp_data.flags & 1),
    };
}

UdpRecvData output_to_recv(Output output) {
    uint16_t count = output.neighbours.count;
    std::vector<Movable*>* movables = new std::vector<Movable*>();
    for(int i = 0; i < count; ++i) {
        movables->push_back(output.neighbours.movables[i]);
    }
    return UdpRecvData {
        .timestamp = output.timestamp,
        .red = output.red,
        .blue = output.blue,
        .ammo = output.player_data.ammo,
        .reload = output.player_data.reload,
        .rearm = output.player_data.rearm,
        .respawn = output.player_data.respawn,
        .movables_count = count,
        .movables = movables
    };
}
