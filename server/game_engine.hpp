#pragma once
#include <set>
#include <map>
#include <cstdint>

#include "../common.hpp"

#define SPAWN_RADIUS 0.5

// TODO: set appropriate
#define SHIP_SPEED 5.0
#define BULLET_SPEED 10.0
#define ASTEROID_SPEED 1.0

#define SHIP_ROTATION 0.1
#define SHIP_ACCELERATION 0.1

// predeclared
class Player;
class Movables;
class Grid;

// Movables

struct vec2 {
    double x;
    double y;
};

class Movable {
    public:
    uint8_t id;
    vec2 position;
    vec2 speed;
    Movable(uint16_t id, vec2 position, vec2 speed):
    id(id),position(position),speed(speed){}
    Movable(){};
};

class Asteroid : public Movable {
    public:
    Asteroid(uint16_t id, vec2 position, vec2 speed):
    Movable(id, position, speed){}
};

class Ship : public Movable {
    public:
    double angle;
    Player* player;
    bool side;
    Ship(uint16_t id, vec2 position, vec2 speed, double angle, Player* player, bool side):
    Movable(id, position, speed),angle(angle),player(player),side(side){}
};

class Bullet : public Movable {
    public:
    double angle;
    uint16_t lifetime;
    bool side;
    Bullet(uint16_t id, vec2 position, vec2 speed, double angle, uint16_t lifetime, bool side):
    Movable(id, position, speed),angle(angle),lifetime(lifetime),side(side){}
};

// output

struct PlayerData {
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint8_t ship_id;
};

struct Neighbours {
    uint8_t count;
    std::vector<SpaceObject*>* movables;
};

// base

struct Base { uint16_t hp; };

// Main classes

class Player {
    public:
    int id;
    Ship* ship;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint16_t current_bullet;
    GameIn last_input;
    Player();
    Player(int player_id, Ship* ship);
    void update_ship();
    void shoot(Movables* movables);
    // (make new?)
    PlayerData generate_player_data();
};

class Grid {
    public:
    std::set<Movable*> fields[GRID_SIZE*GRID_SIZE];
    void update_state(Movables* movables);
    void update_collisions(Movables* movables);
    void update_zone(Movables* movables, vec2 where);
    void update_base(Base* base, vec2 where, bool side);
    // NEW
    Neighbours getNeighbours(Player* player);
};

class Movables {
    public:
    Movable* items[TOTAL_ENTITIES];
    unsigned int seed;
    Movables(unsigned int seed);
    void move(double dt);
    void add_asteroid(uint16_t id);
    void shoot(Player* player);
    void update_asteroids();
    void update_bullets();
    void respawn(Ship* ship);
    void pull_ships(double dt);
    // TODO: add destructor (DELETE)
};

// Game Engine

struct Team {
    Player players[PLAYERS_PER_TEAM];
    Base base;
    uint8_t size;
};

class GameEngine {
    public:
    int timestamp;
    Team blue;
    Team red;
    Movables movables;
    Grid grid;
    GameEngine();
    void update_physics(double dt);
    void update_input(int ship_id, GameIn input);
    GameOut get_output(int ship_id);
    private:
    Player* get_player(int ship_id);
};
