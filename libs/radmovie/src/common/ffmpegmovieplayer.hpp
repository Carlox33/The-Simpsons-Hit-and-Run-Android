//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        movieplayer.hpp
//
// Subsystem:	Foundation Technologies - Movie Player
//
// Description:	This file contains the declaration for the Foundation Tech 
//              Movie Player
//
// Date:    	May 16, 2002
//
//=============================================================================

#ifndef MOVIEPLAYER_HPP
#define MOVIEPLAYER_HPP

//=============================================================================
// Include Files
//=============================================================================

#include <radoptions.hpp>

#ifndef RAD_MOVIEPLAYER_USE_BINK


#include <al.h>
#include <radtime.hpp>
#include <radmovie2.hpp>
#include <radlinkedclass.hpp>
#include <radsound.hpp>
#include <radstring.hpp>
#include "radmovieinterfaces.hpp"

//=============================================================================
// Forward Class/Struct Declarations
//=============================================================================

class radMoviePlayer;
struct AVFormatContext;
struct AVCodecContext;
struct SwsContext;
struct SwrContext;
struct AVFrame;
struct AVPacket;

//=============================================================================
// Type Definitions
//=============================================================================

//=============================================================================
// Class radMoviePlayer
//=============================================================================

class radMoviePlayer 
    : 
    public IRadMoviePlayer2,
    public radLinkedClass< radMoviePlayer >,
    public radRefCount
{
    public:

        IMPLEMENT_REFCOUNTED( "radMoviePlayer" )
    
        //
        // Constructor / Destructor
        //

        radMoviePlayer( void );
        virtual ~radMoviePlayer( void );

        //
        // IRadMoviePlayer
        //

        virtual void Initialize( 
            IRadMovieRenderLoop * pIRadMovieRenderLoop, 
            IRadMovieRenderStrategy * pIRadMovieRenderStrategy );

        virtual bool Render( void );

        virtual void Load( const char * pVideoFileName, unsigned int audioTrackIndex );
        virtual void Unload( void );
        virtual void Play( void );
        virtual void Pause( void );

        virtual void  SetPan( float pan ); // -1.0 to 1.0 ( left to right )
        virtual float GetPan( void );
        virtual void  SetVolume( float volume ); // 0.0 to 1.0 ( min to max )
        virtual float GetVolume( void );
        virtual State GetState( void );
        virtual bool  GetVideoFrameInfo(VideoFrameInfo *frameInfo);
        virtual float GetFrameRate( void );
        virtual unsigned int GetCurrentFrameNumber( void );

        //
        // Internal public functions
        //

        void Service( void );

    private:

        void SetState( IRadMoviePlayer2::State state );
        void InternalPlay( void );
        void FlushAudioQueue( void );

        ref< IRadMovieRenderStrategy > m_refIRadMovieRenderStrategy;
        ref< IRadMovieRenderLoop > m_refIRadMovieRenderLoop;
        ref< IRadStopwatch > m_refIRadStopwatch;
        
        IRadMoviePlayer2::State m_State;

        enum VideoFrameState { VideoFrame_Unlocked, VideoFrame_Locked };
        VideoFrameState m_VideoFrameState;

        unsigned int m_VideoTrackIndex;
        unsigned int m_AudioTrackIndex;
        unsigned int m_PresentationTime;
        unsigned int m_PresentationDuration;

        AVFormatContext* m_pFormatCtx;
        AVCodecContext * m_pVideoCtx;
        AVCodecContext * m_pAudioCtx;
        SwsContext * m_pSwsCtx;
        SwrContext * m_pSwrCtx;

        AVPacket* m_pPacket;
        AVFrame * m_pVideoFrame;
        AVFrame * m_pAudioFrame;

        ALuint m_AudioSource;
		
		// --- ANDROID/FFMPEG timing fixes ---
		bool m_HasPtsBase;
		int64_t m_PtsBase;
		unsigned int m_DefaultFrameDurationMs;

        bool m_ClockSynced;
        unsigned int m_ClockOffsetMs;
        // opcional (si lo haces como miembro, mejor que static):
        bool m_RenderedFirstEarly;
};

#endif // ! RAD_MOVIEPLAYER_USE_BINK
#endif // MOVIEPLAYER_HPP
