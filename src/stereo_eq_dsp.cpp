/**
 * stereo_eq_dsp.cpp — Three-band biquad EQ implementation.
 *
 * Two channels, each with a low-shelf / peaking-mid / high-shelf series.
 * Coefficients are recomputed lazily via SetChannel(); the audio path is
 * branch-free per sample.
 */

#include "stereo_eq_dsp.h"
#include <cmath>

namespace eq_dsp {

namespace {

constexpr float kPi = 3.14159265358979323846f;

struct BiquadState
{
    float x1 = 0.f, x2 = 0.f;
    float y1 = 0.f, y2 = 0.f;
};

struct BiquadCoeffs
{
    float b0 = 1.f, b1 = 0.f, b2 = 0.f;
    float a1 = 0.f, a2 = 0.f;
};

inline float Process1(BiquadState& s, const BiquadCoeffs& c, float x)
{
    const float y = c.b0*x + c.b1*s.x1 + c.b2*s.x2 - c.a1*s.y1 - c.a2*s.y2;
    s.x2 = s.x1;  s.x1 = x;
    s.y2 = s.y1;  s.y1 = y;
    return y;
}

inline BiquadCoeffs LowShelf(float freq_hz, float gain_db, float sample_rate)
{
    const float A     = powf(10.f, gain_db / 40.f);
    const float w0    = 2.f * kPi * freq_hz / sample_rate;
    const float cw    = cosf(w0);
    const float sqA   = sqrtf(A);
    const float alpha = sinf(w0) * 0.7071068f;
    const float a0inv = 1.f / ((A+1.f) + (A-1.f)*cw + 2.f*sqA*alpha);
    return {
          A * ((A+1.f) - (A-1.f)*cw + 2.f*sqA*alpha) * a0inv,
        2.f*A * ((A-1.f) - (A+1.f)*cw              ) * a0inv,
          A * ((A+1.f) - (A-1.f)*cw - 2.f*sqA*alpha) * a0inv,
        -2.f * ((A-1.f) + (A+1.f)*cw              ) * a0inv,
               ((A+1.f) + (A-1.f)*cw - 2.f*sqA*alpha) * a0inv,
    };
}

inline BiquadCoeffs HighShelf(float freq_hz, float gain_db, float sample_rate)
{
    const float A     = powf(10.f, gain_db / 40.f);
    const float w0    = 2.f * kPi * freq_hz / sample_rate;
    const float cw    = cosf(w0);
    const float sqA   = sqrtf(A);
    const float alpha = sinf(w0) * 0.7071068f;
    const float a0inv = 1.f / ((A+1.f) - (A-1.f)*cw + 2.f*sqA*alpha);
    return {
          A * ((A+1.f) + (A-1.f)*cw + 2.f*sqA*alpha) * a0inv,
       -2.f*A * ((A-1.f) + (A+1.f)*cw              ) * a0inv,
          A * ((A+1.f) + (A-1.f)*cw - 2.f*sqA*alpha) * a0inv,
        2.f * ((A-1.f) - (A+1.f)*cw               ) * a0inv,
              ((A+1.f) - (A-1.f)*cw - 2.f*sqA*alpha) * a0inv,
    };
}

inline BiquadCoeffs Peaking(float freq_hz, float gain_db, float Q, float sample_rate)
{
    const float A     = powf(10.f, gain_db / 40.f);
    const float w0    = 2.f * kPi * freq_hz / sample_rate;
    const float cw    = cosf(w0);
    const float alpha = sinf(w0) / (2.f * Q);
    const float a0inv = 1.f / (1.f + alpha / A);
    return {
        (1.f + alpha * A) * a0inv,
        (-2.f * cw      ) * a0inv,
        (1.f - alpha * A) * a0inv,
        (-2.f * cw      ) * a0inv,
        (1.f - alpha / A) * a0inv,
    };
}

float        sample_rate_hz = 48000.f;
BiquadState  lo_[2], mid_[2], hi_[2];
BiquadCoeffs lo_c_[2], mid_c_[2], hi_c_[2];

} // namespace

void Init(float sample_rate) { sample_rate_hz = sample_rate; }

void SetChannel(uint8_t ch, const ChannelParams& p)
{
    if (ch > 1) return;
    lo_c_ [ch] = LowShelf (p.lo_freq_hz,  p.lo_gain_db,            sample_rate_hz);
    mid_c_[ch] = Peaking  (p.mid_freq_hz, p.mid_gain_db, p.mid_q,  sample_rate_hz);
    hi_c_ [ch] = HighShelf(p.hi_freq_hz,  p.hi_gain_db,            sample_rate_hz);
}

void Process(daisy::AudioHandle::InputBuffer  in,
             daisy::AudioHandle::OutputBuffer out,
             size_t                           n)
{
    for (size_t i = 0; i < n; i++)
    {
        float l = in[0][i];
        float r = in[1][i];

        l = Process1(lo_ [0], lo_c_ [0], l);
        l = Process1(mid_[0], mid_c_[0], l);
        l = Process1(hi_ [0], hi_c_ [0], l);

        r = Process1(lo_ [1], lo_c_ [1], r);
        r = Process1(mid_[1], mid_c_[1], r);
        r = Process1(hi_ [1], hi_c_ [1], r);

        out[0][i] = l;
        out[1][i] = r;
    }
}

} // namespace eq_dsp
