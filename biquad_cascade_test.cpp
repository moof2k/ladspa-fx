
#include <stdio.h>

#include <cassert>

#include "EffectBiquadCascade.h"

#include "sine.h"

int main(int argc, char **argv)
{
    const int kSampleRate = 48000;
    const unsigned long kNumSamples = 4096;

    EffectBiquadCascade effect(kSampleRate);

    /* Parse parameters */
    assert(argc == (3 * kNumBiquads + 1 + 1));

    int c = 1;
    for (int i = 0; i < kNumBiquads; i++)
    {
        int t = 0;
        sscanf(argv[c++], "%d", &t);

        float f = 1;
        sscanf(argv[c++], "%f", &f);

        float q = 1;
        sscanf(argv[c++], "%f", &q);

        effect.set_type(i, t);
        effect.set_freq(i, f);
        effect.set_q(i, q);
    }
    
    float freq = 1;
    sscanf(argv[c++], "%f", &freq);

    /* Initialize audio data */
    float audio_in0[kNumSamples] = {0.0f};
    float audio_in1[kNumSamples] = {0.0f};
    float *audio_in[2] = { audio_in0, audio_in1 };

    sine(audio_in, freq, kNumSamples, kSampleRate);

    float audio_out0[kNumSamples] = {0.0f};
    float audio_out1[kNumSamples] = {0.0f};
    float *audio_out[2] = { audio_out0, audio_out1 };

    /* Run test */
    effect.activate();
    effect.run((const float**) audio_in, audio_out, kNumSamples);

    /* Print results */
    for (unsigned long i = 0; i < kNumSamples; i++)
    {
        printf("%f %f %f %f\n", audio_in[0][i], audio_in[1][i], audio_out[0][i], audio_out[1][i]);
    }

    return 0;
}