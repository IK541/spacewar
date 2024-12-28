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
    void addAll(DrawDataI* source);
    void clear();
    void draw(sf::RenderWindow* window);
    ~Drawer();
};

class GameState : public GameStateI, public DrawDataI {
    private:
    uint32_t timestamp=0;
    bool objects_present[TOTAL_ENTITIES]{};
    SpaceObject objects[TOTAL_ENTITIES]{};
    std::mutex mtx;
    public:
    // NEW
    void get_space_objects(std::vector<SpaceObject*>* objects);
    // CONSUMES movables
    void set_space_objects(uint32_t timestamp, std::vector<SpaceObject*>* objects);
    bool is_game_running();
};

struct UdpRecvData {
    uint32_t timestamp;
    uint16_t blue_hp;
    uint16_t red_hp;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint8_t ship_id;
    uint16_t movables_count;
    std::vector<SpaceObject*>* objects;
};

// NEW
// NULL on failure
UdpRecvData* UdpInputTranslator(uint8_t* data, int size);
