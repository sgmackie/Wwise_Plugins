#include <AK/SoundEngine/Common/IAkPlugin.h>
#include "WavetableArrays.h"

#define PI32 3.14159265359f
#define TWO_PI32 (2.0 * PI32)

typedef struct WAVETABLE
{
    AkReal64 FrequencyLimit;
    AkUInt32 Length;
    AkReal32 *Samples;
} WAVETABLE;


typedef struct ADSR
{
    bool IsActive;
    AkReal32 MaxAmplitude;
    AkReal32 Attack;
    AkReal32 Decay;
    AkReal32 SustainAmplitude;
    AkReal32 Release;
    AkUInt64 Index;
    AkUInt64 DurationInSamples;
} ADSR;

#define MAX_WAVETABLES  16
#define MAX_PRESETS     4
typedef struct WAVETABLE_OSCILLATOR
{
    AkReal64 PhaseCurrent;
    AkReal64 PhaseIncrement;
    AkReal64 PhaseOffset;
    AkUInt32 WavetableCount;
    WAVETABLE Tables[MAX_WAVETABLES];
    ADSR *Envelope;
} WAVETABLE_OSCILLATOR;
