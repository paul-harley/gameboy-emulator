## Game Boy Emulator

Status as of this writeup: complete working DMG emulator.  
CPU (passes Blargg suite incl. halt_bug, and 500k/500k on SingleStepTests),   
PPU (passes dmg-acid2), MBC1/2/3/5,  
full 4-channel APU,   
save states,   
battery saves,   
native ROM picker,  
Verified against Tetris, Alleyway, Super Mario Land, Link's Awakening, Pokémon Blue, Pokémon Gold/Silver, Wario Land 2, F1 Race.

---

### Known gaps / deferred items (DMG)

PPU timing: currently scanline-batched, not per-dot/per-M-cycle, Correct output for the majority of games, but not hardware perfect for extremely timing-sensitive titles 
(mid-scanline raster effects beyond what's already been tested),  

Rarer MBC types not implemented: MBC6, MBC7 (motion sensor), HuC1, HuC3, MMM01. Small % of the library, mostly obscure titles,   

Battery save autosave: currently a flat 25s timer, no dirty-flag check to save each time written to.  

---

### Future Work - Game Boy Colour
Main things that need reworking:  
WRAM banking  
VRAM banking  
background/sprite palettes  
BG tile attributes (VRAM bank 1)  
Double-speed mode  
HDMA  
