#include "apu.h"
#include <iostream>


// FF10 - FF3F
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
		ch1.length_timer = (data & 0x3F);
		return;
	}
	case 0xFF12: {
		ch1.initial_volume = (data & 0xF0) >> 4;
		ch1.envelope_increase = (data & 0x8) >> 3;
		ch1.sweep_pace = (data & 0x7);

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




	}

}

void APU::tick(word t_cycles) {
	if (!audio_enabled) return;

	ch1.tick(t_cycles);
	ch2.tick(t_cycles);

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
	}
	if (frame_sequencer_step == 2 || frame_sequencer_step == 6) {
		ch1.tick_sweep();
	}
	if (frame_sequencer_step == 7) {
		ch1.tick_envelope();
		ch2.tick_envelope();
	}
}

void APU::generate_sample() {

	byte ch1_out = ch1.get_output(); // 0-15
	//byte ch2_out = ch2.get_output();
	byte ch2_out = 0;

	// NR51 panning
	float left = 0.0f, right = 0.0f;
	if (NR51 & 0x10) left += ch1_out; // CH1 left
	if (NR51 & 0x01) right += ch1_out; // CH1 right
	if (NR51 & 0x20) left += ch2_out; // CH2 left
	if (NR51 & 0x02) right += ch2_out; // CH2 right

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
	envelope_timer = envelope_pace;
	current_volume = initial_volume;

}

void PulseChannel::tick_envelope() {
	if (envelope_pace == 0) return;

	if (envelope_timer > 0) envelope_timer--;

	if (envelope_timer == 0) {
		envelope_timer = envelope_pace;
		if (envelope_increase && current_volume < 15) current_volume++;

		else if (!envelope_increase && current_volume > 0) current_volume--;
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