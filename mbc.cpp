#include "mbc.h"
#include <iostream>

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





// ********** MBC1 **********
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


// ********** MBC2 **********
void MBC2::write_ram(word address, byte data) {
	if (!ram_enabled) return;
	word local = address & 0x1FF; // only bottom 9 bits used, enough to see all 512 addresses (2^9)
	ext_ram[local] = data & 0x0F; 
}

byte MBC2::read_ram(word address) {
	if (!ram_enabled) return 0xFF;
	word local = address & 0x1FF;
	return ext_ram[local] | 0xF0; // upper nibble floats high
}

void MBC2::write_rom(word address, byte data) {

	// select rom bank
	if (address & 0x0100) {
		rom_bank = data & 0xF;
		if (rom_bank == 0) {
			rom_bank = 1;
		}
		return;
	}

	// control ram enable
	ram_enabled = ((data & 0xF) == 0xA) ? true : false;
	return;

}

byte MBC2::read_rom(word address) {

	// first 16kib of rom
	if (address <= 0x3FFF) {
		return rom_data[address];
	}

	// second 16kib of rom
	if (address <= 0x7FFF) {
		size_t offset = (size_t)rom_bank * 0x4000 + (address - 0x4000);
		if (offset >= rom_data.size()) return 0xFF; // out of range
		return rom_data[offset];
	}

	return 0xFF;
}



// ********** MBC3 **********

void MBC3::write_ram(word address, byte data) {
	if (!ram_enabled) return;
	
	std::cout << "RAM write: select=" << std::hex << (int)ram_rtc_select
		<< " addr=" << address << " data=" << (int)data << "\n";


	if (ram_rtc_select <= 0x07) {
		size_t offset = (size_t)ram_rtc_select * 0x2000 + (address - 0xA000);
		if (offset >= ext_ram.size()) return;
		ext_ram[offset] = data;
		return;
	}

	if (ram_rtc_select >= 0x08 && ram_rtc_select <= 0x0C) {

		long long elapsed = get_current_elapsed();
		byte seconds = elapsed % 60;
		byte minutes = (elapsed / 60) % 60;
		byte hours = (elapsed / 3600) % 24;
		int  days = elapsed / 86400;

		switch (ram_rtc_select) {
		case 0x08: seconds = data % 60; break;
		case 0x09: minutes = data % 60; break;
		case 0x0A: hours = data % 24; break;
		case 0x0B: days = (days & 0x100) | data; break;             // replace low 8 bits, keep bit 8
		case 0x0C: {
			bool new_halt = (data & 0x40) != 0;
			if (new_halt && !rtc_halted) {
				frozen_elapsed = elapsed; // just entered halt: freeze right here
			}
			rtc_halted = new_halt;
			days = (days & 0xFF) | ((data & 0x01) << 8);
			break;
		}

		}

		long long total_seconds = (long long)days * 86400 + hours * 3600 + minutes * 60 + seconds;
		set_current_elapsed(total_seconds);


		return;
	}
}

byte MBC3::read_ram(word address) {
	if (!ram_enabled) return 0xFF;

	std::cout << "RAM read: select=" << std::hex << (int)ram_rtc_select
		<< " addr=" << address << "\n";


	if (ram_rtc_select <= 0x07) {
		size_t offset = (size_t)ram_rtc_select * 0x2000 + (address - 0xA000);
		if (offset >= ext_ram.size()) return 0xFF;
		return ext_ram[offset];
	}

	if (ram_rtc_select >= 0x08 && ram_rtc_select <= 0x0C) {
		
		switch (ram_rtc_select) {
		case 0x08:
			return RTC_S;
		case 0x09:
			return RTC_M;
		case 0x0A:
			return RTC_H;
		case 0x0B:
			return RTC_DL;
		case 0x0C:
			return RTC_DH;
		}

	}

	return 0xFF;
}

void MBC3::write_rom(word address, byte data) {

	if (address <= 0x1FFF) {
		if (data == 0x0A) {
			ram_enabled = true;
		}
		else if (data == 0) {
			ram_enabled = false;
		}
		return;
	}

	if (address <= 0x3fff) {
		rom_bank = data & 0x7F;
		rom_bank &= rom_bank_mask;
		if (rom_bank == 0) rom_bank = 1;
		return;
	}

	if (address <= 0x5fff) {
		ram_rtc_select = data;
		return;
	}

	if (address <= 0x7fff) {
		if (last_latch_data == 0 && data == 1) {
			//write to rtc regs
			long long elapsed = get_current_elapsed(); 
			byte seconds = elapsed % 60;
			byte minutes = (elapsed / 60) % 60;
			byte hours = (elapsed / 3600) % 24;
			word days = elapsed / 86400;

			RTC_S = seconds;
			RTC_M = minutes;
			RTC_H = hours;
			RTC_DL = days & 0xFF; //last 8 bits of day counter                

			byte dh = (days >> 8) & 0x01; //extra bit of day counter
			if (days >= 512) dh |= 0x80;  // day counter overflow
			if (rtc_halted) dh |= 0x40;

			RTC_DH = dh;

		}

		last_latch_data = data;
		return;
	}


}

byte MBC3::read_rom(word address) {

	if (address <= 0x3fff) {
		return rom_data[address];
	}

	else {
		size_t offset = (size_t)rom_bank * 0x4000 + (address - 0x4000);
		if (offset >= rom_data.size()) return 0xFF; // out of range
		return rom_data[offset];
	}


}

long long MBC3::get_current_elapsed() {
	if (rtc_halted) return frozen_elapsed;
	return std::time(nullptr) - rtc_base_time;
}

void MBC3::set_current_elapsed(long long new_elapsed) {
	if (rtc_halted) {
		frozen_elapsed = new_elapsed;
	}
	else {
		rtc_base_time = std::time(nullptr) - new_elapsed;
	}
}

void MBC3::serialize(std::ofstream& out) {
	out.write(reinterpret_cast<char*>(&ram_enabled), sizeof(ram_enabled)); // from MBC base
	out.write(reinterpret_cast<char*>(&rom_bank), sizeof(rom_bank));
	out.write(reinterpret_cast<char*>(&ram_rtc_select), sizeof(ram_rtc_select));
	out.write(reinterpret_cast<char*>(&rtc_base_time), sizeof(rtc_base_time));
	out.write(reinterpret_cast<char*>(&rtc_halted), sizeof(rtc_halted));
	out.write(reinterpret_cast<char*>(&frozen_elapsed), sizeof(frozen_elapsed));
	out.write(reinterpret_cast<char*>(&last_latch_data), sizeof(last_latch_data));
	out.write(reinterpret_cast<char*>(&RTC_S), sizeof(RTC_S));
	out.write(reinterpret_cast<char*>(&RTC_M), sizeof(RTC_M));
	out.write(reinterpret_cast<char*>(&RTC_H), sizeof(RTC_H));
	out.write(reinterpret_cast<char*>(&RTC_DL), sizeof(RTC_DL));
	out.write(reinterpret_cast<char*>(&RTC_DH), sizeof(RTC_DH));
	out.write(reinterpret_cast<char*>(ext_ram.data()), ext_ram.size());
}

void MBC3::deserialize(std::ifstream& in) {
	in.read(reinterpret_cast<char*>(&ram_enabled), sizeof(ram_enabled)); // from MBC base
	in.read(reinterpret_cast<char*>(&rom_bank), sizeof(rom_bank));
	in.read(reinterpret_cast<char*>(&ram_rtc_select), sizeof(ram_rtc_select));
	in.read(reinterpret_cast<char*>(&rtc_base_time), sizeof(rtc_base_time));
	in.read(reinterpret_cast<char*>(&rtc_halted), sizeof(rtc_halted));
	in.read(reinterpret_cast<char*>(&frozen_elapsed), sizeof(frozen_elapsed));
	in.read(reinterpret_cast<char*>(&last_latch_data), sizeof(last_latch_data));
	in.read(reinterpret_cast<char*>(&RTC_S), sizeof(RTC_S));
	in.read(reinterpret_cast<char*>(&RTC_M), sizeof(RTC_M));
	in.read(reinterpret_cast<char*>(&RTC_H), sizeof(RTC_H));
	in.read(reinterpret_cast<char*>(&RTC_DL), sizeof(RTC_DL));
	in.read(reinterpret_cast<char*>(&RTC_DH), sizeof(RTC_DH));
	in.read(reinterpret_cast<char*>(ext_ram.data()), ext_ram.size());
}