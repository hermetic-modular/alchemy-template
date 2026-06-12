/**
 * stereo_eq_dsp.h — Three-band biquad EQ, stereo, independent per-channel.
 *
 * The DSP knows nothing about VirtualKnob, ControlLoop, or any SDK surface.
 * The control side computes band parameters in engineering units (Hz, dB)
 * and pushes them to the DSP via SetChannel(). The audio callback is a
 * plain libDaisy AudioCallback the caller wires into hw.StartAudio().
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include "daisy_seed.h"

namespace eq_dsp {

/** Per-channel band parameters in engineering units. */
struct ChannelParams
{
    float lo_freq_hz, lo_gain_db;
    float mid_freq_hz, mid_gain_db, mid_q;
    float hi_freq_hz, hi_gain_db;
};

/** Cache the sample rate. Call once after hw.Init(). */
void Init(float sample_rate);

/**
 * Update biquad coefficients for one channel (0 = left, 1 = right).
 * Cheap; safe to call from a per-frame UpdateCoeffs hook.
 */
void SetChannel(uint8_t ch, const ChannelParams& p);

/**
 * Audio callback. Pass directly to hw.StartAudio(eq_dsp::Process).
 * Processes left and right independently through three series biquads each.
 */
void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           n);

} // namespace eq_dsp
