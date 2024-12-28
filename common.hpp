#pragma once
#include <stdint.h>
#include <vector>

#define FPS 60

#define INNER_RADIUS 6
#define BASE_ZONE_RADIUS 2
#define BASE_RADIUS 1
#define OUTER_WIDTH 2
#define BARRIER_WIDTH 2
#define NOFLY_WIDTH 6
#define OUTER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH)
#define BARRIER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH)
#define TOTAL_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH+NOFLY_WIDTH)
#define GRID_SIZE (2*TOTAL_RADIUS)
#define BASE_DIST (INNER_RADIUS+BASE_ZONE_RADIUS)
// TODO - set appropriate
// #define SIGHT_LIMIT 5
#define SIGHT_LIMIT 32

#define PLAYERS_PER_TEAM 3
#define BULLETS_PER_PLAYER 5
#define ASTEROID_COUNT 45
#define TOTAL_ENTITIES (2*(PLAYERS_PER_TEAM*(1+BULLETS_PER_PLAYER))+ASTEROID_COUNT)
#define BLUE_TEAM_BEGIN 1
#define RED_TEAM_BEGIN (PLAYERS_PER_TEAM+1)
#define BLUE_BULLETS_BEGIN (1+2*PLAYERS_PER_TEAM)
#define RED_BULLETS_BEGIN (1+PLAYERS_PER_TEAM+PLAYERS_PER_TEAM*(1+BULLETS_PER_PLAYER))
#define ASTEROIDS_BEGIN (1+2*PLAYERS_PER_TEAM*(1+BULLETS_PER_PLAYER))

#define SHIP_SIZE 0.25
#define BULLET_SIZE 0.05
#define ASTEROID_SIZE 0.5

#define TYPE_SHIP 1
#define TYPE_BULLET 2
#define TYPE_ASTEROID 3

#define EPSILON 0.01

// Movables - TODO move to game engine

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
    double direction;
    void* player;
    bool side;
    Ship(uint16_t id, vec2 position, vec2 speed, double direction, void* player, bool side):
    Movable(id, position, speed),direction(direction),player(player),side(side){}
};

class Bullet : public Movable {
    public:
    double direction;
    uint16_t lifetime;
    bool side;
    Bullet(uint16_t id, vec2 position, vec2 speed, double direction, uint16_t lifetime, bool side):
    Movable(id, position, speed),direction(direction),lifetime(lifetime),side(side){}
};

// Space object (movable outside game engine) - TODO

struct SpaceObject {
    uint8_t id;
    double x;
    double y;
    double angle;
};

// Input

struct Input {
    double direction;
    bool shoot;
    bool engine_on;
};

// Output

struct PlayerData {
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint8_t ship_id;
};

struct Neighbours {
    uint8_t count;
    std::vector<Movable*>* movables;
};

struct Base {
    uint16_t hp;
};

struct Output {
    uint32_t timestamp;
    Base blue;
    Base red;
    PlayerData player_data;
    Neighbours neighbours;
};

// shared functions

inline int get_type(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return TYPE_SHIP;
    if(id >= BLUE_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return TYPE_BULLET;
    if(id >= ASTEROIDS_BEGIN && id <= TOTAL_ENTITIES) return TYPE_ASTEROID;
    return 0;
}
inline double get_size(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return SHIP_SIZE;
    if(id >= BLUE_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return BULLET_SIZE;
    if(id >= ASTEROIDS_BEGIN && id <= TOTAL_ENTITIES) return ASTEROID_SIZE;
    return 0.0;
}
inline bool get_side(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < RED_TEAM_BEGIN) return 0;
    if(id >= RED_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return 1;
    if(id >= BLUE_BULLETS_BEGIN && id < RED_BULLETS_BEGIN) return 0;
    if(id >= RED_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return 1;
    return 0;
}
