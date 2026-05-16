//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        memoryspacewin32.cpp
//
// Subsystem:	Foundation Technologies - Memory Operation Services
//
// Description:	This file contains the win 32implementation of the 
//              remote memory space operations
//
// Revisions:   13-Oct-2001     Creation     Rob
//
//=============================================================================

#include "pch.hpp"
#include <radmemory.hpp>
#include <string.h>
#include <radmemorymonitor.hpp>

#ifdef RAD_VITA
#include <psp2/ctrl.h>
#include <psp2/kernel/sysmem.h>
#endif

#include "memoryspacewin32.hpp"

#define USER_SIZE_IN_BYTES ( 1024 * 1024 * 32 )
#define MAX_USER_ALLOCATIONS 100
#define CDRAM_SIZE_IN_BYTES ( 1024 * 1024 * 64 )
#define MAX_CDRAM_ALLOCATIONS 100

//============================================================================
//
// Global Functions
//
//============================================================================

#ifdef RAD_VITA
SceUID g_User_UID, g_Cdram_UID;
IRadMemoryHeap * g_pIRadMemoryHeap_User = NULL;
IRadMemoryHeap * g_pIRadMemoryHeap_Cdram = NULL;
#endif

// XBox doesn't have any optimal memory space alignment or multiple requirements

unsigned int radMemorySpace_OptimalMultiple = 4;
unsigned int radMemorySpace_OptimalAlignment = 4;

// Only need one object global object because win32/xbox copies are synchronous.

MemorySpaceAsyncRequest_Copy g_MemorySpaceAsyncRequest_Copy_Placeholder;

void radMemorySpaceInitialize( void )
{
#ifdef RAD_VITA
    int err = SCE_OK;
    void* mem = NULL;

    g_User_UID = sceKernelAllocMemBlock("user",
                                        SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE,
                                        USER_SIZE_IN_BYTES, NULL);
    rAssert(g_User_UID >= SCE_OK);

    err = sceKernelGetMemBlockBase(g_User_UID, &mem);
    rAssert(err == SCE_OK);

    g_pIRadMemoryHeap_User = radMemoryCreateDougLeaHeap(
        mem, USER_SIZE_IN_BYTES, RADMEMORY_ALLOC_DEFAULT, "user");

    g_Cdram_UID = sceKernelAllocMemBlock("cdram",
                                         SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW,
                                         CDRAM_SIZE_IN_BYTES, NULL);
    rAssert(g_Cdram_UID >= SCE_OK);

    err = sceKernelGetMemBlockBase(g_Cdram_UID, &mem);
    rAssert(err == SCE_OK);

    g_pIRadMemoryHeap_Cdram = radMemoryCreateDougLeaHeap(
        mem, CDRAM_SIZE_IN_BYTES, RADMEMORY_ALLOC_DEFAULT, "cdram");
#endif
}

void radMemorySpaceTerminate( void )
{
#ifdef RAD_VITA
    int err = SCE_OK;
    void* mem = NULL;

    rAssert( g_pIRadMemoryHeap_User != NULL );
    rAssert( g_pIRadMemoryHeap_Cdram != NULL );

    g_pIRadMemoryHeap_User->Release();
    g_pIRadMemoryHeap_User = NULL;

    g_pIRadMemoryHeap_Cdram->Release();
    g_pIRadMemoryHeap_Cdram = NULL;

    err = sceKernelFreeMemBlock(g_User_UID);
    rAssert(err == SCE_OK);

    err = sceKernelFreeMemBlock(g_Cdram_UID);
    rAssert(err == SCE_OK);
#endif
}

//============================================================================
// radMemoryCopyAsync
//============================================================================

IRadMemorySpaceCopyRequest * radMemorySpaceCopyAsync
(
	void * pDest,
	radMemorySpace spaceDest,
	const void * pSrc,
	radMemorySpace spaceSrc,
	unsigned int bytes
)
{   
    rAssert( pDest != NULL );
    //rAssert( spaceDest = radMemorySpace_Main );
	rAssert( spaceDest == radMemorySpace_Main );// new line 
    rAssert( pSrc != NULL );
	rAssert( spaceSrc == radMemorySpace_Main );// new line 
    //rAssert( spaceSrc = radMemorySpace_Main );
    rAssert( bytes > 0 );

	::memcpy( pDest, pSrc, bytes ); 

	return & g_MemorySpaceAsyncRequest_Copy_Placeholder;
}

MemorySpaceAsyncRequest_Copy::MemorySpaceAsyncRequest_Copy( void )
    :
    m_RefCount( 0 )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "MemorySpaceAsyncRequest_Copy" );
}

MemorySpaceAsyncRequest_Copy::~MemorySpaceAsyncRequest_Copy( void )
{
    rAssert( m_RefCount == 0 ); // The game is not reference counting properly
}

void MemorySpaceAsyncRequest_Copy::AddRef( void )
{
    m_RefCount++;
}

void MemorySpaceAsyncRequest_Copy::Release( void )
{
    rAssert( m_RefCount > 0 );
    m_RefCount--;
}

bool MemorySpaceAsyncRequest_Copy::IsDone( void )
{
    return true;
}

//============================================================================
// radMemoryAlloc
//============================================================================

void * radMemorySpaceAlloc( radMemorySpace space, radMemoryAllocator allocator, unsigned int numBytes )
{
#ifdef RAD_VITA
    if(space == radMemorySpace_User)
    {
        return g_pIRadMemoryHeap_User->GetMemory( numBytes );
    }
    else if(space == radMemorySpace_Cdram)
    {
        return g_pIRadMemoryHeap_Cdram->GetMemory( numBytes );
    }
#endif
	return ::radMemoryAlloc( allocator, numBytes );
}

//============================================================================
// radMemoryAllocAligned
//============================================================================

void * radMemorySpaceAllocAligned
( 
	radMemorySpace space, 
	radMemoryAllocator allocator,
	unsigned int numBytes, 
	unsigned int alignment 
)
{
#ifdef RAD_VITA
    if(space == radMemorySpace_User)
    {
        return g_pIRadMemoryHeap_User->GetMemoryAligned( numBytes, alignment );
    }
    else if(space == radMemorySpace_Cdram)
    {
        return g_pIRadMemoryHeap_Cdram->GetMemoryAligned( numBytes, alignment );
    }
#endif
	return ::radMemoryAllocAligned( allocator, numBytes, alignment );
}

//============================================================================
// radMemorySpaceGetAllocator
//============================================================================

IRadMemoryAllocator * radMemorySpaceGetAllocator( radMemorySpace memSpace, radMemoryAllocator allocator )
{
#ifdef RAD_VITA
    if(memSpace == radMemorySpace_User)
    {
        return g_pIRadMemoryHeap_User;
    }
    else if(memSpace == radMemorySpace_Cdram)
    {
        return g_pIRadMemoryHeap_Cdram;
    }
#endif
    rAssert( memSpace == radMemorySpace_Main );

    return ::radMemoryGetAllocator( allocator );
}

//============================================================================
// radMemoryFree
//============================================================================

void radMemorySpaceFree
(  
	radMemorySpace space, 
	radMemoryAllocator allocator,
	void * pMemory
)
{
#ifdef RAD_VITA
    if(space == radMemorySpace_User)
    {
        return g_pIRadMemoryHeap_User->FreeMemory( pMemory );
    }
    else if(space == radMemorySpace_Cdram)
    {
        return g_pIRadMemoryHeap_Cdram->FreeMemory( pMemory );
    }
#endif
	::radMemoryFree( allocator, pMemory );
}

//============================================================================
// radMemoryFreeAligned
//============================================================================

void radMemorySpaceFreeAligned
( 
	radMemorySpace space, 
	radMemoryAllocator allocator,
	void * pMemory
)
{
#ifdef RAD_VITA
    if(space == radMemorySpace_User)
    {
        return g_pIRadMemoryHeap_User->FreeMemoryAligned( pMemory );
    }
    else if(space == radMemorySpace_Cdram)
    {
        return g_pIRadMemoryHeap_Cdram->FreeMemoryAligned( pMemory );
    }
#endif
	::radMemoryFreeAligned( allocator, pMemory );
}




