//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================



#include "pch.hpp"
#include <raddebug.hpp>
#include "platalloc.hpp"
#include <cstdlib>   // malloc, free, posix_memalign
#include <stdlib.h>  // también válido en NDK
#include <cerrno>    // opcional

#if defined RAD_PS2

    #include <malloc.h>

#elif defined WIN32 || defined RAD_XBOX
    
    #include <stdlib.h>
    #include <malloc.h>
    
    #if defined MALLOC_DEBUG
        #include <crtdbg.h>
    #endif

#elif defined RAD_GAMECUBE

    #include <dolphin.h>
    #include <dolphin/vm.h>

static bool sVMMInitialized = false;
OSHeapHandle gGCHeap = -1;

#endif

//============================================================================
// ::radMemoryPlatInitialize
//============================================================================

#ifdef RAD_GAMECUBE
void radMemoryPlatInitialize( unsigned int sizeVMMainMemory, unsigned int sizeVMARAM)
{
    // Looking for some VMM?
    sVMMInitialized = false;
    if ((sizeVMMainMemory != 0) && (sizeVMARAM != 0))
    {
        unsigned int baseVMARAM = (1024 * 1024 * 16) - sizeVMARAM;

        VMInit(sizeVMMainMemory, baseVMARAM, sizeVMARAM);
        sVMMInitialized = true;
    }

	// Initializes the Dolphin OS memory allocator and ensures that
	// new and delete will work properly.

	void *pArenaLo = OSGetArenaLo( );
	void *pArenaHi = OSGetArenaHi( );

	// Create a heap
	// OSInitAlloc should only ever be invoked once.

	pArenaLo = OSInitAlloc( pArenaLo, pArenaHi, 1); // 1 heap size
	OSSetArenaLo( pArenaLo );

	// Ensure boundaries are 32B aligned

	pArenaLo = (void*) OSRoundUp32B( pArenaLo );
	pArenaHi = (void*) OSRoundDown32B( pArenaHi );

	// The boundaries given to OSCreateHeap should be 32B aligned
    gGCHeap = OSCreateHeap( pArenaLo, pArenaHi );
    rAssert( gGCHeap != -1 );
	OSSetCurrentHeap( gGCHeap );

	//
	// From here on out, OSAlloc and OSFree behave like malloc and free
	// respectively.
	//
	OSSetArenaLo( pArenaLo = pArenaHi );
}
#else
// ---Non-GameCube Memory Plat init --------------------------------------------------
void radMemoryPlatInitialize( void )
{

}
#endif


//============================================================================
// ::radMemoryPlatTerminate
//============================================================================

void radMemoryPlatTerminate( void )
{
#ifdef RAD_GAMECUBE
    if (sVMMInitialized)
    {
        VMQuit();
        sVMMInitialized = false;
    }

#endif
}

//============================================================================
// ::radMemoryPlatAlloc
//============================================================================

void * radMemoryPlatAlloc( unsigned int numberOfBytes )
{
    void * pMemory;
    //
    // C++ standard says you can allocate 0 byte memory object.
    //
    if ( numberOfBytes == 0 )
    {
        numberOfBytes = 1;
    }

    pMemory = malloc( numberOfBytes );
    
    rWarningMsg( pMemory != NULL, "radMemory: Platform (malloc) allocator failed to allocate memory\n" );
    return pMemory;
}

//============================================================================
// ::radMemoryPlatFree
//============================================================================

void radMemoryPlatFree( void * pMemory )
{
    free( pMemory );
}

//============================================================================
// ::radMemoryPlatAllocAligned
//============================================================================
void* radMemoryPlatAllocAligned(unsigned int numberOfBytes, unsigned int alignment)
{
#if defined(WIN32)
    return _aligned_malloc(numberOfBytes, alignment);
#else
    // Android/Linux: use posix_memalign (aligned_alloc may not exist on NDK/minSdk)
    if (alignment < sizeof(void*)) alignment = sizeof(void*);

    void* p = nullptr;

    // posix_memalign requires alignment to be power-of-two AND multiple of sizeof(void*)
    int rc = posix_memalign(&p, alignment, numberOfBytes);
    if (rc != 0) return nullptr;

    return p;
#endif
}

//============================================================================
// ::radMemoryPlatFreeAligned
//============================================================================
void radMemoryPlatFreeAligned(void* pAlignedMemory)
{
#if defined(WIN32)
    _aligned_free(pAlignedMemory);
#else
    free(pAlignedMemory);
#endif
}

