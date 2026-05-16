//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================
//
// File: system.hpp
//
//=============================================================================

#ifndef SOUND_HAL_OPENAL_SYSTEM_HPP
#define SOUND_HAL_OPENAL_SYSTEM_HPP

//============================================================================
// Include Files
//============================================================================
#include <radsound_hal.hpp>
#include "radsoundwin.hpp"
#include "../../common/radsoundupdatableobject.hpp"

// OpenAL (OpenAL Soft también en Android)
#include <AL/al.h>
#include <AL/alc.h>

// EFX types (solo tipos; si no está disponible en tu build, lo cambiamos)
#include <AL/efx.h>

//============================================================================
// Definitions
//============================================================================
#define RSD_SYSTEM_MAX_AUX_SENDS 2

//============================================================================
// Component: radSoundHalSystem
//============================================================================
class radSoundHalSystem
    :
    public IRadSoundHalSystem,
    public radSoundObject
{
public:
    IMPLEMENT_REFCOUNTED("radSoundHalSystem")

    radSoundHalSystem(radMemoryAllocator allocator);
    ~radSoundHalSystem(void);

    virtual void Initialize(const SystemDescription& systemDescription);
    virtual IRadSoundHalMemoryRegion* GetRootMemoryRegion(void);
    virtual unsigned int GetNumAuxSends();
    virtual void SetOutputMode(radSoundOutputMode mode);
    virtual radSoundOutputMode GetOutputMode(void);
    virtual void Service(void);
    virtual void ServiceOncePerFrame(void);
    virtual void GetStats(IRadSoundHalSystem::Stats* pStats);

    virtual void SetAuxEffect(unsigned int auxNumber, IRadSoundHalEffect* pIRadSoundHalEffect);
    virtual IRadSoundHalEffect* GetAuxEffect(unsigned int auxNumber);

    virtual void SetAuxGain(unsigned int aux, float gain);
    virtual float GetAuxGain(unsigned int aux);

    // Estos métodos deben existir también en Android si usamos OpenAL Soft
    ALCdevice*  GetOpenALDevice(void);
    ALCcontext* GetOpenALContext(void);
    ALuint      GetOpenALAuxSlot(unsigned int aux);

    static radSoundHalSystem* GetInstance(void);
    static radSoundHalSystem* s_pRsdSystem;

private:
// este private tiene inizializaciones a 0 y a nullptr lo cual el port original de switch no posee 
    void*       m_pSoundMemory = nullptr;

    ALCint      m_NumAuxSends  = 0;
    ALuint      m_AuxSlots[RSD_SYSTEM_MAX_AUX_SENDS] = {0};

    ALCdevice*  m_pDevice  = nullptr;
    ALCcontext* m_pContext = nullptr;

    ref<IRadSoundHalEffect> m_refIRadSoundHalEffect[RSD_SYSTEM_MAX_AUX_SENDS];

    unsigned int m_LastServiceTime = 0;
	/*
	 LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
        LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
        LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
        LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
*/

    // EFX function pointers (si no hay EFX, quedan nullptr y m_NumAuxSends se fuerza a 0)
    LPALGENAUXILIARYEFFECTSLOTS     alGenAuxiliaryEffectSlots     = nullptr;
    LPALDELETEAUXILIARYEFFECTSLOTS  alDeleteAuxiliaryEffectSlots  = nullptr;
    LPALAUXILIARYEFFECTSLOTF        alAuxiliaryEffectSlotf        = nullptr;
    LPALGETAUXILIARYEFFECTSLOTF     alGetAuxiliaryEffectSlotf     = nullptr;
};

#endif // SOUND_HAL_OPENAL_SYSTEM_HPP
