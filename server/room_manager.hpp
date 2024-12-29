#pragma once
#include <cstring>
#include "../common.hpp"

#pragma pack(push, 1)
struct CompressedMovable {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t direction;
};
#pragma pack(pop)

// NEW
uint8_t* UdpOutputTranslator(GameOut output);

GameIn UdpInputTranslator(uint8_t* input);
