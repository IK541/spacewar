#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <stdio.h>
#include <math.h>

#define FPS 100
#define WINDOW_SIZE 800
#define BASE_HP_WIDTH 20
#define SIGHT_LIMIT 8

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 10000

#define TEST_SIZE 1.f

struct SendData {
    float direction;
    bool shoot;
    bool engine_on;
};

int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Spacewar");
    window.setFramerateLimit(FPS);
    sf::Vector2u window_size = sf::Vector2u(WINDOW_SIZE, WINDOW_SIZE);

    // Test Shape
    sf::CircleShape shape(TEST_SIZE);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(TEST_SIZE, TEST_SIZE);

    // Network
    sf::UdpSocket socket;
    sf::IpAddress addr = SERVER_ADDR;
    unsigned short port = SERVER_PORT;

    int x = 0;
    while (window.isOpen()) {
        // init phase
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::Resized) {
                window_size = sf::Vector2u(event.size.width, event.size.height);
                sf::Vector2f window_sizef = sf::Vector2f(window_size);
                float vx = SIGHT_LIMIT/sqrtf(1+(window_sizef.y*window_sizef.y)/(window_sizef.x*window_sizef.x));
                float vy = SIGHT_LIMIT/sqrtf(1+(window_sizef.x*window_sizef.x)/(window_sizef.y*window_sizef.y));
                window.setView(sf::View(sf::FloatRect(-vx, -vy, 2*vx, 2*vy)));
            }
        }

        // server phase

        // draw phase
        window.clear();
        window.draw(shape);
        window.display();

        // user phase
        sf::Vector2i mouse_position = sf::Mouse::getPosition(window) - sf::Vector2i(window_size.x/2,window_size.y/2);
        float mouse_direction = atan2f((float)mouse_position.y, (float)mouse_position.x);
        SendData send_data = {
            mouse_direction,
            sf::Mouse::isButtonPressed(sf::Mouse::Left),
            sf::Mouse::isButtonPressed(sf::Mouse::Right)
        };

        if (socket.send((char*) &send_data, sizeof(send_data), addr, port) != sf::Socket::Done) {
            printf("UDP error\n");
        }
    }

    return 0;
}
