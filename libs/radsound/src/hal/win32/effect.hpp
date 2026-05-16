//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================
//
// File:        effect.hpp
//
// Subsystem:	Radical Sound System
//
// Description:	Effects on the PC
//
// Creation:    RWS
//
//=============================================================================

#ifndef EFFECT_HPP
#define EFFECT_HPP

//============================================================================
// Include Files
//============================================================================

#include "../../common/radsoundobject.hpp"
#include <radsound_win32.hpp>

#ifndef RAD_ANDROID
  #include <efx.h>
#endif
#ifdef RAD_ANDROID
  #include <AL/efx.h>
#endif

//============================================================================
// Forward Declarations
//============================================================================

class radSoundHalEffectEAX2Reverb;

//============================================================================
// class radSoundHalEffectEAX2Reverb
//============================================================================

class radSoundHalEffectEAX2Reverb
    :
    public IRadSoundHalEffectEAX2Reverb,
    public radSoundObject
{
    public:

        IMPLEMENT_REFCOUNTED( "radSoundHalEffectEAX2Reverb" );

        radSoundHalEffectEAX2Reverb( void );
        virtual ~radSoundHalEffectEAX2Reverb( void );

        // IRadSoundHalEffectWin

        virtual void Attach( unsigned int auxSend );
        virtual void Detach( void );
        virtual void Update( void );

        // Interface IRadSoundHalEffectI3DL2ReverbWin32

        virtual void  SetRoom( int mBValue );
        virtual int   GetRoom( void );
        virtual void  SetRoomHF( int mBValue );
        virtual int   GetRoomHF( void );
        virtual void  SetRoomRolloffFactor( float value );
        virtual float GetRoomRolloffFactor( void );
        virtual void  SetDecayTime( float value );
        virtual float GetDecayTime( void );
        virtual void  SetDecayHFRatio( float value );
        virtual float GetDecayHFRatio( void );
        virtual void  SetReflections( int mBValue );
        virtual int   GetReflections( void );
        virtual void  SetReflectionsDelay( float value );
        virtual float GetReflectionsDelay( void );
        virtual void  SetReverb( int mBValue );
        virtual int   GetReverb( void );
        virtual void  SetReverbDelay( float value );
        virtual float GetReverbDelay( void );
        virtual void  SetEnvironmentSize( float size ); 
        virtual float GetEnvironmentSize( void ); 
        virtual void  SetEnvironmentDiffusion( float value );
        virtual float GetEnvironmentDiffusion( void );
        virtual void  SetAirAbsorptionHF( float value );
        virtual float GetAirAbsorptionHF( void );

    private:

        void OnParameterUpdated( void );

        ALuint m_AuxSlot;
        ALuint m_Effect;

        LPALGENEFFECTS alGenEffects;
        LPALDELETEEFFECTS alDeleteEffects;
        LPALEFFECTI alEffecti;
        LPALEFFECTF alEffectf;
        LPALGETEFFECTI alGetEffecti;
        LPALGETEFFECTF alGetEffectf;
        LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
};

#endif // EFFECT_HPP