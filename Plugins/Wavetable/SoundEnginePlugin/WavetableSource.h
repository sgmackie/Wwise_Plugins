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

#ifndef WavetableSource_H
#define WavetableSource_H


#include <stdint.h>
//Types include longform and short form names
//Standard integers
//Unsigned
//8-bit
typedef uint8_t uint8;
typedef uint8_t u8;
//16-bit
typedef uint16_t uint16;
typedef uint16_t u16;
//32-bit
typedef uint32_t uint32;
typedef uint32_t u32;
//64-bit
typedef uint64_t uint64;
typedef uint64_t u64;

//Signed
//8-bit
typedef int8_t int8;
typedef int8_t i8;
//16-bit
typedef int16_t int16;
typedef int16_t i16;
//32-bit
typedef int32_t int32;
typedef int32_t i32;
//64-bit
typedef int64_t int64;
typedef int64_t i64;

//Floating points
//32-bit
typedef float float32;
typedef float f32;

//64-bit
typedef double float64;
typedef double f64;


#define PI32 3.14159265359f
#define TWO_PI32 (2.0 * PI32)




#include "WavetableSourceParams.h"

#include <AK/Plugin/PluginServices/AkFXDurationHandler.h>

/// See https://www.audiokinetic.com/library/edge/?source=SDK&id=soundengine__plugins__source.html
/// for the documentation about source plug-ins
class WavetableSource
    : public AK::IAkSourcePlugin
{
public:
    WavetableSource();
    ~WavetableSource();

    /// Plug-in initialization.
    /// Prepares the plug-in for data processing, allocates memory and sets up the initial conditions.
    AKRESULT Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkSourcePluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat);

    /// Release the resources upon termination of the plug-in.
    AKRESULT Term(AK::IAkPluginMemAlloc* in_pAllocator);

    /// The reset action should perform any actions required to reinitialize the
    /// state of the plug-in to its original state (e.g. after Init() or on effect bypass).
    AKRESULT Reset();

    /// Plug-in information query mechanism used when the sound engine requires
    /// information about the plug-in to determine its behavior.
    AKRESULT GetPluginInfo(AkPluginInfo& out_rPluginInfo);

    /// Source plug-in DSP execution.
    void Execute(AkAudioBuffer* io_pBuffer);

    /// This method is called to determine the approximate duration (in ms) of the source.
    AkReal32 GetDuration() const;

private:
    AkReal64 *Table;
    AkUInt64 SampleBlock;
    AkUInt32 SampleRate;


    WavetableSourceParams* m_pParams;
    AK::IAkPluginMemAlloc* m_pAllocator;
    AK::IAkSourcePluginContext* m_pContext;
    AkFXDurationHandler m_durationHandler;
};

#endif // WavetableSource_H
