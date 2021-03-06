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

#include "WavetableSource.h"
#include "../WavetableConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateWavetableSource(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, WavetableSource());
}

AK::IAkPluginParam* CreateWavetableSourceParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, WavetableSourceParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(WavetableSource, AkPluginTypeSource, WavetableConfig::CompanyID, WavetableConfig::PluginID)

WavetableSource::WavetableSource()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

WavetableSource::~WavetableSource()
{
}

AKRESULT WavetableSource::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkSourcePluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (WavetableSourceParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    //Set duration for output buffer size
    m_durationHandler.Setup(m_pParams->RTPC.fDuration, in_pContext->GetNumLoops(), in_rFormat.uSampleRate);

    //Oscillator initialisation
    SampleRate = in_rFormat.uSampleRate;
    TableLength = 4096;
    PresetList = (AkReal64 **) in_pAllocator->Malloc(((sizeof(AkReal64) * TableLength) * MAX_PRESETS));
    BuildPresets(in_pAllocator, PresetList, TableLength); //Build the preset tables
    
    AkReal64 *Table = new AkReal64 [TableLength];
    for(AkUInt32 i = 0; i < TableLength; ++i)
    {
        //0 = Table1  1 = Table2
        Table[i] = Lerp(WAVE_TABLE_Mopho_Vox_1[i], WAVE_TABLE_Mopho_Vox_2[i], 0.5f);
    }
    
    Oscillator = CreateWavetableOscillator(Table, TableLength); //Create the oscillator

    //Create ADSR envelope
    Oscillator->Envelope = ADSRCreate(in_pAllocator, SampleRate, 1.0f, m_pParams->RTPC.Attack, m_pParams->RTPC.Decay, m_pParams->RTPC.Sustain, m_pParams->RTPC.Release);

    delete [] Table;

    return AK_Success;
}

AKRESULT WavetableSource::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    in_pAllocator->Free(PresetList);
    in_pAllocator->Free(Oscillator->Envelope);
    in_pAllocator->Free(Oscillator);
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT WavetableSource::Reset()
{
    return AK_Success;
}

AKRESULT WavetableSource::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeSource;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void WavetableSource::Execute(AkAudioBuffer* out_pBuffer)
{
    //Create output buffer
    m_durationHandler.SetDuration(m_pParams->RTPC.fDuration);
    m_durationHandler.ProduceBuffer(out_pBuffer);

    //Get number of channels
    const AkUInt32 uNumChannels = out_pBuffer->NumChannels();
    AkUInt16 uFramesProduced;

    //Set frequency and amplitude
    AkReal32 Amplitude = m_pParams->RTPC.Amplitude;
    AkReal64 Frequency = ((m_pParams->RTPC.Frequency + 0.1) / SampleRate);
    // AkReal64 Multi = 1.0 + (log(20000.0 / SampleRate) - log(Frequency)) / Oscillator->Envelope->DurationInSamples;

    //TODO: Switch to interleaved processsing?
    for(AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT OutputBuffer = (AkReal32* AK_RESTRICT)out_pBuffer->GetChannel(i);

        uFramesProduced = 0;
        while(uFramesProduced < out_pBuffer->uValidFrames)
        {
            Oscillator->PhaseIncrement = Frequency;

            //Generate output here
            AkReal32 CurrentSample = (AkReal32) GetSample(Oscillator, false) * (Amplitude * ADSRTick(Oscillator->Envelope));
			OutputBuffer[uFramesProduced] = CurrentSample;

            //Increment loop
            ++uFramesProduced;

            //Update phase
            Oscillator->PhaseCurrent += Oscillator->PhaseIncrement;
            if(Oscillator->PhaseCurrent >= 1.0)
            {
                Oscillator->PhaseCurrent -= 1.0;
            }
        }
        uFramesProduced = 0;
    }
}

AkReal32 WavetableSource::GetDuration() const
{
    return m_durationHandler.GetDuration() * 1000.0f;
}
