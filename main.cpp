#include "gameboy.h"
#include <iostream>
#include <cassert>


int main() {
    //test_decode_tile_basic();
	Gameboy gb;
    gb.load_rom("");
    //bool log = false;
    //gb.run(log);


	bool running = true;
	SDL_Event event;
	byte p = 0x2;
	gb.bus.ppu.set_palette(0, p);

	while (running)
	{
		// handle events
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_EVENT_QUIT)
				running = false;
		}
        gb.bus.ppu.draw_tilemap();
	}

	return 0;
}


