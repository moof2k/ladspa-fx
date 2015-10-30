
#include "Effect.h"


class LPF : public Effect
{
public:
    LPF(unsigned long sample_rate)
    : m_sample_rate(sample_rate)
    , m_2pi_over_sample_rate((2 * M_PI) / (float) sample_rate)
    , m_last_output0(0)
    , m_last_output1(0)
    , m_last_cutoff(0)
    , m_amount_of_current(0)
    , m_amount_of_last(0)
    {
    }

    virtual ~LPF()
    {
    }

    void activate();

    void set_cutoff(float cutoff);

    void run(float **audio_in, float **audio_out, unsigned long samples, unsigned int channels);

private:

    float m_sample_rate;
    float m_2pi_over_sample_rate;

    float m_last_output0;
    float m_last_output1;

    float m_last_cutoff;
    float m_amount_of_current;
    float m_amount_of_last;
};

void LPF::activate()
{
    m_last_output0 = 0;
    m_last_output1 = 0;
}

void LPF::set_cutoff(float cutoff)
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

void LPF::run(float **audio_in, float **audio_out, unsigned long samples, unsigned int channels)
{
    for (unsigned long lSampleIndex = 0; lSampleIndex < samples; lSampleIndex++)
    {
        *(audio_out[0]++)
            = m_last_output0
            = (m_amount_of_current * *(audio_in[0]++) + m_amount_of_last * m_last_output0);
        *(audio_out[1]++)
            = m_last_output1
            = (m_amount_of_current * *(audio_in[1]++) + m_amount_of_last * m_last_output1);
    }

}
