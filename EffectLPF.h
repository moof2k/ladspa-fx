
#ifndef _H_EFFECT_LPF
#define _H_EFFECT_LPF

#include <math.h>

#include "Effect.h"


class EffectLPF : public Effect
{
public:
    EffectLPF(unsigned long sample_rate)
    : m_sample_rate(sample_rate)
    , m_2pi_over_sample_rate((2 * M_PI) / (float) sample_rate)
    , m_last_output0(0)
    , m_last_output1(0)
    , m_last_cutoff(0)
    , m_amount_of_current(0)
    , m_amount_of_last(0)
    {
    }

    virtual ~EffectLPF()
    {
    }

    void activate();

    void set_cutoff(float cutoff);

    void run(const float *audio_in[2], float *audio_out[2], unsigned long samples);

private:

    float m_sample_rate;
    float m_2pi_over_sample_rate;

    float m_last_output0;
    float m_last_output1;

    float m_last_cutoff;
    float m_amount_of_current;
    float m_amount_of_last;
};

void EffectLPF::activate()
{
    m_last_output0 = 0;
    m_last_output1 = 0;
}

void EffectLPF::set_cutoff(float cutoff)
{
    if (cutoff != m_last_cutoff)
    {
        m_last_cutoff = cutoff;

        if (m_last_cutoff <= 0)
        {
            /* Reject everything. */
            m_amount_of_current = m_amount_of_last = 0;
        }
        else if (m_last_cutoff > m_sample_rate * 0.5)
        {
            /* Above Nyquist frequency. Let everything through. */
            m_amount_of_current = 1;
            m_amount_of_last = 0;
        }
        else
        {
            m_amount_of_last = 0;
            float fComp = 2 - cos(m_2pi_over_sample_rate * m_last_cutoff);
            m_amount_of_last = fComp - sqrt(fComp * fComp - 1);
            m_amount_of_current = 1 - m_amount_of_last;
        }
    }
}

void EffectLPF::run(const float *audio_in[2], float *audio_out[2], unsigned long samples)
{
    const float *in[2] = { audio_in[0], audio_in[1] };
    float *out[2] = { audio_out[0], audio_out[1] };

    for (unsigned long i = 0; i < samples; i++)
    {
        *(out[0]++)
            = m_last_output0
            = (m_amount_of_current * *(in[0]++) + m_amount_of_last * m_last_output0);
            
        *(out[1]++)
            = m_last_output1
            = (m_amount_of_current * *(in[1]++) + m_amount_of_last * m_last_output1);
    }

}


#endif // _H_EFFECT_LPF
