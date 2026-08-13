#pragma once
#include <optional>
#include <tuple>
#include <SDL3/SDL.h>

#include "interrupts.h"
#include "types.h"


class Bus;

struct Palette {
	//    r	   g    b  opacity
	colour colour1 = std::make_tuple(255, 255, 255, 255);
	colour colour2 = std::make_tuple(255, 255, 255, 255);
	colour colour3 = std::make_tuple(255, 255, 255, 255);
	colour colour4 = std::make_tuple(255, 255, 255, 255);

};

class PPU {
public:
    PPU(Bus& bus, Interrupts& interrupts) : bus(bus), interrupts(interrupts) {}

    void tick(int cycles);

    byte get_ly();
	Bus& bus;
    Interrupts& interrupts;

	std::array<byte, 16> get_tile_data(byte index);
	tile decode_tile(const std::array<byte, 16>& tile_data);

	byte LCDC;
	// top left cords of visible background area
	byte SCY;
	byte SCX;

	word get_tile_base_pointer();
	word get_map_base_pointer();

	void create_background();


	SDL_Window* window;
	SDL_Renderer* renderer;
	static constexpr byte WINDOW_WIDTH = 64;
	static constexpr byte WINDOW_HEIGHT = 32;
	std::array<byte, WINDOW_WIDTH* WINDOW_HEIGHT > display;	
	void set_rend_col(colour col);
	void draw_background();



private:
    byte ly = 0;
    int dot_counter = 0;
	Palette palette;
	byte SCALE = 20;

	//tile_map t_map;
	std::optional<tile> t_map[32][32];


	bool tile_is_visible(byte tile_row, byte tile_col); //check this thing if visuals look weird
};

