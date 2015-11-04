
#include <stdio.h>

#include <cassert>

#include "EffectBiquad.h"

void sine(float **audio_out, float freq, unsigned long samples, int sample_rate)
{
    float w = 2.0f * M_PI * freq / ((float) sample_rate);

    for (unsigned long i = 0; i < samples; i++)
    {
        audio_out[0][i] = sin(((float) i) * w);
        audio_out[1][i] = sin(((float) i) * w);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 3);

    int freq = 1;
    sscanf(argv[1], "%d", &freq);

    int f0 = 1;
    sscanf(argv[2], "%d", &f0);

    const int kSampleRate = 48000;
    const unsigned long kNumSamples = 4096;

    float audio_in0[kNumSamples] = {0.0f};
    float audio_in1[kNumSamples] = {0.0f};
    float *audio_in[2] = { audio_in0, audio_in1 };

    sine(audio_in, (float) freq, kNumSamples, kSampleRate);

    float audio_out0[kNumSamples] = {0.0f};
    float audio_out1[kNumSamples] = {0.0f};
    float *audio_out[2] = { audio_out0, audio_out1 };

    EffectBiquad effect(kSampleRate);
    effect.set_cutoff((float) f0);
    effect.set_q(1.0f);
    effect.activate();
    effect.run((const float**) audio_in, audio_out, kNumSamples);

    for (unsigned long i = 0; i < kNumSamples; i++)
    {
        printf("%f %f %f %f\n", audio_in[0][i], audio_in[1][i], audio_out[0][i], audio_out[1][i]);
    }

    return 0;
}