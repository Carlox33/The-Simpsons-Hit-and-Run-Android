//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        radthread.hpp
//
// Subsystem:	Foundation Technologies - Platform Independant Threading
//
// Description:	This file contains the interface definitions to the FTECH
//              threading services. These services provide simple thread management
//              and synchronization primitives.
//
// Revisions:   v1.00   Jan 3, 2002     Creation
//
//=============================================================================

#ifndef	RADTHREAD_HPP
#define RADTHREAD_HPP

//=============================================================================
// Include Files
//=============================================================================

#include <radobject.hpp>
#include <radmemory.hpp>

//=============================================================================
// Defintions
//=============================================================================

//
// This is the prototype for thread entry function. The function is passed the
// user data and can return any value.
//
typedef unsigned int (*RADTHREADENTRY)( void* userData );

//
// This prototype is used to define the fiber entry point. Fibers normally
// never return, hence no return code.
//
typedef void (*RADFIBERENTRY)( void* userData );

//=============================================================================
// Interface Specifications
//=============================================================================

//
// The following interface defintion is used to interact with a mutex object.
// The mutex provides a way of serializing thread access to a non-rentrant
// section of code. Note: It is invalid to destroy the mutex will it is in a 
// locked state.
//
struct IRadThreadMutex : public IRefCount
{
    //
    // Invoke the lock member to lock the mutex object. The same thread
    // can invoke lock more than once without being blocked. Must call unlock
    // same number of times. 
    //
    virtual void Lock( void ) = 0;

    //
    // Invoke unlock to free the mutex object. 
    //
    virtual void Unlock( void ) = 0;
};

//
// The following interface defintion is used to interact with a semaphore object.
// The semaphore provides a way suspending and resuming a thread based on
// a count. When a thread issues a wait, the the count is decremented. If the
// count is negative, the thread is suspended. The signal increments the count 
// and releases a thread that is waiting for the semaphore.
//
struct IRadThreadSemaphore : public IRefCount
{
    //
    // This member will decrement count. If less than zero, thread is suspended.
    //
    virtual void Wait( void ) = 0 ;

    //
    // This member increments semaphore count.
    //
    virtual void Signal( void ) = 0;
};

// NEW LINE FOR ANDROID 
void radThreadAttachCurrentThreadIfNeeded(void);  


//
// This interface is used to interact with a thread. Releasing the last reference
// to the thread object will kill the thread. This is a harsh method and it is 
// better to have the thread terminate itself ( ie return ).
//
struct IRadThread : public IRefCount
{
    //
    // Priorities are normalized across all platforms. The main
    // thread is adjusted to run a normal priority when the threading system
    // is initialized.
    //
    enum Priority
    {
        PriorityIdle,           
        PriorityBelowNormal,    
        PriorityNormal,          
        PriorityAboveNormal,
        PriorityHigh
    };             

    virtual void SetPriority( Priority priority ) = 0;
    virtual Priority GetPriority( void ) = 0;
    
    //
    // Use these members to explicitly suspend and resume the thread. They
    // use an internal count, to the number of resumes must equal the number of
    // suspends 
    //
    virtual void Suspend( void ) = 0;
    virtual void Resume( void ) = 0;

    //
    // This member function can be used to determine if the thread is still running.
    // If it has terminated (returned from the entry point), the functions return code is returned.          
    // The return code is only valid if the thread has terminated (this member function returns false).
    //
    virtual bool IsRunning( unsigned int* pReturnCode ) = 0;

    //
    // This member waits until the thread terminates and returns the return code of
    // the entry point.
    // Of course if the thread doesn't terminate, neither will this function.
    //
    virtual unsigned int WaitForTermination( void ) = 0;
};

//
// This interface is used for accessing thread local storage. Every thread that accesses the
// object backed by this interface can set and get values that are specific to the thread.
// If GetValue( ) is called before SetValue( ), a value of NULL is returned.
//
struct IRadThreadLocalStorage : public IRefCount
{
    virtual void* GetValue( void ) = 0;
    virtual void  SetValue( void* value ) = 0;    
};

//
// This interface is used to interact with a fiber. Releasing all references kills the fiber
// and releases any resources ( ie stack memory ). Care should be taken not to kill the active 
// fiber.
//
struct IRadThreadFiber : public IRefCount
{
    //
    // This member is used to switch to this fiber. This function should not be invoked
    // on the active fiber or a fiber created by another thread.
    //
    virtual void SwitchTo( void ) = 0;    

    //
    // These functions are provide for convience. The allow data to be set and retrived. 
    // The value initially set is user data that was provided when the fiber was created.
    //
    virtual void* GetValue( void ) = 0;
    virtual void  SetValue( void* value ) = 0;
};

//=============================================================================
// Function Declarations
//=============================================================================

//
// These functions are used to intialize and terminate the threading system.
// They should frame all other calls made to this system. Initialize takes 
// a paremeter which governs round robin scheduling period. This is the period
// at which threads of equal priority will be premptively rescheduled. Please note 
// that this parameter has no effect on the WIN32 and XBOX platforms yet. Setting
// the value to zero disables round robin scheduling. Default is arbritarily choosen
//
void radThreadInitialize( unsigned int milliseconds = 50 );
void radThreadTerminate( void );

//
// This function is used to create a mutex object. A mutex object is a simple object
// that allows only one thread to hold it at a time. It is used to prevent more than
// one thread from entering a critical section of code.
//
void radThreadCreateMutex( IRadThreadMutex** ppMutex, 
                           radMemoryAllocator allocator = RADMEMORY_ALLOC_DEFAULT );

// 
// This function is used to create a semaphore object. A semaphore allows one thread 
// to suspend itself, waiting to be signal from another thread. An intial count can
// be specified which controls the number of waits that can be issued before suspension
// occurs. This is useful for controlling the number of threads that can share a
// resource. 
//
void radThreadCreateSemaphore( IRadThreadSemaphore** ppSemaphore,
                               unsigned int count = 0,
                               radMemoryAllocator allocator = RADMEMORY_ALLOC_DEFAULT );
                                
//
// This function is used to create a thread. Threads use preemptive scheduling. Threads can
// terminate themselves and do so by simply returing from thread entry point. 
//
void radThreadCreateThread( IRadThread**         ppThread,               
                            RADTHREADENTRY       pEntryFunction,         
                            void*                userData,               
                            IRadThread::Priority priority = IRadThread::PriorityNormal,
                            unsigned int         stackSize = 4096,
                            radMemoryAllocator   allocator = RADMEMORY_ALLOC_DEFAULT );
                                   
//
// This function is invoked to the the currently running thread. This can be used to initially
// get the main thread of execution.
//
IRadThread* radThreadGetActiveThread( void );

//
// This function is used by a thread to sleep ( suspend ) itself for a the specified duration.
// A value of zero, will simply cause the thread to relinquish its time slice an allow other threads
// to execute.
//
void radThreadSleep( unsigned int milliseconds );

//
// This member is used to allocate thread local storage. It can be used to get and set
// data specific to a thread. 
//
void radThreadCreateLocalStorage( IRadThreadLocalStorage** pThreadLocalStorage,
                                  radMemoryAllocator allocator = RADMEMORY_ALLOC_DEFAULT );

//
// The following functions are used to deal with fibers. Fibers provide a mechanism for
// allowing direct control of scheduling. Fibers run in the context of a thread, and
// have there own stack. By default, each thread in the system is also a fiber. New fibers
// can be created and switched to. Fibers should never return. The first function
// is used to create a fiber. The fiber does not run until explicitly switched to.
//
void radThreadCreateFiber( IRadThreadFiber** ppFiber,            // Fiber interface
                           RADFIBERENTRY     pEntryFunction,     // Function
                           void*             userData,           // User Data.
                           unsigned int      stackSize = 4096,  // Stack size
                           radMemoryAllocator allocator = RADMEMORY_ALLOC_DEFAULT );

//
// This function can be invoked any time to get the currently running fiber. By default,
// the every thread is fiber.
//
IRadThreadFiber* radThreadGetActiveFiber( void );



//
// An object of class radSingleLock represents the access-control mechanism used in controlling
// access to a resource in a multithreaded program.
// Template allow use of any class type that has Lock(), Unlock(), AddRef(), and Release() function.
//
// for example instead of writing code
// ...
// pMutex->Lock( );
// ...
// if ( bFoundData1 )
// {
//     pMutex->Unlock( );
//     return;
// }
// ...
//
// if ( bFoundData2 )
// {
//     pMutex->Unlock( );
//     return;
// }
//
// pMutex->Unlock( );
// return;
// ...
//
// you can write code like this, and sync object will be lock and unlocked automatically.
//
// radSingleLock< IRadThreadMutex > singleLock( pMutex, true );
// 
// ...
// if ( bFoundData1 )
// {
//     return;
// }
// ...
//
// if ( bFoundData2 )
// {
//     return;
// }
//
// return;
// ...
//
template< class SyncObjType >
class radSingleLock
{
public:

    //
    // if user specified bInitialLock to be true, the constructor will
    // automatically lock the sync object, and destructor will automatically
    // unlock the sync object.
    //
    radSingleLock( SyncObjType * pSyncObject, bool bInitialLock = false ) :
        m_pSyncObj( NULL ),
        m_bInitialLock( bInitialLock )
    {
        rAssert( pSyncObject );
        m_pSyncObj = pSyncObject;

        if ( m_bInitialLock )
        {
            m_pSyncObj->Lock( );
        }
    }

    ~radSingleLock( )
    {
        if ( m_bInitialLock )
        {
            m_pSyncObj->Unlock( );
        }
    }

    void Lock( )
    {
        m_pSyncObj->Lock( );
    }

    void Unlock( )
    {
        m_pSyncObj->Unlock( );
    }

private:
    SyncObjType *     m_pSyncObj;
    bool              m_bInitialLock;
};


#endif 
