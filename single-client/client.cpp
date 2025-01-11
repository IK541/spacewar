#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <mutex>

#include "game_in.hpp"
#include "game_out.hpp"
#include "lobby.hpp"

#define WINDOW_SIZE 800

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 10000
#define CLIENT_PORT 9001

#define TEST_SIZE 1.f

#define SHIP_ID 1

#define BUFFER_SIZE 4096

#define STATE_NAME 0
#define STATE_LOBBY 1
#define STATE_ROOM 2
#define STATE_GAME 3

// globals
sf::Vector2f window_size = sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE);
sf::RenderWindow window(sf::VideoMode((unsigned)window_size.x, (unsigned)window_size.y), "Spacewar");

int state;
NameState name_state;
LobbyState lobby_state;
RoomState room_state;
GameState game_state; // TODO: add reset, TODO: move to mutex proteted class with lobby State

sf::TcpSocket tcp_socket;
sf::UdpSocket udp_socket;

// functions

void resize(sf::Event* event, sf::Vector2f* window_size);
void set_view(sf::RenderWindow* window, sf::Vector2f window_size, sf::Vector2f center);

void udp_receiver() {
    std::size_t size = 0;
    uint8_t buffer[BUFFER_SIZE];
    while(1) {
        sf::IpAddress addr; unsigned short port;
        udp_socket.receive(buffer, BUFFER_SIZE, size, addr, port);
        GameOut data = UdpInputTranslator((uint8_t*)buffer, size);
        game_state.set_game_state(data);
        delete_GameOut(&data);
    }
}

void tcp_receiver() {
    std::size_t size = 0;
    uint8_t buffer[BUFFER_SIZE];
    while(1) {
        tcp_socket.receive(buffer, BUFFER_SIZE, size);
        // TODO: handle tcp data
    }
}

void handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        if (event.type == sf::Event::Resized) resize(&event, &window_size);
        if (event.type == sf::Event::KeyPressed && state != STATE_GAME) {
            // TODO: handle keyboard
        }
    }
}

int main() {
    // Window
    window.setFramerateLimit(FPS);

    // Drawing
    GameDrawer drawer;

    // Font - TODO load from memory
    sf::Font font;
    if (!font.loadFromFile("./target/font.ttf")) {
        printf("font error"); return -1;
    }

    // Network
    udp_socket.bind(CLIENT_PORT);
    std::thread recv_thread(udp_receiver);
    recv_thread.detach();

    // input
    InputCollector input_collector(&window);
    InputTranslator input_translator(&window);

    // State init
    state = STATE_GAME;
    name_state.name = std::string("BEG-TEST-END");
    lobby_state.room_selected = 1;
    lobby_state.rooms = std::vector<RoomInfo>();
    lobby_state.rooms.push_back(RoomInfo{0,0,2,2});
    lobby_state.rooms.push_back(RoomInfo{1,0,1,0});
    lobby_state.rooms.push_back(RoomInfo{2,1,3,1});
    room_state.blue = std::vector<PlayerInfo>();
    room_state.blue.push_back(PlayerInfo{false,std::string("IK")});
    room_state.blue.push_back(PlayerInfo{true,std::string("MP")});
    room_state.red.push_back(PlayerInfo{true,std::string("JK")});

    while (window.isOpen()) {

        // init phase
        handle_events();

        switch (state) {
        
        // IN NAME:
        case STATE_NAME:
        draw_name(name_state, &window, &font);
        // TODO: user input
        break;

        // IN LOBBY:
        case STATE_LOBBY:
        draw_lobby(lobby_state, &window);
        // TODO: user input
        break;

        // IN ROOM:
        case STATE_ROOM:
        draw_room(room_state, &window, &font);
        // TODO: user input
        break;

        // GAME RUNNING
        case STATE_GAME:

        // draw phase
        WindowData window_data = WindowData{&window,window_size,game_state.get_center()};
        set_view(&window, window_size, game_state.get_center());
        drawer.add_all(&game_state);
        window.clear();
        drawer.draw(window_data);
        window.display();
        drawer.clear();

        // user phase
        UserInput user_input = input_collector.collect();
        GameIn out_data = input_translator.translate(user_input);
        uint8_t* bytes = UdpOutputTranslator(out_data);
        udp_socket.send(bytes, 9, sf::IpAddress(SERVER_ADDR), SERVER_PORT);
        delete [] bytes;
        break;
        
        }
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
