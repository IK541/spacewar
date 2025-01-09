#include <cmath>
#include "room_manager.hpp"

#pragma pack(push, 1)
struct CompressedMovable {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t angle;
};
#pragma pack(pop)

uint8_t* UdpOutputTranslator(GameOut output) {
    int size = 18 + (output.movables_count) * sizeof(CompressedMovable);
    uint8_t* bytes = new uint8_t[size];
    int pos = 0;
    memcpy(bytes+pos, &output.timestamp, sizeof(output.timestamp)); pos += sizeof(output.timestamp);
    memcpy(bytes+pos, &output.blue_hp, sizeof(output.blue_hp)); pos += sizeof(output.blue_hp);
    memcpy(bytes+pos, &output.red_hp, sizeof(output.red_hp)); pos += sizeof(output.red_hp);
    memcpy(bytes+pos, &output.ammo, sizeof(output.ammo)); pos += sizeof(output.ammo);
    memcpy(bytes+pos, &output.reload, sizeof(output.reload)); pos += sizeof(output.reload);
    memcpy(bytes+pos, &output.rearm, sizeof(output.rearm)); pos += sizeof(output.rearm);
    memcpy(bytes+pos, &output.respawn, sizeof(output.respawn)); pos += sizeof(output.respawn);
    memcpy(bytes+pos, &output.ship_id, sizeof(output.ship_id)); pos += sizeof(output.ship_id);
    memcpy(bytes+pos, &output.movables_count, sizeof(output.movables_count)); pos += sizeof(output.movables_count);
    for(SpaceObject* movable: *output.objects) {
        double angle = movable->angle;
        CompressedMovable compressed_movable = CompressedMovable{
            movable->id,
            (uint16_t)((movable->x + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            (uint16_t)((movable->y + TOTAL_RADIUS) / GRID_SIZE * UINT16_MAX),
            (uint8_t)(fmod(angle+2*M_PI,2*M_PI)/(2*M_PI)*UINT8_MAX)
        };
        memcpy(bytes+pos, &compressed_movable, sizeof(CompressedMovable));
        pos += 6;
    }
    return bytes;
}

GameIn UdpInputTranslator(uint8_t* input) {
    return GameIn {
        *(uint32_t*)(input),
        *(float*)(input+4),
        (bool)(*(input+8) & 0x02),
        (bool)(*(input+8) & 0x01)
    };
}
