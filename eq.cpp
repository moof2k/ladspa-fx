
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LPF.h"

#include "ladspa.h"


enum {
    kPortInCutoff = 0,
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

    LPF m_effect;

    LADSPA_Data * m_pfInCutoff;
    LADSPA_Data * m_pfInAudio0;
    LADSPA_Data * m_pfInAudio1;
    LADSPA_Data * m_pfOutAudio0;
    LADSPA_Data * m_pfOutAudio1;

};

/*****************************************************************************/

/* Construct a new plugin instance. In this case, as the SimpleFilter
   structure can be used for low- or high-pass filters we can get away
   with only only writing one of these functions. Normally one would
   be required for each plugin type. */
LADSPA_Handle instantiateSimpleFilter(const LADSPA_Descriptor * Descriptor, unsigned long SampleRate)
{
    LPFEffectInstance *psFilter = new LPFEffectInstance(SampleRate);

    return psFilter;
}

/*****************************************************************************/

/* Initialise and activate a plugin instance. Normally separate
   functions would have to be written for the different plugin types,
   however we can get away with a single function in this case. */
void activateSimpleFilter(LADSPA_Handle Instance)
{
    LPFEffectInstance *psSimpleFilter = (LPFEffectInstance *) Instance;
    psSimpleFilter->m_effect.activate();
}

/*****************************************************************************/

/* Connect a port to a data location. Normally separate functions
   would have to be written for the different plugin types, however we
   can get away with a single function in this case. */
void connectPortToSimpleFilter(LADSPA_Handle Instance, unsigned long Port, LADSPA_Data * DataLocation)
{
    LPFEffectInstance *psFilter = (LPFEffectInstance *) Instance;

    switch (Port)
    {
        case kPortInCutoff:
            psFilter->m_pfInCutoff = DataLocation;
            break;
        case kPortInAudio0:
            psFilter->m_pfInAudio0 = DataLocation;
            break;
        case kPortInAudio1:
            psFilter->m_pfInAudio1 = DataLocation;
            break;
        case kPortOutAudio0:
            psFilter->m_pfOutAudio0 = DataLocation;
            break;
        case kPortOutAudio1:
            psFilter->m_pfOutAudio1 = DataLocation;
            break;
    }
}

/* Run the LPF algorithm for a block of SampleCount samples. */
void runSimpleLowPassFilter(LADSPA_Handle Instance, unsigned long SampleCount)
{
    LPFEffectInstance *psFilter = (LPFEffectInstance *) Instance;

    float *audio_in[2] = {
        psFilter->m_pfInAudio0,
        psFilter->m_pfInAudio1
    };

    float *audio_out[2] = {
        psFilter->m_pfOutAudio0,
        psFilter->m_pfOutAudio1
    };

    psFilter->m_effect.set_cutoff(*psFilter->m_pfInCutoff);

    psFilter->m_effect.run(audio_in, audio_out, SampleCount, 2);

}

/* Throw away a filter instance. Normally separate functions
   would have to be written for the different plugin types, however we
   can get away with a single function in this case. */
void cleanupSimpleFilter(LADSPA_Handle Instance)
{
    free(Instance);
}


void deleteDescriptor(LADSPA_Descriptor * psDescriptor)
{
    unsigned long lIndex;
    if (psDescriptor)
    {
        free((char *)psDescriptor->Label);
        free((char *)psDescriptor->Name);
        free((char *)psDescriptor->Maker);
        free((char *)psDescriptor->Copyright);
        free((LADSPA_PortDescriptor *)psDescriptor->PortDescriptors);
        for (lIndex = 0; lIndex < psDescriptor->PortCount; lIndex++)
            free((char *)(psDescriptor->PortNames[lIndex]));
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
          = 1041;
        g_psLPFDescriptor->Label
          = strdup("lpf");
        g_psLPFDescriptor->Properties
          = LADSPA_PROPERTY_HARD_RT_CAPABLE;
        g_psLPFDescriptor->Name 
          = strdup("Simple Low Pass Filter");
        g_psLPFDescriptor->Maker
          = strdup("Richard Furse (LADSPA example plugins)");
        g_psLPFDescriptor->Copyright
          = strdup("None");

        g_psLPFDescriptor->PortCount = 5;

        piPortDescriptors = (LADSPA_PortDescriptor *) calloc(5, sizeof(LADSPA_PortDescriptor));
        g_psLPFDescriptor->PortDescriptors = (const LADSPA_PortDescriptor *) piPortDescriptors;
        piPortDescriptors[kPortInCutoff] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[kPortInAudio0] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortInAudio1] = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortOutAudio0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[kPortOutAudio1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;

        pcPortNames = (char **) calloc(5, sizeof(char *));
        g_psLPFDescriptor->PortNames = (const char **) pcPortNames;
        pcPortNames[kPortInCutoff] = strdup("Cutoff Frequency (Hz)");
        pcPortNames[kPortInAudio0] = strdup("Input (Left)");
        pcPortNames[kPortInAudio1] = strdup("Input (Right)");
        pcPortNames[kPortOutAudio0] = strdup("Output (Left)");
        pcPortNames[kPortOutAudio1] = strdup("Output (Right)");

        psPortRangeHints = ((LADSPA_PortRangeHint *) calloc(5, sizeof(LADSPA_PortRangeHint)));
        g_psLPFDescriptor->PortRangeHints = (const LADSPA_PortRangeHint *) psPortRangeHints;

        psPortRangeHints[kPortInCutoff].HintDescriptor
            = (LADSPA_HINT_BOUNDED_BELOW 
            | LADSPA_HINT_BOUNDED_ABOVE
            | LADSPA_HINT_SAMPLE_RATE
            | LADSPA_HINT_LOGARITHMIC
            | LADSPA_HINT_DEFAULT_440);
        psPortRangeHints[kPortInCutoff].LowerBound = 0;
        psPortRangeHints[kPortInCutoff].UpperBound = 0.125; /* 1/8 the sample rate */

        psPortRangeHints[kPortInAudio0].HintDescriptor = 0;
        psPortRangeHints[kPortInAudio1].HintDescriptor = 0;
        psPortRangeHints[kPortOutAudio0].HintDescriptor = 0;
        psPortRangeHints[kPortOutAudio1].HintDescriptor = 0;

        g_psLPFDescriptor->instantiate = instantiateSimpleFilter;
        g_psLPFDescriptor->connect_port = connectPortToSimpleFilter;
        g_psLPFDescriptor->activate = activateSimpleFilter;
        g_psLPFDescriptor->run = runSimpleLowPassFilter;

        g_psLPFDescriptor->run_adding = NULL;
        g_psLPFDescriptor->set_run_adding_gain = NULL;
        g_psLPFDescriptor->deactivate = NULL;
        g_psLPFDescriptor->cleanup = cleanupSimpleFilter;
    }
}


/**
 * fini() is called automatically when the library is unloaded.
 */
extern "C" void _fini()
{
    deleteDescriptor(g_psLPFDescriptor);
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
