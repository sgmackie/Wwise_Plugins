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

#include "TransientFXParams.h"

#include <AK/Tools/Common/AkBankReadHelpers.h>

TransientFXParams::TransientFXParams()
{
}

TransientFXParams::~TransientFXParams()
{
}

TransientFXParams::TransientFXParams(const TransientFXParams& in_rParams)
{
    RTPC = in_rParams.RTPC;
    NonRTPC = in_rParams.NonRTPC;
    m_paramChangeHandler.SetAllParamChanges();
}

AK::IAkPluginParam* TransientFXParams::Clone(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, TransientFXParams(*this));
}

AKRESULT TransientFXParams::Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    if (in_ulBlockSize == 0)
    {
        // Initialize default parameters here
        RTPC.Attack             = 0.0f;
        RTPC.AttackSensitivity  = 0.0f;
        RTPC.Sustain            = 0.0f;
        RTPC.SustainSensitivity = 0.0f;
        RTPC.ClippingEnable     = 0.0f;
        RTPC.Drive              = 0.0f;
        RTPC.DCOffset           = 0.0f;
        RTPC.DryWetMix          = 0.0f;
        m_paramChangeHandler.SetAllParamChanges();
        return AK_Success;
    }

    return SetParamsBlock(in_pParamsBlock, in_ulBlockSize);
}

AKRESULT TransientFXParams::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT TransientFXParams::SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize)
{
    AKRESULT eResult = AK_Success;
    AkUInt8* pParamsBlock = (AkUInt8*)in_pParamsBlock;

    // Read bank data here
    RTPC.Attack             = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.AttackSensitivity  = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Sustain            = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.SustainSensitivity = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.ClippingEnable     = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.Drive              = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.DCOffset           = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    RTPC.DryWetMix          = READBANKDATA(AkReal32, pParamsBlock, in_ulBlockSize);
    CHECKBANKDATASIZE(in_ulBlockSize, eResult);
    m_paramChangeHandler.SetAllParamChanges();

    return eResult;
}

AKRESULT TransientFXParams::SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize)
{
    AKRESULT eResult = AK_Success;

    // Handle parameter change here
    switch (in_paramID)
    {
    case PARAM_ATTACK_ID:
        RTPC.Attack = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ATTACK_ID);
        break;
    case PARAM_ATTACK_SENSITIVITY_ID:
        RTPC.AttackSensitivity = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_ATTACK_SENSITIVITY_ID);
        break;                
    case PARAM_SUSTAIN_ID:
        RTPC.Sustain = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_SUSTAIN_ID);
        break;
    case PARAM_SUSTAIN_SENSITIVITY_ID:
        RTPC.SustainSensitivity = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_SUSTAIN_SENSITIVITY_ID);
        break; 
    case PARAM_CLIPPING_ENABLE_ID:
        RTPC.ClippingEnable = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_CLIPPING_ENABLE_ID);
        break;            
    case PARAM_DRIVE_ID:
        RTPC.Drive = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_DRIVE_ID);
        break;         
    case PARAM_DC_OFFSET_ID:
        RTPC.DCOffset = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_DC_OFFSET_ID);
        break;            
    case PARAM_DRY_WET_ID:
        RTPC.DryWetMix = *((AkReal32*)in_pValue);
        m_paramChangeHandler.SetParamChange(PARAM_DRY_WET_ID);
        break;                          
    default:
        eResult = AK_InvalidParameter;
        break;
    }

    return eResult;
}
