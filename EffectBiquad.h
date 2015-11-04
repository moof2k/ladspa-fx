
#ifndef _H_EFFECT_BIQUAD
#define _H_EFFECT_BIQUAD

#include <math.h>

#include "Effect.h"

class EffectBiquad : public Effect
{
public:
    EffectBiquad(unsigned long sample_rate)
    : m_sample_rate(sample_rate)
    , m_2pi_over_sample_rate((2 * M_PI) / (float) sample_rate)
    , m_b0(0.0f)
    , m_b1(0.0f)
    , m_b2(0.0f)
    , m_a0(0.0f)
    , m_a1(0.0f)
    , m_a2(0.0f)
    , m_cutoff(0.0f)
    , m_q(1.0f)
    {
    }

    virtual ~EffectBiquad()
    {
    }

    void activate();

    void set_cutoff(float cutoff);
    void set_q(float q);

    void run(const float *audio_in[2], float *audio_out[2], unsigned long samples);

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

    float m_b0;
    float m_b1;
    float m_b2;
    float m_a0;
    float m_a1;
    float m_a2;

    float m_cutoff;
    float m_q;

};

void EffectBiquad::activate()
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

void EffectBiquad::set_cutoff(float cutoff)
{
    if (cutoff != m_cutoff)
    {
        m_cutoff = cutoff;
        update_parameters();
    }
}

void EffectBiquad::set_q(float q)
{
    if (q != m_q)
    {
        m_q = q;
        update_parameters();
    }
}

void EffectBiquad::update_parameters()
{
    if (m_cutoff <= 0)
    {
        /* Reject everything. */
        m_b0 = 0.0f;
        m_b1 = 0.0f;
        m_b2 = 0.0f;
        m_a0 = 0.0f;
        m_a1 = 0.0f;
        m_a2 = 0.0f;
    }
    else if (m_cutoff > m_sample_rate * 0.5)
    {
        /* Above Nyquist frequency. Let everything through. */
        m_b0 = 1.0f;
        m_b1 = 0.0f;
        m_b2 = 0.0f;
        m_a0 = 0.0f;
        m_a1 = 0.0f;
        m_a2 = 0.0f;
    }
    else
    {
        float w = m_2pi_over_sample_rate * m_cutoff;
        float cosw = cos(w);
        float sinw = sin(w);
        float alpha = sinw / (2.0f * m_q);

        /* Low pass filter */
        m_b0 = (1.0f - cosw) / 2.0f;
        m_b1 = 1.0f - cosw;
        m_b2 = (1.0f - cosw) / 2.0f;
        m_a0 = 1.0f + alpha;
        m_a1 = -2.0f * cosw;
        m_a2 = 1.0f - alpha;

        m_b0 /= m_a0;
        m_b1 /= m_a0;
        m_b2 /= m_a0;
        m_a1 /= m_a0;
        m_a2 /= m_a0;
    }
}

void EffectBiquad::run(const float *audio_in[2], float *audio_out[2], unsigned long samples)
{
    const float *in[2] = { audio_in[0], audio_in[1] };
    float *out[2] = { audio_out[0], audio_out[1] };

    for (unsigned long i = 0; i < samples; i++)
    {
        out[0][i] = m_b0 * in[0][i] + m_b1 * m_x0_1 + m_b2 * m_x0_2 - m_a1 * m_y0_1 - m_a2 * m_y0_2;

        m_y0_2 = m_y0_1;
        m_y0_1 = audio_out[0][i];

        m_x0_2 = m_x0_1;
        m_x0_1 = audio_in[0][i];

        out[1][i] = m_b0 * in[1][i] + m_b1 * m_x0_1 + m_b2 * m_x0_2 - m_a1 * m_y0_1 - m_a2 * m_y0_2;

        m_y1_2 = m_y0_1;
        m_y1_1 = audio_out[1][i];

        m_x1_2 = m_x0_1;
        m_x1_1 = audio_in[1][i];
    }

}


#endif // _H_EFFECT_BIQUAD
