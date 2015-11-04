
#ifndef _H_EFFECT_BIQUAD_CASCADE
#define _H_EFFECT_BIQUAD_CASCADE

#include "Effect.h"

class BiquadCoeffs
{
public:
    BiquadCoeffs()
    : m_b0(0.0f)
    , m_b1(0.0f)
    , m_b2(0.0f)
    , m_a0(0.0f)
    , m_a1(0.0f)
    , m_a2(0.0f)
    , m_freq(0.0f)
    , m_q(1.0f)
    {}

    float m_b0;
    float m_b1;
    float m_b2;
    float m_a0;
    float m_a1;
    float m_a2;

    float m_freq;
    float m_q;
};

class EffectBiquadCascade : public Effect
{
public:
    EffectBiquadCascade(unsigned long sample_rate)
    : m_sample_rate(sample_rate)
    , m_2pi_over_sample_rate((2 * M_PI) / (float) sample_rate)
    {
    }

    virtual ~EffectBiquadCascade()
    {
    }

    void activate();

    void set_freq(float freq);
    void set_q(float q);

    void run(float **audio_in, float **audio_out, unsigned long samples, unsigned int channels);

private:

    void update_parameters();

    float m_sample_rate;
    float m_2pi_over_sample_rate;

    float m_x0_1;
    float m_x0_2;
    float m_x1_1;
    float m_x1_2;

    float m_y0_1;
    float m_y0_2;
    float m_y1_1;
    float m_y1_2;

    BiquadCoeffs m_coeffs[1];
};

void EffectBiquadCascade::activate()
{
    m_x0_1 = 0.0f;
    m_x0_2 = 0.0f;
    m_x1_1 = 0.0f;
    m_x1_2 = 0.0f;

    m_y0_1 = 0.0f;
    m_y0_2 = 0.0f;
    m_y1_1 = 0.0f;
    m_y1_2 = 0.0f;
}

void EffectBiquadCascade::set_freq(float freq)
{
    if (freq != m_coeffs[0].m_freq)
    {
        m_coeffs[0].m_freq = freq;
        update_parameters();
    }
}

void EffectBiquadCascade::set_q(float q)
{
    if (q != m_coeffs[0].m_q)
    {
        m_coeffs[0].m_q = q;
        update_parameters();
    }
}

void EffectBiquadCascade::update_parameters()
{
    if (m_coeffs[0].m_freq <= 0)
    {
        /* Reject everything. */
        m_coeffs[0].m_b0 = 0.0f;
        m_coeffs[0].m_b1 = 0.0f;
        m_coeffs[0].m_b2 = 0.0f;
        m_coeffs[0].m_a0 = 0.0f;
        m_coeffs[0].m_a1 = 0.0f;
        m_coeffs[0].m_a2 = 0.0f;
    }
    else if (m_coeffs[0].m_freq > m_sample_rate * 0.5)
    {
        /* Above Nyquist frequency. Let everything through. */
        m_coeffs[0].m_b0 = 1.0f;
        m_coeffs[0].m_b1 = 0.0f;
        m_coeffs[0].m_b2 = 0.0f;
        m_coeffs[0].m_a0 = 0.0f;
        m_coeffs[0].m_a1 = 0.0f;
        m_coeffs[0].m_a2 = 0.0f;
    }
    else
    {
        float w = m_2pi_over_sample_rate * m_coeffs[0].m_freq;
        float cosw = cos(w);
        float sinw = sin(w);
        float alpha = sinw / (2.0f * m_coeffs[0].m_q);

        /* Low pass filter */
        m_coeffs[0].m_b0 = (1.0f - cosw) / 2.0f;
        m_coeffs[0].m_b1 = 1.0f - cosw;
        m_coeffs[0].m_b2 = (1.0f - cosw) / 2.0f;
        m_coeffs[0].m_a0 = 1.0f + alpha;
        m_coeffs[0].m_a1 = -2.0f * cosw;
        m_coeffs[0].m_a2 = 1.0f - alpha;

        m_coeffs[0].m_b0 /= m_coeffs[0].m_a0;
        m_coeffs[0].m_b1 /= m_coeffs[0].m_a0;
        m_coeffs[0].m_b2 /= m_coeffs[0].m_a0;
        m_coeffs[0].m_a1 /= m_coeffs[0].m_a0;
        m_coeffs[0].m_a2 /= m_coeffs[0].m_a0;
    }
}

void EffectBiquadCascade::run(float **audio_in, float **audio_out, unsigned long samples, unsigned int channels)
{
    for (unsigned long i = 0; i < samples; i++)
    {
        audio_out[0][i] = m_coeffs[0].m_b0 * audio_in[0][i] + m_coeffs[0].m_b1 * m_x0_1 + m_coeffs[0].m_b2 * m_x0_2
            - m_coeffs[0].m_a1 * m_y0_1 - m_coeffs[0].m_a2 * m_y0_2;

        m_y0_2 = m_y0_1;
        m_y0_1 = audio_out[0][i];

        m_x0_2 = m_x0_1;
        m_x0_1 = audio_in[0][i];

        audio_out[1][i] = m_coeffs[0].m_b0 * audio_in[1][i] + m_coeffs[0].m_b1 * m_x0_1 + m_coeffs[0].m_b2 * m_x0_2
            - m_coeffs[0].m_a1 * m_y0_1 - m_coeffs[0].m_a2 * m_y0_2;

        m_y1_2 = m_y0_1;
        m_y1_1 = audio_out[1][i];

        m_x1_2 = m_x0_1;
        m_x1_1 = audio_in[1][i];
    }

}


#endif // _H_EFFECT_BIQUAD_CASCADE
