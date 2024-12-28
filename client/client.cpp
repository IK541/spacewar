#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include "user_input.hpp"
#include "draw.hpp"
#include "../common.hpp"

#define WINDOW_SIZE 800

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 10000

#define TEST_SIZE 1.f

void resize_window(sf::RenderWindow* window, sf::Event* event, float sight_limit);

int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Spacewar");
    window.setFramerateLimit(FPS);

    // Drawing
    Drawer drawer;
    Ship ship1(1,vec2{1.0,0.0},vec2{0.0,0.0},0.0,NULL,false);
    Ship ship2(4,vec2{-1.0,0.0},vec2{0.0,0.0},0.0,NULL,false);
    Asteroid asteroid(ASTEROIDS_BEGIN,vec2{0.0,0.0},vec2{5.0,5.0});
    drawer.add(&ship1);
    drawer.add(&ship2);
    drawer.add(&asteroid);

    // Network
    sf::UdpSocket socket;
    sf::IpAddress addr = SERVER_ADDR;
    unsigned short port = SERVER_PORT;

    // input
    MockGameState game_state;
    InputCollector input_collector(&window);
    InputTranslator input_translator(&game_state, &window);

    while (window.isOpen()) {
        // init phase
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) resize_window(&window, &event, SIGHT_LIMIT);
        }

        // server phase

        // draw phase
        window.clear();
        drawer.draw(&window);
        window.display();

        // user phase
        UserInput user_input = input_collector.collect();
        SendData out_data = input_translator.translate(user_input);
        uint8_t* binary_out_data = UdpOutputTranslator(out_data.udp_data);
        if (socket.send(binary_out_data, sizeof(UdpSendData), addr, port) != sf::Socket::Done) printf("UDP error\n");
        delete binary_out_data;
    }

    return 0;
}

void resize_window(sf::RenderWindow* window, sf::Event* event, float sight_limit) {
    sf::Vector2f window_size = sf::Vector2f(sf::Vector2u(event->size.width, event->size.height));
    float vx = SIGHT_LIMIT/sqrtf(1+(window_size.y*window_size.y)/(window_size.x*window_size.x));
    float vy = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.x)/(window_size.y*window_size.y));
    window->setView(sf::View(sf::FloatRect(-vx, -vy, 2*vx, 2*vy)));
}
