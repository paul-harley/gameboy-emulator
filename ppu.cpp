#include "ppu.h"
#include "bus.h" 

PPU::PPU(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts) {

	//	Init Display
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Main", WINDOW_WIDTH * SCALE, WINDOW_HEIGHT * SCALE, 0);
	renderer = SDL_CreateRenderer(window, NULL);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, 160, 144);
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

}

void PPU::reset() {
	ly = 0;
	dot_counter = 0;
	window_line_counter = 0;
	current_mode = 2;
	STAT = 0;
}


void PPU::tick(int cycles){

	if (!(LCDC & 0x80)) {
		ly = 0;
		dot_counter = 0;
		window_line_counter = 0;
		STAT = (STAT & ~0x03); // mode 0
		return;
	}


    dot_counter += cycles;

	while (dot_counter >= 456)
	{
		dot_counter -= 456;

		// draw before incrementing, get weird lines otherwise
		if (ly < 144) {
			render_row_bg(ly);
			render_row_win(ly);
			render_row_sprites(ly);
		}

		ly++;
		check_lyc();

		if (ly == 144) {
			interrupts.request(VBlank);
			SDL_UpdateTexture(texture, NULL, framebuffer, 160 * sizeof(uint32_t));
			SDL_RenderClear(renderer);
			SDL_RenderTexture(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);

			frame_ready = true;
		}
		else if (ly == 154) {
			ly = 0;
			window_line_counter = 0;
		}

	}
	update_stat_mode();
}

byte PPU::get_ly() {
    return ly;
}



std::array<byte, 16> PPU::get_tile_data(byte index) {

	word tile_bp = get_tile_base_pointer();
	return get_tile_data(tile_bp, index);
}

std::array<byte, 16> PPU::get_tile_data(word tile_bp, byte index) {
	std::array<byte, 16> tile_data{};

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

word PPU::get_map_base_pointer_bg() {

	if ((LCDC & 0x8) >> 3) {
		return  0x9C00;
	}
	return 0x9800;
}

word PPU::get_map_base_pointer_win() {
	if ((LCDC & 0x40) >> 6) {
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

void PPU::set_rend_col(colour col) {
	SDL_SetRenderDrawColor(renderer, get<0>(col), get<1>(col), get<2>(col), get<3>(col));
}

void PPU::render_row_bg(byte ly) {

	if (!(LCDC & 0x01)) {
		for (int screen_x = 0; screen_x < WINDOW_WIDTH; screen_x++) {
			bg_window_color[ly][screen_x] = 0;
			framebuffer[ly][screen_x] = to_pixel(bg_palette.colour0);
		}
		return;
	}



	// make sure im checking lcdc every row i need it
	word map_bp = get_map_base_pointer_bg(); 
	word tile_bp = get_tile_base_pointer();

	byte bg_y = SCY + ly;
	byte tile_row = bg_y / 8;
	byte px_y = bg_y % 8;

	byte last_tile_col = 0xFF; //forces first decode
	tile decoded;


	for (int screen_x = 0; screen_x < WINDOW_WIDTH; screen_x++) {

		byte bg_x = (SCX + screen_x); // byte will wrap around 256 anyway which is total tile map size
		byte tile_col = bg_x / 8;
		byte px_x = bg_x % 8;

		if (tile_col != last_tile_col) {
			word map_offset = tile_row * 32 + tile_col;
			byte tile_id = bus.read_memory(map_bp + map_offset);

			std::array<byte, 16> tile_data = get_tile_data(tile_id);
			decoded = decode_tile(tile_data);

			last_tile_col = tile_col;
		}

		// drawing
		byte color_index = decoded[px_y][px_x];
		bg_window_color[ly][screen_x] = color_index;
		framebuffer[ly][screen_x] = to_pixel(bg_palette.get_current_colour(color_index));
	}

}


void PPU::render_row_win(byte ly) {

	if (!can_draw_window(ly)) return;


	// make sure im checking lcdc every row i need it
	word map_bp = get_map_base_pointer_win();
	word tile_bp = get_tile_base_pointer();

	byte tile_row = window_line_counter / 8;
	byte px_y = window_line_counter % 8;

	byte last_tile_col = 0xFF;
	tile decoded;


	for (int screen_x = 0; screen_x < WINDOW_WIDTH; screen_x++) {

		sword win_x = screen_x - (WX - 7);
		if (win_x < 0) continue; // this screen column is before the window starts

		byte tile_col = win_x / 8;
		byte px_x = win_x % 8;

		if (tile_col != last_tile_col) {

			word map_offset = tile_row * 32 + tile_col;
			byte tile_id = bus.read_memory(map_bp + map_offset);

			std::array<byte, 16> tile_data = get_tile_data(tile_id);
			decoded = decode_tile(tile_data);

			last_tile_col = tile_col;
		}

		// drawing
		byte color_index = decoded[px_y][px_x];
		bg_window_color[ly][screen_x] = color_index;
		framebuffer[ly][screen_x] = to_pixel(bg_palette.get_current_colour(color_index));

	}

	window_line_counter++; 
}


void PPU::render_row_sprites(byte ly){
	if (!(LCDC & 0x02)) return;

	byte sprite_height = 8;
	if ((LCDC & 0x04)) {
		sprite_height = 16;
	}

	std::vector<word> visible_sprites_add;

	for (byte offset = 0; offset < 40; offset++) {
		word oam_address = 0xFE00 + (offset * 4);
		
		//the 16 is cause of a weird way y is stored
		byte sprite_y = bus.read_memory(oam_address) - 16;
		
		if (ly >= sprite_y && ly < sprite_y + sprite_height) {
			visible_sprites_add.push_back(oam_address);
			if (visible_sprites_add.size() == 10) break;
		}
	}

	sort_sprites_x(visible_sprites_add);
	byte sprites_to_draw = visible_sprites_add.size();

	for (sbyte i = sprites_to_draw -1; i >= 0; i--) {

		byte y_pos = bus.read_memory(visible_sprites_add.at(i));
		byte x_pos = bus.read_memory(visible_sprites_add.at(i) + 1);
		byte tile_index = bus.read_memory(visible_sprites_add.at(i) + 2);
		byte attributes = bus.read_memory(visible_sprites_add.at(i) + 3);

		draw_sprite(ly, y_pos, x_pos, tile_index, attributes, sprite_height);

	}


}


void PPU::sort_sprites_x(std::vector<word>& sprites){
	for (size_t i = 0; i < sprites.size(); i++) {
		for (size_t j = 0; j < sprites.size() - i - 1; j++) {

			byte x_a = bus.read_memory(sprites[j] + 1);
			byte x_b = bus.read_memory(sprites[j + 1] + 1);

			if (x_a > x_b) {
				std::swap(sprites[j], sprites[j + 1]);
			}
		}
	}
}

void PPU::draw_sprite(byte ly, byte y_pos, byte x_pos, byte tile_index, byte attributes, byte height) {

	sword screen_y = y_pos - 16;
	sword screen_x = x_pos - 8;

	if (screen_y >= 144 || screen_y + height <= 0) return;
	if (screen_x >= 160 || screen_x + 8 <= 0) return;

	sbyte row_in_sprite = ly - screen_y; 

	// Y flip across whole sprite
	if (attributes & 0x40) {
		row_in_sprite = (height - 1) - row_in_sprite;
	}

	byte tile_id;
	byte row_in_tile;

	if (height == 16) {
		if (row_in_sprite < 8) {
			tile_id = tile_index & 0xFE; // top tile
			row_in_tile = row_in_sprite;
		}
		else {
			tile_id = tile_index | 0x01; // bottom tile
			row_in_tile = row_in_sprite - 8;
		}
	}
	else {
		tile_id = tile_index;
		row_in_tile = row_in_sprite;
	}

	tile decoded = decode_tile(get_tile_data(0x8000, tile_id));

	for (byte pixel_x = 0; pixel_x < 8; pixel_x++) {

		sword draw_x = screen_x + pixel_x;
		if (draw_x < 0 || draw_x >= 160) continue;

		int sprite_x = pixel_x;
		if (attributes & 0x20) {
			sprite_x = 7 - sprite_x; // X flip
		}

		byte color_index = decoded[row_in_tile][sprite_x];

		if (color_index == 0) continue; // sprite-transparent
		if ((attributes & 0x80) && bg_window_color[ly][draw_x] != 0) continue; // bg priority

		if (attributes & 0x10) {
			framebuffer[ly][draw_x] = to_pixel(obj1_palette.get_current_colour(color_index));
		}
		else {
			set_rend_col(obj0_palette.get_current_colour(color_index));
			framebuffer[ly][draw_x] = to_pixel(obj0_palette.get_current_colour(color_index));

		}
	}
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

void PPU::update_stat_mode() {
	byte new_mode;

	if (ly >= 144) {
		new_mode = 1; // VBlank
	}
	else if (dot_counter < 80) {
		new_mode = 2; // OAM scan
	}
	else if (dot_counter < 252) {
		new_mode = 3; // Drawing
	}
	else {
		new_mode = 0; // HBlank
	}

	if (new_mode != current_mode) {
		current_mode = new_mode;
		STAT = (STAT & 0xFC) | new_mode; // update mode bits 0-1

		bool fire = false;
		switch (new_mode) {
		case 0: fire = STAT & 0x08; break; // Mode 0 HBlank interrupt enable
		case 1: fire = STAT & 0x10; break; // Mode 1 VBlank STAT interrupt enable
		case 2: fire = STAT & 0x20; break; // Mode 2 OAM interrupt enable
		}
		if (fire) {
			interrupts.request(STAT_i); // or whatever your enum calls this source
		}
	}
}

bool PPU::can_draw_window(byte ly) {
	if ((LCDC & 0x20) == 0) return false;
	if (ly < WY)  return false;
	if (WX > 166) return false;

	return true;
}


uint32_t PPU::to_pixel(colour c) {
	byte r = std::get<0>(c);
	byte g = std::get<1>(c);
	byte b = std::get<2>(c);
	byte a = std::get<3>(c);
	return (a << 24) | (r << 16) | (g << 8) | b;
}



void PPU::serialize(std::ofstream& out) {
	out.write(reinterpret_cast<char*>(&LYC), sizeof(LYC));
	out.write(reinterpret_cast<char*>(&STAT), sizeof(STAT));
	out.write(reinterpret_cast<char*>(&LCDC), sizeof(LCDC));
	out.write(reinterpret_cast<char*>(&SCY), sizeof(SCY));
	out.write(reinterpret_cast<char*>(&SCX), sizeof(SCX));
	out.write(reinterpret_cast<char*>(&WX), sizeof(WX));
	out.write(reinterpret_cast<char*>(&WY), sizeof(WY));
	out.write(reinterpret_cast<char*>(&BGP), sizeof(BGP));
	out.write(reinterpret_cast<char*>(&OBP0), sizeof(OBP0));
	out.write(reinterpret_cast<char*>(&OBP1), sizeof(OBP1));

	out.write(reinterpret_cast<char*>(&ly), sizeof(ly));
	out.write(reinterpret_cast<char*>(&dot_counter), sizeof(dot_counter));
	out.write(reinterpret_cast<char*>(&window_line_counter), sizeof(window_line_counter));
	out.write(reinterpret_cast<char*>(&current_mode), sizeof(current_mode));

	out.write(reinterpret_cast<char*>(&bg_palette), sizeof(bg_palette));
	out.write(reinterpret_cast<char*>(&obj0_palette), sizeof(obj0_palette));
	out.write(reinterpret_cast<char*>(&obj1_palette), sizeof(obj1_palette));

	for (auto& row : bg_window_color) {
		out.write(reinterpret_cast<char*>(row.data()), row.size());
	}
}

void PPU::deserialize(std::ifstream& in) {
	in.read(reinterpret_cast<char*>(&LYC), sizeof(LYC));
	in.read(reinterpret_cast<char*>(&STAT), sizeof(STAT));
	in.read(reinterpret_cast<char*>(&LCDC), sizeof(LCDC));
	in.read(reinterpret_cast<char*>(&SCY), sizeof(SCY));
	in.read(reinterpret_cast<char*>(&SCX), sizeof(SCX));
	in.read(reinterpret_cast<char*>(&WX), sizeof(WX));
	in.read(reinterpret_cast<char*>(&WY), sizeof(WY));
	in.read(reinterpret_cast<char*>(&BGP), sizeof(BGP));
	in.read(reinterpret_cast<char*>(&OBP0), sizeof(OBP0));
	in.read(reinterpret_cast<char*>(&OBP1), sizeof(OBP1));

	in.read(reinterpret_cast<char*>(&ly), sizeof(ly));
	in.read(reinterpret_cast<char*>(&dot_counter), sizeof(dot_counter));
	in.read(reinterpret_cast<char*>(&window_line_counter), sizeof(window_line_counter));
	in.read(reinterpret_cast<char*>(&current_mode), sizeof(current_mode));

	in.read(reinterpret_cast<char*>(&bg_palette), sizeof(bg_palette));
	in.read(reinterpret_cast<char*>(&obj0_palette), sizeof(obj0_palette));
	in.read(reinterpret_cast<char*>(&obj1_palette), sizeof(obj1_palette));

	for (auto& row : bg_window_color) {
		in.read(reinterpret_cast<char*>(row.data()), row.size());
	}
}