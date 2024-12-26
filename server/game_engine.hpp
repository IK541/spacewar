#pragma once

#include <set>
#include <map>
#include <cstdint>

#include "../common.hpp"

#define GRID_SIZE (2*TOTAL_RADIUS)
#define SPAWN_RADIUS 0.5

// TODO: set appropriate
#define SHIP_SPEED 6.0
#define BULLET_SPEED 20.0
#define ASTEROID_SPEED 4.0

#define MAX_AMMO 20
#define BULLET_LIFETIME 50
#define RESPAWN_TIME (5*FPS)
#define REARM_TIME (FPS/10)
#define RELOAD_TIME (FPS/4) // RELOAD_TIME > BULLET_LIFETIME / BULLETS_PER_PLAYER

#define BASE_HP 200

// predeclared
class Player;
class Movables;
class Grid;

// Main classes

class Player {
    public:
    int id;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint16_t current_bullet;
    Ship* ship;
    Input last_input; // TODO: add timestamps
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
    Movable* items[TOTAL_ENTITIES]; // map? int => Movable*
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
    void set_player(int id, int player_id);
    void update_physics(double dt);
    void update_input(int player_id, Input input);
    Output get_output(int player_id);
    private:
    Player* get_player(int player_id);
};
