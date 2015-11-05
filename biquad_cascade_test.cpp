
#include <stdio.h>

#include <cassert>

#include "EffectBiquadCascade.h"

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
    assert(argc == 5);

    int t0 = 0;
    sscanf(argv[1], "%d", &t0);

    float f0 = 1;
    sscanf(argv[2], "%f", &f0);

    float q = 1;
    sscanf(argv[3], "%f", &q);

    float freq = 1;
    sscanf(argv[4], "%f", &freq);

    const int kSampleRate = 48000;
    const unsigned long kNumSamples = 4096;

    float audio_in0[kNumSamples] = {0.0f};
    float audio_in1[kNumSamples] = {0.0f};
    float *audio_in[2] = { audio_in0, audio_in1 };

    sine(audio_in, freq, kNumSamples, kSampleRate);

    float audio_out0[kNumSamples] = {0.0f};
    float audio_out1[kNumSamples] = {0.0f};
    float *audio_out[2] = { audio_out0, audio_out1 };

    EffectBiquadCascade effect(kSampleRate);
    effect.set_type(0, t0);
    effect.set_freq(0, f0);
    effect.set_q(0, q);
    effect.set_type(1, kBiquadTypeHighPassFilter);
    effect.set_freq(1, f0);
    effect.set_q(1, q);
    effect.activate();
    effect.run((const float**) audio_in, audio_out, kNumSamples);

    for (unsigned long i = 0; i < kNumSamples; i++)
    {
        printf("%f %f %f %f\n", audio_in[0][i], audio_in[1][i], audio_out[0][i], audio_out[1][i]);
    }

    return 0;
}