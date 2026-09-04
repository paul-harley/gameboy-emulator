#pragma once
#include "types.h"
#include "timer.h"

#include <SDL3/SDL.h>
#include <vector>


class PulseChannel {

public:
	bool enabled = false;
	bool dac_enabled = false;

	byte duty = 0;              // NRx1 bits 6-7
	byte length_timer = 0;      // NRx1 bits 0-5, counts DOWN toward 0
	bool length_enabled = false; // NRx4 bit 6

	byte initial_volume = 0;    // NRx2 bits 4-7
	bool envelope_increase = false; // NRx2 bit 3
	byte envelope_pace = 0;     // NRx2 bits 0-2
	byte current_volume = 0;
	byte envelope_timer = 0;

	word period = 0;            // combined 11-bit value from NRx3/NRx4

	void tick(word t_cycles);
	virtual void trigger();
	void tick_envelope(); // called at 64Hz
	void tick_length(); // called at 256Hz
	byte get_output();

	byte period_low = 0;
	byte period_high = 0;
	word get_written_period() { return period_low | (period_high << 8); };

protected:

	int period_timer = 0;
	byte waveform_position = 0;

};

class SquareChannel : public PulseChannel {

public:
	byte sweep_pace = 0;       // NR10 bits 4-6
	bool sweep_decrease = false; // NR10 bit 3
	byte sweep_step = 0;       // NR10 bits 0-2

	void trigger() override; //same as pulse with sweep added

	void tick_sweep(); // called at 128Hz


private:

	word sweep_shadow_period = 0;
	byte sweep_timer = 0;
	bool sweep_enabled = false;
	word compute_sweep(); // returns new period, disables channel on overflow
};



class APU {

public:
	APU(Timer& timer);

	byte read(word address);
	void write(word address, byte data);
	void tick(word t_cycles);
	void step_frame_sequencer();

	double cycles_per_sample = 4194304.0 / 44100.0; // ~95.1
	double sample_accumulator = 0.0;
	std::vector<float> sample_buffer;
	SDL_AudioStream* audio_stream;
	SDL_AudioSpec spec;

private:

	void generate_sample();

	bool audio_enabled = true;
	byte NR51 = 0;
	byte NR50 = 0;

	SquareChannel ch1;
	PulseChannel ch2;


	byte frame_sequencer_step = 0;
	bool prev_div_bit = false; // for falling edge detection

	Timer& timer;
};

