#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <mutex>

#include "user_input.hpp"
#include "draw.hpp"

#define WINDOW_SIZE 800

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 10000
#define CLIENT_PORT 9000

#define TEST_SIZE 1.f

#define SHIP_ID 1

#define BUFFER_SIZE 4096

void resize(sf::Event* event, sf::Vector2f* window_size);
void set_view(sf::RenderWindow* window, sf::Vector2f window_size, sf::Vector2f center);

void receiver(GameState* game_state, sf::UdpSocket* socket) {
    std::size_t size = 0;
    uint8_t buffer[BUFFER_SIZE];
    while(1) {
        sf::IpAddress addr; unsigned short port;
        socket->receive(buffer, BUFFER_SIZE, size, addr, port);
        GameOut data = UdpInputTranslator((uint8_t*)buffer, size);
        game_state->set_game_state(data);
    }
}

int main() {
    // Window
    sf::Vector2f window_size = sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE);
    sf::RenderWindow window(sf::VideoMode((unsigned)window_size.x, (unsigned)window_size.y), "Spacewar");
    window.setFramerateLimit(FPS);

    // Drawing
    GameState* game_state = new GameState;
    Drawer drawer;

    // Network
    sf::UdpSocket* socket = new sf::UdpSocket;
    socket->bind(CLIENT_PORT);
    std::thread recv_thread(receiver, game_state, socket);
    recv_thread.detach();

    // input
    MockGameState mock_game_state;
    InputCollector input_collector(&window);
    InputTranslator input_translator(&mock_game_state, &window);

    while (window.isOpen()) {
        // init phase
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) resize(&event, &window_size);
        }

        // draw phase
        set_view(&window, window_size, game_state->get_center());
        drawer.add_all(game_state);
        window.clear();
        draw_bases(&window);
        drawer.draw(&window);
        draw_hp(&window, game_state->get_bases(), WindowData{game_state->get_center(),window_size});
        window.display();
        drawer.clear();

        // user phase
        UserInput user_input = input_collector.collect();
        SendData out_data = input_translator.translate(user_input);
        uint8_t* bytes = UdpOutputTranslator(*(GameIn*)out_data.data);
        socket->send(bytes, 9, sf::IpAddress(SERVER_ADDR), SERVER_PORT);
        delete (GameIn*)out_data.data;
        delete [] bytes;
    }

    return 0;
}

void resize(sf::Event* event, sf::Vector2f* window_size) {
    *window_size = sf::Vector2f(sf::Vector2u(event->size.width, event->size.height));
}

void set_view(sf::RenderWindow* window, sf::Vector2f window_size, sf::Vector2f center) {
    sf::Vector2f v = get_view_size(window_size);
    window->setView(sf::View(sf::FloatRect(center-v, 2.f*v)));
}
