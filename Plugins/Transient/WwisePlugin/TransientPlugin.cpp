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

#include "TransientPlugin.h"
#include "../SoundEnginePlugin/TransientFXFactory.h"

#include <AK/Tools/Common/AkAssert.h>

TransientPlugin::TransientPlugin()
    : m_pPSet(nullptr)
{
}

TransientPlugin::~TransientPlugin()
{
}

void TransientPlugin::Destroy()
{
    delete this;
}

void TransientPlugin::SetPluginPropertySet(AK::Wwise::IPluginPropertySet* in_pPSet)
{
    m_pPSet = in_pPSet;
}

bool TransientPlugin::GetBankParameters(const GUID& in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter) const
{
    // Write bank data here
    CComVariant varProp;
    m_pPSet->GetValue(in_guidPlatform, L"Dummy", varProp);
    in_pDataWriter->WriteReal32(varProp.fltVal);

    return true;
}
