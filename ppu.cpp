#include "ppu.h"
#include "bus.h" 

PPU::PPU(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts) {

	//	Init Display
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Main", WINDOW_WIDTH * SCALE, WINDOW_HEIGHT * SCALE, 0);
	renderer = SDL_CreateRenderer(window, NULL);

	LCDC = 0;
	SCY = 0;
	SCX = 0;
}


void PPU::tick(int cycles)
{
    dot_counter += cycles;

    while (dot_counter >= 456)
    {
        dot_counter -= 456;

        ly++;
		check_lyc();

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

		if (index != 0) {
			std::cout << "LETS SEE\n";
		}
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

void PPU::set_palette(word palette_reg, byte new_vals) {
	Palette* p;

	switch (palette_reg) {
	case 0xFF47: 
		p = &bg_palette;
		BGP = new_vals;
		break;
	case 0xFF48: 
		p = &obj0_palette; 
		OBP0 = new_vals;
		break;
	case 0xFF49: 
		p = &obj1_palette; 
		OBP1 = new_vals;
		break;
	default: p = &bg_palette; break;
	}

	p->colour0 = p->get_shade(new_vals & 0x3);
	p->colour1 = p->get_shade((new_vals>>2) & 0x3);
	p->colour2 = p->get_shade((new_vals>>4) & 0x3);
	p->colour3 = p->get_shade((new_vals>>6) & 0x3);

}

void PPU::create_background() {

	word map_bp = get_map_base_pointer();


	if (get_tile_base_pointer() == 0x9000) {
		std::cout << "LCDC=" << std::hex << (int)LCDC
			<< " MAP=" << map_bp
			<< " TILEBP=" << get_tile_base_pointer()
			<< " SCX=" << (int)SCX
			<< " SCY=" << (int)SCY
			<< '\n';
	}

	for (word i = 0; i < 1024; i++) {
		byte row = i / 32;
		byte col = i % 32;

		if (tile_is_visible(row, col)) {
			byte tile_id = bus.read_memory(map_bp + i);

			if (tile_id != 0 && get_tile_base_pointer() == 0x9000) {
				std::cout << "i=" << std::hex << i
					<< " row=" << (int)row
					<< " col=" << (int)col
					<< " map_addr=" << map_bp + i
					<< " tile_id=" << (int)tile_id
					<< '\n';
			}



			tile current_tile = decode_tile(get_tile_data(tile_id));
			t_map.at(row, col) = current_tile;
		}
		else {
			t_map.at(row, col) = std::nullopt;
		}

	}
}

void PPU::set_rend_col(colour col) {
	SDL_SetRenderDrawColor(renderer, get<0>(col), get<1>(col), get<2>(col), get<3>(col));
}

void PPU::draw_tilemap() {
	// clear 
	set_rend_col(bg_palette.colour0);
	SDL_RenderClear(renderer);

	for (int screen_y = 0; screen_y < WINDOW_HEIGHT; screen_y++) {
		for (int screen_x = 0; screen_x < WINDOW_WIDTH; screen_x++) {

			byte bg_x = (SCX + screen_x); // byte will wrap around 256 anyway which is total tile map size
			byte bg_y = (SCY + screen_y);

			byte tile_col = bg_x / 8;
			byte tile_row = bg_y / 8;

			if (!t_map.at(tile_row, tile_col).has_value()) {
				continue;
			}

			byte px_x = bg_x % 8;
			byte px_y = bg_y % 8;

			// drawing
			byte color_index = t_map.at(tile_row, tile_col).value()[px_y][px_x];
			set_rend_col(bg_palette.get_current_colour(color_index));
			SDL_FRect square = { screen_x * SCALE, screen_y * SCALE, SCALE, SCALE };
			SDL_RenderFillRect(renderer, &square);
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


void PPU::check_lyc() {
	bool coincidence = (ly == LYC);

	if (coincidence) {
		STAT |= 0x04; // set bit 2
	}
	else {
		STAT &= ~0x04; // clear bit 2
	}
	bool interrupt_enabled = (STAT & 0x40) != 0;

	if (coincidence && interrupt_enabled) {
		interrupts.request(STAT_i);
	}

}