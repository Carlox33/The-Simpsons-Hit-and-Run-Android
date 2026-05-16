//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        memorymanager.cpp
//
// Subsystem:	Foundation Technologies - Memory Management System
//
// Description:	This file contains the implementation of the Memory Management
//              System core functions.  The memory manager is a centralized
//              system for registering and unregistereing allocation
//              strategies.  These strategies may be backed by the application
//              in any way the designers feel is appropriate.
//
// Date:    	July 27, 2000
//
// Author:      Tim Hinds
//
//=============================================================================

//============================================================================
// Include Files
//============================================================================

#include "pch.hpp"
#include <stdlib.h>
#include <raddebug.hpp>
#include <radobject.hpp>
#include <radmemory.hpp>
#include <radthread.hpp>
#include <radmemorymonitor.hpp>
#include "memory.hpp"
#include "platalloc.hpp"
#include "staticheap.h"

#include <string.h>

#define TOOLS

#include <memory/leakdetection.h>

#ifdef RAD_ANDROID
#include <android/log.h>
#include <unistd.h>
#endif

//========================================
// Logging helper (cross-platform)
//========================================
#if defined(RAD_ANDROID)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SimpsonsHitAndRun", __VA_ARGS__)
#elif defined(RAD_VITA)
    #define LOGI(...) sceClibPrintf(__VA_ARGS__), sceClibPrintf("\n")
#else
    #define LOGI(...) do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while(0)
#endif




#ifdef RAD_GAMECUBE
    IRadMemoryHeap *vmmHeap = NULL; //This is for exterman linkage...  Shitty.
    extern void MemoryHackCallback();
#endif

#if ( defined RAD_XBOX ) || (defined RAD_GAMECUBE) || (defined RAD_MW) || (defined RAD_WIN32)
    extern void MemoryHackCallback();
#endif

extern void do_check_malloc_state();

void* gEmergencyMemory = NULL;

#ifdef RAD_PS2
#define    HACK_AUDIO_PERSISTENT    ((radMemoryAllocator)15)
#define    HACK_PERSISTENT          ((radMemoryAllocator)3)
#define    HACK_SMALL_ALLOC       ((radMemoryAllocator)16)   
#endif

//----------------------------------------------------------------------------
// IRadmemoryAllocator Stats Stub
//----------------------------------------------------------------------------
void IRadMemoryAllocator::GetStatus(unsigned int *totalFreeMemory,
                                    unsigned int *largestBlock,
                                    unsigned int *numberOfObjects,
                                    unsigned int *highWaterMark )
{
    rAssert(totalFreeMemory != NULL);
    rAssert(largestBlock    != NULL);
    rAssert(numberOfObjects != NULL);
    rAssert(highWaterMark   != NULL);

    *totalFreeMemory = GetSize();
    *largestBlock    = GetSize();
    *numberOfObjects = 0;
    *highWaterMark   = 0;
}



//----------------------------------------------------------------------------
unsigned int IRadMemoryAllocator::GetSize( void )
{
    return 0xFFFFFFFFU;
}



//----------------------------------------------------------------------------
// Platform (malloc()) Allocator
//----------------------------------------------------------------------------

// This object is a special allocator that wraps calls to radMemoryPlatAlloc().
// and fits into our IRadMemoryAllocator() model.  It represents the special
// case "Root Node" of our allocator tree, that is, all allocations ultimately
// come from malloc() through this allocator.

struct radMemoryAllocatorMalloc : public IRadMemoryAllocator
{
    // Don't need AddRef()/Release() functions for this allocator, it is a
    // singleton that merely wraps calls to platAlloc() with no attributes.
    radMemoryAllocatorMalloc( )
    {
        radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radMemoryAllocatorMalloc" );
    }

    virtual void AddRef( void ) { };
    virtual void Release( void ) { };

    virtual void* GetMemory( unsigned int size )
    {
        void * pMemory = radMemoryPlatAlloc( size );

        radMemoryMonitorDeclareAllocation( pMemory, size );

        return pMemory;
    }

	virtual void FreeMemory( void* pMemory )
    {
        radMemoryPlatFree( pMemory );

        radMemoryMonitorRescindAllocation( pMemory );
    }

    virtual void* GetMemoryAligned( unsigned int size, unsigned int alignment )
    {
        void * pMemory = radMemoryPlatAllocAligned( size, alignment );

        radMemoryMonitorDeclareAllocation( pMemory, size );

        return pMemory;
    }

	virtual void FreeMemoryAligned( void * pMemory )
    {
        radMemoryPlatFreeAligned( pMemory );

        radMemoryMonitorRescindAllocation( pMemory );
    }

    virtual bool CanFreeMemory( void * pMemory )
    {
        return true;
    }

    virtual bool CanFreeMemoryAligned( void * pMemory )
    {
        return true;
    }
    
	void * operator new( size_t size, void * pMemory )
	{
		return pMemory;
	}
};


//----------------------------------------------------------------------------
// Allocator Tree structures
//----------------------------------------------------------------------------

// When the user registers an allocator they must provide the parent allocator
// from which the other allocator's memory was allocated.  These libraries
// maintain the tree--this is a node in that tree.

struct radMemoryAllocatorTreeNode
{
    IRadMemoryAllocator        * m_pIRadMemoryAllocator; // the allocator pointer
    radMemoryAllocatorTreeNode * m_pChildren_Head; // list of sub allocators
    radMemoryAllocatorTreeNode * m_pSibling_Next;  // list pointer used by parent 
    radMemoryAllocatorTreeNode * m_pParent;        // optomization                 
};

// An instance of the special case singleton malloc() allocator and
// its coresponding root note.  They get attached in radMemoryInitialize()

radMemoryAllocatorMalloc * g_pRadMemoryAllocator_Malloc = NULL;
static unsigned int g_MemoryForMalloc[ (sizeof( radMemoryAllocatorMalloc ) / sizeof( unsigned int)) + 1 ];
static radMemoryAllocatorTreeNode g_AllocatorTreeNode_Root;

// Fixed size array for storing our allocators, the table element of an
// allocator maps 1 to 1 with #define'd radMemoryAllocator used by the game.
// We could make the size dynamic dow that we have a radMemoryInitizlize()
// call.

static radMemoryAllocatorTreeNode g_AllocatorTreeNodes[ ALLOCATOR_TABLE_SIZE ];

// The current allocator for Pure3d style per thread global allocator
// settings. The current allocator is maintained using thread local storage.

static IRadThreadLocalStorage* g_CurrentAllocator = NULL;
static IRadThreadLocalStorage* g_CurrentAllocatorCallback = NULL;
static IRadMemoryActivityCallback* g_MemoryActivityCallback = NULL;

// Flag to indicate whether the libraries have been initialized.

static bool g_Initialized = false;


static radMemoryAllocator* g_UsableAllocators = NULL;
static unsigned int g_NumUseableAllocators = 0;
void radMemorySetUsableAllocators( radMemoryAllocator* allocs, unsigned int numAllocs )
{
    g_UsableAllocators = allocs;
    g_NumUseableAllocators = numAllocs;
}


//============================================================================
// Forward Declarations
//============================================================================

IRadMemoryAllocator * radMemoryFindAllocatorRecursive( radMemoryAllocatorTreeNode * pNode, void * pMemory );
IRadMemoryAllocator * radMemoryFindAllocatorAlignedRecursive( radMemoryAllocatorTreeNode * pNode, void * pMemory );

// We initialize the external memory space system, don't put these functions
// in the header file or it might confuse people.

#ifdef RAD_GAMECUBE
    extern void radMemorySpaceInitialize( unsigned int aramSizeInBytes );
    extern void radMemorySpaceTerminate( void );
    static bool sVMMDLHeapInitialized = false;
#else
    extern void radMemorySpaceInitialize( void );
    extern void radMemorySpaceTerminate( void );
#endif

#ifdef RAD_MW
    static bool sVMMDLHeapInitialized = false;
#endif

//============================================================================
// ::radMemoryInitialize
//============================================================================

#ifdef RAD_GAMECUBE
#include <dolphin/vm.h>

void gcnVMMLogStats(unsigned long realVirtualAddress,
                    unsigned long physicalAddress, 
                    unsigned long pageNumber,
                    unsigned long pageMissLatency,
                    BOOL pageSwappedOut );

static gcnVMMStats sVMMStats;

void radMemoryInitialize( unsigned int sizeVMMainMemory, unsigned int sizeVMARAM )
#else
void radMemoryInitialize( void )
#endif
{
    if( g_Initialized )
    {
        return;
    }

#ifdef RAD_GAMECUBE
    ::radMemoryPlatInitialize( sizeVMMainMemory, sizeVMARAM );
#else
    ::radMemoryPlatInitialize( );
#endif

    //This is memory reserved to really bad situations where we need to printf.
#ifndef RAD_GAMECUBE
    gEmergencyMemory = radMemoryPlatAlloc( 1024 * 32 );
#endif

    rAssert( g_Initialized == false );
    g_Initialized = true;

    g_pRadMemoryAllocator_Malloc = new ( g_MemoryForMalloc ) radMemoryAllocatorMalloc( );

    g_AllocatorTreeNode_Root.m_pIRadMemoryAllocator = g_pRadMemoryAllocator_Malloc;
    g_AllocatorTreeNode_Root.m_pChildren_Head = NULL;
    g_AllocatorTreeNode_Root.m_pSibling_Next = NULL;
    g_AllocatorTreeNode_Root.m_pParent = NULL;

    for( unsigned int i = 0; i < ALLOCATOR_TABLE_SIZE; i ++ )
    {
        g_AllocatorTreeNodes[ i ].m_pChildren_Head = NULL;
        g_AllocatorTreeNodes[ i ].m_pParent = NULL;
        g_AllocatorTreeNodes[ i ].m_pSibling_Next = NULL;
        g_AllocatorTreeNodes[ i ].m_pIRadMemoryAllocator = g_pRadMemoryAllocator_Malloc;
    }

#ifdef RAD_GAMECUBE
    unsigned aramSize = (1024 * 1024 * 16) - sizeVMARAM;
    radMemorySpaceInitialize( aramSize );
    sVMMDLHeapInitialized = false;

    if ((sizeVMMainMemory != 0) && (sizeVMARAM != 0))
    {
        bool ok = VMAlloc( 0x7E000000, sizeVMARAM );
        rAssert( ok );
        vmmHeap = radMemoryCreateDougLeaHeap( (void *)0x7E000000, sizeVMARAM, RADMEMORY_ALLOC_DEFAULT, "GameCube_VMM" );
        rAssert(vmmHeap != NULL);
        radMemoryRegisterAllocator( RADMEMORY_ALLOC_VMM, RADMEMORY_ALLOC_DEFAULT, vmmHeap );
        sVMMDLHeapInitialized = true;
#ifndef RAD_RELEASE
        VMSetLogStatsCallback(&gcnVMMLogStats);
#endif
    }

#else
    radMemorySpaceInitialize( );
#endif

    //
    // Initialize static heap
    //
    //g_StaticHeap.CreateHeap( STATIC_HEAP_SIZE );
}

//============================================================================
// ::radMemoryTerminate
//============================================================================

void radMemoryTerminate( void )
{
    radMemorySpaceTerminate( );

    rAssert( g_Initialized == true );

    //
    // Free the thread local storage object if it was allocated.
    //
    if( g_CurrentAllocator != NULL )
    {
        radRelease( g_CurrentAllocator, NULL );
    }        

    g_Initialized = false;
    
    g_pRadMemoryAllocator_Malloc->~radMemoryAllocatorMalloc( );

#ifdef RAD_GAMECUBE
    if (sVMMDLHeapInitialized)
    {
        IRadMemoryAllocator *vmmHeap = radMemoryGetAllocator( RADMEMORY_ALLOC_VMM );
        rAssert(vmmHeap != NULL);
        vmmHeap->Release();
        radMemoryUnregisterAllocator( RADMEMORY_ALLOC_VMM );
        sVMMDLHeapInitialized = false;

        VMFreeAll();
    }
#endif

    ::radMemoryPlatTerminate( );
    
    //
    // Check to make sure the game cleaned up all of its allocators
    //    

    rAssert( g_AllocatorTreeNode_Root.m_pChildren_Head == NULL );
}

//============================================================================
// ::radMemoryRegisterOutOfMemoryCallback
//
// The game calls this function to register a function to call back when any
// allocator runs out of memory.
//============================================================================

radMemoryOutOfMemoryCallback * g_pRadMemoryOutOfMemoryCallback = NULL;
void *                         g_pRadMemoryOutOfMemoryCallbackUserData = NULL;

void radMemorySetOutOfMemoryCallback
(
	radMemoryOutOfMemoryCallback * pCallback,
	void * pUserData
)
{
	g_pRadMemoryOutOfMemoryCallback         = pCallback;
	g_pRadMemoryOutOfMemoryCallbackUserData = pUserData;
}

//============================================================================
// ::CheckForOutOfMemory
//
// Called as a final test in each allocation function, the function will call
// the installed game global out of memory callback if there is one installed
// otherwise it will print out a debug message.
//============================================================================
inline void CheckForOutOfMemory
(
	void * pMemory,
	unsigned int numberOfBytes,
	radMemoryAllocator allocator
)
{
	if(	pMemory == NULL )
	{
        radMemoryPlatFree( gEmergencyMemory );

        rReleasePrintf( "OUT OF MEMORY\n" );
        rReleasePrintf( "Out of memory in allocator: [%d]\n", allocator );
        ::radMemoryMonitorSuspend();

        IRadMemoryAllocator* pIRadMemoryAllocator = NULL;
        if( g_AllocatorTreeNodes != NULL )
        {
            pIRadMemoryAllocator = g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;
            unsigned int totalFree, largestBlock, numberOfObjects, highWaterMark;
            pIRadMemoryAllocator->GetStatus( &totalFree, &largestBlock, &numberOfObjects, &highWaterMark );

            rReleasePrintf( "requested:%d\n", numberOfBytes );
            rReleasePrintf( "totalFree:%d\n", totalFree );
            rReleasePrintf( "largestBlock:%d\n", largestBlock );
        }
        else
        {
            rReleasePrintf( "No Usable Allocators\n" );
        }

        if ( g_pRadMemoryOutOfMemoryCallback != NULL )
		{
			g_pRadMemoryOutOfMemoryCallback(
                g_pRadMemoryOutOfMemoryCallbackUserData, allocator, numberOfBytes );
		}
	}	
}

char g_CurrentMemoryIdentification[ 256 ] = "";
void SetMemoryIdentification( const char* id )
{
    strcpy( g_CurrentMemoryIdentification, id );
}

inline void * radMemoryAllocSearch( unsigned int numberOfBytes, unsigned int alignment )
{

    void* pMemory = NULL;
    if ( g_UsableAllocators != NULL )
    {
        IRadMemoryAllocator * pIRadMemoryAllocator = NULL;

        unsigned int i;
        for ( i = 0; i < g_NumUseableAllocators; ++i )
        {
            rAssert( i < ALLOCATOR_TABLE_SIZE );
            pIRadMemoryAllocator = g_AllocatorTreeNodes[ g_UsableAllocators[ i ] ].m_pIRadMemoryAllocator;

            unsigned int totalFree, largestBlock, numberOfObjects, highWaterMark;
            pIRadMemoryAllocator->GetStatus( &totalFree, &largestBlock, &numberOfObjects, &highWaterMark );
            if ( totalFree < numberOfBytes )
            {
                continue;
            }

            if ( alignment != 0 )
            {

                pMemory = pIRadMemoryAllocator->GetMemoryAligned( numberOfBytes, alignment );
            }
            else
            {
                pMemory = pIRadMemoryAllocator->GetMemory( numberOfBytes );
            }

            if ( pMemory )
            {
                break;
            }
        }
    }

    return pMemory;
}

//============================================================================
// ::radMemoryAlloc
//============================================================================
#ifdef RAD_PS2
bool gbSmallAllocCreated = false;
#endif

//#define TRACK_LEAKS

#ifdef TRACK_LEAKS
bool gbTrackLeaks = false;
struct AllocState
{
    enum CurState
    {
        UNUSED,
        ALLOCATED,
        FREED
    };
    CurState mCurState = UNUSED;
};
std::map<void*,AllocState> gbLeakMap;
#endif

void * radMemoryAlloc( radMemoryAllocator allocator, unsigned int numberOfBytes )
{
#ifdef RAD_PS2
    if(     (numberOfBytes<201)
        && gbSmallAllocCreated 
        && (allocator != HACK_AUDIO_PERSISTENT)
        && (allocator != HACK_PERSISTENT)
        && (allocator != RADMEMORY_ALLOC_TEMP)
        )
    {
        allocator = HACK_SMALL_ALLOC;
    }
#endif
#if ( defined RAD_XBOX ) || ( defined RAD_GAMECUBE ) || ( defined RAD_MW ) || ( defined RAD_WIN32 )
    if ( !g_Initialized )
    {
        MemoryHackCallback();
    }
#endif
    if ( numberOfBytes == 0 )
    {
        return NULL;
    }

    rAssert( g_Initialized == true );
    rAssert( allocator < ALLOCATOR_TABLE_SIZE || allocator == ALLOCATOR_SEARCH );

    #ifdef RAD_XBOX
        //rAssert( allocator != 2 );
    #endif
    void * pMem;

    if ( allocator == ALLOCATOR_SEARCH )
    {
        pMem = radMemoryAllocSearch( numberOfBytes, 0 );
    }
    else
    {
        IRadMemoryAllocator * pIRadMemoryAllocator =
            g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;

        pMem = pIRadMemoryAllocator->GetMemory( numberOfBytes );
        ::radMemoryMonitorIdentifyAllocation ( pMem, g_CurrentMemoryIdentification );
    }

    if (g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryAllocated( allocator, pMem, numberOfBytes );
    }

    CheckForOutOfMemory( pMem, numberOfBytes, allocator );
    LEAK_DETECTION_ADD_ALLOCATION( pMem, numberOfBytes, allocator );
    return pMem;
}

//============================================================================
// ::radMemoryAllocAligned
//============================================================================

void * radMemoryAllocAligned
(
    radMemoryAllocator allocator,
    unsigned int numberOfBytes,
    unsigned int alignment
)
{
#if ( defined RAD_XBOX ) || ( defined RAD_GAMECUBE ) || ( defined RAD_MW ) || ( defined RAD_WIN32 )
    if ( !g_Initialized )
    {
        MemoryHackCallback();
    }
#endif
#ifdef RAD_PS2
    if(     (numberOfBytes<201)
        && gbSmallAllocCreated 
        && (allocator != HACK_AUDIO_PERSISTENT)
        && (allocator != HACK_PERSISTENT)
        && (allocator != RADMEMORY_ALLOC_TEMP)
        )
    {
        allocator = HACK_SMALL_ALLOC;
    }
#endif
    rAssert( ( alignment % 4 ) == 0 );
    rAssert( g_Initialized == true );
    rAssert( allocator < ALLOCATOR_TABLE_SIZE || allocator == ALLOCATOR_SEARCH );

    void * pMem;
    if ( allocator == ALLOCATOR_SEARCH )
    {
        pMem = radMemoryAllocSearch( numberOfBytes, alignment );
    }
    else
    {
        IRadMemoryAllocator * pIRadMemoryAllocator =
            g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;

        pMem = pIRadMemoryAllocator->GetMemoryAligned( numberOfBytes, alignment );
    }

    if (g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryAllocated( allocator, pMem, numberOfBytes );
    }

    CheckForOutOfMemory( pMem, numberOfBytes, allocator );
    LEAK_DETECTION_ADD_ALLOCATION( pMem, numberOfBytes, allocator );
    return pMem;
}

//============================================================================
// ::radMemoryFree
//
// This version of free takes an explicit allocator and thus doesn't require
// a serch through the allocatior tree.
//============================================================================

void radMemoryFree( radMemoryAllocator allocator, void * pMemory )
{
#ifdef RAD_PS2
    return radMemoryFree(pMemory);
#endif
    LEAK_DETECTION_REMOVE_ALLOCATION( pMemory );
    rAssert( g_Initialized == true );

    IRadMemoryAllocator * pIRadMemoryAllocator = NULL;

    if ( allocator >= ALLOCATOR_TABLE_SIZE )
    {
        pIRadMemoryAllocator = radMemoryFindAllocatorRecursive(
            & g_AllocatorTreeNode_Root, pMemory );
    }
    else
    {
        pIRadMemoryAllocator = g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;
    }

    rAssert( pIRadMemoryAllocator != NULL );

    if (pMemory && g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryFreed( allocator, pMemory );
    }

    pIRadMemoryAllocator->FreeMemory( pMemory );
}

//============================================================================
// ::radMemoryFreeAligned
//============================================================================

void radMemoryFreeAligned(  radMemoryAllocator allocator, void * pAlignedMemory )
{
#ifdef RAD_PS2
    return radMemoryFreeAligned(pAlignedMemory);
#endif
    LEAK_DETECTION_REMOVE_ALLOCATION( pAlignedMemory );
    rAssert( g_Initialized == true );

    IRadMemoryAllocator * pIRadMemoryAllocator = NULL;
    if ( allocator >= ALLOCATOR_TABLE_SIZE )
    {
        pIRadMemoryAllocator = radMemoryFindAllocatorAlignedRecursive(
            & g_AllocatorTreeNode_Root, pAlignedMemory );
    }
    else
    {
        pIRadMemoryAllocator = g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;
    }
    
    rAssert( pIRadMemoryAllocator != NULL );

    if (pAlignedMemory && g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryFreed( allocator, pAlignedMemory );
    }

    pIRadMemoryAllocator->FreeMemoryAligned( pAlignedMemory );
}

void radMemoryFreeAligned( void * pMemory )
{
    LEAK_DETECTION_REMOVE_ALLOCATION( pMemory );
    rAssert( g_Initialized == true );

    IRadMemoryAllocator * pIRadMemoryAllocator = radMemoryFindAllocatorAlignedRecursive(
        & g_AllocatorTreeNode_Root, pMemory );

    rAssert( pIRadMemoryAllocator != NULL );

    if (pMemory && g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryFreed( radMemoryGetAllocatorID (pIRadMemoryAllocator), pMemory );
    }

    pIRadMemoryAllocator->FreeMemoryAligned( pMemory );
}

//============================================================================
// ::radMemoryFree
//============================================================================

void radMemoryFree( void * pMemory )
{
    LEAK_DETECTION_REMOVE_ALLOCATION( pMemory );
    rAssert( g_Initialized == true );

    IRadMemoryAllocator * pIRadMemoryAllocator = radMemoryFindAllocatorRecursive(
        & g_AllocatorTreeNode_Root, pMemory );

    rAssert( pIRadMemoryAllocator != NULL );

    if (pMemory && g_MemoryActivityCallback)
    {
        g_MemoryActivityCallback->MemoryFreed( radMemoryGetAllocatorID (pIRadMemoryAllocator), pMemory );
    }

    pIRadMemoryAllocator->FreeMemory( pMemory );
}

//============================================================================
// ::radMemoryRegisterAllocator
//
// Called by the game to map an allocator index to an allocator interface.
// the function insert the allocator into the tree based on the parent
// allocator parameter.
//============================================================================

void radMemoryRegisterAllocator( radMemoryAllocator allocator,
    radMemoryAllocator parent, IRadMemoryAllocator* pIRadMemoryAllocator )
{
    rAssert( g_Initialized == true );

    rAssert( allocator < ALLOCATOR_TABLE_SIZE );
    rAssert( parent < ALLOCATOR_TABLE_SIZE );

    rAssert( pIRadMemoryAllocator != NULL );
    rAssert( g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator == g_pRadMemoryAllocator_Malloc );

    radMemoryAllocatorTreeNode * pNewNode =
        & ( g_AllocatorTreeNodes[ allocator ] );

    radMemoryAllocatorTreeNode * pParentNode =
        & ( g_AllocatorTreeNodes[ parent ] );

    if ( pParentNode->m_pParent == NULL )
    {
        pParentNode = & g_AllocatorTreeNode_Root;
    }

    radAddRef( pIRadMemoryAllocator, NULL );

    pNewNode->m_pIRadMemoryAllocator = pIRadMemoryAllocator;
    pNewNode->m_pChildren_Head = NULL;
    pNewNode->m_pParent = pParentNode;

    pNewNode->m_pSibling_Next = pParentNode->m_pChildren_Head;
    pParentNode->m_pChildren_Head = pNewNode;
}

//============================================================================
// ::radMemoryGetAllocator
//
// Simply returns the currently register allocator at that array element.
//============================================================================

IRadMemoryAllocator * radMemoryGetAllocator( radMemoryAllocator allocator )
{
    rAssert( g_Initialized == true );
    rAssert( allocator < ALLOCATOR_TABLE_SIZE );

    IRadMemoryAllocator * pIRma = g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator;

    return pIRma;
}

//============================================================================
// ::radMemoryGetAllocatorID
//
// Simply returns the currently register allocator at that array element.
//============================================================================
radMemoryAllocator radMemoryGetAllocatorID( IRadMemoryAllocator* allocator )
{
    rAssert( g_Initialized == true );
    for (int i = 0; i < ALLOCATOR_TABLE_SIZE; i++)
    {
        if ( g_AllocatorTreeNodes[ i ].m_pIRadMemoryAllocator == allocator )
        {
            return (radMemoryAllocator)i;
        }
    }
    rAssert (false);
    return 0;
}

//============================================================================
// ::radMemoryUnregisterAllocator
//
// Games can unregister an allocator.  All sub-allocators must have already
// been unregistered or this function will assert.
//============================================================================

void radMemoryUnregisterAllocator( radMemoryAllocator allocator )
{
    rAssert( g_Initialized == true );
    rAssert( allocator < ALLOCATOR_TABLE_SIZE );
    rAssert( g_AllocatorTreeNodes[ allocator ].m_pIRadMemoryAllocator != NULL );

    radMemoryAllocatorTreeNode * pFreeNode = & ( g_AllocatorTreeNodes[ allocator ] );
    rAssert( pFreeNode != NULL );

    // Get the parent node, this cannot ever be null, ultimately the parent
    // must be malloc() which is never destroyed.
     
    radMemoryAllocatorTreeNode * pParentNode = pFreeNode->m_pParent;
    rAssert( pParentNode != NULL );

    // Make sure there are no active children of this heap or memory corruption
    // will result.
    rAssert( pFreeNode->m_pChildren_Head == NULL );

    // Some vars for iterating.

    radMemoryAllocatorTreeNode * pSearchPrev = NULL;
    radMemoryAllocatorTreeNode * pSearch = pParentNode->m_pChildren_Head;
    
    // Find the entry for this allocator in the parent's list and remove it.

    while ( pSearch != NULL )
    {
        if ( pSearch == pFreeNode )
        {
            if ( pSearchPrev != NULL )
            {
                pSearchPrev->m_pSibling_Next = pFreeNode->m_pSibling_Next;
            }
            else
            {
                pParentNode->m_pChildren_Head = pFreeNode->m_pSibling_Next;
            }

            // Must release this allocator

            radRelease( pFreeNode->m_pIRadMemoryAllocator, NULL );

            // Set the node back to the unused state

            pFreeNode->m_pIRadMemoryAllocator = g_pRadMemoryAllocator_Malloc;
            pFreeNode->m_pChildren_Head = NULL;
            pFreeNode->m_pParent = NULL;
            pFreeNode->m_pSibling_Next = NULL;

            break; // ... and we're done.        
        }

        pSearchPrev = pSearch;
        pSearch = pSearch->m_pSibling_Next;
    }
}

//============================================================================
// ::radMemoryFindAllocatorRecursive
//
// Recursive helper function that searches the allocator node tree for the
// allocator that allocated the memory.  Returns whether the memory was freed
// by the sub-tree represented by pNode.
//============================================================================

IRadMemoryAllocator * radMemoryFindAllocatorRecursive( radMemoryAllocatorTreeNode * pNode, void * pMemory )
{
    while ( pNode != NULL )
    {
        if ( pNode->m_pIRadMemoryAllocator->CanFreeMemory( pMemory ) )
        {
            IRadMemoryAllocator * pIRadMemoryAllocator = 
                radMemoryFindAllocatorRecursive( pNode->m_pChildren_Head, pMemory );

            if ( pIRadMemoryAllocator == NULL )
            {
                return pNode->m_pIRadMemoryAllocator;
            }

            return pIRadMemoryAllocator;
        }

        pNode = pNode->m_pSibling_Next;
    }
    
    return NULL;
}

//============================================================================
// ::radMemoryFindAllocatorAlignedRecursive
//
// Recursive helper function that searches the allocator node tree for the
// allocator that allocated the memory.  Returns whether the memory was freed
// by the sub-tree represented by pNode.
//============================================================================

IRadMemoryAllocator * radMemoryFindAllocatorAlignedRecursive( radMemoryAllocatorTreeNode * pNode, void * pMemory )
{
    while ( pNode != NULL )
    {
        if ( pNode->m_pIRadMemoryAllocator->CanFreeMemoryAligned( pMemory ) )
        {
            IRadMemoryAllocator * pIRadMemoryAllocator = 
                radMemoryFindAllocatorAlignedRecursive( pNode->m_pChildren_Head, pMemory );

            if ( pIRadMemoryAllocator == NULL )
            {
                return pNode->m_pIRadMemoryAllocator;
            }

            return pIRadMemoryAllocator;
        }

        pNode = pNode->m_pSibling_Next;
    }
    
    return NULL;
}

//============================================================================
// ::radMemoryFindAllocator
//============================================================================

IRadMemoryAllocator * radMemoryFindAllocator( void * pMemory )
{
    return radMemoryFindAllocatorRecursive( & g_AllocatorTreeNode_Root, pMemory );
}


// Functions for SetCurrentAllocator callbacks
//ORIGINAL FUNCTIONS
/*
void radMemorySetAllocatorCallback( IRadMemorySetAllocatorCallback* callback )
{
    if ( !g_CurrentAllocatorCallback )
    {
        radThreadCreateLocalStorage( &g_CurrentAllocatorCallback );
        g_CurrentAllocatorCallback->SetValue (0);
    }

    g_CurrentAllocatorCallback->SetValue( (void*) callback );
}


IRadMemorySetAllocatorCallback* radMemoryGetAllocatorCallback ()
{
    if ( !g_CurrentAllocatorCallback )
    {
        radThreadCreateLocalStorage( &g_CurrentAllocatorCallback );
        g_CurrentAllocatorCallback->SetValue (0);
    }

    return static_cast<IRadMemorySetAllocatorCallback*>(g_CurrentAllocatorCallback->GetValue( ));
}
*/

// NEW FUNCTIONS, SAME, BUT WITH LOGI FOR DEBUG ANDROID
// Functions for SetCurrentAllocator callbacks
//
void radMemorySetAllocatorCallback(IRadMemorySetAllocatorCallback* callback)
{
//LOGI("[radmem] &g_CurrentAllocatorCallback=%p", (void*)&g_CurrentAllocatorCallback);

    if (!g_CurrentAllocatorCallback)
    {
        radThreadCreateLocalStorage(&g_CurrentAllocatorCallback);
        g_CurrentAllocatorCallback->SetValue(0);

        //LOGI("[radmem] CB-TLS created tid=%d tls=%p init=0",
          //   (int)gettid(),
           //  (void*)g_CurrentAllocatorCallback);
    }

    void* prev = g_CurrentAllocatorCallback->GetValue();

    g_CurrentAllocatorCallback->SetValue((void*)callback);

    //LOGI("[radmem] CALLBACK SET tid=%d tls=%p %p -> %p",
      //   (int)gettid(),
        // (void*)g_CurrentAllocatorCallback,
        // prev,
        // (void*)callback);
}

IRadMemorySetAllocatorCallback* radMemoryGetAllocatorCallback()
{
    //LOGI("[radmem] &g_CurrentAllocatorCallback=%p", (void*)&g_CurrentAllocatorCallback);
    if (!g_CurrentAllocatorCallback)
    {
        radThreadCreateLocalStorage(&g_CurrentAllocatorCallback);
        g_CurrentAllocatorCallback->SetValue(0);

        //LOGI("[radmem] CB-TLS created tid=%d tls=%p init=0",
          //   (int)gettid(),
            // (void*)g_CurrentAllocatorCallback);
    }

    void* cb = g_CurrentAllocatorCallback->GetValue();

    //LOGI("[radmem] CALLBACK GET tid=%d tls=%p -> %p",
       //  (int)gettid(),
         //(void*)g_CurrentAllocatorCallback,
         //cb);

    return static_cast<IRadMemorySetAllocatorCallback*>(cb);
}





void radMemorySetActivityCallback( IRadMemoryActivityCallback* callback )
{
    g_MemoryActivityCallback = callback;
}


//============================================================================
// ::radMemoryGetCurrentAllocator
//
// Returns the current global allocator.  This entry is maintained using thread
// local storage so that each thread can have its on current allocator. Ideally
// the thread local storage object should be allocated during the initialization
// of the memory system. However, this would require the threading system to
// be initialized even if no one is using threading. As a result, ( until we 
// make our system thread safe ), the thread local storage is allocated here.
// Pure3D is the user of these functions all will dictate that the threading
// system be initilaized.
//============================================================================
/*
radMemoryAllocator radMemoryGetCurrentAllocator( void )
{
    IRadMemorySetAllocatorCallback* callback = radMemoryGetAllocatorCallback ();
    if (callback)
    {
        return callback->GetCurrentAllocator ();
    }

    //
    // See if we have obtained our thread local storage.
    //
    if( g_CurrentAllocator == NULL )
    {
        //
        // Get one and initialize the value.
        //
        radThreadCreateLocalStorage( &g_CurrentAllocator );
        g_CurrentAllocator->SetValue( (void*) RADMEMORY_ALLOC_DEFAULT );
    }

    //
    // Return the current allocator.
    //
    return( (radMemoryAllocator)(intptr_t) g_CurrentAllocator->GetValue( ) );
}
*/

radMemoryAllocator radMemoryGetCurrentAllocator(void)
{
    IRadMemorySetAllocatorCallback* callback = radMemoryGetAllocatorCallback();
    if (callback)
    {
        radMemoryAllocator a = callback->GetCurrentAllocator();
        //LOGI("[radmem] GET tid=%d via CALLBACK cb=%p -> %d (TEMP=%d)",
          //   (int)gettid(), callback, (int)a, (int)RADMEMORY_ALLOC_TEMP);
        return a;
    }

    if (g_CurrentAllocator == NULL)
    {
        radThreadCreateLocalStorage(&g_CurrentAllocator);
        g_CurrentAllocator->SetValue((void*)RADMEMORY_ALLOC_DEFAULT);
        //LOGI("[radmem] TLS created tid=%d tls=%p init=%d",
          //   (int)gettid(), g_CurrentAllocator, (int)RADMEMORY_ALLOC_DEFAULT);
    }

    radMemoryAllocator a =
        (radMemoryAllocator)(intptr_t)g_CurrentAllocator->GetValue();

    //LOGI("[radmem] GET tid=%d via TLS -> %d (TEMP=%d)",
      //   (int)gettid(), (int)a, (int)RADMEMORY_ALLOC_TEMP);
    return a;
}


//============================================================================
// ::radMemorySetCurrentAllocator
//
// Sets the current global allocator.  Returns the
// previous allocator so that it may be easily stored and reset by the client
// of the function as in:
//
// radMemoryAllocator old = radMemorySetCurrentAllocator( new )
// ...do some allocations
// radMemorySetCurrentAllocator( old );
//============================================================================
/*
radMemoryAllocator radMemorySetCurrentAllocator( radMemoryAllocator allocator )
{
    IRadMemorySetAllocatorCallback* callback = radMemoryGetAllocatorCallback ();
    if (callback)
    {
        return callback->SetCurrentAllocator (allocator);
    }

    //
    // See if we have obtained our thread local storage.
    //
    if( g_CurrentAllocator == NULL )
    {
        //
        // Get one and initialize the value.
        //
        radThreadCreateLocalStorage( &g_CurrentAllocator );
        g_CurrentAllocator->SetValue( (void*) RADMEMORY_ALLOC_DEFAULT );
    }

    //
    // Get old value and set new value.  
    //
    radMemoryAllocator prevAllocator = (radMemoryAllocator)(intptr_t) g_CurrentAllocator->GetValue( );

    g_CurrentAllocator->SetValue( (void*)(intptr_t) allocator );

    return prevAllocator;
}
*/
radMemoryAllocator radMemorySetCurrentAllocator(radMemoryAllocator allocator)
{
    IRadMemorySetAllocatorCallback* callback = radMemoryGetAllocatorCallback();
    if (callback)
    {
        radMemoryAllocator old = callback->SetCurrentAllocator(allocator);
        //LOGI("[radmem] SET tid=%d via CALLBACK cb=%p %d -> %d",
          //   (int)gettid(), callback, (int)old, (int)allocator);
        return old;
    }

    if (g_CurrentAllocator == NULL)
    {
        radThreadCreateLocalStorage(&g_CurrentAllocator);
        g_CurrentAllocator->SetValue((void*)RADMEMORY_ALLOC_DEFAULT);
        //LOGI("[radmem] TLS created tid=%d tls=%p init=%d",
          //   (int)gettid(), g_CurrentAllocator, (int)RADMEMORY_ALLOC_DEFAULT);
    }

    radMemoryAllocator prev =
        (radMemoryAllocator)(intptr_t)g_CurrentAllocator->GetValue();

    g_CurrentAllocator->SetValue((void*)(intptr_t)allocator);

   // LOGI("[radmem] SET tid=%d via TLS %d -> %d",
     //    (int)gettid(), (int)prev, (int)allocator);
    return prev;
}



const char * g_pAllocationName = NULL;

void radMemorySetAllocationName( const char * pName )
{
    g_pAllocationName = pName;
}

const char * radMemoryGetAllocationName( void )
{
    return g_pAllocationName;
}


#ifdef RAD_GAMECUBE
//-----------------------------------------------------------------------------
// VMM Stats
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void radVMMClearStats( void )
{
    sVMMStats.pageMisses = 0;        
    sVMMStats.pageWrites = 0;
    sVMMStats.pageMissLatency = 0; // in microsceonds

}

//-----------------------------------------------------------------------------
void radVMMGetStats( gcnVMMStats *stats )
{
    rAssert (stats != NULL );
    stats->pageMisses      = sVMMStats.pageMisses;
    stats->pageWrites      = sVMMStats.pageWrites;
    stats->pageMissLatency = sVMMStats.pageMissLatency;
}



void gcnVMMLogStats(unsigned long realVirtualAddress,
                    unsigned long physicalAddress, 
                    unsigned long pageNumber,
                    unsigned long pageMissLatency,
                    BOOL pageSwappedOut )
{
    sVMMStats.pageMisses++;
    sVMMStats.pageWrites      += (unsigned) pageSwappedOut;
    sVMMStats.pageMissLatency += pageMissLatency;
}

#endif

