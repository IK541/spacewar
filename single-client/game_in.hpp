#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>

#include "common.hpp"
#include "../common.hpp"

#define HP_BAR_WIDTH 0.1
#define AMMO_RADIUS 0.1
#define RESPAWN_RADIUS 1
#define RESPAWN_WIDTH 0.2

struct DrawGameData {
    std::vector<SpaceObject*>* objects;
    uint16_t blue;
    uint16_t red;
    uint16_t ammo;
    uint16_t respawn;
};
class DrawDataI {
    public:
    virtual DrawGameData get_game_state() = 0;
};
struct WindowData {
    sf::RenderWindow* window;
    sf::Vector2f size;
    sf::Vector2f center;
};

class GameDrawer {
    public:
    std::vector<Shape*> shapes;
    uint16_t blue;
    uint16_t red;
    uint16_t ammo;
    uint16_t respawn;
    // NEW
    void add_all(DrawDataI* source);
    // DELETE
    void clear();
    void draw(WindowData window);
    ~GameDrawer();
    private:
    // NEW
    void add(SpaceObject* object);
    void draw_objects(WindowData window);
    void draw_zones(WindowData window);
    void draw_bases(WindowData window);
    void draw_hp(WindowData window);
    void draw_ammo(WindowData window);
    void draw_respawn(WindowData window);
};

class GameState : public DrawDataI {
    private:
    uint32_t timestamp;
    uint16_t blue_hp;
    uint16_t red_hp;
    uint16_t ammo;
    uint16_t respawn;
    sf::Vector2f center;
    bool objects_present[TOTAL_ENTITIES];
    SpaceObject objects[TOTAL_ENTITIES];
    std::mutex mtx;
    public:
    GameState();
    void set_game_state(GameOut game_out);
    // NEW
    DrawGameData get_game_state();
    sf::Vector2f get_center();
    private:
    // NEW
    void get_space_objects(std::vector<SpaceObject*>* objects);
};

// NEW
GameOut UdpInputTranslator(uint8_t* data, int size);

inline sf::Vector2f get_view_size(sf::Vector2f window_size) {
    float vx = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.y)/(window_size.x*window_size.x));
    float vy = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.x)/(window_size.y*window_size.y));
    return sf::Vector2f(vx,vy);
}
