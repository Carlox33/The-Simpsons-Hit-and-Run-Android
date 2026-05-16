//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#include <p3d/memory.hpp>
#include <p3d/buildconfig.hpp>

#include <radmemory.hpp>

// when multithreading curernt allcoator must be in thread local storage
#include <radthread.hpp>



static IRadThreadLocalStorage* gCurrentAllocType = NULL;

static inline p3d::AllocType GetCurrentAllocator(void) 
{ 
    // create the TLS if it doesn't exists
    if(!gCurrentAllocType)
    {
        radThreadCreateLocalStorage(&gCurrentAllocType); // TODO : should delete at some point
    }

    return (p3d::AllocType)((intptr_t )gCurrentAllocType->GetValue());
}

static inline void SetCurrentAllocator(p3d::AllocType a) 
{ 
    // create the TLS if it doesn't exists
    if(!gCurrentAllocType)
    {
        radThreadCreateLocalStorage(&gCurrentAllocType); // TODO : should delete at some point
    }

    gCurrentAllocType->SetValue((void*)a); 
}

namespace p3d
{

void MemSetup(void)
{
}

void MemShutdown(void)
{
    if(gCurrentAllocType)
    {
        gCurrentAllocType->Release();
        gCurrentAllocType = NULL;
    }
}

class DefaultRouter : public MemoryRouter
{
public:
    DefaultRouter()
    {
//        SetCurrentAllocator(ALLOC_DEFAULT);
//        radMemorySetCurrentAllocator(RADMEMORY_ALLOC_DEFAULT);
        alloc[ALLOC_DEFAULT] = RADMEMORY_ALLOC_DEFAULT;
        alloc[ALLOC_PERMANENT] = RADMEMORY_ALLOC_DEFAULT;
        alloc[ALLOC_TEMPORARY] = RADMEMORY_ALLOC_TEMP;
        alloc[ALLOC_LOAD_TEMPORARY] = RADMEMORY_ALLOC_TEMP;
        alloc[ALLOC_LOADED] = RADMEMORY_ALLOC_DEFAULT;
    }

    AllocType GetCurrent(void) 
    { 
        AllocType t = GetCurrentAllocator();
        if(t >= ALLOC_MAX_VALUE)
        {
            t = ALLOC_DEFAULT;
        }
        return t;
    }

    // Set current memory type, and return previous 
    virtual void SetCurrent(AllocType t)
    {
        if(t >= ALLOC_MAX_VALUE)
        {
            t = ALLOC_DEFAULT;
        }

        SetCurrentAllocator(t);
        radMemorySetCurrentAllocator(alloc[t]);
    }

    int GetAlloc(AllocType t)
    {
        if(t >= ALLOC_MAX_VALUE)
        {
            t = ALLOC_DEFAULT;
        }

        return alloc[t];
    }

    void SetAlloc(AllocType t, int a)
    {
        if(t >= ALLOC_MAX_VALUE)
        {
            return;
        }

        alloc[t] = a;
    }

protected:
    int alloc[ALLOC_MAX_VALUE];
};

static DefaultRouter defaultRouter;
static MemoryRouter* memoryRouter = &defaultRouter;

// Retreive current memory type
AllocType GetCurrentAlloc(void)
{
    return memoryRouter->GetCurrent();
}

// Set current memory type, and return previous 
AllocType SetCurrentAlloc(AllocType t)
{
    AllocType old =  memoryRouter->GetCurrent();
    memoryRouter->SetCurrent(t);
    return old;
}

// route alloc types to FTech allocators
int GetMemAllocator(AllocType t)
{
    return defaultRouter.GetAlloc(t);
}

void SetMemAllocator(AllocType t, int a)
{
    defaultRouter.SetAlloc(t, a);
}

void SetMemRouter(MemoryRouter* r)
{
    memoryRouter = r;
    if(memoryRouter == 0)
        memoryRouter = &defaultRouter;
}

MemoryRouter* GetMemRouter(void)
{
    return memoryRouter;
}

// legacy temporary allocation functions
void* MallocTemp(unsigned size)
{
    radMemoryAllocator old = ::radMemorySetCurrentAllocator (RADMEMORY_ALLOC_TEMP);

    /*
    AllocType old = GetCurrentAlloc();

    // they go into the loading pool because current uses SHOULD only be in loading
    SetCurrentAlloc(ALLOC_LOAD_TEMPORARY);
    */

    unsigned char* mem = new unsigned char[size];

    ::radMemorySetCurrentAllocator (old);

    /*
    SetCurrentAlloc(old);
    */

    return mem;
}

void  FreeTemp(void* mem)
{
    /*
    AllocType old = GetCurrentAlloc();
    SetCurrentAlloc(ALLOC_LOAD_TEMPORARY);
    */
    delete [] ((unsigned char*)mem);
    /*
    SetCurrentAlloc(old);
    */
}

void  UsePermanentMem( bool flag )
{
    /*
    static AllocType old;

    if( flag ){         //true
        old = GetCurrentAlloc();
        SetCurrentAlloc( ALLOC_PERMANENT );
    }
    else
        SetCurrentAlloc( old );       
    */
}

}
