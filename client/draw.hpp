#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>
#include <cmath>

#include "../common.hpp"

#define HP_BAR_WIDTH 0.1

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

class GameStateI {
    public:
    virtual bool is_game_running() = 0;
};
class DrawDataI {
    public:
    virtual void get_space_objects(std::vector<SpaceObject*>* objects) = 0;
};

class Drawer {
    public:
    std::vector<Shape*> shapes;
    // NEW
    void add(SpaceObject* object);
    void add_all(DrawDataI* source);
    void clear();
    void draw(sf::RenderWindow* window);
    ~Drawer();
};

void draw_bases(sf::RenderWindow* window);
struct Bases { uint16_t blue; uint16_t red; };
struct WindowData { sf::Vector2f center; sf::Vector2f windows_size; };
void draw_hp(sf::RenderWindow* window, Bases bases, WindowData window_data);

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
    // NEW
    void get_space_objects(std::vector<SpaceObject*>* objects);
    sf::Vector2f get_center();
    Bases get_bases();
    // CONSUMES movables
    void set_game_state(GameOut game_out);
    bool is_game_running();
};

// NEW
// NULL on failure
GameOut UdpInputTranslator(uint8_t* data, int size);

inline sf::Vector2f get_view_size(sf::Vector2f window_size) {
    float vx = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.y)/(window_size.x*window_size.x));
    float vy = SIGHT_LIMIT/sqrtf(1+(window_size.x*window_size.x)/(window_size.y*window_size.y));
    return sf::Vector2f(vx,vy);
}
