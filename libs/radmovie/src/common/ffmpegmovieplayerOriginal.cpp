//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        movieplayer.cpp
//
// Subsystem:	Foundation Technologies - Movie
//
// Description:	This file contains the implementation of the Foundation 
//              Technologies movie player
//
// Date:    	May 16, 2002
//
//=============================================================================

//=============================================================================
// Include Files
//=============================================================================

#include <radoptions.hpp>

#ifndef RAD_MOVIEPLAYER_USE_BINK

#include <raddebug.hpp>
#include <radmath/radmath.hpp>
#include "radmoviefile.hpp"
#include "ffmpegmovieplayer.hpp"
#include "audiodatasource.hpp"

#include <vector>


extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libswresample/swresample.h>
}

//=============================================================================
// Constants
//=============================================================================

const char * radMovieDebugChannel2 = "radMovie";
unsigned int const radMovie_NoAudioTrack = 0xFFFFFFFF;

//=============================================================================
// Local Definitions
//=============================================================================

//
// If the video of the movie player starts falling behind the audio,
// the movie player will not render frames until it catches up.  This
// value controls how many milliseconds the video can fall behind before
// the catch-up process begins
//

#define RAD_MOVIE_PLAYER_VIDEO_LAG 10

#define AV_CHK(x) if (int error = (x) < 0) { \
        char str[AV_ERROR_MAX_STRING_SIZE]; \
        av_strerror( error, str, AV_ERROR_MAX_STRING_SIZE ); \
        rDebugPrintf( "%s at %s:%d\n", str, __FILE__, __LINE__ ); \
        SetState( IRadMoviePlayer2::NoData ); \
        return; \
    };

//=============================================================================
// Static Members
//=============================================================================

template<> radMoviePlayer* radLinkedClass< radMoviePlayer >::s_pLinkedClassHead = NULL;
template<> radMoviePlayer* radLinkedClass< radMoviePlayer >::s_pLinkedClassTail = NULL;

//=============================================================================
// Public Member Functions
//=============================================================================

//=============================================================================
// radMoviePlayer::radMoviePlayer
//=============================================================================

radMoviePlayer::radMoviePlayer( void )
    :
    radRefCount( 0 ),
    m_refIRadMovieRenderStrategy( NULL ),
    m_refIRadMovieRenderLoop( NULL ),
    m_refIRadStopwatch( NULL ),
    m_State( IRadMoviePlayer2::NoData ),
    m_VideoFrameState( VideoFrame_Unlocked ),
    m_VideoTrackIndex( 0 ),
    m_AudioTrackIndex( 0 ),
    m_PresentationTime( 0 ),
    m_PresentationDuration( 0 ),
    m_pFormatCtx( NULL ),
    m_pVideoCtx( NULL ),
    m_pAudioCtx( NULL ),
    m_pSwsCtx( NULL ),
    m_pSwrCtx( NULL ),
    m_pPacket( NULL ),
    m_pVideoFrame( NULL ),
    m_pAudioFrame( NULL ),
    m_AudioSource( 0 )
{
    radTimeCreateStopwatch( &m_refIRadStopwatch, radTimeUnit_Millisecond, GetThisAllocator( ) );
}

//=============================================================================
// radMoviePlayer::~radMoviePlayer
//=============================================================================

radMoviePlayer::~radMoviePlayer( void )
{
}

//=============================================================================
// radMoviePlayer::Initialize
//=============================================================================

void radMoviePlayer::Initialize
( 
    IRadMovieRenderLoop * pIRadMovieRenderLoop, 
    IRadMovieRenderStrategy * pIRadMovieRenderStrategy
)
{
    rAssert( pIRadMovieRenderStrategy != NULL );

    // Hang on to the loop and strategy

    m_refIRadMovieRenderLoop = pIRadMovieRenderLoop;
    m_refIRadMovieRenderStrategy = pIRadMovieRenderStrategy;

    //
    // Update the state
    //

    SetState( IRadMoviePlayer2::NoData );
}

//=============================================================================
// radMoviePlayer::Render
//=============================================================================

bool radMoviePlayer::Render( void )
{
    rAssert( m_refIRadMovieRenderStrategy != NULL );
    rAssert( m_pVideoFrame != NULL );
        
    if( m_pVideoFrame != NULL )
    {
        bool ret = m_refIRadMovieRenderStrategy->Render( );

        ALint state;
        alGetSourcei( m_AudioSource, AL_SOURCE_STATE, &state );
        if( state != AL_PLAYING )  // have we fallen behind, if so reset
            alSourcePlay( m_AudioSource );

        FlushAudioQueue();

        //
        // The video frame has been used.  The next can be decoded
        //

        m_VideoFrameState = VideoFrame_Unlocked;

        return ret;
    }
    else
    {
        rAssert( false );
        return false;
    }
}

//=============================================================================
// radMoviePlayer::Load
//=============================================================================

void radMoviePlayer::Load( const char * pVideoFileName, unsigned int audioTrackIndex )
{   
    rAssert( m_State == IRadMoviePlayer2::NoData );
    rAssert( pVideoFileName != NULL );

    rDebugPrintf( "radMoviePlayer: Loading %s\n", pVideoFileName );

    m_refIRadStopwatch->Stop( );
    m_refIRadStopwatch->Reset( );

    //
    // Reset the variables
    //

    m_VideoFrameState = VideoFrame_Unlocked;

    SetState( IRadMoviePlayer2::Loading );

    m_pFormatCtx = avformat_alloc_context();
    AV_CHK( avformat_open_input( &m_pFormatCtx, pVideoFileName, NULL, NULL ) );
    AV_CHK( avformat_find_stream_info( m_pFormatCtx, NULL ) );

    const AVCodec* pVideoCodec = NULL;
    m_VideoTrackIndex = av_find_best_stream( m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pVideoCodec, 0 );
    AVCodecParameters* pVideoParams = m_pFormatCtx->streams[m_VideoTrackIndex]->codecpar;
    m_pVideoCtx = avcodec_alloc_context3( pVideoCodec );
    AV_CHK( avcodec_parameters_to_context( m_pVideoCtx, pVideoParams ) );
    AV_CHK( avcodec_open2( m_pVideoCtx, pVideoCodec, NULL ) );

#ifndef RAD_VITAGL
    m_pSwsCtx = sws_getContext(
        pVideoParams->width,
        pVideoParams->height,
        AV_PIX_FMT_YUV420P,
        pVideoParams->width,
        pVideoParams->height,
        AV_PIX_FMT_BGRA,
        0, NULL, NULL, NULL
    );
#endif

    if( audioTrackIndex != radMovie_NoAudioTrack )
    {
        const AVCodec* pAudioCodec = NULL;
        m_AudioTrackIndex = av_find_best_stream( m_pFormatCtx, AVMEDIA_TYPE_AUDIO, audioTrackIndex + 1, -1, &pAudioCodec, 0 );
        AVCodecParameters* pAudioParams = m_pFormatCtx->streams[m_AudioTrackIndex]->codecpar;
        m_pAudioCtx = avcodec_alloc_context3( pAudioCodec );
        AV_CHK( avcodec_parameters_to_context( m_pAudioCtx, pAudioParams ) );
        AV_CHK( avcodec_open2( m_pAudioCtx, pAudioCodec, NULL ) );

        AVChannelLayout layout = { AV_CHANNEL_ORDER_NATIVE, 2, AV_CH_LAYOUT_STEREO };
        AV_CHK( swr_alloc_set_opts2( &m_pSwrCtx,
            &layout,
            AV_SAMPLE_FMT_S16,
            pAudioParams->sample_rate,
            &pAudioParams->ch_layout,
            (AVSampleFormat)pAudioParams->format,
            pAudioParams->sample_rate,
            0,
            NULL ) );
        swr_init( m_pSwrCtx );
    }
    else
    {
        m_AudioTrackIndex = 0;
    }

    m_pPacket = av_packet_alloc();
    m_pVideoFrame = av_frame_alloc();
    m_pAudioFrame = av_frame_alloc();

    alGenSources( 1, &m_AudioSource );
    alSourcei( m_AudioSource, AL_SOURCE_RELATIVE, AL_TRUE );

    //
    // Initialize the render strategy & and pass the file to
    // the decoder to deal with
    //

    m_refIRadMovieRenderStrategy->ChangeParameters( pVideoParams->width, pVideoParams->height );

    //
    // Print out some helpful information
    //
    rDebugPrintf( "\nradMoviePlayer: Summary\n" \
                  "     * Resolution         [%dx%d]\n" \
                  "     * Format             [%d]\n" \
                  "     * Audio Track        [%d]\n\n",
        pVideoParams->width, pVideoParams->height,
        pVideoParams->format, m_AudioTrackIndex );

    // The buffered data source's input must be set before the stream player's

    SetState( IRadMoviePlayer2::ReadyToPlay );

    Service( );
}

//=============================================================================
// radMoviePlayer::Unload
//=============================================================================

void radMoviePlayer::Unload( void )
{
    if( m_State != IRadMoviePlayer2::NoData )
    {
        alSourceStop( m_AudioSource );
        FlushAudioQueue();
        alDeleteSources( 1, &m_AudioSource );

#ifndef RAD_VITAGL
        sws_freeContext( m_pSwsCtx );
#endif
        swr_free( &m_pSwrCtx );
        av_packet_free( &m_pPacket );
        av_frame_free( &m_pVideoFrame );
        av_frame_free( &m_pAudioFrame );
        avcodec_free_context( &m_pVideoCtx );
        avcodec_free_context( &m_pAudioCtx );
        avformat_close_input( &m_pFormatCtx );
        avformat_free_context( m_pFormatCtx );

        m_refIRadStopwatch->Stop( );
        m_refIRadStopwatch->Reset( );

        m_VideoFrameState = VideoFrame_Unlocked;

        SetState( IRadMoviePlayer2::NoData );
    }
}

//=============================================================================
// radMoviePlayer::Play
//=============================================================================

void radMoviePlayer::Play( void )
{
    if( m_State == IRadMoviePlayer2::ReadyToPlay )
    {
        SetState( IRadMoviePlayer2::Playing );
        InternalPlay( );
    }
    else if( m_State == IRadMoviePlayer2::Loading )
    {
        SetState( IRadMoviePlayer2::LoadToPlay );
    }
}

//=============================================================================
// radMoviePlayer::Pause
//=============================================================================

void radMoviePlayer::Pause( void )
{
    if( m_State == IRadMoviePlayer2::Playing )
    {
        m_refIRadStopwatch->Stop( );
        SetState( IRadMoviePlayer2::ReadyToPlay );
    }
    else if( m_State == IRadMoviePlayer2::LoadToPlay )
    {
        SetState( Loading );
    }
}

//=============================================================================
// radMoviePlayer::SetPan
//=============================================================================

void radMoviePlayer::SetPan( float pan )
{
    // Not supported
}

//=============================================================================
// radMoviePlayer::GetPan
//=============================================================================

float radMoviePlayer::GetPan( void )
{
    return 0.0f;
}

//=============================================================================
// radMoviePlayer::SetVolume
//=============================================================================

void radMoviePlayer::SetVolume( float volume )
{
    // Not supported
}

//=============================================================================
// radMoviePlayer::GetVolume
//=============================================================================

float radMoviePlayer::GetVolume( void )
{
    return 1.0f;
}

//=============================================================================
// radMoviePlayer::GetState
//=============================================================================

IRadMoviePlayer2::State radMoviePlayer::GetState( void )
{
    return m_State;
}

//=============================================================================
// radMoviePlayer::GetVideoFrameInfo
//=============================================================================

bool radMoviePlayer::GetVideoFrameInfo( VideoFrameInfo * frameInfo)
{
    rAssert( frameInfo != NULL );

    if( m_State == IRadMoviePlayer2::Playing ||
        m_State == IRadMoviePlayer2::ReadyToPlay )
    {
        ( * frameInfo ).Width = m_pVideoFrame->width;
        ( * frameInfo ).Height = m_pVideoFrame->height;
        return true;
    }
    else
    {
        return false;
    }
}

//=============================================================================
// radMoviePlayer::GetFrameRate
//=============================================================================

float radMoviePlayer::GetFrameRate( void )
{
    // Sweet little lies, frame number is in milliseconds
    return 1000.0f;
}

//=============================================================================
// radMoviePlayer::GetCurrentFrameNumber
//=============================================================================

unsigned int radMoviePlayer::GetCurrentFrameNumber( void )
{
    return m_PresentationTime;
}

//=============================================================================
// radMoviePlayer::Service
//=============================================================================

void radMoviePlayer::Service( void )
{
    //
    // Start off by letting the decoder figure out where it is
    //

    //
    // Now figure out where we are.
    //



    if( m_State == IRadMoviePlayer2::NoData )
    {
        // Nothing to do
    }
    else if( m_State == IRadMoviePlayer2::Playing )
    {
        //
        // If the video frame is unlocked, we can issue the next decode request.
        //

        if( m_VideoFrameState == VideoFrame_Unlocked )
        {
            //
            // Keep an eye on the states of things to detect the end of the movie
            //

            if( av_read_frame( m_pFormatCtx, m_pPacket ) >= 0 )
            {
                if( m_pPacket->stream_index == m_VideoTrackIndex )
                {
                    AV_CHK( avcodec_send_packet( m_pVideoCtx, m_pPacket ) );

                    //
                    // Decompress all pending video frames
                    //

                    while( avcodec_receive_frame( m_pVideoCtx, m_pVideoFrame ) >= 0 )
                    {
                        // Reset the list of render destinations

                        m_refIRadMovieRenderStrategy->ResetDestinations();

                        // Now go through the list destinations until we've filled 
                        // them all up with the freshly decoded data

                        IRadMovieRenderStrategy::LockedDestination dest;

                        while( m_refIRadMovieRenderStrategy->LockNextDestination( &dest ) > 0 )
                        {
                            uint8_t* pDest = (uint8_t*)dest.m_pDest;

#ifdef RAD_VITAGL
                            memcpy( pDest, m_pVideoFrame->data[0], m_pVideoFrame->linesize[0] * m_pVideoFrame->height );
                            pDest += m_pVideoFrame->linesize[0] * m_pVideoFrame->height;
                            memcpy( pDest, m_pVideoFrame->data[1], m_pVideoFrame->linesize[1] * m_pVideoFrame->height / 2 );
                            pDest += m_pVideoFrame->linesize[1] * m_pVideoFrame->height / 2;
                            memcpy( pDest, m_pVideoFrame->data[2], m_pVideoFrame->linesize[2] * m_pVideoFrame->height / 2 );
#else

                            // If one of these copies fail, we'll have to skip this frame
                            // and not iterate the loop
                            if( sws_scale( m_pSwsCtx,
                                m_pVideoFrame->data, m_pVideoFrame->linesize,
                                dest.m_SrcPosY, m_pVideoFrame->height - dest.m_SrcPosY,
                                &pDest, &dest.m_DestPitch ) >= 0 )
#endif
                            {
                                AVRational rational = m_pFormatCtx->streams[0]->time_base;
                                m_PresentationTime = (m_pVideoFrame->pts * rational.num * 1000) / rational.den;
                                m_PresentationDuration = (m_pVideoFrame->duration * rational.num * 1000) / rational.den;
                                m_VideoFrameState = VideoFrame_Locked;
                            }

                            m_refIRadMovieRenderStrategy->UnlockDestination();
                        }
                    }
                }
                else if( m_AudioTrackIndex > 0 && m_pPacket->stream_index == m_AudioTrackIndex )
                {
                    AV_CHK( avcodec_send_packet( m_pAudioCtx, m_pPacket ) );

                    //
                    // Decompress all pending audio frames
                    //

                    while( avcodec_receive_frame( m_pAudioCtx, m_pAudioFrame ) >= 0 )
                    {
                        uint8_t* output;
                        int outSamples = swr_get_out_samples( m_pSwrCtx, m_pAudioFrame->nb_samples );
                        av_samples_alloc( &output, NULL, 2, outSamples,
                                     AV_SAMPLE_FMT_S16, 0 );
                        outSamples = swr_convert( m_pSwrCtx, &output, outSamples,
                                                  (const uint8_t**)m_pAudioFrame->data, m_pAudioFrame->nb_samples );
                        int bufferSize = av_samples_get_buffer_size( NULL, 2,
                                outSamples, AV_SAMPLE_FMT_S16, 0 );

                        // TODO: Add buffer queuing support to radsound and move this code to that module.
                        ALuint buffer;
                        alGenBuffers( 1, &buffer );
                        alBufferData( buffer, AL_FORMAT_STEREO16, output, bufferSize, m_pAudioFrame->sample_rate );
                        alSourceQueueBuffers( m_AudioSource, 1, &buffer );
                        av_freep( &output );
                    }
                }

                av_packet_unref( m_pPacket );
            }
            else
            {
                rDebugChannelPrintf( radMovieDebugChannel2, "radMoviePlayer: Out of data at [%lld]\n", m_pVideoFrame->pts );

                // We've hit the end of the movie.  Unload!

                Unload( );
                return;
            }
        }

        if( m_VideoFrameState == VideoFrame_Locked )
        {
            //
            // We will render the frame at the appropriate time.  We know that the 
            // image will be presented on a vsync.  As long as the time between now
            // and the presentation time is less than a vsync period, we can begin
            // rendering
            //

            if( m_refIRadMovieRenderLoop != NULL )
            {
                // If the client gave us a render loop, initiate a render of the new frame.
                // It's okay for us to fall a little behind the presentation time.  We might
                // catch up without dropping frames.  If we do fall far enough behind, throw
                // away frames until we're all the way caught up

                unsigned int currentTime = m_refIRadStopwatch->GetElapsedTime();
                
                if( currentTime > m_PresentationTime &&
                    currentTime <= m_PresentationTime + m_PresentationDuration + RAD_MOVIE_PLAYER_VIDEO_LAG )
                {
                    m_refIRadMovieRenderLoop->IterateLoop( this );
                }
                else if( currentTime > m_PresentationTime + m_PresentationDuration )
                {
                    rTunePrintf( "radMoviePlayer: NOT RENDERING THIS FRAME (must catch up)\n" );
                    rTunePrintf( "current time[ %d ] expected presentation time[ %d ~ %d ]\n",
                        currentTime, m_PresentationTime, m_PresentationTime + m_PresentationDuration );

                    ALint state;
                    alGetSourcei( m_AudioSource, AL_SOURCE_STATE, &state );
                    if( state != AL_STOPPED )  // allow audio to re-sync
                        alSourceStop( m_AudioSource );

                    //
                    // Not rendering will save us up to a vsync period this round.
                    // Eventually we're bound to catch up
                    //

                    m_VideoFrameState = VideoFrame_Unlocked;
                }
            }
        }
    }
}

//=============================================================================
// radMoviePlayer::SetState
//=============================================================================

void radMoviePlayer::SetState( IRadMoviePlayer2::State state )
{
    if( state != m_State )
    {
        rDebugChannelPrintf( radMovieDebugChannel2, "radMoviePlayer::SetState [%d]\n", state );
        m_State = state;
    }
}

//=============================================================================
// radMoviePlayer::InternalPlay
//=============================================================================

void radMoviePlayer::InternalPlay( void )
{
    m_refIRadStopwatch->Start( );
    Service( );
}

//=============================================================================
// radMoviePlayer::FlushAudioQueue
//=============================================================================

void radMoviePlayer::FlushAudioQueue( void )
{
    // Flush the queue, deleting processed buffers
    ALint processed = 0;
    alGetSourcei( m_AudioSource, AL_BUFFERS_PROCESSED, &processed );
    if( processed > 0 )
    {
        std::vector<ALuint> buffers( processed );
        alSourceUnqueueBuffers( m_AudioSource, processed, buffers.data() );
        alDeleteBuffers( processed, buffers.data() );
    }
}

//=============================================================================
// Function:    radMoviePlayerCreate2
//=============================================================================

IRadMoviePlayer2 * radMoviePlayerCreate2( radMemoryAllocator alloc )
{
	return new( alloc )radMoviePlayer( );
}

//=============================================================================
// Function:    radMovieInitialize2
//=============================================================================

void radMovieInitialize2( radMemoryAllocator alloc )
{
}

//=============================================================================
// Function:    radMovieTerminate2
//=============================================================================

void radMovieTerminate2( void )
{
}

//=============================================================================
// Function:    radMovieService2
//=============================================================================

void radMovieService2( void )
{
    ref< radMoviePlayer > refRadMoviePlayer = radMoviePlayer::GetLinkedClassHead( );

    while( refRadMoviePlayer != NULL )
    {
        refRadMoviePlayer->Service( );

        refRadMoviePlayer = refRadMoviePlayer->GetLinkedClassNext( );
    }
}

#endif // ! RAD_MOVIEPLAYER_USE_BINK