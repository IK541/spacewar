#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

#include "game_in.hpp"
#include "game_out.hpp"
#include "lobby.hpp"
#include "../resources/font.h"

#define WINDOW_SIZE 800

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 9001

#define TEST_SIZE 1.f

#define SHIP_ID 1

#define BUFFER_SIZE 4096

#define STATE_NAME 0
#define STATE_LOBBY 1
#define STATE_ROOM 2
#define STATE_GAME 3
#define STATE_BLUE 4
#define STATE_RED 5

#define MAX_NICK_LENGTH 12

// globals
sf::Vector2f window_size = sf::Vector2f(WINDOW_SIZE, WINDOW_SIZE);
sf::RenderWindow window(sf::VideoMode((unsigned)window_size.x, (unsigned)window_size.y), "Spacewar");

std::mutex mtx;
int state;
NameState name_state;
LobbyState lobby_state;
RoomState room_state;
GameState game_state;

sf::TcpSocket tcp_socket;
sf::UdpSocket udp_socket;

// functions
void resize(sf::Event* event, sf::Vector2f* window_size);
void set_view(sf::RenderWindow* window, sf::Vector2f window_size, sf::Vector2f center);
void udp_receiver();
void tcp_receiver();
void handle_events();

int main() {
    // Window
    window.setFramerateLimit(FPS);

    // Drawing
    GameDrawer drawer;

    sf::Font font;
    if (!font.loadFromMemory(resources_font_ttf, resources_font_ttf_len)) {
        printf("font error"); return -1;
    }

    // Network
    if(udp_socket.bind(CLIENT_PORT)) {
        printf("bind error"); return -1;
    }
    std::thread udp_recv_thread(udp_receiver);
    udp_recv_thread.detach();
    if(tcp_socket.connect(sf::IpAddress(SERVER_ADDR), SERVER_PORT)) {
        printf("connection error"); return -1;
    }
    std::thread tcp_recv_thread(tcp_receiver);
    tcp_recv_thread.detach();

    // input
    InputCollector input_collector(&window);
    InputTranslator input_translator(&window);

    // State init
    state = STATE_NAME;
    name_state.name = std::string();
    name_state.failed = false;
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
        printf("%d\n", state);

        // init phase
        handle_events();

        int current_state; {
            std::lock_guard<std::mutex> lock(mtx);
            current_state = state;
        } switch (current_state) {
        
        // IN NAME:
        case STATE_NAME:
        draw_name(name_state, &window, &font, &mtx);
        break;

        // IN LOBBY:
        case STATE_LOBBY:
        draw_lobby(lobby_state, &window, &mtx);
        break;

        // IN ROOM:
        case STATE_ROOM:
        draw_room(room_state, &window, &font, &mtx);
        break;

        case STATE_BLUE:
        draw_blue(&window, &font);
        break;

        case STATE_RED:
        draw_red(&window, &font);
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

void handle_events() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        if (event.type == sf::Event::Resized) resize(&event, &window_size);
        if (event.type == sf::Event::KeyPressed && state != STATE_GAME) {
            std::lock_guard<std::mutex> lock(mtx);
            if(state == STATE_NAME && name_state.name.size() < MAX_NICK_LENGTH) {
                if(event.key.code >= 0 && event.key.code < 26) name_state.name.push_back('A' + event.key.code);
                if(event.key.code >= 26 && event.key.code < 36) name_state.name.push_back('0' + event.key.code - 26);
                if(event.key.code == 56) name_state.name.push_back('-');
            }
            if(state == STATE_NAME && event.key.code == 59 && name_state.name.size()) name_state.name.pop_back();
            if(state == STATE_NAME && event.key.code == 58) {
                std::string out = std::string("A");
                // out.push_back(name_state.name.size()); // msg len
                out.append(std::to_string(name_state.name.size())); // msg len
                out.append(name_state.name);
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_LOBBY && event.key.code == 73) lobby_state.room_selected = (lobby_state.room_selected - 1 + ROOM_COUNT) % ROOM_COUNT;
            if(state == STATE_LOBBY && event.key.code == 74) lobby_state.room_selected = (lobby_state.room_selected + 1) % ROOM_COUNT;
            if(state == STATE_LOBBY && event.key.code == 58) {
                std::string out = std::string("D");
                out.push_back(lobby_state.room_selected);
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 57) {
                std::string out = std::string("E");
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 58) {
                std::string out = std::string("F");
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 59) {
                std::string out = std::string("D");
                out.push_back('X');
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_BLUE || state == STATE_RED) {
                state = STATE_ROOM;
            }
        }
    }
}

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
    std::queue<char> data;

    char opcode = '-';
    unsigned int bytes_expected = 0;
    
    while(1) {
        tcp_socket.receive(buffer, BUFFER_SIZE, size);
        for(unsigned int i = 0; i < size; ++i) data.push(buffer[i]);
        while(data.size()) {
            if(opcode == '-' && data.size()) {
                opcode = data.front();
                data.pop();
                if(opcode == 'L') bytes_expected = 4*ROOM_COUNT;
            }
            if(opcode == 'R') { if(data.size()) {
                bytes_expected = (int) data.front();
                opcode = '>';
                data.pop();
            } else break; }
            std::lock_guard<std::mutex> lock(mtx);
            if(opcode == 'N') {
                name_state.failed = true;
                opcode = '-';
            }
            if(opcode == 'G') {
                for(PlayerInfo player: room_state.blue) player.ready = false;
                for(PlayerInfo player: room_state.red) player.ready = false;
                game_state.reset();
                state = STATE_GAME;
                opcode = '-';
            }
            if(opcode == 'H') {
                state = STATE_ROOM;
                opcode = '-';
            }
            if(opcode == 'I') {
                state = STATE_BLUE;
                opcode = '-';
            }
            if(opcode == 'J') {
                state = STATE_RED;
                opcode = '-';
            }
            if(opcode == 'L') { if (data.size() >= bytes_expected) {
                state = STATE_LOBBY;
                for(int i = 0; i < ROOM_COUNT; ++i) {
                    lobby_state.rooms[i].id = data.front() - '0'; data.pop();
                    lobby_state.rooms[i].is_game_running = data.front() - '0'; data.pop();
                    lobby_state.rooms[i].blue_count = data.front() - '0'; data.pop();
                    lobby_state.rooms[i].red_count = data.front() - '0'; data.pop();
                }
                opcode = '-';
                bytes_expected = 0;
            } else break; }
            if(opcode == '>') { if(data.size() >= bytes_expected) {
                room_state.blue.clear(); room_state.red.clear();
                data.pop(); data.pop();
                int blue = data.front() - '0'; data.pop();
                int red = data.front() - '0'; data.pop();
                for(int i = 0; i < blue; ++i) {
                    bool ready = data.front(); data.pop();
                    int len = data.front() - '0'; data.pop();
                    len = 10 * len + data.front() - '0'; data.pop();
                    std::string name;
                    for(int j = 0; j < len; ++j) {
                        name.push_back(data.front());
                        data.pop();
                    }
                    room_state.blue.push_back(PlayerInfo{ready,name});
                }
                for(int i = 0; i < red; ++i) {
                    bool ready = data.front(); data.pop();
                    int len = data.front() - '0'; data.pop();
                    len = 10 * len + data.front() - '0'; data.pop();
                    std::string name;
                    for(int j = 0; j < len; ++j) {
                        name.push_back(data.front());
                        data.pop();
                    }
                    room_state.red.push_back(PlayerInfo{ready,name});
                }
                opcode = '-';
                bytes_expected = 0;
            } else break; }
        }
    }
}
