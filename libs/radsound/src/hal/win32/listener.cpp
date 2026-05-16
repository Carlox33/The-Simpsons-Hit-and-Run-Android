//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        listener.cpp
//
// Subsystem:	Radical Sound System
//
// Description:	Listener object for positional sound
//
// Creation TH RS
//
//=============================================================================

//============================================================================
// Include Files
//============================================================================

#include "pch.hpp"
#include "listener.hpp"
#include "system.hpp"
#ifndef RAD_ANDROID
  #include <efx.h>
#endif
#ifdef RAD_ANDROID
  #include <AL/efx.h>
#endif

//============================================================================
// Static definitions
//============================================================================

/* static */ radSoundHalListener * radSoundHalListener::s_pTheRadSoundHalListener = NULL;


//============================================================================
// radSoundHalListener::radSoundHalListener
//============================================================================

radSoundHalListener::radSoundHalListener
(
    ALCcontext * pContext
)
	:
    m_EnvEffectsEnabled( false ),
	m_RolloffFactor( 1.0f ),
    m_EnvAuxSend( 0 ),
	m_IsEfxListenerClean( true )
{
	rAssert( s_pTheRadSoundHalListener == NULL );
	rAssert( pContext != NULL );

	s_pTheRadSoundHalListener = this;
	m_pContext = pContext;

    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alGetError();
}

//============================================================================
// radSoundHalListener::~radSoundHalListener
//============================================================================

radSoundHalListener::~radSoundHalListener
(
	void
)
{
	rAssert( s_pTheRadSoundHalListener == this );
	s_pTheRadSoundHalListener = NULL;
}

//========================================================================
// radSoundHalListener::SetPosition
//========================================================================

void radSoundHalListener::SetPosition
(
	radSoundVector * pPosition
)
{
	rAssert( pPosition != NULL );

	alListener3f(AL_POSITION, pPosition->m_x, pPosition->m_y, -pPosition->m_z);
}

//========================================================================
// radSoundHalListener::GetPosition
//========================================================================

void radSoundHalListener::GetPosition
(
	radSoundVector * pPosition
)
{
	rAssert( pPosition != NULL );

	alGetListenerfv(AL_POSITION, (float*)pPosition);
	pPosition->m_z *= -1;
}

//========================================================================
// radSoundHalListener::SetVelocity
//========================================================================

void radSoundHalListener::SetVelocity
(
    radSoundVector * pVelocity
)
{
	rAssert( pVelocity != NULL );

#ifdef RAD_DEBUG
	
	if ( pVelocity->GetLength( ) > 100.0f )
	{
		rDebugPrintf( "The radSoundHalListener is going really really fast\n" );
	}
	
#endif
	
	alListener3f(AL_VELOCITY, pVelocity->m_x, pVelocity->m_y, -pVelocity->m_z);
}

//========================================================================
// radSoundHalListener::GetVelocity
//========================================================================

void radSoundHalListener::GetVelocity
(
    radSoundVector * pVelocity
)
{
    rAssert( pVelocity != NULL );

	alGetListenerfv(AL_VELOCITY, (float*)pVelocity);
	pVelocity->m_z *= -1;
}

//========================================================================
// radSoundHalListener::SetOrientation
//========================================================================

void radSoundHalListener::SetOrientation
(
	radSoundVector * pOrientationFront,
	radSoundVector * pOrientationTop
)
{
	rAssert( pOrientationFront != NULL );
	rAssert( pOrientationTop != NULL );

    radSoundVector orientation[] = {
        *pOrientationFront,
        *pOrientationTop
    };
	orientation[0].m_z *= -1;
	orientation[1].m_z *= -1;
    alListenerfv(AL_ORIENTATION, (float*)orientation);
}

//========================================================================
// radSoundHalListener::GetOrientation
//========================================================================

void radSoundHalListener::GetOrientation
(
	radSoundVector * pOrientationFront,
	radSoundVector * pOrientationTop
)
{
    radSoundVector orientation[2];
    alGetListenerfv(AL_ORIENTATION, (float*)orientation);

	if( pOrientationFront != NULL )
	{
		*pOrientationFront = orientation[0];
		pOrientationFront->m_z *= -1;
	}

	if( pOrientationTop != NULL )
	{
		*pOrientationTop = orientation[1];
		pOrientationTop->m_z *= -1;
	}
}

//============================================================================
// radSoundHalListener::SetDistanceFactor
//============================================================================

void radSoundHalListener::SetDistanceFactor
(
	float distanceFactor
)
{
	alListenerf(AL_METERS_PER_UNIT, distanceFactor);
	alSpeedOfSound(343.3f / distanceFactor);
}

//============================================================================
// radSoundHalListener::GetDistanceFactor
//============================================================================

float radSoundHalListener::GetDistanceFactor( void )
{
	ALfloat distanceFactor;
	alGetListenerf(AL_METERS_PER_UNIT, &distanceFactor);
	return distanceFactor;
}

//============================================================================
// radSoundHalListener::SetRollOffFactor
//============================================================================

void radSoundHalListener::SetRollOffFactor
(
	float rolloffFactor
)
{
	m_RolloffFactor = rolloffFactor;
}

//============================================================================
// radSoundHalListener::GetRollOffFactor
//============================================================================

float radSoundHalListener::GetRollOffFactor( void )
{
	return m_RolloffFactor;
}

//============================================================================
// radSoundHalListener::SetDopplerFactor
//============================================================================

void radSoundHalListener::SetDopplerFactor
(
	float dopplerFactor
)
{
    alDopplerFactor(dopplerFactor);
}

//============================================================================
// radSoundHalListener::GetDopplerFactor
//============================================================================

float radSoundHalListener::GetDopplerFactor( void )
{
	ALfloat dopplerFactor;
	alGetListenerf(AL_DOPPLER_FACTOR, &dopplerFactor);
	return dopplerFactor;
}

//============================================================================
// radSoundHalListener::SetEnvEffectsEnabled
//============================================================================

void radSoundHalListener::SetEnvEffectsEnabled( bool enabled )
{
    m_EnvEffectsEnabled = enabled;
	m_IsEfxListenerClean = false;
}

//============================================================================
// radSoundHalListener::GetEnvEffectsEnabled
//============================================================================

bool radSoundHalListener::GetEnvEffectsEnabled( void )
{
    return m_EnvEffectsEnabled;
}

//============================================================================
// radSoundHalListener::SetEnvironmentAuxSend
//============================================================================

void radSoundHalListener::SetEnvironmentAuxSend( unsigned int auxsend )
{
    rAssert( auxsend < radSoundHalSystem::GetInstance( )->GetNumAuxSends( ) );
    m_EnvAuxSend = auxsend;
	m_IsEfxListenerClean = false;
}

//============================================================================
// radSoundHalListener::GetEnvironmentAuxSend
//============================================================================

unsigned int radSoundHalListener::GetEnvironmentAuxSend( void )
{
    return m_EnvAuxSend;
}

//============================================================================
// radSoundHalListener::UpdatePositionalSettings
//============================================================================

void radSoundHalListener::UpdatePositionalSettings
(
	void
)
{
    // Update positional info

	radSoundHalPositionalGroup * pRadSoundHalPositionalGroup =
		radSoundHalPositionalGroup::GetLinkedClassHead( );

	alcSuspendContext(m_pContext);

	if (!m_IsEfxListenerClean)
	{
		m_IsEfxListenerClean = true;
		ref<radSoundHalSystem> refSystem = radSoundHalSystem::GetInstance();
		for (unsigned int i = 0; i < refSystem->GetNumAuxSends(); i++)
		{
			alAuxiliaryEffectSloti(
				refSystem->GetOpenALAuxSlot(i),
				AL_EFFECTSLOT_AUXILIARY_SEND_AUTO,
				m_EnvEffectsEnabled && i == m_EnvAuxSend);
			rAssert(alGetError() == AL_NO_ERROR);
		}
	}

	while ( pRadSoundHalPositionalGroup != NULL )
	{
		pRadSoundHalPositionalGroup->UpdatePositionalSettings( m_RolloffFactor );

		pRadSoundHalPositionalGroup = pRadSoundHalPositionalGroup->GetLinkedClassNext( );
	}

	alcProcessContext(m_pContext);
}

//============================================================================
// radSoundHalListener::GetInstance
//============================================================================

radSoundHalListener * radSoundHalListener::GetInstance( void )
{
    return s_pTheRadSoundHalListener;
}

//============================================================================
// ::radSoundHalListenerGet
//============================================================================

/* static */ IRadSoundHalListener * radSoundHalListenerGet( void )
{
    return radSoundHalListener::GetInstance( );
}

//============================================================================
// ::radSoundHalListenerWinInitialize
//============================================================================

/* static */ void radSoundHalListener::Initialize
( 
	radMemoryAllocator allocator,
	ALCcontext * pContext
)
{
    rAssert( radSoundHalListener::s_pTheRadSoundHalListener == NULL );

    new ( "radSoundHalListener", allocator ) radSoundHalListener( pContext );
    radSoundHalListener::s_pTheRadSoundHalListener->AddRef( );
}


//========================================================================
// radSoundHalListener::Terminate
//========================================================================

/* static */ void radSoundHalListener::Terminate( void )
{
	rAssert( radSoundHalListener::s_pTheRadSoundHalListener != NULL );
	radSoundHalListener::s_pTheRadSoundHalListener->Release( );
}
