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

void FillTable(AkReal64 *Table, AkUInt64 SampleBlock)
{
    AkReal64 Step = (TWO_PI32 / SampleBlock);

    AkUInt64 i = 0;
    for(i = 0; i < SampleBlock; ++i) 
    {
        Table[i] = (AkReal64) sin(Step * i);
    }

    //Wraparound
    Table[i] = Table[0];
}


AKRESULT WavetableSource::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkSourcePluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (WavetableSourceParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    m_durationHandler.Setup(m_pParams->RTPC.fDuration, in_pContext->GetNumLoops(), in_rFormat.uSampleRate);

    AkUInt32 Channels = in_rFormat.channelConfig.uNumChannels;
    if(Channels < 2)
    {
        Channels = 2;
    }

    SampleRate = in_rFormat.uSampleRate;
    SampleBlock = (8192 + 1);
    Table = (AkReal64 *) m_pAllocator->Malloc((size_t) (sizeof(AkReal64) * SampleBlock) * Channels);
    FillTable(Table, SampleBlock);

    return AK_Success;
}

AKRESULT WavetableSource::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    m_pAllocator->Free(Table);

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
    m_durationHandler.SetDuration(m_pParams->RTPC.fDuration);
    m_durationHandler.ProduceBuffer(out_pBuffer);

    const AkUInt32 uNumChannels = out_pBuffer->NumChannels();

    AkReal64 CurrentPhase = 0.0;
    AkReal64 TableLength = (AkReal64) SampleBlock;
    AkReal64 SizeOverSampleRate = (AkReal64) SampleBlock / SampleRate;
    AkReal64 PhaseIncrement = SizeOverSampleRate * m_pParams->RTPC.fFrequency;

    AkUInt16 uFramesProduced;
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)out_pBuffer->GetChannel(i);

        //Interpolated loop
        AkInt32 Base = 0;
        AkInt32 Next = 0;
        AkReal64 Fraction = 0;
        AkReal64 Value = 0;
        AkReal64 Slope = 0;

        uFramesProduced = 0;
        while (uFramesProduced < out_pBuffer->uValidFrames)
        {
            Base = (AkInt32) CurrentPhase;
            Next = (Base + 1);

            Fraction = CurrentPhase - Base;
            Value = Table[Base];
            Slope = (Table[Next] - Value);

            Value += (Fraction * Slope);
            
            // Generate output here
            *pBuf++ = (AkReal32) Value;

            CurrentPhase += PhaseIncrement;

            while(CurrentPhase >= TableLength) 
            {
                CurrentPhase -= TableLength;
            }

            while(CurrentPhase < 0)
            {
                CurrentPhase += TableLength;
            }

            //Increment loop
            ++uFramesProduced;
        }
    }
}

AkReal32 WavetableSource::GetDuration() const
{
    return m_durationHandler.GetDuration() * 1000.0f;
}
