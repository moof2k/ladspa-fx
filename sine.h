
#ifndef _H_SINE
#define _H_SINE

void sine(float **audio_out, float freq, unsigned long samples, int sample_rate)
{
    float w = 2.0f * M_PI * freq / ((float) sample_rate);

    for (unsigned long i = 0; i < samples; i++)
    {
        audio_out[0][i] = sin(((float) i) * w);
        audio_out[1][i] = sin(((float) i) * w);
    }
}

#endif
