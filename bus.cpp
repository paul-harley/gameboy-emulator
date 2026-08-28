#include "bus.h"
#include <iostream>


word Bus::fix_echo_address(word address) {

	if (address >= 0xE000 && address <= 0xFDFF) {
		return address - 0x2000; // mirrors into 0xC000-0xDDFF (wram)
	}
	return address;
}

MemoryRegion* Bus::get_correct_memory(word address) {

	for (MemoryRegion* region : main_memory) {

		if (address >= region->start_address && address <= region->end_address) {
			return region;
		}

	}

	std::cout << std::hex<< "Address: " << int(address) << " does not exist \n";
	throw std::runtime_error("Invalid memory address");
}


byte Bus::read_memory(word address) {
	if (test_ram_override) return test_ram_override[address];

	if (boot_rom_enabled && address <= 0x00FF) {
		return boot_rom[address];
	}

	// first 16kib of rom
	if (address <= 0x3FFF) {
		byte bank = 0;
		if (bank_mode == ADVANCED) {
			bank = (current_rom_bank_up2 << 5); // only banks 0/32/64/96 reachable here
		}
		size_t offset = (size_t)bank * 0x4000 + address;
		return rom_data[offset];

	}

	// second 16kib of rom
	if (address <= 0x7FFF) {
		byte bank = current_rom_bank_low5 | (current_rom_bank_up2 << 5);
		size_t offset = (size_t)bank * 0x4000 + (address - 0x4000);
		if (offset >= rom_data.size()) return 0xFF; // out of range
		return rom_data[offset];
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		if (!ram_enabled || ext_ram.empty()) return 0xFF;

		byte bank;
		if (bank_mode == ADVANCED) {
			bank = ram_bank;
		}
		else {
			bank = 0;
		}

		size_t offset = (size_t)bank * 0x2000 + (address - 0xA000);
		return ext_ram[offset];
	}




	if (address >= 0xFEA0 && address <= 0xFEFF) {
		return 0;
	}

	switch (address) {
	case 0xFF00:
	{
		byte value = joypad.read();
		return value;
	}

	case 0xFF01:
		return serial_data;

	case 0xFF02:
		return serial_control;

	case 0xFF04:
		return timer.DIV;
	case 0xFF05:
		return timer.TIMA;
	case 0xFF06:
		return timer.TMA;
	case 0xFF07:
		return timer.TAC;

	case 0xFF40:
		return ppu.LCDC;
	case 0xFF41: 
		return ppu.STAT;
	case 0xFF42:
		return ppu.SCY;
	case 0xFF43:
		return ppu.SCX;
	case 0xFF44:
		return ppu.get_ly();
	case 0xFF45: 
		return ppu.LYC;

	case 0xFF47: 
		return ppu.BGP;
	case 0xFF48: 
		return ppu.OBP0;
	case 0xFF49: 
		return ppu.OBP1;

	case 0xFF4A:
		return ppu.WY;
	case 0xFF4B:
		return ppu.WX;

	case 0xFF0F:
		return interrupts.IF | 0xE0;
	case 0xFFFF:
		return interrupts.IE;

	}

	address = fix_echo_address(address);


	const MemoryRegion* mem_region = get_correct_memory(address);

	word local_address = address - mem_region->start_address;
	return mem_region->memory[local_address];
	return 0;
}


void Bus::write_memory(word address, byte data) {

	if (test_ram_override) { test_ram_override[address] = data; return; }

	if (address >= 0x0000 && address <= 0x1FFF) {
		ram_enabled = ((data & 0x0F) == 0x0A);
		return;
	}
	if (address >= 0x2000 && address <= 0x3FFF) {
		set_current_bank_low(data);
		return;
	}
	if (address >= 0x4000 && address <= 0x5FFF) {
		set_high_rom_ram(data);
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		if (!ram_enabled || ext_ram.empty()) return;

		byte bank;
		if (bank_mode == ADVANCED) {
			bank = ram_bank;
		}
		else {
			bank = 0;
		}

		size_t offset = (size_t)bank * 0x2000 + (address - 0xA000);
		ext_ram[offset] = data;
		return;
	}


	
	if (address >= 0xFEA0 && address <= 0xFEFF) {
		return;
	}

	if (address == 0xFF50) {
		boot_rom_enabled = false;
		return;
	}



	if (address <= 0x7FFF) {
		//std::cout << "WARNING: write to ROM area! addr="
		//	<< std::hex << address << " data=" << (int)data << std::endl;
		return;
	}


	switch (address) {
	case 0xFF00:
		joypad.write(data);
		return;

	case 0xFF01:
		serial_data = data;
		break;

	case 0xFF02: {
		serial_control = data;
		if ((data & 0x80) && (data & 0x01)) { // start bit + internal clock only
			transfer_active = true;
			transfer_cycles_remaining = 4096; // 8 bits * 512 T-cycles/bit
		}
		return;
	}
	case 0xFF04:
		timer.DIV = 0;
		timer.reset_sys_counter();
		return;
	case 0xFF05:
		timer.TIMA = data;
		return;
	case 0xFF06:
		timer.TMA = data;
		return;
	case 0xFF07:
		timer.TAC = data;
		return;

	case 0xFF40:
		ppu.LCDC = data;
		return;
	case 0xFF41: 
		ppu.STAT = (ppu.STAT & 0x07) | (data & 0xF8); //ensure bottom 3 bits are read only
		return;
	case 0xFF42:
		ppu.SCY = data;
		return;
	case 0xFF43:
		ppu.SCX = data;
		return;
	case 0xFF45: 
		ppu.LYC = data; 
		return;

	case 0xFF46: {
		word source_base = data << 8;
		for (byte i = 0; i < 0xA0; i++) {
			byte value = read_memory(source_base + i);
			write_memory(0xFE00 + i, value);
		}
		return;
	}

	case 0xFF47:
		ppu.set_palette(address, data);
		return;
	case 0xFF48:
		ppu.set_palette(address, data);
		return;
	case 0xFF49:
		ppu.set_palette(address, data);
		return;

	case 0xFF4A:
		ppu.WY = data;
		return;
	case 0xFF4B:
		ppu.WX = data;
		return;

	case 0xFF0F:
		interrupts.IF = data & 0x1F;
		return;
	case 0xFFFF:
		interrupts.IE = data;
		return;
	}


	address = fix_echo_address(address);


	MemoryRegion* mem_region = get_correct_memory(address);
	word local_address = address - mem_region->start_address;
	mem_region->memory[local_address] = data;

}

void Bus::serial_tick(int t_cycles) {
	if (!transfer_active) return;

	transfer_cycles_remaining -= t_cycles;

	if (transfer_cycles_remaining <= 0) {
		transfer_active = false;
		serial_data = 0xFF; // no link cable connected, so nothing comes back
		serial_control &= ~0x80; // clear the start/active bit
		interrupts.request(Serial_i);
	}
}


void Bus::dump_memory(word start_loc, byte num_bytes) {

	MemoryRegion* mem_region = get_correct_memory(start_loc);


	word start_local_address = start_loc - mem_region->start_address;
	word end = std::min(mem_region->end_address, static_cast<word>(start_local_address + num_bytes));


	for (word i = start_local_address; i < end; i++) {
		std::cout
			<< "0x" << std::hex << i << " : "
			<< std::dec << static_cast<int>(mem_region->memory[i]) << "\n";

	}
}




void Bus::load_rom(const std::string filename) {

	std::ifstream rom(filename, std::ios::binary);

	if (!rom) {
		std::cout << "ROM not found..." << std::endl;
		return;
	}
	
	rom.seekg(0, std::ios::end);
	size_t rom_size = rom.tellg();
	rom.seekg(0, std::ios::beg);

	rom_data.resize(rom_size);
	rom.read(reinterpret_cast<char*>(rom_data.data()), rom_size);

	rom_num_banks = 2 << rom_data[0x0148];
	rom_bank_mask = rom_num_banks - 1;

	byte ram_size_code = rom_data[0x0149];
	size_t ram_size_from_header = 0;

	switch (ram_size_code) {
		case 0x00: ram_size_from_header = 0; break;
		case 0x02: ram_size_from_header = 8 * 1024; break;
		case 0x03: ram_size_from_header = 32 * 1024; break;
		case 0x04: ram_size_from_header = 128 * 1024; break;
		case 0x05: ram_size_from_header = 64 * 1024; break;
		default: ram_size_from_header = 0; break;
	}

	ext_ram.resize(ram_size_from_header, 0);

	std::cout << "ROM LOADED!\n";

}



void Bus::load_boot_rom(const std::string filename) {
	std::ifstream rom(filename, std::ios::binary);

	if (!rom) {
		std::cout << "Boot ROM not found...\n";
		boot_rom_enabled = false; // fail safe: fall back to skipping it
		return;
	}

	byte current_byte = 0;
	word address = 0;

	while (rom.read(reinterpret_cast<char*>(&current_byte), 1)) {
		if (address >= 256) {
			std::cout << "Boot ROM too large\n";
			break;
		}
		boot_rom[address] = current_byte;
		address++;
	}

	boot_rom_enabled = true;
}



void Bus::set_current_bank_low(byte data) {
	//top 3 bits not needed
	word requested_bank = data & 0x1F;
	requested_bank &= rom_bank_mask;
	if (requested_bank == 0) requested_bank = 1;
	current_rom_bank_low5 = requested_bank;
}

void Bus::set_high_rom_ram(byte data) {
	current_rom_bank_up2 = data & 0x3;
	ram_bank = data & 0x03;
}

void Bus::set_bank_mode(byte data) {

	if (data == 0) {
		bank_mode = SIMPLE;
		return;
	}

	bank_mode = ADVANCED;
}