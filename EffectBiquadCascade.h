
#ifndef _H_EFFECT_BIQUAD_CASCADE
#define _H_EFFECT_BIQUAD_CASCADE

#include <math.h>

#include "Effect.h"

enum
{
    kBiquadTypePassthru = 0,
    kBiquadTypeLowPassFilter,
    kBiquadTypeHighPassFilter,
    kNumBiquadTypes
};

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
    , m_type(kBiquadTypeLowPassFilter)
    {}

    float m_b0;
    float m_b1;
    float m_b2;
    float m_a0;
    float m_a1;
    float m_a2;

    float m_freq;
    float m_q;
    int m_type;
};

const int kNumBiquads = 2;

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

    void set_type(int biquad, int type);
    void set_freq(int biquad, float freq);
    void set_q(int biquad, float q);

    void run(const float *audio_in[2], float *audio_out[2], unsigned long samples);

private:

    void update_parameters();

    float m_sample_rate;
    float m_2pi_over_sample_rate;

    float m_x0_0[kNumBiquads + 1];
    float m_x0_1[kNumBiquads + 1];
    float m_x0_2[kNumBiquads + 1];

    float m_x1_0[kNumBiquads + 1];
    float m_x1_1[kNumBiquads + 1];
    float m_x1_2[kNumBiquads + 1];

    BiquadCoeffs m_coeffs[kNumBiquads];
};

void EffectBiquadCascade::activate()
{
    for (int i = 0; i < kNumBiquads + 1; i++)
    {
        m_x0_0[i] = 0.0f;
        m_x0_1[i] = 0.0f;
        m_x0_2[i] = 0.0f;
        m_x1_0[i] = 0.0f;
        m_x1_1[i] = 0.0f;
        m_x1_2[i] = 0.0f;
    }
}

void EffectBiquadCascade::set_type(int biquad, int type)
{
    if (biquad < 0 || biquad >= kNumBiquads)
        return;

    if (type < 0)
        type = 0;

    if (type >= kNumBiquadTypes)
        type = kNumBiquadTypes - 1;

    if (type != m_coeffs[biquad].m_type)
    {
        m_coeffs[biquad].m_type = type;
        update_parameters();
    }
}

void EffectBiquadCascade::set_freq(int biquad, float freq)
{
    if (biquad < 0 || biquad >= kNumBiquads)
        return;

    if (freq < 0.0f)
        freq = 0.0f;

    if (freq != m_coeffs[biquad].m_freq)
    {
        m_coeffs[biquad].m_freq = freq;
        update_parameters();
    }
}

void EffectBiquadCascade::set_q(int biquad, float q)
{
    if (biquad < 0 || biquad >= kNumBiquads)
        return;

    if (q < 0.0f)
        q = 0.0f;

    if (q != m_coeffs[biquad].m_q)
    {
        m_coeffs[biquad].m_q = q;
        update_parameters();
    }
}

void EffectBiquadCascade::update_parameters()
{
    for (int b = 0; b < kNumBiquads; b++)
    {
        if (m_coeffs[b].m_freq <= 0)
        {
            /* Reject everything. */
            m_coeffs[b].m_b0 = 0.0f;
            m_coeffs[b].m_b1 = 0.0f;
            m_coeffs[b].m_b2 = 0.0f;
            m_coeffs[b].m_a0 = 0.0f;
            m_coeffs[b].m_a1 = 0.0f;
            m_coeffs[b].m_a2 = 0.0f;
        }
        else if (m_coeffs[b].m_freq > m_sample_rate * 0.5)
        {
            /* Above Nyquist frequency. Let everything through. */
            m_coeffs[b].m_b0 = 1.0f;
            m_coeffs[b].m_b1 = 0.0f;
            m_coeffs[b].m_b2 = 0.0f;
            m_coeffs[b].m_a0 = 0.0f;
            m_coeffs[b].m_a1 = 0.0f;
            m_coeffs[b].m_a2 = 0.0f;
        }
        else
        {
            /* http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt */
            
            float w = m_2pi_over_sample_rate * m_coeffs[b].m_freq;
            float cosw = cos(w);
            float sinw = sin(w);
            float alpha = sinw / (2.0f * m_coeffs[b].m_q);

            switch (m_coeffs[b].m_type)
            {
                case kBiquadTypeLowPassFilter:
                    m_coeffs[b].m_b0 = (1.0f - cosw) / 2.0f;
                    m_coeffs[b].m_b1 = 1.0f - cosw;
                    m_coeffs[b].m_b2 = (1.0f - cosw) / 2.0f;
                    m_coeffs[b].m_a0 = 1.0f + alpha;
                    m_coeffs[b].m_a1 = -2.0f * cosw;
                    m_coeffs[b].m_a2 = 1.0f - alpha;
                    break;
                case kBiquadTypeHighPassFilter:
                    m_coeffs[b].m_b0 = (1.0f - cosw) / 2.0f;
                    m_coeffs[b].m_b1 = -(1.0f - cosw);
                    m_coeffs[b].m_b2 = (1.0f - cosw) / 2.0f;
                    m_coeffs[b].m_a0 = 1.0f + alpha;
                    m_coeffs[b].m_a1 = -2.0f * cosw;
                    m_coeffs[b].m_a2 = 1.0f - alpha;
                    break;
                case kBiquadTypePassthru:
                default:
                    m_coeffs[b].m_b0 = 1.0f;
                    m_coeffs[b].m_b1 = 0.0f;
                    m_coeffs[b].m_b2 = 0.0f;
                    m_coeffs[b].m_a0 = 1.0f;
                    m_coeffs[b].m_a1 = 0.0f;
                    m_coeffs[b].m_a2 = 0.0f;
                    break;
            }

            m_coeffs[b].m_b0 /= m_coeffs[b].m_a0;
            m_coeffs[b].m_b1 /= m_coeffs[b].m_a0;
            m_coeffs[b].m_b2 /= m_coeffs[b].m_a0;
            m_coeffs[b].m_a1 /= m_coeffs[b].m_a0;
            m_coeffs[b].m_a2 /= m_coeffs[b].m_a0;
        }
    }
}

void EffectBiquadCascade::run(const float *audio_in[2], float *audio_out[2], unsigned long samples)
{
    const float *in[2] = { audio_in[0], audio_in[1] };

    for (unsigned long i = 0; i < samples; i++)
    {
        m_x0_0[0] = in[0][i];
        m_x1_0[0] = in[1][i];

        for (int b = 0; b < kNumBiquads; b++)
        {
            /* Left channel (0) */
            m_x0_0[b+1] = m_coeffs[b].m_b0 * m_x0_0[b] + m_coeffs[b].m_b1 * m_x0_1[b] + m_coeffs[b].m_b2 * m_x0_2[b]
                - m_coeffs[b].m_a1 * m_x0_1[b+1] - m_coeffs[b].m_a2 * m_x0_2[b+1];

            m_x0_2[b] = m_x0_1[b];
            m_x0_1[b] = m_x0_0[b];

            /* Right channel (1) */
            m_x1_0[b+1] = m_coeffs[b].m_b0 * m_x1_0[b] + m_coeffs[b].m_b1 * m_x1_1[b] + m_coeffs[b].m_b2 * m_x1_2[b]
                - m_coeffs[b].m_a1 * m_x1_1[b+1] - m_coeffs[b].m_a2 * m_x1_2[b+1];

            m_x1_2[b] = m_x1_1[b];
            m_x1_1[b] = m_x1_0[b];
        }

        m_x0_2[kNumBiquads] = m_x0_1[kNumBiquads];
        m_x0_1[kNumBiquads] = m_x0_0[kNumBiquads];

        m_x1_2[kNumBiquads] = m_x1_1[kNumBiquads];
        m_x1_1[kNumBiquads] = m_x1_0[kNumBiquads];

        audio_out[0][i] = m_x0_0[kNumBiquads];
        audio_out[1][i] = m_x1_0[kNumBiquads];
    }

}


#endif // _H_EFFECT_BIQUAD_CASCADE
