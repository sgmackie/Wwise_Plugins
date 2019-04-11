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

#include "WavetableSourceParams.h"
#include <AK/Tools/Common/AkBankReadHelpers.h>

WavetableSourceParams::WavetableSourceParams()
{
}

WavetableSourceParams::~WavetableSourceParams()
{
}

WavetableSourceParams::WavetableSourceParams(const WavetableSourceParams& in_rParams)
{
    RTPC = in_rParams.RTPC;
    NonRTPC = in_rParams.NonRTPC;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* WavetableSourceParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, WavetableSourceParams(*this));
}

AKRESULT WavetableSourceParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if(in_ulBlockSize == 0)
    {
        // Initialize default parameters here
        RTPC.fDuration  = 5.0f;
        RTPC.Amplitude  = 0.9f;
        RTPC.Attack     = 1.2f;
        RTPC.Decay      = 0.5f;
        RTPC.Sustain    = 0.7f;
        RTPC.Release    = 2.2f;
        RTPC.Frequency  = 20.0f;
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT WavetableSourceParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT WavetableSourceParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    RTPC.fDuration  = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Amplitude  = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Attack     = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Decay      = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Sustain    = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Release    = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Frequency  = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT WavetableSourceParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    switch (in_paramID)
    {
        case PARAM_DURATION_ID:
        {
            RTPC.fDuration = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_DURATION_ID);
            break;
        }
        case PARAM_AMPLITUDE_ID:
        {
            RTPC.Amplitude = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_AMPLITUDE_ID);
            break;
        }
        case PARAM_ATTACK_ID:
        {
            RTPC.Attack = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_ATTACK_ID);
            break;
        }      
        case PARAM_DECAY_ID:
        {
            RTPC.Decay = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_DECAY_ID);
            break;
        }      
        case PARAM_SUSTAIN_ID:
        {
            RTPC.Sustain = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_SUSTAIN_ID);
            break;
        }      
        case PARAM_RELEASE_ID:
        {
            RTPC.Release = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_RELEASE_ID);
            break;
        }     
        case PARAM_FREQUENCY_ID:
        {
            RTPC.Frequency = *((AkReal32*)in_pValue);
            m_paramChangeHandler.SetParamChange(PARAM_FREQUENCY_ID);
            break;
        }                                                
        default:
        {
            eResult = AK_InvalidParameter;
            break;
        }
    }

    return eResult;
}