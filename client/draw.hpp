#pragma once
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <mutex>

#include "../common.hpp"

class Shape {
    public:
    sf::Vector2f pos;
    float scale;
    float angle;
    sf::Color color;
    Shape(sf::Vector2f pos, float scale, float angle, sf::Color color);
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
    Triangle(sf::Vector2f pos, float scale, float angle, sf::Color color);
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
    void draw_bases(sf::RenderWindow* window);
    ~Drawer();
};

class GameState : public GameStateI, public DrawDataI {
    private:
    uint32_t timestamp;
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
    // CONSUMES movables
    void set_game_state(GameOut game_out);
    bool is_game_running();
};

// NEW
// NULL on failure
GameOut UdpInputTranslator(uint8_t* data, int size);
