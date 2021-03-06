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

#include "BitcrushFXParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

BitcrushFXParams::BitcrushFXParams()
{
}

BitcrushFXParams::~BitcrushFXParams()
{
}

BitcrushFXParams::BitcrushFXParams(const BitcrushFXParams& in_rParams)
{
    RTPC = in_rParams.RTPC;
    NonRTPC = in_rParams.NonRTPC;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* BitcrushFXParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, BitcrushFXParams(*this));
}

AKRESULT BitcrushFXParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if (in_ulBlockSize == 0)
    {
        // Initialize default parameters here
        RTPC.InputAmplitude     = 0.0f;
        RTPC.OutputAmplitude    = 0.0f;
        RTPC.BitRate            = 0.0f;
        RTPC.SampleRate         = 0.0f;
        RTPC.Drive              = 0.0f;        
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT BitcrushFXParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT BitcrushFXParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    RTPC.InputAmplitude     = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.OutputAmplitude    = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.BitRate            = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.SampleRate         = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.ClipType           = READBANKDATA(bool, pParamsBlock, in_ulBlockSize);
    RTPC.Drive              = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT BitcrushFXParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    switch (in_paramID)
    {
    case PARAM_ID_INPUT_AMPLITUDE:
        RTPC.InputAmplitude = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_INPUT_AMPLITUDE);
        break;
    case PARAM_ID_OUTPUT_AMPLITUDE:
        RTPC.OutputAmplitude = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_OUTPUT_AMPLITUDE);
        break;       
    case PARAM_ID_BIT_RATE:
        RTPC.BitRate = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_BIT_RATE);
        break;     
    case PARAM_ID_SAMPLE_RATE:
        RTPC.SampleRate = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_SAMPLE_RATE);
        break;    
    case PARAM_ID_CLIP_TYPE:
        RTPC.ClipType = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_CLIP_TYPE);
        break;           
    case PARAM_ID_DRIVE:
        RTPC.Drive = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ID_DRIVE);
        break;                                    
    default:
        eResult = AK_InvalidParameter;
        break;
    }

    return eResult;
}
