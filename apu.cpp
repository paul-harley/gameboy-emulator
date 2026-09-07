#include "apu.h"
#include <iostream>


// general apu functions

APU::APU(Timer& timer) : timer(timer) {

	spec.freq = 44100;
	spec.format = SDL_AUDIO_F32;
	spec.channels = 2; // stereo for left/right panning
	SDL_Init(SDL_INIT_AUDIO);

	audio_stream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);

	SDL_ResumeAudioStreamDevice(audio_stream); // starts playback
}

byte APU::read(word address) {

	if (address >= 0xFF30 && address <= 0xFF3F) {
		word local_address = address - 0xFF30;
		return ch3.wave_ram[local_address];
	}

	switch (address) {


	case 0xFF26: {
		byte result = 0x80;
		if (!audio_enabled) {
			result = 0;
		}

		result |= 0x70; // bits 4-6 unused, float high
		if (ch1.enabled) result |= 0x01;
		if (ch2.enabled) result |= 0x02;

		//these will be added later
		//if (ch3.enabled) result |= 0x04;
		//if (ch4.enabled) result |= 0x08;
		return result;
	}
	case 0xFF25: {
		return NR51;
	}


	//******* CHANNEL 1
	case 0xFF10: {
		return (ch1.sweep_pace << 4) | (ch1.sweep_decrease << 3) | ch1.sweep_step;
	}
	case 0xFF11: {
		return (ch1.duty << 6) | ch1.length_timer;
	}
	case 0xFF12: {
		return (ch1.initial_volume << 4) | (ch1.envelope_increase << 3) | ch1.sweep_pace;
	}
	case 0xFF13: {
		return 0XFF; //write only
	}
	case 0xFF14: {
		return ch1.length_enabled << 6;
	}


	//******* CHANNEL 2
	case 0xFF16: {
		return (ch2.duty << 6) | ch2.length_timer;
	}
	case 0xFF17: {
		return (ch2.initial_volume << 4) | (ch2.envelope_increase << 3);
	}
	case 0xFF18: {
		return 0XFF; //write only
	}
	case 0xFF19: {
		return ch2.length_enabled << 6;
	}


	//******* CHANNEL 3
	case 0xFF1A: {
		return (ch3.dac_enabled << 7) | 0x7F;

	}
	case 0xFF1B: {
		return 0xFF; // write only
	}
	case 0xFF1C: {
		return (ch3.output_level << 5) | 0x9F;
	}
	case 0xFF1D: {
		return 0xFF; //write only
	}
	case 0xFF1E: {
		return (ch3.length_enabled << 6) | 0xBF;
	}


	//******* CHANNEL 4
	case 0xFF20: {
		return 0xFF; // write only
	}
	case 0xFF21: {
		return (ch4.initial_volume << 4) | (ch4.envelope_increase << 3) | ch4.envelope_pace;
	}
	case 0xFF22: {
		return (ch4.clock_shift << 4) | (!ch4.is_15_bit << 3) | ch4.clock_divider;
	}
	case 0xFF23: {
		return (ch4.length_enabled << 6) | 0xBF; // unused float high
	}

	}
}

void APU::write(word address, byte data) {

	// master audio
	if(address ==0xFF26) {
		bool new_power = data & 0x80;
		if (audio_enabled && !new_power) {
			//turning off need to reset
		}
		audio_enabled = new_power;
		return;
	}

	if (address >= 0xFF30 && address <= 0xFF3F) {
		word local_address = address - 0xFF30;
		ch3.wave_ram[local_address] = data;
		return;
	}

	if (!audio_enabled) {
		return;
	}

	switch (address) {

	// sound panning
	case 0xFF25:{
		NR51 = data;
		return;
	}

	//******* CHANNEL 1
	case 0xFF10: {
		ch1.sweep_pace = (data & 0x70) >> 4;
		ch1.sweep_decrease = (data & 0x8) >> 3;
		ch1.sweep_step = (data & 0x3);
		return;
	}
	case 0xFF11: {
		ch1.duty = (data & 0xC0) >> 6;
		ch1.length_timer = 64 - (data & 0x3F);
		return;
	}
	case 0xFF12: {
		ch1.initial_volume = (data & 0xF0) >> 4;
		ch1.envelope_increase = (data & 0x8) >> 3;
		ch1.envelope_pace = (data & 0x7);

		ch1.dac_enabled = (data & 0xF8) != 0; // any of the top 5 bits nonzero = DAC on
		if (!ch1.dac_enabled) ch1.enabled = false; // DAC off immediately silences the channel

		return;
	}
	case 0xFF13: {
		ch1.period_low = data;
		return;
	}
	case 0xFF14: {
		ch1.period_high = data & 0x7;
		ch1.length_enabled = (data & 0x40) >> 6;
		if (data & 0x80) {
			ch1.trigger();
		}
		return;
	}


	//******* CHANNEL 2
	case 0xFF16: {
		ch1.duty = (data & 0xC0) >> 6;
		ch1.length_timer = 64 - (data & 0x3F);
		return;
	}
	case 0xFF17: {
		ch2.initial_volume = (data & 0xF0) >> 4;
		ch2.envelope_increase = (data & 0x8) >> 3;
		ch2.envelope_pace = data & 0x7; 

		ch2.dac_enabled = (data & 0xF8) != 0; // any of the top 5 bits nonzero = DAC on
		if (!ch2.dac_enabled) ch2.enabled = false; // DAC off immediately silences the channel

		return;
	}
	case 0xFF18: {
		ch2.period_low = data;
		return;
	}
	case 0xFF19: {
		ch2.period_high = data & 0x7;
		ch2.length_enabled = (data & 0x40) >> 6;
		if (data & 0x80) {
			ch2.trigger();
		}
		return;
	}


	//******* CHANNEL 3
	case 0xFF1A: {
		ch3.dac_enabled = data & 0x80;
		return;
	}
	case 0xFF1B: {
		ch3.inital_length_timer = data;
		ch3.length_timer = 256 - data;
		return;

	}
	case 0xFF1C: {
		ch3.output_level = (data & 0x60) >>5;
		return;
	}
	case 0xFF1D: {
		ch3.period_low = data;
		return;
	}
	case 0xFF1E: {
		ch3.period_high = data & 0x7;
		ch3.length_enabled = (data & 0x40) >> 6;
		if (data & 0x80) {
			ch3.trigger();
		}
		return;
	}

	
	//******* CHANNEL 4
	case 0xFF20: {
		ch4.inital_length_timer = data & 0x3F; 
		ch4.length_timer = 64 - (data & 0x3F);
		return;
	}
	case 0xFF21: {
		ch4.initial_volume = (data >> 4) & 0x0F;
		ch4.envelope_increase = (data & 0x08) != 0;
		ch4.envelope_pace = data & 0x07;
		ch4.dac_enabled = (data & 0xF8) != 0;
		if (!ch4.dac_enabled) ch4.enabled = false;
		return;
	}
	case 0xFF22: {
		ch4.clock_shift = (data >> 4) & 0x0F;
		ch4.is_15_bit = !(data & 0x08); // NR43 bit 3: 0=15-bit, 1=7-bit
		ch4.clock_divider = data & 0x07;
		return;
	}
	case 0xFF23: {
		ch4.length_enabled = (data & 0x40) != 0;

		if (data & 0x80) {
			ch4.trigger();
		}
		return;
	}

	}

}

void APU::tick(word t_cycles) {
	if (!audio_enabled) return;

	ch1.tick(t_cycles);
	ch2.tick(t_cycles);
	ch3.tick(t_cycles);
	ch4.tick(t_cycles);

	bool current_div_bit = (timer.DIV & 0x10) != 0;

	if (prev_div_bit && !current_div_bit) { // falling edge
		step_frame_sequencer();
	}
	prev_div_bit = current_div_bit;

	sample_accumulator += t_cycles;
	while (sample_accumulator >= cycles_per_sample) {
		sample_accumulator -= cycles_per_sample;
		generate_sample();
	}

}

void APU::step_frame_sequencer() {

	frame_sequencer_step = (frame_sequencer_step + 1) % 8;

	if (frame_sequencer_step % 2 == 0) {
		ch1.tick_length();
		ch2.tick_length();
		ch3.tick_length();
		ch4.tick_length();
	}
	if (frame_sequencer_step == 2 || frame_sequencer_step == 6) {
		ch1.tick_sweep();
	}
	if (frame_sequencer_step == 7) {
		ch1.tick_envelope();
		ch2.tick_envelope();
		ch4.tick_envelope();
	}
}

void APU::generate_sample() {

	byte ch1_out = ch1.get_output(); // 0-15
	byte ch2_out = ch2.get_output();
	byte ch3_out = ch3.get_output();
	byte ch4_out = ch4.get_output();


	// NR51 panning
	float left = 0.0f, right = 0.0f;
	if (NR51 & 0x10) left += ch1_out; // CH1 left
	if (NR51 & 0x01) right += ch1_out; // CH1 right
	if (NR51 & 0x20) left += ch2_out; // CH2 left
	if (NR51 & 0x02) right += ch2_out; // CH2 right
	if (NR51 & 0x40) left += ch3_out; // CH3 left
	if (NR51 & 0x04) right += ch3_out; // CH3 right
	if (NR51 & 0x80) left += ch4_out; // CH4 left
	if (NR51 & 0x08) right += ch4_out; // CH4 right

	// normalize 0-15 range to -1.0...1.0, and scale by NR50 master volume
	byte left_vol = (NR50 >> 4) & 0x07;
	byte right_vol = NR50 & 0x07;


	// rough mixing for now, will clean it later with the docs
	left = (left / 15.0f) * ((left_vol + 1) / 8.0f);
	right = (right / 15.0f) * ((right_vol + 1) / 8.0f);


	sample_buffer.push_back(left);
	sample_buffer.push_back(right);

}



//pulse channel

void PulseChannel::tick(word t_cycles) {
	if (!enabled) return;

	period_timer -= t_cycles;
	while (period_timer <= 0) {
		period_timer += (2048 - period) * 4;
		period = get_written_period();
		waveform_position = (waveform_position + 1) % 8;
	}
}

void PulseChannel::trigger() {
	enabled = dac_enabled;

	if (length_timer == 0) length_timer = 64;
	period_timer = (2048 - period) * 4;
	period = get_written_period();
	envelope_timer = envelope_pace;
	current_volume = initial_volume;

}

void PulseChannel::tick_envelope() {
	if (envelope_pace == 0) return;

	if (envelope_timer > 0) envelope_timer--;

	if (envelope_timer == 0) {
		envelope_timer = envelope_pace;
		if (envelope_increase && current_volume < 15) current_volume++;

		else if (!envelope_increase && current_volume > 0) {
			current_volume--;
		}
	}
}

void PulseChannel::tick_length() {
	if (!length_enabled) return;

	if (length_timer > 0) {
		length_timer--;
		if (length_timer == 0) enabled = false;
	}
}

byte PulseChannel::get_output() {
	if (!enabled || !dac_enabled) return 0;

	static const bool duty_table[4][8] = {
		{0,0,0,0,0,0,0,1}, //12.5%
		{1,0,0,0,0,0,0,1}, //25%
		{1,0,0,0,0,1,1,1}, //50%
		{0,1,1,1,1,1,1,0}, //75%
	};

	if (duty_table[duty][waveform_position] == 1) {
		return current_volume;
	}
	else {
		return 0;
	}
}



// square channel


word SquareChannel::compute_sweep() {

	word delta = sweep_shadow_period >> sweep_step;
	word new_period;

	if (sweep_decrease) {

		new_period = sweep_shadow_period - delta;
	}
	else {
		new_period = sweep_shadow_period + delta;
	}


	if (new_period > 0x7FF) {
		enabled = false; // overflow turns off immediately
	}

	return new_period;

}

void SquareChannel::tick_sweep() {
	if (sweep_timer > 0) sweep_timer--;

	if (sweep_timer == 0) {
		if(sweep_pace){
			sweep_timer = sweep_pace;
		}
		else {
			sweep_timer = 8;
		}

		if (sweep_enabled && sweep_pace != 0) {
			word new_period = compute_sweep();

			if (new_period <= 0x7FF && sweep_step != 0) {
				// only actually write back if the step is nonzero -
				// step = 0 means "check for overflow but never change period"
				period = new_period;
				sweep_shadow_period = new_period;

				compute_sweep(); // result not saved but can disable channel
			}
		}
	}
}

void SquareChannel::trigger() {
	PulseChannel::trigger();

	sweep_shadow_period = period;
	sweep_timer = sweep_pace ? sweep_pace : 8;
	sweep_enabled = (sweep_pace != 0) || (sweep_step != 0);

	if (sweep_step != 0) {
		compute_sweep(); // result not saved but can disable channel
	}
}



// wave channel

void WaveChannel::tick(word t_cycles) {
	if (!enabled) return;
	period_timer -= t_cycles;
	while (period_timer <= 0) {
		period_timer += (2048 - period) * 2; // 2, not 4 like ch1 and ch2
		period = get_written_period();
		waveform_position = (waveform_position + 1) % 32; 
	}
}

byte WaveChannel::get_output() {
	if (!enabled || !dac_enabled) return 0;

	byte byte_index = waveform_position / 2;
	bool upper_nibble = (waveform_position % 2 == 0);
	byte raw_sample = 0;

	if (upper_nibble) {
		raw_sample = (wave_ram[byte_index] >> 4) & 0x0F;
	}
	else {
		raw_sample = wave_ram[byte_index] & 0x0F;
	}

	switch (output_level) {
	case 0: return 0;               // mute
	case 1: return raw_sample;      // 100%
	case 2: return raw_sample >> 1; // 50%
	case 3: return raw_sample >> 2; // 25%
	}
	return 0;

}

void WaveChannel::trigger() {
	enabled = dac_enabled;
	if (length_timer == 0) length_timer = 256;
	period_timer = (2048 - period) * 2;
	waveform_position = 1;
}

void WaveChannel::tick_length() {
	if (!length_enabled) return;
	if (length_timer > 0) {
		length_timer--;
		if (length_timer == 0) enabled = false;
	}
}



// noise channel

void NoiseChannel::tick(word t_cycles) {
	if (!enabled) return;

	period_timer -= t_cycles;
	while (period_timer <= 0) {
		static const int divisor_table[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
		int divisor = divisor_table[clock_divider];
		period_timer += divisor << clock_shift;

		// clock the LFSR
		bool bit0 = lfsr & 0x01;
		bool bit1 = (lfsr >> 1) & 0x01;
		bool new_bit = (bit0 == bit1);

		lfsr >>= 1;
		lfsr |= (new_bit << 14); // feed into bit 14 (top of 15-bit mode)

		if (is_15_bit) { // actually "7-bit" mode per NR43 bit meaning
			lfsr = (lfsr & ~0x40) | (new_bit << 6); // also feed into bit 6
		}

	}
}

byte NoiseChannel::get_output() {
	if (!enabled || !dac_enabled) return 0;

	if (lfsr & 0x01) {
		return 0;
	}
	else {
		return current_volume;
	}
}

void NoiseChannel::trigger() {
	enabled = dac_enabled;

	if (length_timer == 0) length_timer = 64;
	envelope_timer = envelope_pace;
	current_volume = initial_volume;
	lfsr = 0;
}

// taken straight from pulse class
void NoiseChannel::tick_envelope() {
	if (envelope_pace == 0) return;

	if (envelope_timer > 0) envelope_timer--;

	if (envelope_timer == 0) {
		envelope_timer = envelope_pace;
		if (envelope_increase && current_volume < 15) current_volume++;

		else if (!envelope_increase && current_volume > 0) current_volume--;
	}
}

void NoiseChannel::tick_length() {
	if (!length_enabled) return;

	if (length_timer > 0) {
		length_timer--;
		if (length_timer == 0) enabled = false;
	}
}