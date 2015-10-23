
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ladspa.h"

/*****************************************************************************/

#define SF_CUTOFF  0
#define SF_INPUT   1
#define SF_OUTPUT  2

/*****************************************************************************/

/* Instance data for the simple filter. We can get away with using
   this structure for both low- and high-pass filters because the data
   stored is the same. Note that the actual run() calls differ
   however. */
typedef struct {

    LADSPA_Data m_fSampleRate;
    LADSPA_Data m_fTwoPiOverSampleRate;

    LADSPA_Data m_fLastOutput;
    LADSPA_Data m_fLastCutoff;
    LADSPA_Data m_fAmountOfCurrent;
    LADSPA_Data m_fAmountOfLast;

    /* Ports:
     ------ */

    LADSPA_Data * m_pfCutoff;
    LADSPA_Data * m_pfInput;
    LADSPA_Data * m_pfOutput;

} SimpleFilter;

/*****************************************************************************/

/* Construct a new plugin instance. In this case, as the SimpleFilter
   structure can be used for low- or high-pass filters we can get away
   with only only writing one of these functions. Normally one would
   be required for each plugin type. */
LADSPA_Handle instantiateSimpleFilter(const LADSPA_Descriptor * Descriptor, unsigned long SampleRate)
{
    SimpleFilter * psFilter;

    psFilter = (SimpleFilter *)malloc(sizeof(SimpleFilter));

    if (psFilter)
    {
        psFilter->m_fSampleRate = (LADSPA_Data)SampleRate;
        psFilter->m_fTwoPiOverSampleRate = (2 * M_PI) / (LADSPA_Data)SampleRate;
        psFilter->m_fLastOutput = 0;
        psFilter->m_fLastCutoff = 0;
        psFilter->m_fAmountOfCurrent = 0;
        psFilter->m_fAmountOfLast = 0;
    }

    return psFilter;
}

/*****************************************************************************/

/* Initialise and activate a plugin instance. Normally separate
   functions would have to be written for the different plugin types,
   however we can get away with a single function in this case. */
void activateSimpleFilter(LADSPA_Handle Instance)
{
    SimpleFilter * psSimpleFilter;
    psSimpleFilter = (SimpleFilter *)Instance;
    psSimpleFilter->m_fLastOutput = 0;
}

/*****************************************************************************/

/* Connect a port to a data location. Normally separate functions
   would have to be written for the different plugin types, however we
   can get away with a single function in this case. */
void connectPortToSimpleFilter(LADSPA_Handle Instance,
              unsigned long Port,
              LADSPA_Data * DataLocation)
    {

    SimpleFilter * psFilter;

    psFilter = (SimpleFilter *)Instance;

    switch (Port)
    {
        case SF_CUTOFF:
        psFilter->m_pfCutoff = DataLocation;
        break;
    case SF_INPUT:
        psFilter->m_pfInput = DataLocation;
        break;
    case SF_OUTPUT:
        psFilter->m_pfOutput = DataLocation;
        break;
    }
}

/*****************************************************************************/

/* Run the LPF algorithm for a block of SampleCount samples. */
void runSimpleLowPassFilter(LADSPA_Handle Instance, unsigned long SampleCount)
{
    LADSPA_Data * pfInput;
    LADSPA_Data * pfOutput;
    LADSPA_Data fAmountOfCurrent;
    LADSPA_Data fAmountOfLast;
    LADSPA_Data fComp;
    LADSPA_Data fLastOutput;
    SimpleFilter * psFilter;
    unsigned long lSampleIndex;

    psFilter = (SimpleFilter *)Instance;

    pfInput = psFilter->m_pfInput;
    pfOutput = psFilter->m_pfOutput;

    if (*psFilter->m_pfCutoff != psFilter->m_fLastCutoff)
    {
        psFilter->m_fLastCutoff = *psFilter->m_pfCutoff;
        if (psFilter->m_fLastCutoff <= 0)
        {
            /* Reject everything. */
            psFilter->m_fAmountOfCurrent = psFilter->m_fAmountOfLast = 0;
        }
        else if (psFilter->m_fLastCutoff > psFilter->m_fSampleRate * 0.5)
        {
            /* Above Nyquist frequency. Let everything through. */
            psFilter->m_fAmountOfCurrent = 1;
            psFilter->m_fAmountOfLast = 0;
        }
        else
        {
            psFilter->m_fAmountOfLast = 0;
            fComp = 2 - cos(psFilter->m_fTwoPiOverSampleRate * psFilter->m_fLastCutoff);
            psFilter->m_fAmountOfLast = fComp - (LADSPA_Data) sqrt(fComp * fComp - 1);
            psFilter->m_fAmountOfCurrent = 1 - psFilter->m_fAmountOfLast;
        }
    }

    fAmountOfCurrent = psFilter->m_fAmountOfCurrent;
    fAmountOfLast = psFilter->m_fAmountOfLast;
    fLastOutput = psFilter->m_fLastOutput;

    for (lSampleIndex = 0; lSampleIndex < SampleCount; lSampleIndex++)
    {
        *(pfOutput++)
            = fLastOutput
            = (fAmountOfCurrent * *(pfInput++) + fAmountOfLast * fLastOutput);
    }

    psFilter->m_fLastOutput = fLastOutput;
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
        g_psLPFDescriptor->PortCount
          = 3;
        piPortDescriptors
          = (LADSPA_PortDescriptor *) calloc(3, sizeof(LADSPA_PortDescriptor));
        g_psLPFDescriptor->PortDescriptors
          = (const LADSPA_PortDescriptor *)piPortDescriptors;
        piPortDescriptors[SF_CUTOFF]
          = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
        piPortDescriptors[SF_INPUT]
          = LADSPA_PORT_INPUT | LADSPA_PORT_AUDIO;
        piPortDescriptors[SF_OUTPUT]
          = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;

        pcPortNames
          = (char **)calloc(3, sizeof(char *));
        g_psLPFDescriptor->PortNames 
          = (const char **)pcPortNames;
        pcPortNames[SF_CUTOFF]
          = strdup("Cutoff Frequency (Hz)");
        pcPortNames[SF_INPUT]
          = strdup("Input");
        pcPortNames[SF_OUTPUT]
          = strdup("Output");

        psPortRangeHints = ((LADSPA_PortRangeHint *)
                calloc(3, sizeof(LADSPA_PortRangeHint)));
        g_psLPFDescriptor->PortRangeHints
          = (const LADSPA_PortRangeHint *)psPortRangeHints;
        psPortRangeHints[SF_CUTOFF].HintDescriptor
          = (LADSPA_HINT_BOUNDED_BELOW 
         | LADSPA_HINT_BOUNDED_ABOVE
         | LADSPA_HINT_SAMPLE_RATE
         | LADSPA_HINT_LOGARITHMIC
         | LADSPA_HINT_DEFAULT_440);
        psPortRangeHints[SF_CUTOFF].LowerBound 
          = 0;
        psPortRangeHints[SF_CUTOFF].UpperBound
          = 0.5; /* Nyquist frequency (half the sample rate) */
        psPortRangeHints[SF_INPUT].HintDescriptor
          = 0;
        psPortRangeHints[SF_OUTPUT].HintDescriptor
          = 0;

        g_psLPFDescriptor->instantiate 
          = instantiateSimpleFilter;
        g_psLPFDescriptor->connect_port 
          = connectPortToSimpleFilter;
        g_psLPFDescriptor->activate
          = activateSimpleFilter;
        g_psLPFDescriptor->run
          = runSimpleLowPassFilter;

        g_psLPFDescriptor->run_adding
          = NULL;
        g_psLPFDescriptor->set_run_adding_gain
          = NULL;
        g_psLPFDescriptor->deactivate
          = NULL;
        g_psLPFDescriptor->cleanup
          = cleanupSimpleFilter;
    }

}


/* _fini() is called automatically when the library is unloaded. */
extern "C" void _fini()
{
    deleteDescriptor(g_psLPFDescriptor);
}

/* Return a descriptor of the requested plugin type. There are two
   plugin types available in this library. */
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
