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

struct Tile_Map {
	std::array<std::array<std::optional<tile>, 32>, 32> tiles;

	std::optional<tile>& at(byte row, byte col) {
		return tiles[row][col];
	}

	void clear() {
		for (auto& row : tiles)
			for (auto& cell : row)
				cell = std::nullopt;
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

	byte LYC;
	byte STAT;
	byte LCDC;
	byte SCY; // top left cords of visible background area
	byte SCX;
	byte BGP;
	byte OBP0;
	byte OBP1;

	word get_tile_base_pointer();
	word get_map_base_pointer();

	void set_palette(word palette_reg, byte new_vals);
	void create_background();


	SDL_Window* window;
	SDL_Renderer* renderer;
	static constexpr byte WINDOW_WIDTH = 160;
	static constexpr byte WINDOW_HEIGHT = 144;
	std::array<byte, WINDOW_WIDTH* WINDOW_HEIGHT > display;	
	void set_rend_col(colour col);
	void draw_tilemap();


private:
    byte ly = 0;
    int dot_counter = 0;
	byte SCALE = 5;
	Palette bg_palette;
	Palette obj0_palette;
	Palette obj1_palette;

	Tile_Map t_map;

	bool tile_is_visible(byte tile_row, byte tile_col); //check this thing if visuals look weird

	void check_lyc();
};

