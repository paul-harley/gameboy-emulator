#pragma once
#include "types.h"
#include <vector>


enum Bank_Mode {
	SIMPLE,
	ADVANCED
};



class MBC {

public:

	virtual void write_ram(word address, byte data) = 0;
	virtual byte read_ram(word address) = 0;

	virtual void write_rom(word address, byte data) = 0;
	virtual byte read_rom(word address) = 0;

	virtual ~MBC() = default;


protected:
	// same functionality across all mbcs

	bool ram_enabled = false;
	byte rom_num_banks = 2;
	byte rom_bank_mask = 0;

	void ram_enable_write(byte data) {
		ram_enabled = ((data & 0x0F) == 0x0A);
	}

	std::vector<byte> ext_ram;
	void init_rom_bank_info(byte rom_size_header_byte);
	void init_ram_size(byte ram_size_byte);

};


class MBC1 : public MBC {

public:
	MBC1(const std::vector<byte>& rom) : rom_data(rom) {
		init_rom_bank_info(rom_data[0x0148]);
		init_ram_size(rom_data[0x0149]);
	}

	void write_ram(word address, byte data) override;
	byte read_ram(word address) override;

	void write_rom(word address, byte data) override;
	byte read_rom(word address) override;

private:
	byte current_rom_bank_low5 = 0;
	byte current_rom_bank_up2 = 0;

	const std::vector<byte>& rom_data;
	byte ram_bank = 0;
	Bank_Mode bank_mode = SIMPLE;

	void set_current_bank_low(byte data);
	void set_high_rom_ram(byte data);
	void set_bank_mode(byte data);

};


class MBC2 : public MBC {
public:
	MBC2(const std::vector<byte>& rom) : rom_data(rom) {
		ext_ram.resize(512, 0);
	}

	void write_ram(word address, byte data) override;
	byte read_ram(word address) override;

	void write_rom(word address, byte data) override;
	byte read_rom(word address) override;

private:
	byte rom_bank = 1;
	const std::vector<byte>& rom_data;

};