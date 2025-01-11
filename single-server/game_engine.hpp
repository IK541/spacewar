#pragma once
#include <set>
#include <map>
#include <cstdint>

#include "../common.hpp"

// #ifndef Gamespace_NAMESPACE_H
// #define Gamespace_NAMESPACE_H

// namespace Gamespace {


#define SPAWN_RADIUS 0.5

#define SHIP_SPEED 5.0
#define BULLET_SPEED 10.0
#define ASTEROID_SPEED 1.0

#define SHIP_ROTATION 0.1
#define SHIP_ACCELERATION 0.1

#define NO_WIN 0
#define BLUE_WIN 2
#define DRAW 1
#define RED_WIN 3

// predeclared
class PlayerGE;
class Movables;
class Collider;

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
    PlayerGE* player;
    bool side;
    Ship(uint16_t id, vec2 position, vec2 speed, double angle, PlayerGE* player, bool side):
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

class PlayerGE {
    public:
    int id;
    Ship* ship;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint16_t current_bullet;
    GameIn last_input;
    PlayerGE();
    PlayerGE(int player_id, Ship* ship);
    void update_ship();
    void shoot(Movables* movables);
    PlayerData generate_player_data();
};

class Collider {
    public:
    void update_collisions(Movables* movables);
    void update_base(Movables* movables, Base* base, vec2 where, bool side);
    // NEW
    Neighbours get_neighbours(Movables* movables, PlayerGE* player);
};

class Movables {
    public:
    Movable* items[TOTAL_ENTITIES];
    unsigned int seed;
    // NEW
    Movables(unsigned int seed);
    void move(double dt);
    void add_asteroid(uint16_t id);
    void shoot(PlayerGE* player);
    void update_asteroids();
    void update_bullets();
    void respawn(Ship* ship);
    void pull_ships(double dt);
    // DELETE
    ~Movables();
};

// Game Engine

struct Team {
    PlayerGE players[PLAYERS_PER_TEAM];
    Base base;
    uint8_t size;
};

class GameEngine {
    public:
    GameEngine();
    int update_physics(double dt); // returns winner
    void update_input(int ship_id, GameIn input);
    GameOut get_output(int ship_id);
    void set_ship(int ship_id);
    void unset_ship(int ship_id);
    private:
    int timestamp;
    Team blue;
    Team red;
    Movables movables;
    Collider grid;
    PlayerGE* get_player(int ship_id);
    int get_winner();
};

#define OUTPUT_HEAD_SIZE 18
// }
// #endif // Gamespace_NAMESPACE_H