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

#include "BitcrushFX.h"
#include "../BitcrushConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateBitcrushFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, BitcrushFX());
}

AK::IAkPluginParam* CreateBitcrushFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, BitcrushFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(BitcrushFX, AkPluginTypeEffect, BitcrushConfig::CompanyID, BitcrushConfig::PluginID)

BitcrushFX::BitcrushFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

BitcrushFX::~BitcrushFX()
{
}

AKRESULT BitcrushFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (BitcrushFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    // Initialise sample rate temp variables
    SampleIndex = 0;
    Index = 0.0f;
    PreviousSample = 0.0f;

    return AK_Success;
}

AKRESULT BitcrushFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT BitcrushFX::Reset()
{
    return AK_Success;
}

AKRESULT BitcrushFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}


//! Fix stepping! How can you interpolate/smooth out the reduction steps?
AkReal32 BitcrushFX::BitReduction(AkReal32 InputSample, AkReal32 BitRate)
{
    AkReal32 Result = 0.0f;

    // Find range of amplitude values, 2^Bits
    AkReal32 AmplitudeRange = powf(2, floorf(BitRate));

    // Shrink down to 0 - 1 range
    Result = (0.5f * InputSample + 0.5f);

    // Scale up by the maxium bits
    Result *= AmplitudeRange;

    // Round to nearest bit
    Result = roundf(Result);

    // Invert scaling for output
    Result /= AmplitudeRange;
    Result = 2 * Result - 1;

    return Result;
}

AkReal32 BitcrushFX::DownSample(AkReal32 InputSample, AkReal32 Factor)
{
    AkReal32 Result = 0.0f;

    // Check if index can be increased
    if(Index < SampleIndex) 
    {
        Index += Factor;
        PreviousSample = InputSample;
        Result = PreviousSample;
    }
    // Stall output by the previous sample
    else 
    {
        Result = PreviousSample;
    }

    ++SampleIndex;

    return Result;
}

AkReal32 BitcrushFX::HardClip(AkReal32 InputSample, AkReal32 Threshold)
{
    AkReal32 Result = 0.0f;

    // Check if +ve or -ve is above the threshold
    if(InputSample >= Threshold)
    {
        Result = Threshold;
    }
    else if(InputSample <= -Threshold)
    {
        Result = -Threshold;
    }
    else
    {
        Result = InputSample;
    }
    
    return Result;
}

AkReal32 BitcrushFX::SoftClip(AkReal32 InputSample, AkReal32 Alpha)
{
    AkReal32 Result = 0.0f;

    // Apply arctangent function
    //! Worth changing to a polynomial?
    Result = (2 / PI64) * atanf(InputSample * Alpha);

    return Result;
}

void BitcrushFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    AkReal32 InputLinear = powf(10, (m_pParams->RTPC.InputAmplitude / 20));
    AkReal32 OutputLinear = powf(10, (m_pParams->RTPC.OutputAmplitude / 20));
    AkReal32 Drive = m_pParams->RTPC.Drive;

    //! Need way of check if BitRate is between two integers
    bool IsFractional = false;
    if(floor(m_pParams->RTPC.BitRate) - m_pParams->RTPC.BitRate != 0)
    {
        IsFractional = true;
    }

    // Check clipping type
    bool IsHardClipped = false;
    if(m_pParams->RTPC.ClipType == 1.0)
    {
        IsHardClipped = true;

        // Flip input threshold paramter to mimic typical drive knob
        Drive = (1 - m_pParams->RTPC.Drive);
    }
    else
    {
        // Scale up soft clip drive by 10
        Drive *= 10;
    }

    //! Bit confusing having two seperate processing paths that are largely the same - more elegant way of handling the soft/hard decision? 
    //! Check drive/alpha for soft clippingm, 0 = no volume?
    if(IsHardClipped)
    {
        AkUInt16 uFramesProcessed;
        for (AkUInt32 i = 0; i < uNumChannels; ++i)
        {
            AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

            uFramesProcessed = 0;
            while (uFramesProcessed < io_pBuffer->uValidFrames)
            {
                // Execute DSP in-place here
                // Get sample
                AkReal32 CurrentSample = pBuf[uFramesProcessed];

                // Apply input amplitude boost
                CurrentSample *= InputLinear;

                // Bit Reduction
                CurrentSample = BitReduction(CurrentSample, m_pParams->RTPC.BitRate);

                // Sample Rate
                CurrentSample = DownSample(CurrentSample, m_pParams->RTPC.SampleRate);

                // Drive - Hard Clip
                CurrentSample = HardClip(CurrentSample, Drive);

                // Apply output amplitude
                CurrentSample *= OutputLinear;

                // Copy sample back
	    		pBuf[uFramesProcessed] = CurrentSample;

                // Increment buffer
                ++uFramesProcessed;
            }
        }
    }
    else
    {
        AkUInt16 uFramesProcessed;
        for (AkUInt32 i = 0; i < uNumChannels; ++i)
        {
            AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

            uFramesProcessed = 0;
            while (uFramesProcessed < io_pBuffer->uValidFrames)
            {
                // Execute DSP in-place here
                // Get sample
                AkReal32 CurrentSample = pBuf[uFramesProcessed];

                // Apply input amplitude boost
                CurrentSample *= InputLinear;

                // Bit Reduction
                CurrentSample = BitReduction(CurrentSample, m_pParams->RTPC.BitRate);

                // Sample Rate
                CurrentSample = DownSample(CurrentSample, m_pParams->RTPC.SampleRate);

                // Drive - Soft Clip
                CurrentSample = SoftClip(CurrentSample, Drive);

                // Apply output amplitude
                CurrentSample *= OutputLinear;

                // Copy sample back
	    		pBuf[uFramesProcessed] = CurrentSample;

                // Increment buffer
                ++uFramesProcessed;
            }
        }        
    }
}

AKRESULT BitcrushFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
