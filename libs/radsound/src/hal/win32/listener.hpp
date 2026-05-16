//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        listener.hpp
//
// Subsystem:	Radical Sound System
//
// Description:	Listener object for positional sound
//
// Creation TH RS
//
//=============================================================================

#ifndef LISTENER_HPP
#define LISTENER_HPP


//============================================================================
// Include Files
//============================================================================

#include "radsoundwin.hpp"
#include "positionalgroup.hpp"
#include "../common/rolloff.hpp"
#ifndef RAD_ANDROID
  #include <efx.h>
#endif
#ifdef RAD_ANDROID
  #include <AL/efx.h>
#endif

//============================================================================
// Component: radSoundHalListener
//============================================================================

struct radSoundHalListener
	:
    public IRadSoundHalListener,
    public radSoundObject
{
	public:

		IMPLEMENT_REFCOUNTED( "radSoundHalListener" )

		static void Initialize( radMemoryAllocator allocator, ALCcontext * context );
		static radSoundHalListener * GetInstance( void );
		static void Terminate( void );

		virtual void SetPosition( radSoundVector * pPos );
		virtual void GetPosition( radSoundVector * pPos );
		virtual void SetOrientation( radSoundVector * pOrientationFront, radSoundVector * pOrientationTop );
		virtual void GetOrientation( radSoundVector * pOrientationFront, radSoundVector * pOrientationTop );
		virtual void SetVelocity( radSoundVector * pVelocity );
		virtual void GetVelocity( radSoundVector * pVelocity );
		virtual void SetDistanceFactor( float distanceFactor );
		virtual float GetDistanceFactor( void );
		virtual void SetDopplerFactor( float doppleFactor );
		virtual float GetDopplerFactor( void );
		virtual void SetRollOffFactor( float rollOffFactor );
		virtual float GetRollOffFactor( void );
        virtual void  SetEnvEffectsEnabled( bool enabled );
        virtual bool  GetEnvEffectsEnabled( void );
        virtual void  SetEnvironmentAuxSend( unsigned int auxsend );
        virtual unsigned int GetEnvironmentAuxSend( void );

		void UpdatePositionalSettings( void );

	private:

		radSoundHalListener( ALCcontext * pContext );
		~radSoundHalListener( void );

		static radSoundHalListener * s_pTheRadSoundHalListener;
		ALCcontext * m_pContext;

        bool m_EnvEffectsEnabled;
        unsigned int m_EnvAuxSend;
        
		float m_RolloffFactor;

        bool m_IsEfxListenerClean;
		LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
};

#endif // LISTENER_HPP