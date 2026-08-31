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

	if (address <= 0x7FFF) {
		return mbc->read_rom(address);
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		return mbc->read_ram(address);
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

	if (address >= 0x0000 && address <= 0x7FFF) {
		//write to rom
		mbc->write_rom(address, data);
		return;
	}

	if (address >= 0xA000 && address <= 0xBFFF) {
		//write to ram
		mbc->write_ram(address, data);
		return;
	}

	
	if (address >= 0xFEA0 && address <= 0xFEFF) {
		return;
	}

	if (address == 0xFF50) {
		boot_rom_enabled = false;
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


	mbc = std::make_unique<MBC3>(rom_data);

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



void Bus::serialize(std::ofstream& out) {
	for (auto* region : main_memory) {
		out.write(reinterpret_cast<char*>(region->memory.data()), region->memory.size());
	}

	out.write(reinterpret_cast<char*>(&boot_rom_enabled), sizeof(boot_rom_enabled));
	out.write(reinterpret_cast<char*>(&serial_data), sizeof(serial_data));
	out.write(reinterpret_cast<char*>(&serial_control), sizeof(serial_control));
	out.write(reinterpret_cast<char*>(&transfer_active), sizeof(transfer_active));
	out.write(reinterpret_cast<char*>(&transfer_cycles_remaining), sizeof(transfer_cycles_remaining));

	mbc->serialize(out); 
}

void Bus::deserialize(std::ifstream& in) {
	for (auto* region : main_memory) {
		in.read(reinterpret_cast<char*>(region->memory.data()), region->memory.size());
	}

	in.read(reinterpret_cast<char*>(&boot_rom_enabled), sizeof(boot_rom_enabled));
	in.read(reinterpret_cast<char*>(&serial_data), sizeof(serial_data));
	in.read(reinterpret_cast<char*>(&serial_control), sizeof(serial_control));
	in.read(reinterpret_cast<char*>(&transfer_active), sizeof(transfer_active));
	in.read(reinterpret_cast<char*>(&transfer_cycles_remaining), sizeof(transfer_cycles_remaining));

	mbc->deserialize(in);
}