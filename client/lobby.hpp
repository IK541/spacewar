#include <vector>
#include <string>
#include "common.hpp"
#include "../common.hpp"
#include "game_in.hpp"

struct NameState {
    bool failed;
    std::string name;
};

struct RoomInfo {
    uint8_t id;
    bool is_game_running;
    uint8_t blue_count;
    uint8_t red_count;
};

struct PlayerInfo {
    bool ready;
    std::string name;
};

struct LobbyState {
    uint8_t room_selected;
    std::vector<RoomInfo> rooms;
};

struct RoomState {
    uint8_t id;
    std::vector<PlayerInfo> blue;
    std::vector<PlayerInfo> red;
};

class ClientState {
    public:
    int state;
    NameState name_state;
    LobbyState lobby_state;
    RoomState room_state;
    GameState game_state;
    ClientState();
};

void draw_name(NameState name_state, sf::RenderWindow* window, sf::Font* font, std::mutex* mtx);
void draw_lobby(LobbyState lobby_state, sf::RenderWindow* window, std::mutex* mtx);
void draw_room(RoomState room_state, sf::RenderWindow* window, sf::Font* font, std::mutex* mtx);
void draw_blue(sf::RenderWindow* window, sf::Font* font);
void draw_red(sf::RenderWindow* window, sf::Font* font);

