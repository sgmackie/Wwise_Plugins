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

#ifndef TransientFXParams_H
#define TransientFXParams_H

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

// Add parameters IDs here, those IDs should map to the AudioEnginePropertyID
// attributes in the xml property definition.
static const AkPluginParamID PARAM_ATTACK_ID = 0;
static const AkPluginParamID PARAM_ATTACK_SENSITIVITY_ID = 1;
static const AkPluginParamID PARAM_SUSTAIN_ID = 2;
static const AkPluginParamID PARAM_SUSTAIN_SENSITIVITY_ID = 3;
static const AkPluginParamID PARAM_CLIPPING_ENABLE_ID = 4;
static const AkPluginParamID PARAM_DRIVE_ID = 5;
static const AkPluginParamID PARAM_DC_OFFSET_ID = 6;
static const AkPluginParamID PARAM_DRY_WET_ID = 7;
static const AkUInt32 NUM_PARAMS = 8;

struct TransientRTPCParams
{
    AkReal32 Attack;
    AkReal32 AttackSensitivity;
    AkReal32 Sustain;
    AkReal32 SustainSensitivity;
    AkReal32 ClippingEnable;
    AkReal32 Drive;
    AkReal32 DCOffset;
    AkReal32 DryWetMix;
};

struct TransientNonRTPCParams
{
};

struct TransientFXParams
    : public AK::IAkPluginParam
{
    TransientFXParams();
    TransientFXParams(const TransientFXParams& in_rParams);

    ~TransientFXParams();

    /// Create a duplicate of the parameter node instance in its current state.
    IAkPluginParam* Clone(AK::IAkPluginMemAlloc* in_pAllocator);

    /// Initialize the plug-in parameter node interface.
    /// Initializes the internal parameter structure to default values or with the provided parameter block if it is valid.
    AKRESULT Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize);

    /// Called by the sound engine when a parameter node is terminated.
    AKRESULT Term(AK::IAkPluginMemAlloc* in_pAllocator);

    /// Set all plug-in parameters at once using a parameter block.
    AKRESULT SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize);

    /// Update a single parameter at a time and perform the necessary actions on the parameter changes.
    AKRESULT SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize);

    AK::AkFXParameterChangeHandler<NUM_PARAMS> m_paramChangeHandler;

    TransientRTPCParams RTPC;
    TransientNonRTPCParams NonRTPC;
} AK_ALIGN_DMA;

#endif // TransientFXParams_H
