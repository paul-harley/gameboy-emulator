#pragma once
#include <array>
#include "types.h"

class Graphics {

public:
	tile decode_tile(const std::array<byte, 16>& tile_data);


};
