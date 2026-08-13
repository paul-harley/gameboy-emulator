#pragma once
#include <stdint.h>
#include <array>

typedef uint8_t byte;
typedef uint16_t word;


typedef int8_t sbyte;
typedef int16_t sword;

typedef std::array<std::array<byte, 8>, 8> tile;
typedef std::array<std::array<tile, 32>, 32> tile_map;

typedef std::tuple<byte, byte, byte, byte> colour;