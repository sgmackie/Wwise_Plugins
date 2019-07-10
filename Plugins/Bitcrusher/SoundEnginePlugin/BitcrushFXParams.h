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

#ifndef BitcrushFXParams_H
#define BitcrushFXParams_H

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

// Add parameters IDs here, those IDs should map to the AudioEnginePropertyID
// attributes in the xml property definition.
static const AkPluginParamID PARAM_ID_INPUT_AMPLITUDE = 0;
static const AkPluginParamID PARAM_ID_BIT_RATE = 1;
static const AkPluginParamID PARAM_ID_SAMPLE_RATE = 2;
static const AkPluginParamID PARAM_ID_CLIP_TYPE = 3;
static const AkPluginParamID PARAM_ID_DRIVE = 4;
static const AkPluginParamID PARAM_ID_OUTPUT_AMPLITUDE = 5;
static const AkUInt32 NUM_PARAMS = 6;

struct BitcrushRTPCParams
{
    AkReal32 InputAmplitude;
    AkReal32 OutputAmplitude;
    AkReal32 BitRate;
    AkReal32 SampleRate;
    bool ClipType;
    AkReal32 Drive;
};

struct BitcrushNonRTPCParams
{
};

struct BitcrushFXParams
    : public AK::IAkPluginParam
{
    BitcrushFXParams();
    BitcrushFXParams(const BitcrushFXParams& in_rParams);

    ~BitcrushFXParams();

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

    BitcrushRTPCParams RTPC;
    BitcrushNonRTPCParams NonRTPC;
} AK_ALIGN_DMA;

#endif // BitcrushFXParams_H
