#include <cmath>
#include "room_manager.hpp"

// TODO: move to common
inline int get_type(uint16_t id) {
    if(id >= BLUE_TEAM_BEGIN && id < BLUE_BULLETS_BEGIN) return TYPE_SHIP;
    if(id >= BLUE_BULLETS_BEGIN && id < ASTEROIDS_BEGIN) return TYPE_BULLET;
    if(id >= ASTEROIDS_BEGIN && id <= TOTAL_ENTITIES) return TYPE_ASTEROID;
    return 0;
}

uint8_t* UdpOutputTranslator(Output output) {
    int size = sizeof(output.timestamp) + sizeof(output.blue) + sizeof(output.red) + sizeof(output.player_data)\
    + sizeof(output.neighbours.count) + (output.neighbours.count) * sizeof(CompressedMovable);
    uint8_t* bytes = new uint8_t[size];
    int pos = 0;
    memcpy(bytes+pos, &output.timestamp, sizeof(output.timestamp)); pos += sizeof(output.timestamp);
    memcpy(bytes+pos, &output.blue.hp, sizeof(output.blue.hp)); pos += sizeof(output.blue.hp);
    memcpy(bytes+pos, &output.red.hp, sizeof(output.red.hp)); pos += sizeof(output.red.hp);
    memcpy(bytes+pos, &output.player_data.ammo, sizeof(output.player_data.ammo)); pos += sizeof(output.player_data.ammo);
    memcpy(bytes+pos, &output.player_data.reload, sizeof(output.player_data.reload)); pos += sizeof(output.player_data.reload);
    memcpy(bytes+pos, &output.player_data.rearm, sizeof(output.player_data.rearm)); pos += sizeof(output.player_data.rearm);
    memcpy(bytes+pos, &output.player_data.respawn, sizeof(output.player_data.respawn)); pos += sizeof(output.player_data.respawn);
    memcpy(bytes+pos, &output.player_data.ship_id, sizeof(output.player_data.ship_id)); pos += sizeof(output.player_data.ship_id);
    memcpy(bytes+pos, &output.neighbours.count, sizeof(output.neighbours.count)); pos += sizeof(output.neighbours.count);
    for(Movable* movable: *output.neighbours.movables) {
        uint8_t type = get_type(movable->id);
        double direction = type == TYPE_SHIP ? ((Ship*)movable)->direction : type == TYPE_BULLET ? ((Bullet*)movable)->direction : 0.0;
        CompressedMovable compressed_movable = CompressedMovable{
            .id = movable->id,
            .x = (uint16_t)((movable->position.x + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            .y = (uint16_t)((movable->position.y + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            .direction = (uint8_t)(fmod(direction+2*M_PI,2*M_PI)/(2*M_PI)*UINT8_MAX)
        };
        memcpy(bytes+pos, &compressed_movable, sizeof(CompressedMovable));
        pos += sizeof(CompressedMovable);
    }
    // TODO: delete output
    return bytes;
}

Input UdpInputTranslator(uint8_t* input) {
    // TODO: timestamp
    return Input {
        .direction = *(float*)(input+4),
        .shoot = (bool)(*(input+8) & 0x02),
        .engine_on = (bool)(*(input+8) & 0x01)
    };
}
