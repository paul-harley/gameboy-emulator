#pragma once
#include <optional>
#include <tuple>
#include <SDL3/SDL.h>

#include "interrupts.h"
#include "types.h"
#include "colours.h"


class Bus;

struct Palette {

	colour colour0 = Colours::WHITE;
	colour colour1 = Colours::WHITE;
	colour colour2 = Colours::WHITE;
	colour colour3 = Colours::WHITE;

	colour get_current_colour(byte index) {
		switch (index) {
		case 0: return colour0;
		case 1: return colour1;
		case 2: return colour2;
		case 3: return colour3;
		default: return colour0;
		}
	}

	colour get_shade(byte index) {
		switch (index) {
		case 0: return Colours::WHITE;
		case 1: return Colours::LIGHT_GREY;
		case 2: return Colours::DARK_GREY;
		case 3: return Colours::BLACK;
		default: return Colours::WHITE;
		}
	}

};


class PPU {
public:
	PPU(Bus& bus, Interrupts& interrupts);
    void tick(int cycles);

    byte get_ly();
	Bus& bus;
    Interrupts& interrupts;

	std::array<byte, 16> get_tile_data(byte index);
	tile decode_tile(const std::array<byte, 16>& tile_data);

	byte LYC = 0;
	byte STAT = 0;

	byte LCDC = 0;

	byte SCY = 0; // top left cords of visible background area
	byte SCX = 0;
	byte WX = 0;
	byte WY = 0;

	byte BGP = 0;
	byte OBP0 = 0;
	byte OBP1 = 0;

	word get_tile_base_pointer();
	word get_map_base_pointer_bg();
	word get_map_base_pointer_win();

	void set_palette(word palette_reg, byte new_vals);

	SDL_Window* window;
	SDL_Renderer* renderer;
	static constexpr byte WINDOW_WIDTH = 160;
	static constexpr byte WINDOW_HEIGHT = 144;
	std::array<byte, WINDOW_WIDTH* WINDOW_HEIGHT > display;	
	void set_rend_col(colour col);
	void render_row_bg(byte ly);
	void render_row_win(byte ly);
	void render_row_sprites(byte ly);


private:
    byte ly = 0;
    int dot_counter = 0;
	byte window_line_counter = 0;

	byte SCALE = 5;
	Palette bg_palette;
	Palette obj0_palette;
	Palette obj1_palette;

	void check_lyc();
	bool can_draw_window(byte ly);
};

