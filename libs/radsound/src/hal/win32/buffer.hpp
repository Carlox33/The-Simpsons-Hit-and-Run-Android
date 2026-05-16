//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================
#ifndef BUFFER_HPP
#define BUFFER_HPP
//============================================================================
// Include Files
//============================================================================
#include <radlinkedclass.hpp>
#include <radsoundobject.hpp>
#include "radsoundwin.hpp"
//============================================================================
// Component: IRadSoundBufferWin
//============================================================================
class radSoundHalBufferWin
    :
    public IRadSoundHalBuffer,
    public IRadSoundHalBufferLoadCallback,
	public radLinkedClass< radSoundHalBufferWin >,
	public radSoundObject
{
	public:
		// IRadSoundHalBuffer
		virtual void Initialize(
			IRadSoundHalAudioFormat * pIRadSoundHalAudioFormat,
			IRadMemoryObject * pIRadMemoryObject,
			unsigned int sizeInFrames,
			bool looping,
            bool streaming );
		virtual IRadSoundHalAudioFormat * GetFormat( void );
		virtual unsigned int GetSizeInFrames( void );
		virtual IRadMemoryObject * GetMemoryObject( void );
		virtual bool IsLooping( void );   
		virtual void LoadAsync(
			IRadSoundHalDataSource * pIRadSoundHalDataSource,
			unsigned int bufferStartInFrames,
			unsigned int numberOfFrames,
			IRadSoundHalBufferLoadCallback * pIRadSoundHalBufferLoadCallback );
		virtual void ClearAsync( 
			unsigned int startPositionInFrames,
			unsigned int numberOfFrames,
			IRadSoundHalBufferClearCallback * pIRadSoundHalBufferClearCallback );
        virtual void CancelAsyncOperations( void );
        virtual unsigned int GetMinTransferSize( IRadSoundHalAudioFormat::SizeType sizeType );
        virtual void ReSetAudioFormat( IRadSoundHalAudioFormat * pIRadSoundHalAudioFormat ) { };
        // IRadSoundHalBufferLoadCallback
        virtual void OnBufferLoadComplete( unsigned int dataSourceFrames );
		// Internal
		radSoundHalBufferWin( void );
		IMPLEMENT_REFCOUNTED( "radSoundHalBufferWin" )
		unsigned int GetSizeInBytes( void );
        bool IsStreaming( void );
        //#ifndef RAD_ANDROID
		ALuint GetBuffer( void );
      //  #else
        //unsigned int GetBuffer( void );
       // #endif
	private:
		virtual ~radSoundHalBufferWin( void );
		unsigned int m_SizeInFrames;
        unsigned int m_LoadStartInBytes;
        void * m_pLockedLoadBuffer;
        unsigned long m_LockedLoadBytes;
		bool m_Looping;
        bool m_Streaming;
        //#ifndef RAD_ANDROID
		//ALuint m_Buffer;
        //#else
        unsigned int m_Buffer;
        //#endif
		ref< IRadSoundHalAudioFormat >	m_refIRadSoundHalAudioFormat;
		ref< IRadMemoryObject >			m_refIRadMemoryObject;
        ref< IRadSoundHalBufferLoadCallback > m_refIRadSoundHalBufferLoadCallback;
};
#endif