#include "graphics.h"

std::array<byte, 16> Graphics::get_tile_data(byte index) {
	std::array<byte, 16> tile_data{};

	word tile_bp = get_tile_base_pointer();
	word tile_address;
	
	if (tile_bp == 0x8000) {
		tile_address = tile_bp + (index * 16);
	}
	else {
		sbyte signed_index = static_cast<sbyte>(index);
		tile_address = tile_bp + (signed_index * 16);
	}


	for (byte offset = 0; offset < 16; offset++) {
		tile_data[offset] = bus.read_memory(tile_address + offset);
	}

	return tile_data;
}

std::array<std::array<byte, 8>, 8> Graphics::decode_tile(const std::array<byte, 16>& tile_data) {

	std::array<std::array<byte, 8>, 8> pixel_data;

	for (byte row = 0; row < 8; row++) {
		byte low_byte = tile_data[row * 2];
		byte high_byte = tile_data[row*2 + 1];

		for (byte col = 0; col < 8; col++) {
			byte bit_pos = 7 - col;

			byte low = (low_byte >> bit_pos) & 1;
			byte high = (high_byte >> bit_pos) & 1;

			pixel_data[row][col] = (high << 1) | low;

		}

	}
	return pixel_data;
}

word Graphics::get_tile_base_pointer() {

	if ((LCDC & 0x10) >> 4) {
		return  0x8000;
	}
	return 0x9000;
}

word Graphics::get_map_base_pointer() {

	if ((LCDC & 0x8) >> 3) {
		return  0x9C00;
	}
	return 0x9800;
}

void Graphics::create_background() {

	word map_bp = get_map_base_pointer();
	for (word i = 0; i < 1024; i++) {
		byte tile_id = bus.read_memory(map_bp + i);
		tile current_tile = decode_tile(get_tile_data(tile_id));
		word row = i / 32;
		word col = i % 32;
		t_map[row][col] = current_tile;
	}
}