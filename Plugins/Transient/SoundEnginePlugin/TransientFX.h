/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Version: <VERSION>  Build: <BUILDNUMBER>
  Copyright (c) <COPYRIGHTYEAR> Audiokinetic Inc.
*******************************************************************************/

#ifndef TransientFX_H
#define TransientFX_H

#include "TransientFXParams.h"

#define PI64    3.14159265358979323846

/// See https://www.audiokinetic.com/library/edge/?source=SDK&id=soundengine__plugins__effects.html
/// for the documentation about effect plug-ins
class TransientFX
    : public AK::IAkInPlaceEffectPlugin
{
public:
    TransientFX();
    ~TransientFX();

    /// Plug-in initialization.
    /// Prepares the plug-in for data processing, allocates memory and sets up the initial conditions.
    AKRESULT Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat);

    /// Release the resources upon termination of the plug-in.
    AKRESULT Term(AK::IAkPluginMemAlloc* in_pAllocator);

    /// The reset action should perform any actions required to reinitialize the
    /// state of the plug-in to its original state (e.g. after Init() or on effect bypass).
    AKRESULT Reset();

    /// Plug-in information query mechanism used when the sound engine requires
    /// information about the plug-in to determine its behavior.
    AKRESULT GetPluginInfo(AkPluginInfo& out_rPluginInfo);

    // Apply transient shaping by first detecting the amplitude envelope and rescaling the output sample by the new difference envelope
    AkReal32 DetectEnvelopes(AkReal32 InputSample);

    // Soft clipping saturator based on: https://github.com/ccrma/chugins/tree/master/ABSaturator
    AkReal32 ApplySaturation(AkReal32 InputSample, AkReal32 Drive, AkReal32 DCOffset);

    // Analog to digital bilinear transform for saturation FX
    void TransformBilinear(AkReal32 *ACoEffs, AkReal32 *DCoEffs, AkReal32 SampleRate);

    // Biquad IIR filter: https://ccrma.stanford.edu/~jos/fp/Direct_Form_II.html
    AkReal32 BiQuadCompute(AkReal32 *CoEffs, AkReal32 InputSample);

    /// Effect plug-in DSP execution.
    void Execute(AkAudioBuffer* io_pBuffer);

    /// Skips execution of some frames, when the voice is virtual playing from elapsed time.
    /// This can be used to simulate processing that would have taken place (e.g. update internal state).
    /// Return AK_DataReady or AK_NoMoreData, depending if there would be audio output or not at that point.
    AKRESULT TimeSkip(AkUInt32 in_uFrames);

private:
    TransientFXParams* m_pParams;
    AK::IAkPluginMemAlloc* m_pAllocator;
    AK::IAkEffectPluginContext* m_pContext;

    // Transient
    // Detection Parameters
    AkReal32 Attack;
    AkReal32 AttackSensitivity; // Reference amplitude - 0.9991 default
    AkReal32 AttackFeedback; // Used to store previous envelope value

    AkReal32 Sustain;
    AkReal32 SustainSensitivity; // Reference amplitude - 0.9999 default
    AkReal32 SustainFeedback; // Used to store previous envelope value    

    // Detection Envelopes
    AkReal32 EnvelopeFast;
    AkReal32 EnvelopeSlow;
    AkReal32 EnvelopeDifference;

    // Applied Envelopes
    AkReal32 EnvelopeAttack;
    AkReal32 EnvelopeSustain;    

    // Saturation
    // Storage for removal of FC
    AkReal32 DCBlocker[2][7];

    // Analog input and output storage
    AkReal32 AIn[6][7];
    AkReal32 AOut[6][7];    
};

#endif // TransientFX_H
