#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>

#include "../common.hpp"

#define HP_BAR_WIDTH 0.1
#define AMMO_RADIUS 0.1
#define RESPAWN_RADIUS 1
#define RESPAWN_WIDTH 0.2

class Shape {
    public:
    sf::Vector2f pos;
    float scale;
    sf::Color color;
    Shape(sf::Vector2f pos, float scale, sf::Color color);
    virtual ~Shape(){}
    virtual void draw(sf::RenderWindow* window) = 0;
};

class Circle : public Shape {
    public:
    Circle(sf::Vector2f pos, float scale, sf::Color color);
    void draw(sf::RenderWindow* window);
};

class Triangle : public Shape {
    public:
    float angle;
    Triangle(sf::Vector2f pos, float scale, sf::Color color, float angle);
    void draw(sf::RenderWindow* window);
};

struct DrawData {
    std::vector<SpaceObject*>* objects;
    uint16_t blue;
    uint16_t red;
    uint16_t ammo;
    uint16_t respawn;
};
class DrawDataI {
    public:
    // virtual void get_space_objects(std::vector<SpaceObject*>* objects) = 0;
    virtual DrawData get_game_state() = 0;
};
struct WindowData {
    sf::RenderWindow* window;
    sf::Vector2f size;
    sf::Vector2f center;
};

class Drawer {
    public:
    std::vector<Shape*> shapes;
    uint16_t blue;
    uint16_t red;
    uint16_t ammo;
    uint16_t respawn;
    // NEW
    void add(SpaceObject* object);
    void add_all(DrawDataI* source);
    void clear();
    void draw(WindowData window);
    ~Drawer();
    private:
    void draw_objects(WindowData window);
    void draw_zones(WindowData window);
    void draw_bases(WindowData window);
    void draw_hp(WindowData window);
    void draw_ammo(WindowData window);
    void draw_respawn(WindowData window);
};

class GameState : public GameStateI, public DrawDataI {
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
    // CONSUMES movables
    void set_game_state(GameOut game_out);
    DrawData get_game_state();
    sf::Vector2f get_center();
    bool is_game_running();
    private:
    // NEW
    void get_space_objects(std::vector<SpaceObject*>* objects);
};

// NEW
// NULL on failure
GameOut UdpInputTranslator(uint8_t* data, int size);

inline sf::Vector2f get_view_size(sf::Vector2f window_size) {
    float vx = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.y)/(window_size.x*window_size.x));
    float vy = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.x)/(window_size.y*window_size.y));
    return sf::Vector2f(vx,vy);
}
