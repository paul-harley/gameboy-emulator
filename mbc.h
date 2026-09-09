#pragma once
#include "types.h"

#include <vector>
#include <ctime>
#include <fstream>



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

	virtual void serialize(std::ofstream& out) = 0;
	virtual void deserialize(std::ifstream& in) = 0;


	void set_battery_save_path(const std::string& path);
	void load_battery_save();
	void save_battery_save();


protected:
	// same functionality across all mbcs

	bool ram_enabled = false;
	byte rom_num_banks = 2;
	byte rom_bank_mask = 0;
	std::string battery_save_path;

	void ram_enable_write(byte data) {
		ram_enabled = ((data & 0x0F) == 0x0A);
	}

	std::vector<byte> ext_ram;
	void init_rom_bank_info(byte rom_size_header_byte);
	void init_ram_size(byte ram_size_byte);

};


class MBC_NONE: public MBC {

public:
	MBC_NONE(const std::vector<byte>& rom_data): rom_data(rom_data) {

	}

	void write_ram(word address, byte data) override;
	byte read_ram(word address) override;

	void write_rom(word address, byte data) override;
	byte read_rom(word address) override;

	void serialize(std::ofstream& out) override;
	void deserialize(std::ifstream& in) override;

private:
	const std::vector<byte>& rom_data;
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

	void serialize(std::ofstream& out) override;
	void deserialize(std::ifstream& in) override;

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

	void serialize(std::ofstream& out) override;
	void deserialize(std::ifstream& in) override;

private:
	byte rom_bank = 1;
	const std::vector<byte>& rom_data;

};


class MBC3 : public MBC {

public:
	MBC3(std::vector<byte>& rom_data) : rom_data(rom_data) {
		init_rom_bank_info(rom_data[0x0148]);
		init_ram_size(rom_data[0x0149]);

		// THIS PROBABLY NEEDS TO BE ADDED TO SAVES ONCE I GET THERE
		// IF THERE IS A BASETIME IN THE SAVE DONT CHANGE IT,
		// ELSE DO THIS AND SAVE IT
		rtc_base_time = std::time(nullptr);
	}

	void write_rom (word address, byte data) override;
	byte read_rom (word address) override;
	
	void write_ram (word address, byte data) override;
	byte read_ram (word address) override;

	void serialize(std::ofstream& out) override;
	void deserialize(std::ifstream& in) override;


private:
	byte rom_bank = 1;
	const std::vector<byte>& rom_data;

	byte ram_rtc_select = 0;
	std::time_t rtc_base_time;

	bool rtc_halted = false;
	long long frozen_elapsed = 0;
	long long get_current_elapsed();
	void set_current_elapsed(long long new_elapsed);

	byte last_latch_data = 0xFF;
	byte RTC_S = 0;
	byte RTC_M = 0;
	byte RTC_H = 0;
	byte RTC_DL = 0;
	byte RTC_DH = 0;

};


class MBC5 : public MBC {

public:
	MBC5(const std::vector<byte>& rom_data) : rom_data(rom_data) {
		init_rom_bank_info(rom_data[0x0148]);
		init_ram_size(rom_data[0x0149]);
	}

	void write_ram(word address, byte data) override;
	byte read_ram(word address) override; //same as mbc1

	void write_rom(word address, byte data) override;
	byte read_rom(word address) override;


	void serialize(std::ofstream& out) override;
	void deserialize(std::ifstream& in) override;

private:
	const std::vector<byte>& rom_data;
	byte ram_bank = 0;
	byte current_rom_bank_low8 = 0;
	byte current_rom_bank_up1 = 0;

};