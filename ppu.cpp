#include "ppu.h"
#include "bus.h" 


void PPU::tick(int cycles)
{
    dot_counter += cycles;

    while (dot_counter >= 456)
    {
        dot_counter -= 456;

        ly++;

        if (ly == 144)
            interrupts.request(VBlank);
        else if (ly == 154)
            ly = 0;
    }
}

byte PPU::get_ly() {
    return ly;
}



std::array<byte, 16> PPU::get_tile_data(byte index) {
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

tile PPU::decode_tile(const std::array<byte, 16>& tile_data) {

	tile pixel_data;

	for (byte row = 0; row < 8; row++) {
		byte low_byte = tile_data[row * 2];
		byte high_byte = tile_data[row * 2 + 1];

		for (byte col = 0; col < 8; col++) {
			byte bit_pos = 7 - col;

			byte low = (low_byte >> bit_pos) & 1;
			byte high = (high_byte >> bit_pos) & 1;

			pixel_data[row][col] = (high << 1) | low;

		}

	}
	return pixel_data;
}

word PPU::get_tile_base_pointer() {

	if ((LCDC & 0x10) >> 4) {
		return  0x8000;
	}
	return 0x9000;
}

word PPU::get_map_base_pointer() {

	if ((LCDC & 0x8) >> 3) {
		return  0x9C00;
	}
	return 0x9800;
}

void PPU::create_background() {

	word map_bp = get_map_base_pointer();
	for (word i = 0; i < 1024; i++) {
		byte row = i / 32;
		byte col = i % 32;

		if (tile_is_visible(row, col)) {
			byte tile_id = bus.read_memory(map_bp + i);
			tile current_tile = decode_tile(get_tile_data(tile_id));
			t_map[row][col] = current_tile;
		}
		else {
			t_map[row][col] = std::nullopt;
		}

	}
}

void PPU::set_rend_col(colour col) {
	SDL_SetRenderDrawColor(renderer, get<0>(col), get<1>(col), get<2>(col), get<3>(col));
}

void PPU::draw_background() {

	// clear 
	set_rend_col(palette.colour1);
	SDL_RenderClear(renderer);

	// draw everywhere a pixel should be
	set_rend_col(palette.colour1);
	for (uint8_t x = 0; x < WINDOW_WIDTH; x++) {
		for (uint8_t y = 0; y < WINDOW_HEIGHT; y++) {
			if (display[x + y * WINDOW_WIDTH] == 1) {

				SDL_FRect square = { x * SCALE, y * SCALE, SCALE, SCALE };
				SDL_RenderFillRect(renderer, &square);

			}
		}
	}

	SDL_RenderPresent(renderer);
}


bool PPU::tile_is_visible(byte tile_row, byte tile_col) {
	byte left = SCX;
	byte right = (left + 159) % 256;
	byte top = SCY;
	byte bottom = (top + 143) % 256;

	byte tile_left = tile_col * 8;
	byte tile_right = tile_left + 7;
	byte tile_top = tile_row * 8;
	byte tile_bottom = tile_top + 7;


	//no horizontal wrapping
	if (left < right) {
		if (tile_right < left || tile_left > right) {
			return false;
		}
	}
	// h wrapped
	else {
		if (tile_left > right && tile_right < left) {
			return false;
		}
	}

	// no vertical wrapping
	if (bottom > top) {
		if (tile_bottom < top || tile_top > bottom) {
			return false;
		}
	}
	// v wrapped 
	else {
		if (tile_top > bottom && tile_bottom < top) {
			return false;
		}

	}
	return true;
}
