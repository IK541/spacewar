#pragma once
#include <cstring>
#include "../common.hpp"

// NEW
uint8_t* UdpOutputTranslator(GameOut output);

GameIn UdpInputTranslator(uint8_t* input);
