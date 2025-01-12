#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <string>

#include "game_in.hpp"
#include "game_out.hpp"
#include "lobby.hpp"
#include "../resources/font.h"

#define WINDOW_SIZE 800

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 9001
#define BIND_ATTEMPTS 20

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

int port;

// functions
void resize(sf::Event* event, sf::Vector2f* window_size);
void set_view(sf::RenderWindow* window, sf::Vector2f window_size, sf::Vector2f center);
void udp_receiver();
void tcp_receiver();
void handle_events();

int main(int argc, char** argv) {
    if(argc < 3) {
        printf("address & port required\n");
        exit(-1);
    }

    // Window
    window.setFramerateLimit(FPS);

    // Drawing
    GameDrawer drawer;

    sf::Font font;
    if (!font.loadFromMemory(resources_font_ttf, resources_font_ttf_len)) {
        printf("font error\n"); return -1;
    }

    // Network
    bool bind_success = false;
    for(int i = 0; i < BIND_ATTEMPTS; ++i) {
        if(!udp_socket.bind(CLIENT_PORT + i)) {
            bind_success = true;
            break;
        }
    } if(!bind_success) {
        printf("bind error\n"); return -1;
    } port = udp_socket.getLocalPort();
    std::thread udp_recv_thread(udp_receiver);
    udp_recv_thread.detach();
    if(tcp_socket.connect(sf::IpAddress(argv[1]), std::stoi(argv[2]))) {
        printf("connection error\n"); return -1;
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
    lobby_state.room_selected = 0;
    lobby_state.rooms = std::vector<RoomInfo>();
    room_state.blue = std::vector<PlayerInfo>();
    room_state.red = std::vector<PlayerInfo>();

    while (window.isOpen()) {

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
        udp_socket.send(bytes, 9, sf::IpAddress(SERVER_ADDR), SERVER_PORT+room_state.id+1);
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
                std::string out = std::string("A ");
                out.append(name_state.name);
                out.push_back(' ');
                out.append(std::to_string(port));
                out.push_back('\n');
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_LOBBY && event.key.code == 73) lobby_state.room_selected = (lobby_state.room_selected - 1 + ROOM_COUNT) % ROOM_COUNT;
            if(state == STATE_LOBBY && event.key.code == 74) lobby_state.room_selected = (lobby_state.room_selected + 1) % ROOM_COUNT;
            if(state == STATE_LOBBY && event.key.code == 58) {
                std::string out = std::string("D ");
                out.append(std::to_string(lobby_state.room_selected));
                out.push_back('\n');
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 57) {
                std::string out = std::string("E\n");
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 58) {
                std::string out = std::string("F\n");
                tcp_socket.send(out.c_str(), out.size());
            }
            if(state == STATE_ROOM && event.key.code == 59) {
                std::string out = std::string("D -1\n");
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
    std::string item;

    char opcode = '-';    
    while(1) {
        tcp_socket.receive(buffer, BUFFER_SIZE, size);
        for(unsigned int i = 0; i < size; ++i) {
            if(buffer[i] != '\n') {
                data.push(buffer[i]);
                continue;
            } if(!data.size()) continue;
            try {
            opcode = data.front(); data.pop();
            std::lock_guard<std::mutex> lock(mtx);
            if(opcode == 'N') {
                name_state.failed = true;
            }
            if(opcode == 'G') {
                for(PlayerInfo player: room_state.blue) player.ready = false;
                for(PlayerInfo player: room_state.red) player.ready = false;
                game_state.reset();
                state = STATE_GAME;
            }
            if(opcode == 'H') {
                state = STATE_ROOM;
            }
            if(opcode == 'I') {
                state = STATE_BLUE;
            }
            if(opcode == 'J') {
                state = STATE_RED;
            }
            if(opcode == 'L') {
                data.pop();
                std::vector<int> vals;
                std::vector<RoomInfo> new_info;
                while(data.size()) {
                    if(data.front() == ' ') {
                        vals.push_back(stoi(item));
                        item.clear();
                    } else item.push_back(data.front());
                    data.pop();
                } vals.push_back(stoi(item));
                if(vals.size() != 4 * ROOM_COUNT) goto end;
                for(int i = 0; i < ROOM_COUNT; ++i) {
                    if(vals[4*i] < 0 || vals[4*i] >= ROOM_COUNT) goto end;
                    if(vals[4*i+1] < 0 || vals[4*i+1] >= 2) goto end;
                    if(vals[4*i+2] < 0 || vals[4*i+2] >= PLAYERS_PER_TEAM) goto end;
                    if(vals[4*i+3] < 0 || vals[4*i+3] >= PLAYERS_PER_TEAM) goto end;
                    new_info.push_back(RoomInfo{(uint8_t)vals[4*i],(bool)vals[4*i+1],(uint8_t)vals[4*i+2],(uint8_t)vals[4*i+3]});
                } lobby_state.rooms = new_info;
                state = STATE_LOBBY;
            }
            if(opcode == 'R') {
                data.pop();
                std::vector<std::string> vals;
                RoomState new_state;
                while(data.size()) {
                    if(data.front() == ' ') {
                        vals.push_back(item);
                        item.clear();
                    } else item.push_back(data.front());
                    data.pop();
                } vals.push_back(item);
                for(std::string val: vals) printf("%s\n", val.c_str());
                if(vals.size() < 4) goto end;
                uint8_t room_id = (uint8_t)stoi(vals[0]);
                uint8_t blue = (uint8_t)stoi(vals[2]);
                uint8_t red = (uint8_t)stoi(vals[3]);
                printf("blue: %d\n", blue & 0xff);
                printf("red: %d\n", red & 0xff);
                if(vals.size() != 4U + 2U * (blue + red)) goto end;
                room_state.blue.clear(); room_state.red.clear();
                for(int i = 4; i < 4 + 2 * blue; i += 2) {
                    room_state.blue.push_back(PlayerInfo{(bool)stoi(vals[i]),vals[i+1]});
                }
                for(int i = 4 + 2 * blue; i < 4 + 2 * (blue + red); i += 2) {
                    room_state.red.push_back(PlayerInfo{(bool)stoi(vals[i]),vals[i+1]});
                } room_state.id = room_id; state = STATE_ROOM;
            }
            } catch(...) { printf("recv exception\n"); }
            end:
            item.clear();
            data = {};
        }
    }
}
