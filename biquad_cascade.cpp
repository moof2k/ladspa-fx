/**
 * biquad.cpp
 *
 * Free software. No warranty.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EffectBiquadCascade.h"

#include "ladspa.h"


enum {
    kPortInType0 = 0,
    kPortInFreq0,
    kPortInQ0,
    kPortInType1,
    kPortInFreq1,
    kPortInQ1,
    kPortInAudio0,
    kPortInAudio1,
    kPortOutAudio0,
    kPortOutAudio1
};

class LPFEffectInstance
{
public:
    LPFEffectInstance(unsigned long sample_rate)
    : m_effect(sample_rate)
    {
    }

    EffectBiquadCascade m_effect;

    LADSPA_Data * m_pfInType0;
    LADSPA_Data * m_pfInFreq0;
    LADSPA_Data * m_pfInQ0;
    LADSPA_Data * m_pfInType1;
    LADSPA_Data * m_pfInFreq1;
    LADSPA_Data * m_pfInQ1;
    LADSPA_Data * m_pfInAudio0;
    LADSPA_Data * m_pfInAudio1;
    LADSPA_Data * m_pfOutAudio0;
    LADSPA_Data * m_pfOutAudio1;

};


LADSPA_Handle instantiate_filter(const LADSPA_Descriptor * descriptor, unsigned long sample_rate)
{
    LPFEffectInstance *psFilter = new LPFEffectInstance(sample_rate);

    return psFilter;
}


void activate_filter(LADSPA_Handle instance)
{
    LPFEffectInstance *psSimpleFilter = (LPFEffectInstance *) instance;
    psSimpleFilter->m_effect.activate();
}


/**
 * Connect a port to a data location.
 */
void connect_port(LADSPA_Handle instance, unsigned long port, LADSPA_Data * data_location)
{
    LPFEffectInstance *psFilter = (LPFEffectInstance *) instance;

    switch (port)
    {
        case kPortInType0:
            psFilter->m_pfInType0 = data_location;
            break;
        case kPortInFreq0:
            psFilter->m_pfInFreq0 = data_location;
            break;
        case kPortInQ0:
            psFilter->m_pfInQ0 = data_location;
            break;
        case kPortInType1:
            psFilter->m_pfInType1 = data_location;
            break;
        case kPortInFreq1:
            psFilter->m_pfInFreq1 = data_location;
            break;
        case kPortInQ1:
            psFilter->m_pfInQ1 = data_location;
            break;
        case kPortInAudio0:
            psFilter->m_pfInAudio0 = data_location;
            break;
        case kPortInAudio1:
            psFilter->m_pfInAudio1 = data_location;
            break;
        case kPortOutAudio0:
            psFilter->m_pfOutAudio0 = data_location;
            break;
        case kPortOutAudio1:
            psFilter->m_pfOutAudio1 = data_location;
            break;
    }
}

/**
 * Run the effect for a block of sample_count samples.
 */
void run_filter(LADSPA_Handle instance, unsigned long sample_count)
{
    LPFEffectInstance *psFilter = (LPFEffectInstance *) instance;

    const float *audio_in[2] = {
        psFilter->m_pfInAudio0,
        psFilter->m_pfInAudio1
    };

    float *audio_out[2] = {
        psFilter->m_pfOutAudio0,
        psFilter->m_pfOutAudio1
    };

    psFilter->m_effect.set_type(0, *psFilter->m_pfInType0);
    psFilter->m_effect.set_freq(0, *psFilter->m_pfInFreq0);
    psFilter->m_effect.set_q(0, *psFilter->m_pfInQ0);

    psFilter->m_effect.set_type(1, *psFilter->m_pfInType1);
    psFilter->m_effect.set_freq(1, *psFilter->m_pfInFreq1);
    psFilter->m_effect.set_q(1, *psFilter->m_pfInQ1);

    psFilter->m_effect.run(audio_in, audio_out, sample_count);

}

/**
 * Throw away a filter instance.
 */
void cleanup_filter(LADSPA_Handle instance)
{
    LPFEffectInstance *psSimpleFilter = (LPFEffectInstance *) instance;
    delete psSimpleFilter;
}


void delete_descriptor(LADSPA_Descriptor *psDescriptor)
{
    if (psDescriptor)
    {
        free((char *)psDescriptor->Label);
        free((char *)psDescriptor->Name);
        free((char *)psDescriptor->Maker);
        free((char *)psDescriptor->Copyright);
        free((LADSPA_PortDescriptor *) psDescriptor->PortDescriptors);
        for (unsigned long i = 0; i < psDescriptor->PortCount; i++)
        {
            free((char *)(psDescriptor->PortNames[i]));
        }
        free((char **)psDescriptor->PortNames);
        free((LADSPA_PortRangeHint *)psDescriptor->PortRangeHints);
        free(psDescriptor);
    }
}

LADSPA_Descriptor * g_psLPFDescriptor = NULL;

/**
 * _init() is called automatically when the plugin library is first loaded.
 */
extern "C" void _init()
{

    char ** pcPortNames;
    LADSPA_PortDescriptor * piPortDescriptors;
    LADSPA_PortRangeHint * psPortRangeHints;

    g_psLPFDescriptor = (LADSPA_Descriptor *) malloc(sizeof(LADSPA_Descriptor));

    if (g_psLPFDescriptor != NULL)
    {
        g_psLPFDescriptor->UniqueID
          = 1043;
        g_psLPFDescriptor->Label
          = strdup("biquad");
        g_psLPFDescriptor->Properties
          = LADSPA_PROPERTY_HARD_RT_CAPABLE;
        g_psLPFDescriptor->Name 
          = strdup("Biquad Cascade");
        g_psLPFDescriptor->Maker
          = strdup("moof2k");
        g_psLPFDescriptor->Copyright
          = strdup("None");

        g_psLPFDescriptor->PortCount = 10;

        piPortDescriptors = (LADSPA_PortDescriptor *) calloc(g_psLPFDescriptor->PortCount, sizeof(LADSPA_PortDescriptor));
        g_psLPFDescriptor->PortDescriptors = (const LADSPA_PortDescriptor *) piPortDescriptors;
        piPortDescriptors[kPortInType0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInFreq0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInQ0] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInType1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInFreq1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInQ1] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInAudio0] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortInAudio1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortOutAudio0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortOutAudio1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;

        pcPortNames = (char **) calloc(g_psLPFDescriptor->PortCount, sizeof(char *));
        g_psLPFDescriptor->PortNames = (const char **) pcPortNames;
        pcPortNames[kPortInType0] = strdup("Type 0");
        pcPortNames[kPortInFreq0] = strdup("Center Frequency 0 (Hz)");
        pcPortNames[kPortInQ0] = strdup("Resonance 0 (Q)");
        pcPortNames[kPortInType1] = strdup("Type 1");
        pcPortNames[kPortInFreq1] = strdup("Center Frequency 1 (Hz)");
        pcPortNames[kPortInQ1] = strdup("Resonance 1 (Q)");
        pcPortNames[kPortInAudio0] = strdup("Input (Left)");
        pcPortNames[kPortInAudio1] = strdup("Input (Right)");
        pcPortNames[kPortOutAudio0] = strdup("Output (Left)");
        pcPortNames[kPortOutAudio1] = strdup("Output (Right)");

        psPortRangeHints = ((LADSPA_PortRangeHint *) calloc(g_psLPFDescriptor->PortCount, sizeof(LADSPA_PortRangeHint)));
        g_psLPFDescriptor->PortRangeHints = (const LADSPA_PortRangeHint *) psPortRangeHints;

        psPortRangeHints[kPortInType0].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE
            | LADSPA_HINT_INTEGER);
        psPortRangeHints[kPortInType0].LowerBound = -0.1;
        psPortRangeHints[kPortInType0].UpperBound = 1.1;

        psPortRangeHints[kPortInFreq0].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE
            | LADSPA_HINT_SAMPLE_RATE
            | LADSPA_HINT_LOGARITHMIC
            | LADSPA_HINT_DEFAULT_440);
        psPortRangeHints[kPortInFreq0].LowerBound = 0;
        psPortRangeHints[kPortInFreq0].UpperBound = 0.125; /* 1/8 the sample rate */

        psPortRangeHints[kPortInQ0].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE);
        psPortRangeHints[kPortInQ0].LowerBound = 0.1;
        psPortRangeHints[kPortInQ0].UpperBound = 10;

        psPortRangeHints[kPortInType1].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE
            | LADSPA_HINT_INTEGER);
        psPortRangeHints[kPortInType1].LowerBound = -0.1;
        psPortRangeHints[kPortInType1].UpperBound = 1.1;

        psPortRangeHints[kPortInFreq1].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE
            | LADSPA_HINT_SAMPLE_RATE
            | LADSPA_HINT_LOGARITHMIC
            | LADSPA_HINT_DEFAULT_440);
        psPortRangeHints[kPortInFreq1].LowerBound = 0;
        psPortRangeHints[kPortInFreq1].UpperBound = 0.125; /* 1/8 the sample rate */

        psPortRangeHints[kPortInQ1].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE);
        psPortRangeHints[kPortInQ1].LowerBound = 0.1;
        psPortRangeHints[kPortInQ1].UpperBound = 10;

        psPortRangeHints[kPortInAudio0].HintDescriptor = 0;
        psPortRangeHints[kPortInAudio1].HintDescriptor = 0;
        psPortRangeHints[kPortOutAudio0].HintDescriptor = 0;
        psPortRangeHints[kPortOutAudio1].HintDescriptor = 0;

        g_psLPFDescriptor->instantiate = instantiate_filter;
        g_psLPFDescriptor->connect_port = connect_port;
        g_psLPFDescriptor->activate = activate_filter;
        g_psLPFDescriptor->run = run_filter;

        g_psLPFDescriptor->run_adding = NULL;
        g_psLPFDescriptor->set_run_adding_gain = NULL;
        g_psLPFDescriptor->deactivate = NULL;
        g_psLPFDescriptor->cleanup = cleanup_filter;
    }
}


/**
 * fini() is called automatically when the library is unloaded.
 */
extern "C" void _fini()
{
    delete_descriptor(g_psLPFDescriptor);
}

/**
 * Return a descriptor of the requested plugin type. There are two
 * plugin types available in this library.
 */
extern "C" const LADSPA_Descriptor * ladspa_descriptor(unsigned long Index)
{
    /* Return the requested descriptor or null if the index is out of range. */
    switch (Index)
    {
        case 0:
            return g_psLPFDescriptor;
        default:
            return NULL;
    }
}
