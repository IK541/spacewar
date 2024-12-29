#pragma once
#include <stdint.h>
#include <vector>

#define FPS 60

#define INNER_RADIUS 6
#define BASE_ZONE_RADIUS 2
#define BASE_RADIUS 1
#define OUTER_WIDTH 4
#define BARRIER_WIDTH 2
#define NOFLY_WIDTH 8
#define OUTER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH)
#define BARRIER_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH)
#define TOTAL_RADIUS (INNER_RADIUS+2*BASE_ZONE_RADIUS+OUTER_WIDTH+BARRIER_WIDTH+NOFLY_WIDTH)
#define GRID_SIZE (2*TOTAL_RADIUS)
#define BASE_DIST (INNER_RADIUS+BASE_ZONE_RADIUS)
#define SIGHT_LIMIT 8

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

// Space object

struct SpaceObject {
    uint8_t id;
    double x;
    double y;
    double angle;
};

// Input

struct GameIn {
    uint32_t timestamp;
    float angle;
    bool shoot;
    bool engine_on;
};

// Output

struct GameOut {
    uint32_t timestamp;
    uint16_t blue_hp;
    uint16_t red_hp;
    uint16_t ammo;
    uint16_t reload;
    uint16_t rearm;
    uint16_t respawn;
    uint8_t ship_id;
    uint8_t movables_count;
    std::vector<SpaceObject*>* objects;
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

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }
