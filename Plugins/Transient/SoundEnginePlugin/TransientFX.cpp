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

#include "TransientFX.h"
#include "../TransientConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateTransientFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, TransientFX());
}

AK::IAkPluginParam* CreateTransientFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, TransientFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(TransientFX, AkPluginTypeEffect, TransientConfig::CompanyID, TransientConfig::PluginID)

TransientFX::TransientFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

TransientFX::~TransientFX()
{
}


void TransientFX::TransformBilinear(AkReal32 *ACoEffs, AkReal32 *DCoEffs, AkReal32 SampleRate)
{
    // Initialise
    
    // Analog storage
    AkReal32 B0     = 0.0f;
    AkReal32 b1     = 0.0f; 
    AkReal32 b2     = 0.0f; 
    AkReal32 a0     = 0.0f; 
    AkReal32 a1     = 0.0f; 
    AkReal32 a2     = 0.0f;
    
    // Digital Storage
    AkReal32 bz0    = 0.0f;
    AkReal32 bz1    = 0.0f; 
    AkReal32 bz2    = 0.0f; 
    AkReal32 az0    = 0.0f; 
    AkReal32 az1    = 0.0f; 
    AkReal32 az2    = 0.0f;

    // Get filter coeffecients
    B0              = ACoEffs[0]; 
    b1              = ACoEffs[1]; 
    b2              = ACoEffs[2];
    a0              = ACoEffs[3]; 
    a1              = ACoEffs[4]; 
    a2              = ACoEffs[5];

    bz0             = 1.0; 
    bz1             = 0.0; 
    bz2             = 0.0;
    az0             = 1.0; 
    az1             = 0.0; 
    az2             = 0.0;

    az0             = a2 * 4 * SampleRate * SampleRate + a1 * 2 * SampleRate + a0;
    bz2             = (b2 * 4 * SampleRate * SampleRate - b1 * 2 * SampleRate + B0) / az0;
    bz1             = (-b2 * 8 * SampleRate * SampleRate + 2 * B0) / az0;
    bz0             = (b2 * 4 * SampleRate * SampleRate + b1 * 2 * SampleRate + B0) / az0;
    az2             = (a2 * 4 * SampleRate * SampleRate - a1 * 2 * SampleRate + a0) / az0;
    az1             = (-a2 * 8 * SampleRate * SampleRate + 2 * a0) / az0;

    // Set filter coeffecients
    DCoEffs[0]      = bz0; 
    DCoEffs[1]      = bz1; 
    DCoEffs[2]      = bz2;
    DCoEffs[3]      = az1;
    DCoEffs[4]      = az2;
}

AKRESULT TransientFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (TransientFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    // Initialise all to 0.0
    AkReal32 Attack             = 0.0f;
    AkReal32 AttackSensitvity   = 0.0f;
    AkReal32 AttackFeedback     = 0.0f;
    AkReal32 Sustain            = 0.0f;
    AkReal32 SustainSensitvity  = 0.0f;
    AkReal32 SustainFeedback    = 0.0f;  
    AkReal32 EnvelopeFast       = 0.0f;
    AkReal32 EnvelopeSlow       = 0.0f;
    AkReal32 EnvelopeDifference = 0.0f;
    AkReal32 EnvelopeAttack     = 0.0f;
    AkReal32 EnvelopeSustain    = 0.0f;    

    // Saturation properties
    // Filter constants
    AkReal32 Chebyshev2[6][7] =
    {
        {2.60687e-05, 2.98697e-05, 2.60687e-05, -1.31885, 0.437162, 0.0, 0.0},
        {1, -0.800256, 1, -1.38301, 0.496576, 0.0, 0.0},
        {1, -1.42083, 1, -1.48787, 0.594413, 0.0, 0.0},
        {1, -1.6374, 1, -1.60688, 0.707142, 0.0, 0.0},
        {1, -1.7261, 1, -1.7253, 0.822156, 0.0, 0.0},
        {1, -1.75999, 1, -1.84111, 0.938811, 0.0, 0.0}
    };

    // Copy filter coeffeccients
    for(size_t i = 0; i < 6; i++)
    {
        for(size_t j = 0; j < 7; j++)
        {
            AIn[i][j] = Chebyshev2[i][j];
            AOut[i][j] = Chebyshev2[i][j];
        }
    }

    // Bilinear
    AkReal32 DC = 5 * 2 * PI64;
    AkReal32 AnalogCoeffs[6] = {0, 1, 0, DC, 1, 0 };
    AkReal32 DigitalCoeffs[5];    
    TransformBilinear(AnalogCoeffs, DigitalCoeffs, (in_rFormat.uSampleRate * 8));
    
    // Set DC
    for(size_t i = 0; i < 2; i++)
    {
        for(size_t j = 0; j < 5; j++)
        {
            DCBlocker[i][j] = DigitalCoeffs[j];
        }
        
        DCBlocker[i][5] = 0.0;
        DCBlocker[i][6] = 0.0;
    }

    return AK_Success;
}

AKRESULT TransientFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT TransientFX::Reset()
{
    return AK_Success;
}

AKRESULT TransientFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

AkReal32 TransientFX::DetectEnvelopes(AkReal32 InputSample)
{
    AkReal32 Result = 0.0f;

    // Detection Phase
    // Fast
    EnvelopeFast = (1 - m_pParams->RTPC.AttackSensitivity) * 2 * fabsf(InputSample) + m_pParams->RTPC.AttackSensitivity * AttackFeedback;
    AttackFeedback = EnvelopeFast; // Store last value

    // Slow
    EnvelopeSlow = (1 - m_pParams->RTPC.SustainSensitivity) * 3 * fabsf(InputSample) + m_pParams->RTPC.SustainSensitivity * SustainFeedback;
    SustainFeedback = EnvelopeSlow; // Store last value       

    // Find difference
    EnvelopeDifference = EnvelopeFast - EnvelopeSlow;

    // Difference is positive, thus in the attack phase
    if(EnvelopeDifference > 0)
    {
        EnvelopeAttack = (m_pParams->RTPC.Attack * EnvelopeDifference) + 1;
        EnvelopeSustain = 1;
    }
    else
    {
        EnvelopeAttack = 1;
        EnvelopeSustain = (m_pParams->RTPC.Sustain * -EnvelopeDifference) + 1;                
    }

    // Scale the output
    Result = (InputSample * EnvelopeAttack) * EnvelopeSustain;

    return Result;
}

AkReal32 TransientFX::BiQuadCompute(AkReal32 *CoEffs, AkReal32 InputSample)
{
    AkReal32 Result = 0.0f;
    
    // Difference equation
    Result      = CoEffs[5] + InputSample * CoEffs[0];
    CoEffs[5]   = CoEffs[6] + InputSample * CoEffs[1] - Result * CoEffs[3];
    CoEffs[6]   = InputSample * CoEffs[2] - Result * CoEffs[4];;

    return Result;
}

AkReal32 TransientFX::ApplySaturation(AkReal32 InputSample, AkReal32 Drive, AkReal32 DCOffset)
{
    AkReal32 Result = 0.0f;

    AkReal32 fsignal, usignal, dsignal;

    fsignal = Drive * InputSample;
    for(size_t i = 0; i < 8; i++)
    {
        usignal = (i == 0.0f) ? 8.0f * fsignal : 0.0;
        for(size_t j = 0; j < 6; j++)
        {
            usignal = BiQuadCompute(AIn[j], usignal);
        }

        dsignal = (usignal + DCOffset) / (1.0 + fabsf(usignal + DCOffset));

        dsignal = BiQuadCompute(DCBlocker[0], dsignal);
        dsignal = BiQuadCompute(DCBlocker[1], dsignal);

        // Anti-aliasing
        for(size_t k = 0; k < 6; k++)
        {
            Result = BiQuadCompute(AOut[k], dsignal);
        }
    }

    return Result;    
}

void TransientFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    // Apply RTPCs
    //! Convert sensitivities to more readable scales (%)
    Attack              = m_pParams->RTPC.Attack;
    AttackSensitivity   = m_pParams->RTPC.AttackSensitivity;
    Sustain             = m_pParams->RTPC.Sustain;
    SustainSensitivity  = m_pParams->RTPC.SustainSensitivity;

    // Check for clipping
    bool IsClipping = false;
    if(m_pParams->RTPC.ClippingEnable == 1.0)
    {
        IsClipping = true;
    }

    // Convert to linear gain
    AkReal32 DryWetAmplitude = (m_pParams->RTPC.DryWetMix / 100);

    if(IsClipping)
    {
        AkUInt16 uFramesProcessed;
        for(AkUInt32 i = 0; i < uNumChannels; ++i)
        {
            AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

            uFramesProcessed = 0;
            while (uFramesProcessed < io_pBuffer->uValidFrames)
            {
                // Execute DSP in-place here
                // Get sample
                AkReal32 CurrentSample = pBuf[uFramesProcessed];

                // Apply transient shaping
                AkReal32 TransientSample = DetectEnvelopes(CurrentSample);

                // Apply clipping
                AkReal32 ClippedSample = ApplySaturation(TransientSample, m_pParams->RTPC.Drive, m_pParams->RTPC.DCOffset);

                // Apply Dry/Wet mix
                AkReal32 OutSample = (DryWetAmplitude * ClippedSample + (1 - DryWetAmplitude) * CurrentSample);

                // Copy sample back
                pBuf[uFramesProcessed] = OutSample;

                // Increment buffer
                ++uFramesProcessed;
            }
        }
    }
    else
    {
        AkUInt16 uFramesProcessed;
        for(AkUInt32 i = 0; i < uNumChannels; ++i)
        {
            AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

            uFramesProcessed = 0;
            while (uFramesProcessed < io_pBuffer->uValidFrames)
            {
                // Execute DSP in-place here
                // Get sample
                AkReal32 CurrentSample = pBuf[uFramesProcessed];

                // Apply transient shaping
                AkReal32 TransientSample = DetectEnvelopes(CurrentSample);

                // Apply Dry/Wet mix
                AkReal32 OutSample = (DryWetAmplitude * TransientSample + (1 - DryWetAmplitude) * CurrentSample);

                // Copy sample back
                pBuf[uFramesProcessed] = OutSample;

                // Increment buffer
                ++uFramesProcessed;
            }
        }        
    }
}

AKRESULT TransientFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
