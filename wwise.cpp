#include <stdio.h>

#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>


#include <AK/Plugin/WavetableSourceFactory.h>


#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif

#ifdef WIN32
#include "Win32/AkDefaultIOHookBlocking.h"
#include "Win32/AkDefaultIOHookBlocking.cpp"
#include "Common/AkMultipleFileLocation.cpp"

CAkDefaultIOHookBlocking GlobalIOBlockingDevice;
#endif

//Define function hooks for the memory manager depending on the platform
namespace AK
{
#ifdef WIN32
    void *AllocHook(size_t in_size)
    {
        return malloc(in_size);
    }

    void FreeHook(void *in_ptr)
    {
        free(in_ptr);
    }

    void *VirtualAllocHook(void *in_pMemAddress, size_t in_size, DWORD in_dwAllocationType, DWORD in_dwProtect)
    {
        return VirtualAlloc(in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect);
    }

    void VirtualFreeHook(void * in_pMemAddress, size_t in_size, DWORD in_dwFreeType)
    {
        VirtualFree(in_pMemAddress, in_size, in_dwFreeType);
    }
#endif
}

bool wwise_EngineStart()
{
    //Memory Manager
    AkMemSettings MemorySettings;
    MemorySettings.uMaxNumPools = 20;

    if(AK::MemoryMgr::Init(&MemorySettings) != AK_Success)
    {
        printf("Wwise: Failed to create memory manager!\n");
        return false;
    }

    //Stream Manager
    AkStreamMgrSettings StreamSettings;
    AK::StreamMgr::GetDefaultSettings(StreamSettings);
        
    if(!AK::StreamMgr::Create(StreamSettings))
    {
        printf("Wwise: Failed to create streaming manager!\n");
        return false;
    }
    
    AkDeviceSettings DeviceSettings;
    AK::StreamMgr::GetDefaultDeviceSettings(DeviceSettings);

    if(GlobalIOBlockingDevice.Init(DeviceSettings) != AK_Success)
    {
        printf("Wwise: Failed to create streaming device and I/O blocking system!\n");
        return false;
    }    

    //Set generated sound banks path
    GlobalIOBlockingDevice.SetBasePath(AKTEXT("./Wwise_Project/SDK_Bench/GeneratedSoundBanks/Windows"));

    //Sound Engine
    AkInitSettings InitialSettings;
    AkPlatformInitSettings PlatformInitialSettings;
    AK::SoundEngine::GetDefaultInitSettings(InitialSettings);
    AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitialSettings);

    if(AK::SoundEngine::Init(&InitialSettings, &PlatformInitialSettings) != AK_Success)
    {
        printf("Wwise: Failed to initialise sound engine for the given platform!\n");
        return false;
    }

    //Communications (not included in Release builds!)
#ifndef AK_OPTIMIZED
    AkCommSettings CommunicationSettings;
    AK::Comm::GetDefaultInitSettings(CommunicationSettings);
    
    if(AK::Comm::Init(CommunicationSettings) != AK_Success)
    {
        printf("Wwise: Failed to initialise communications library\n");
        return false;
    }

#endif
    return true;
}

void wwise_EngineStop()
{
    //Communications
#ifndef AK_OPTIMIZED
    AK::Comm::Term();
#endif

    //Sound Engine
    AK::SoundEngine::Term();

    //Stream Manager
    GlobalIOBlockingDevice.Term();
    if(AK::IAkStreamMgr::Get())
    {
        AK::IAkStreamMgr::Get()->Destroy();
    }

    //Memory Manager
    AK::MemoryMgr::Term();    
}

AKRESULT wwise_BankLoad(AkBankID BankID, const char *BankName)
{
    AKRESULT Result = AK_Fail;
    
    //Can pass the ID instead of the name to load banks
    Result = AK::SoundEngine::LoadBank(BankName, AK_DEFAULT_POOL_ID, BankID);

    return Result;
}

AKRESULT wwise_EventCreate(int ID, char *Event)
{
    AKRESULT Result = AK_Fail;
    Result = AK::SoundEngine::RegisterGameObj(ID, Event);
    return Result;
}

AkPlayingID wwise_EventPost(int ID, char *Event)
{
    AkPlayingID Result = 0;
    Result = AK::SoundEngine::PostEvent(Event, ID);
    return Result;
}


int main()
{
    if(wwise_EngineStart())
    {
        printf("Wwise: Engine started\n");
    }

    if(wwise_BankLoad(0, "Init.bnk") == AK_Success)
    {
        printf("Wwise: Loaded Init.bnk\n");
    }

    if(wwise_BankLoad(0, "Ambience.bnk") == AK_Success)
    {
        printf("Wwise: Loaded Ambience.bnk\n");
    }

    //Create default listener
    AkGameObjectID PlayerListener = 0;
    AK::SoundEngine::RegisterGameObj(PlayerListener, "LN_Player");
    AK::SoundEngine::SetDefaultListeners(&PlayerListener, 1);

    //Create objects used to process events
    if(wwise_EventCreate(100, "SO_Animals") == AK_Fail);
    {
        printf("Wwise: Failed to create SO_Animals!\n");
    }

    if(wwise_EventCreate(200, "SO_Wavetable") == AK_Fail);
    {
        printf("Wwise: Failed to create SO_Wavetable!\n");
    }
    

    bool GlobalRunning = true;
    int i = 0;
    
    while(GlobalRunning)
    {
        if(i == 5000)
        {
            // AkPlayingID Playing = wwise_EventPost(100, "Animals");
            AkPlayingID Playing = wwise_EventPost(100, "Wavetable");
            printf("%lu\n", Playing);
        }

        //!Call this once per frame!
        AK::SoundEngine::RenderAudio();

        ++i;   
    }

    wwise_EngineStop();

    return 0;
}