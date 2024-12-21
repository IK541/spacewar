#pragma once
#include <stdint.h>

#define INNER_RADIUS 6
#define BASE_ZONE_RADIUS 4
#define BASE_RADIUS 2
#define OUTER_WIDTH 2
#define BARRIER_WIDTH 2
#define NOFLY_WIDTH 6
#define OUTER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH)
#define BARRIER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH)
#define TOTAL_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH+NOFLY_WIDTH)
#define SIGHT_LIMIT 5
#define BASE_DIST (INNER_RADIUS+BASE_ZONE_RADIUS)

#define PLAYERS_PER_TEAM 3
#define BULLETS_PER_PLAYER 5
#define ASTEROID_COUNT 32
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

// Movables

struct vec2 {
    double x;
    double y;
};

class Movable {
    public:
    uint16_t id;
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
    uint16_t ship_id;
};

struct Neighbours {
    uint16_t count;
    Movable** movables;
};

struct Base {
    uint16_t hp;
};
