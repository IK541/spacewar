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
    virtual void getMovables(std::vector<Movable*>* movables) = 0;
};

class Drawer {
    public:
    std::vector<Shape*> shapes;
    // NEW
    void add(Movable* movable);
    void addAll(DrawDataI* source);
    void clear();
    void draw(sf::RenderWindow* window);
    ~Drawer();
};

class GameState : public GameStateI, public DrawDataI {
    private:
    uint32_t timestamp;
    bool movables_present[TOTAL_ENTITIES];
    Movable movables[TOTAL_ENTITIES];
    std::mutex mtx;
    public:
    // NEW
    void getMovables(std::vector<Movable*>* movables);
    // CONSUMES movables
    void setMovables(uint32_t timestamp, std::vector<Movable*>* movables);
    bool is_game_running();
};

struct UdpRecvData {
    uint32_t timestamp;
    Base red;
    Base blue;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    // uint16_t last_id;
    uint16_t movables_count;
    std::vector<Movable*>* movables;
};

// NEW
// NULL on failure
UdpRecvData* UdpInputTranslator(uint8_t* data, int size);
