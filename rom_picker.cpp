#include "rom_picker.h"
#include <SDL3/SDL_dialog.h>
#include <iostream>
#include <mutex>



void on_rom_selected(void* userdata, const char* const* filelist, int filter) {
    Gameboy* gb = static_cast<Gameboy*>(userdata);

    if (!filelist) {
        std::cout << "File dialog error: " << SDL_GetError() << "\n";
        return;
    }
    if (!filelist[0]) {
        return; // user cancelled
    }

    std::lock_guard<std::mutex> lock(gb->rom_load_mutex);
    gb->pending_rom_path = filelist[0];
}

void show_rom_picker(Gameboy* gb, SDL_Window* window) {
    static const SDL_DialogFileFilter filters[] = {
        { "Game Boy ROMs", "gb;gbc" },
        { "All files", "*" }
    };

    SDL_ShowOpenFileDialog( on_rom_selected, gb, window, filters, 2, ".", false );
}