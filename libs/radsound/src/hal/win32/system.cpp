//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include "pch.hpp"
#include "system.hpp"
#include "listener.hpp"
#include "buffer.hpp"
#include "voice.hpp"
#include "../common/banner.hpp"
#include "../common/memoryregion.hpp"
#include <radplatform.hpp>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>


typedef ALuint ALbitfield;



#include "radinprogext.h"


#include <cstdio>
#include <cstring>



// ---- OpenAL Soft map buffer extension typedefs ----
/**
#ifndef AL_SOFTX_map_buffer

typedef void (AL_APIENTRY* LPALBUFFERSTORAGESOFT)(
    ALuint buffer,
    ALenum format,
    const ALvoid* data,
    ALsizei size,
    ALsizei freq,
    ALbitfield flags
);

typedef ALvoid* (AL_APIENTRY* LPALMAPBUFFERSOFT)(
    ALuint buffer,
    ALsizei offset,
    ALsizei length,
    ALbitfield access
);

typedef void (AL_APIENTRY* LPALUNMAPBUFFERSOFT)(
    ALuint buffer
);

#endif
*/

// ----------------------------------------------------------------------------
// OpenAL Soft optional extensions used by streaming code
// ----------------------------------------------------------------------------
LPALBUFFERSTORAGESOFT radBufferStorageSOFT = nullptr;
LPALMAPBUFFERSOFT     radMapBufferSOFT     = nullptr;
LPALUNMAPBUFFERSOFT   radUnmapBufferSOFT   = nullptr;

//================================================================================
// Static Members
//================================================================================
radSoundHalSystem* radSoundHalSystem::s_pRsdSystem = NULL;
static int g_RadSoundInitializeCount = 0;

//============================================================================
// Debug callback (only if AL_EXT_debug exists)
//============================================================================
typedef void (AL_APIENTRY* ALDEBUGPROCEXT)(
    ALenum source, ALenum type, ALuint id, ALenum severity,
    ALsizei length, const ALchar* message, void* userParam);

typedef void (AL_APIENTRY* LPALDEBUGMESSAGECALLBACKEXT)(ALDEBUGPROCEXT callback, void* userParam);

static void AL_APIENTRY PrintOpenALErrors(
    ALenum source, ALenum type, ALuint id, ALenum severity,
    ALsizei length, const ALchar* message, void* userParam)
{
    (void)length;
    (void)userParam;
    std::fprintf(stderr, "OpenAL: source=%u type=%u id=%u severity=%u '%s'\n",
                 (unsigned)source, (unsigned)type, (unsigned)id, (unsigned)severity, message);
}

//============================================================================
// radSoundHalSystem::radSoundHalSystem
//============================================================================
radSoundHalSystem::radSoundHalSystem(radMemoryAllocator allocator)
    :
    m_NumAuxSends(0),
    m_pSoundMemory(nullptr),
    m_pDevice(nullptr),
    m_pContext(nullptr),
    m_LastServiceTime(::radTimeGetMilliseconds()),
    alGenAuxiliaryEffectSlots(nullptr),
    alDeleteAuxiliaryEffectSlots(nullptr),
    alAuxiliaryEffectSlotf(nullptr),
    alGetAuxiliaryEffectSlotf(nullptr)
{
    s_pRsdSystem = this;

    for (unsigned int i = 0; i < RSD_SYSTEM_MAX_AUX_SENDS; i++)
        m_refIRadSoundHalEffect[i] = NULL;

    std::memset(m_AuxSlots, 0, sizeof(m_AuxSlots));

    ::radSoundPrintBanner();
}

//============================================================================
// radSoundHalSystem::~radSoundHalSystem
//============================================================================
radSoundHalSystem::~radSoundHalSystem(void)
{
    radSoundHalListener::Terminate();

    // Delete aux slots only if we actually have the function pointer + slots
    if (alDeleteAuxiliaryEffectSlots && m_NumAuxSends > 0)
    {
        alDeleteAuxiliaryEffectSlots(m_NumAuxSends, m_AuxSlots);
    }
    m_NumAuxSends = 0;
    std::memset(m_AuxSlots, 0, sizeof(m_AuxSlots));

    // Destroy context/device safely
    if (m_pContext)
    {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(m_pContext);
        m_pContext = NULL;
    }

    if (m_pDevice)
    {
        alcCloseDevice(m_pDevice);
        m_pDevice = NULL;
    }

    radSoundHalMemoryRegion::Terminate();
    ::radMemoryFreeAligned(GetThisAllocator(), m_pSoundMemory);
    m_pSoundMemory = nullptr;

    s_pRsdSystem = NULL;
}

//============================================================================
// radSoundHalSystem::Initialize
//============================================================================
void radSoundHalSystem::Initialize(const SystemDescription& systemDescription)
{
    rAssertMsg(systemDescription.m_SamplingRate != 0,
        "ERROR radsound: system sampling rate must be set to the highest sampling rate required (e.g. 48000Hz)");

    // Clamp requested aux sends to our compile-time limit
    m_NumAuxSends = (ALCint)systemDescription.m_NumAuxSends;
    if (m_NumAuxSends < 0) m_NumAuxSends = 0;
    if (m_NumAuxSends > (ALCint)RSD_SYSTEM_MAX_AUX_SENDS) m_NumAuxSends = RSD_SYSTEM_MAX_AUX_SENDS;

    // ---------------------------------------------------------------------
    // 1) Open device
    // ---------------------------------------------------------------------
    m_pDevice = alcOpenDevice(NULL);
    rAssertMsg(m_pDevice != NULL, "OpenAL device couldn't be opened.");
    if (!m_pDevice)
        return;

    // Clear any previous error
    (void)alcGetError(m_pDevice);

    // ---------------------------------------------------------------------
    // 2) Create context (frequency + best-effort aux send request)
    // ---------------------------------------------------------------------
    ALCint attr[] = {
        ALC_FREQUENCY, (ALCint)systemDescription.m_SamplingRate,
        ALC_MAX_AUXILIARY_SENDS, m_NumAuxSends,
        0
    };

    m_pContext = alcCreateContext(m_pDevice, attr);
    rAssertMsg(m_pContext != NULL, "OpenAL context couldn't be created.");
    if (!m_pContext)
        return;

    alcMakeContextCurrent(m_pContext);

    // ---------------------------------------------------------------------
    // 3) Optional debug extension
    // ---------------------------------------------------------------------
    if (alIsExtensionPresent("AL_EXT_debug"))
    {
        const ALenum AL_DEBUG_OUTPUT_EXT = alGetEnumValue("AL_DEBUG_OUTPUT_EXT");
        auto alDebugMessageCallbackEXT =
            (LPALDEBUGMESSAGECALLBACKEXT)alGetProcAddress("alDebugMessageCallbackEXT");

        if (AL_DEBUG_OUTPUT_EXT != 0 && alDebugMessageCallbackEXT)
        {
            alEnable(AL_DEBUG_OUTPUT_EXT);
            alDebugMessageCallbackEXT(PrintOpenALErrors, nullptr);
        }
    }

    // ---------------------------------------------------------------------
    // 4) Map/bufferstorage (best-effort; do NOT assert)
    // ---------------------------------------------------------------------
    radBufferStorageSOFT = nullptr;
    radMapBufferSOFT     = nullptr;
    radUnmapBufferSOFT   = nullptr;

    if (alIsExtensionPresent("AL_SOFTX_map_buffer"))
    {
        radBufferStorageSOFT = (LPALBUFFERSTORAGESOFT)alGetProcAddress("alBufferStorageSOFT");
        radMapBufferSOFT     = (LPALMAPBUFFERSOFT)alGetProcAddress("alMapBufferSOFT");
        radUnmapBufferSOFT   = (LPALUNMAPBUFFERSOFT)alGetProcAddress("alUnmapBufferSOFT");

        // If any is missing, disable this path completely to avoid crashes
        if (!radBufferStorageSOFT || !radMapBufferSOFT || !radUnmapBufferSOFT)
        {
            radBufferStorageSOFT = nullptr;
            radMapBufferSOFT     = nullptr;
            radUnmapBufferSOFT   = nullptr;
        }
    }

    // ---------------------------------------------------------------------
    // 5) EFX aux slots (best-effort)
    // ---------------------------------------------------------------------
    alGenAuxiliaryEffectSlots    = nullptr;
    alDeleteAuxiliaryEffectSlots = nullptr;
    alAuxiliaryEffectSlotf       = nullptr;
    alGetAuxiliaryEffectSlotf    = nullptr;

    if (m_NumAuxSends > 0 && alcIsExtensionPresent(m_pDevice, "ALC_EXT_EFX"))
    {
        alGenAuxiliaryEffectSlots =
            (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
        alDeleteAuxiliaryEffectSlots =
            (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
        alAuxiliaryEffectSlotf =
            (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
        alGetAuxiliaryEffectSlotf =
            (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");

        if (!alGenAuxiliaryEffectSlots || !alDeleteAuxiliaryEffectSlots ||
            !alAuxiliaryEffectSlotf || !alGetAuxiliaryEffectSlotf)
        {
            m_NumAuxSends = 0;
        }
        else
        {
            // Cap to device max
            ALCint deviceMaxAux = 0;
            alcGetIntegerv(m_pDevice, ALC_MAX_AUXILIARY_SENDS, 1, &deviceMaxAux);
            if (deviceMaxAux < 0) deviceMaxAux = 0;

            if (m_NumAuxSends > deviceMaxAux) m_NumAuxSends = deviceMaxAux;
            if (m_NumAuxSends > (ALCint)RSD_SYSTEM_MAX_AUX_SENDS) m_NumAuxSends = RSD_SYSTEM_MAX_AUX_SENDS;

            if (m_NumAuxSends > 0)
            {
                std::memset(m_AuxSlots, 0, sizeof(m_AuxSlots));
                alGenAuxiliaryEffectSlots(m_NumAuxSends, m_AuxSlots);

                if (alGetError() != AL_NO_ERROR)
                {
                    m_NumAuxSends = 0;
                    std::memset(m_AuxSlots, 0, sizeof(m_AuxSlots));
                }
            }
        }
    }
    else
    {
        m_NumAuxSends = 0;
    }

    // ---------------------------------------------------------------------
    // Listener init (needs context)
    // ---------------------------------------------------------------------
    radSoundHalListener::Initialize(GetThisAllocator(), m_pContext);

    // ---------------------------------------------------------------------
    // Allocate memory (unchanged)
    // ---------------------------------------------------------------------
    m_pSoundMemory = ::radMemoryAllocAligned(
        GetThisAllocator(),
        systemDescription.m_ReservedSoundMemory,
        radSoundHalDataSourceReadAlignmentGet());

    radSoundHalMemoryRegion::Initialize(
        m_pSoundMemory,
        systemDescription.m_ReservedSoundMemory,
        systemDescription.m_MaxRootAllocations,
        radSoundHalDataSourceReadAlignmentGet(),
        radMemorySpace_Local, GetThisAllocator());
}

//============================================================================
// radSoundHalSystem::GetRootMemoryRegion
//============================================================================
IRadSoundHalMemoryRegion* radSoundHalSystem::GetRootMemoryRegion(void)
{
    return radSoundHalMemoryRegion::GetRootRegion();
}

//============================================================================
// radSoundHalSystem::GetNumAuxSends
//============================================================================
unsigned int radSoundHalSystem::GetNumAuxSends()
{
    return (unsigned int)m_NumAuxSends;
}

//============================================================================
// radSoundHalSystem::SetOutputMode
//============================================================================
void radSoundHalSystem::SetOutputMode(radSoundOutputMode mode)
{
    (void)mode;
    rDebugString("radSoundHalSystem: SetOutputMode() not supported here.\n");
}

//============================================================================
// radSoundHalSystem::GetOutputMode
//============================================================================
radSoundOutputMode radSoundHalSystem::GetOutputMode(void)
{
    return radSoundOutputMode_Stereo;
}

//============================================================================
// radSoundHalSystem::Service
//============================================================================
void radSoundHalSystem::Service(void)
{
    unsigned int now = ::radTimeGetMilliseconds();
    radSoundUpdatableObject::UpdateAll(now - m_LastServiceTime);
    m_LastServiceTime = now;
}

//============================================================================
// radSoundHalSystem::ServiceOncePerFrame
//============================================================================
void radSoundHalSystem::ServiceOncePerFrame(void)
{
    radSoundHalListener::GetInstance()->UpdatePositionalSettings();
}

//============================================================================
// radSoundHalSystem::GetStats
//============================================================================
void radSoundHalSystem::GetStats(IRadSoundHalSystem::Stats* pStats)
{
    rAssert(pStats);
    ::memset(pStats, 0, sizeof(IRadSoundHalSystem::Stats));

    // Voices
    radSoundHalVoiceWin* pVoiceSearch = radSoundHalVoiceWin::GetLinkedClassHead();
    while (pVoiceSearch != NULL)
    {
        if (pVoiceSearch->GetPositionalGroup() != NULL)
        {
            pStats->m_NumPosVoices++;
            if (pVoiceSearch->IsPlaying()) pStats->m_NumPosVoicesPlaying++;
        }
        else
        {
            pStats->m_NumVoices++;
            if (pVoiceSearch->IsPlaying()) pStats->m_NumVoicesPlaying++;
        }

        pVoiceSearch = pVoiceSearch->GetLinkedClassNext();
    }

    // Buffers
    radSoundHalBufferWin* pBufferSearch = radSoundHalBufferWin::GetLinkedClassHead();
    while (pBufferSearch != NULL)
    {
        pStats->m_NumBuffers++;
        pStats->m_BufferMemoryUsed += pBufferSearch->GetSizeInBytes();
        pBufferSearch = pBufferSearch->GetLinkedClassNext();
    }

    pStats->m_EffectsMemoryUsed = 0;

    radSoundHalMemoryRegion::GetRootRegion()->GetStats(&pStats->m_TotalFreeSoundMemory, NULL, NULL, true);
}

//============================================================================
// radSoundHalSystem::SetAuxEffect
//============================================================================
void radSoundHalSystem::SetAuxEffect(unsigned int auxNumber, IRadSoundHalEffect* pIRadSoundHalEffect)
{
    rAssert(auxNumber < (unsigned)m_NumAuxSends);

    if (m_refIRadSoundHalEffect[auxNumber] != NULL)
        m_refIRadSoundHalEffect[auxNumber]->Detach();

    m_refIRadSoundHalEffect[auxNumber] = pIRadSoundHalEffect;

    if (m_refIRadSoundHalEffect[auxNumber] != NULL)
        m_refIRadSoundHalEffect[auxNumber]->Attach(auxNumber);
}

//============================================================================
// radSoundHalSystem::GetAuxEffect
//============================================================================
IRadSoundHalEffect* radSoundHalSystem::GetAuxEffect(unsigned int auxNumber)
{
    rAssert(auxNumber < (unsigned)m_NumAuxSends);
    return m_refIRadSoundHalEffect[auxNumber];
}

//============================================================================
// radSoundHalSystem::SetAuxGain
//============================================================================
void radSoundHalSystem::SetAuxGain(unsigned int aux, float gain)
{
    rAssert(aux < (unsigned)m_NumAuxSends);
    if (!alAuxiliaryEffectSlotf) return; // EFX not available
    alAuxiliaryEffectSlotf(m_AuxSlots[aux], AL_EFFECTSLOT_GAIN, gain);
}

//============================================================================
// radSoundHalSystem::GetAuxGain
//============================================================================
float radSoundHalSystem::GetAuxGain(unsigned int aux)
{
    rAssert(aux < (unsigned)m_NumAuxSends);
    if (!alGetAuxiliaryEffectSlotf) return 0.0f; // EFX not available
    ALfloat gain = 0.0f;
    alGetAuxiliaryEffectSlotf(m_AuxSlots[aux], AL_EFFECTSLOT_GAIN, &gain);
    return gain;
}

//============================================================================
// radSoundHalSystem::GetOpenALDevice
//============================================================================
ALCdevice* radSoundHalSystem::GetOpenALDevice(void)
{
    return m_pDevice;
}

//============================================================================
// radSoundHalSystem::GetOpenALContext
//============================================================================
ALCcontext* radSoundHalSystem::GetOpenALContext(void)
{
    return m_pContext;
}

//============================================================================
// radSoundHalSystem::GetOpenALAuxSlot
//============================================================================
ALuint radSoundHalSystem::GetOpenALAuxSlot(unsigned int aux)
{
    rAssert(aux < (unsigned)m_NumAuxSends);
    return m_AuxSlots[aux];
}


//============================================================================
// radSoundHalSystem::GetInstance
//============================================================================
radSoundHalSystem* radSoundHalSystem::GetInstance(void)
{
    return s_pRsdSystem;
}

//================================================================================
// ::rsdGetSystem
//================================================================================
IRadSoundHalSystem* radSoundHalSystemGet(void)
{
    rAssert(radSoundHalSystem::s_pRsdSystem != NULL);
    return radSoundHalSystem::s_pRsdSystem;
}

//================================================================================
// ::radSoundHalSystemInitialize
//================================================================================
void radSoundHalSystemInitialize(radMemoryAllocator allocator)
{
    // Allow multiple init calls safely
    g_RadSoundInitializeCount++;
    if (g_RadSoundInitializeCount > 1)
        return;

    rAssert(radSoundHalSystem::s_pRsdSystem == NULL);

    new ("radSoundHalSystem", allocator) radSoundHalSystem(allocator);
    radSoundHalSystem::s_pRsdSystem->AddRef();
}

//================================================================================
// ::radSoundHalSystemTerminate
//================================================================================
void radSoundHalSystemTerminate(void)
{
    // Safe terminate pairing with Initialize
    if (g_RadSoundInitializeCount <= 0)
        return;

    g_RadSoundInitializeCount--;
    if (g_RadSoundInitializeCount > 0)
        return;

    rAssert(radSoundHalSystem::s_pRsdSystem != NULL);
    radSoundHalSystem::s_pRsdSystem->Release();
}
