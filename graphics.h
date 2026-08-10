#pragma once
#include <array>
#include "types.h"
#include "bus.h"

class Graphics {

public:
	Bus& bus;

	Graphics(Bus& bus)
		: bus(bus) {
		LCDC = 0;
	}

	std::array<byte, 16> get_tile_data(byte index);
	tile decode_tile(const std::array<byte, 16>& tile_data);
	byte LCDC;
	word get_tile_base_pointer();
	word get_map_base_pointer();

	void create_background();


private:
	 tile_map t_map;
};
