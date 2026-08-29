#include "mbc.h"

void MBC::init_rom_bank_info(byte rom_size_byte) {
	rom_num_banks = 2 << rom_size_byte; 
	rom_bank_mask = rom_num_banks - 1;
}


void MBC::init_ram_size(byte ram_size_byte) {
	size_t ram_size = 0;

	switch (ram_size_byte) {
		case 0x00: ram_size = 0; break;
		case 0x02: ram_size = 8 * 1024; break;
		case 0x03: ram_size = 32 * 1024; break;
		case 0x04: ram_size = 128 * 1024; break;
		case 0x05: ram_size = 64 * 1024; break;
		default: ram_size = 0; break;
	}
	ext_ram.resize(ram_size, 0);

}

void MBC1::set_current_bank_low(byte data) {
	//top 3 bits not needed
	word requested_bank = data & 0x1F;
	requested_bank &= rom_bank_mask;
	if (requested_bank == 0) requested_bank = 1;
	current_rom_bank_low5 = requested_bank;
}



void MBC1::set_high_rom_ram(byte data) {
	current_rom_bank_up2 = data & 0x3;
	ram_bank = data & 0x03;
}


void MBC1::set_bank_mode(byte data) {
	if (data == 0) {
		bank_mode = SIMPLE;
		return;
	}

	bank_mode = ADVANCED;
}

void MBC1::write_ram(word address, byte data){

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

byte MBC1::read_ram(word address) {

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


void MBC1::write_rom(word address, byte data) {

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

	return;
}

byte MBC1::read_rom(word address) {

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

	return 0xFF;
}

