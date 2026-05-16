//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        thread.cpp
//
// Subsystem:	Radical Threading Services - Thread and Fiber Implementation
//
// Description:	This file contains the implementation of the threading services
//              thread and fibers. It also contains the thread local storage
//              implementation.
//
// Author:		Peter Mielcarski
//
// Revisions:	V1.00	Jan 8, 2002
//
//=============================================================================

//=============================================================================
// Include Files
//=============================================================================

#include "pch.hpp"

#include <radthread.hpp>
#include <radmemorymonitor.hpp>
#include "system.hpp"
#include "thread.hpp"
#include <SDL.h>
#if defined(RAD_ANDROID)
  #include <android/log.h>
  #define LOG_TAG "SimpsonsHitAndRun"
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#elif defined(RAD_VITA)
  #include <psp2/kernel/clib.h>
  #define LOGI(...) do { sceClibPrintf(__VA_ARGS__); sceClibPrintf("\n"); } while(0)
  #define LOGE(...) do { sceClibPrintf(__VA_ARGS__); sceClibPrintf("\n"); } while(0)

#else
  #include <cstdio>
  #define LOGI(...) do { std::printf(__VA_ARGS__); std::printf("\n"); std::fflush(stdout); } while(0)
  #define LOGE(...) do { std::printf(__VA_ARGS__); std::printf("\n"); std::fflush(stdout); } while(0)
#endif

static void LogActiveThread(const char* where, unsigned index, void* value_or_null)
{
    unsigned long tid = (unsigned long)SDL_ThreadID();
    radThread* t = (radThread*)radThread::GetActiveThread(); // si es estática pública, perfecto

    if (t)
    {
        LOGI("[radTLS] %s tid=%lu activeRadThread=%p index=%u value=%p",
             where, tid, (void*)t, index, value_or_null);
    }
    else
    {
        LOGE("[radTLS] %s tid=%lu activeRadThread=NULL index=%u value=%p",
             where, tid, index, value_or_null);
    }
}



//=============================================================================
// Local Definitions
//=============================================================================

//=============================================================================
// Statics
//=============================================================================

//
// This memory is used for the thread object that exists. We do not want to new
// up memory as this will require the memory system to be initialized.
// 
static unsigned int s_theThreadMemory[ ((sizeof( radThread)) / sizeof(unsigned int)) + 1 ];

// NEW LINES FOR ANDROID 
// --- Thread system init flag (para evitar attach antes de Initialize()) ---
static bool s_RadThreadSystemInitialized = false;

// --- Pool estático para wrappers "attached" (sin heap) ---
// Reservamos hasta MAX_RADTHREADS-1 porque el slot 0 es el "main thread" de radThread::Initialize().
static unsigned int s_attachedThreadMemory[ MAX_RADTHREADS - 1 ]
                                      [ ((sizeof(radThread)) / sizeof(unsigned int)) + 1 ];

static bool s_attachedThreadUsed[ MAX_RADTHREADS - 1 ] = { false };

//
// This table is used to manage pointers to currently created threads. Pointers
// are no reference counted.
//
radThread* radThread::s_ThreadTable[ MAX_RADTHREADS ];

//
// The following table are provided to map our priorities to OS specific 
// priorities.
//
static SDL_ThreadPriority s_PriorityMap[ radThread::PriorityHigh + 1 ] =
        { SDL_THREAD_PRIORITY_LOW, SDL_THREAD_PRIORITY_LOW, SDL_THREAD_PRIORITY_NORMAL,
          SDL_THREAD_PRIORITY_HIGH, SDL_THREAD_PRIORITY_HIGH };

//
// This static is used to manage free thread local storgage objects. The
// entry is set true if the index has been consumed. This table relies on 
// statics being initialized to zero (false) by the compiler.
//
bool radThreadLocalStorage::s_InUseIndexTable[ MAX_THREADLOCALSTORAGE_OBJECTS ];
 
//=============================================================================
// Public Functions
//=============================================================================

//=============================================================================
// Function:    radThreadCreateThread
//=============================================================================
// Description: This is the factory for the thread object. 
//
// Parameters:  ppThread    -      where to return interface
//              pEntryFunction     thread entry
//              userdata
//              priority           
//              stack size         
//              memory allocator
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

void radThreadCreateThread
( 
    IRadThread**            ppThread,
    RADTHREADENTRY          pEntryFunction,
    void*                   userData,   
    IRadThread::Priority    priority,   
    unsigned int            stackSize,  
    radMemoryAllocator      allocator
)
{
    *ppThread = new( allocator ) radThread( pEntryFunction, userData,
                                            priority, stackSize );
}



//=============================================================================
// Function:    radThreadGetActiveThread
//=============================================================================
// Description: This member will return the actively running thread.
//
// Parameters:  none
//              
// Returns:     Interace pointer to thread object
//
// Notes:
//------------------------------------------------------------------------------

IRadThread* radThreadGetActiveThread( void )
{
    return( radThread::GetActiveThread( ) );
}

//=============================================================================
// Function:    radThreadSleep
//=============================================================================
// Description: This function causes the calling thread to sleep for the 
//              specified duration. If zero is specified, the calling thread
//              just relinquishes its time slice.
//
// Parameters:  milliseconds
//              
// Returns:     N/A
//
// Notes:       Need platform specific implementation of this function.
//------------------------------------------------------------------------------


// NEW FUNCTION FOR CAN DO AttachCurrentThreadIfNeeded from main(ANDROID USE  THIS )

void radThreadAttachCurrentThreadIfNeeded(void)
{
    radThread::AttachCurrentThreadIfNeeded();
}


void radThreadSleep
( 
    unsigned int milliseconds
)
{
    SDL_Delay( milliseconds );
}

//=============================================================================
// Function:    PS2SleepAlarm
//=============================================================================
// Description: This function is invoked under interrupts by the Sony alarms.
//              It is used to create a sleep functionality. Its job is
//              to wake the sleeping thread after time has elapsed.
//
// Parameters:  id - id of alarm.
//              time - time of alarm setting.
//              pInfo - used data.
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

#ifdef RAD_PS2

void PS2SleepAlarm( int id, unsigned short time, PS2SleepInfo* pInfo )
{
    //
    // The Sony alarm has exired. Update the alarm count and check if
    // zero.
    //
    pInfo->m_AlarmCount--;

    if( pInfo->m_AlarmCount == 0 )
    {
        //
        // We are done. Indicate no alarm needs to be terminated and wake up
        // the thread.
        //
        pInfo->m_AlarmId = -1;

        iWakeupThread( pInfo->m_ThreadId );
    }
    else
    {
        //
        // Reset the alarm for another count.
        //
        pInfo->m_AlarmId = iSetAlarm( pInfo->m_AlarmSetting,
                                      (void(*)(int,unsigned short,void*)) PS2SleepAlarm,
                                      pInfo );
    }
}

#endif

//=============================================================================
// Function:    GCNSleepAlarm
//=============================================================================
// Description: This function is invoked by the game cube os after a specified
//              duration has expired. We wake up the thread.
//
// Parameters:  alarm
//              context - not used
//              
// Returns:     N/A
//
// Note:        We must perform a hack as the GameCube libs provide no used
//              data. We need to get the thread queue to wake up.
//
//------------------------------------------------------------------------------

#ifdef RAD_GAMECUBE

void GCNSleepAlarm( OSAlarm *pAlarm, OSContext* pContext )
{   
    (void) pContext;
    
    GCNSleepInfo* pSleepInfo = (GCNSleepInfo*) pAlarm;
    
    OSWakeupThread( &pSleepInfo->m_ThreadQueue );

}
#endif

//=============================================================================
// Function:    radThreadCreateLocalStorage
//=============================================================================
// Description: This is the factory for the thread local storage object. 
//              Just new up a thread local storage object.
//
// Parameters:  pThreadLocalStorage  -  where to return interface
//              memory allocator
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

void radThreadCreateLocalStorage
( 
    IRadThreadLocalStorage** pThreadLocalStorage,
    radMemoryAllocator       allocator
)
{
    *pThreadLocalStorage = new( allocator ) radThreadLocalStorage( );
}

#if 0
//=============================================================================
// Function:    radThreadCreateFiber
//=============================================================================
// Description: This is the factory for the fiber object. 
//              Just new up a fiber object.
//
// Parameters:  ppFiber - return interface
//              pEntryFunction - entry point of the fiber
//              userData - user data
//              stackSize
//              memoryAllocator
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

void radThreadCreateFiber
( 
    IRadThreadFiber** ppFiber,
    RADFIBERENTRY     pEntryFunction,
    void*             userData,
    unsigned int      stackSize,
    radMemoryAllocator allocator
)
{
    *ppFiber = new( allocator ) radThreadFiber( pEntryFunction,
                                                userData,
                                                stackSize );
}

//=============================================================================
// Function:    radThreadGetActiveFiber
//=============================================================================
// Description: This member will return the actively running fiber.
//
// Parameters:  none
//              
// Returns:     Interace pointer to fiber object
//
// Notes:
//------------------------------------------------------------------------------

IRadThreadFiber* radThreadGetActiveFiber( void )
{
    return( radThread::GetActiveFiber( ) );
}
#endif

//=============================================================================
// Public Member Functions
//=============================================================================

//=============================================================================
// Function:    radThread::Initialize
//=============================================================================
// Description: This static member is used to initialize this system. Essentially
//              it clears the thread table and sets up an initial thread object
//              to represent the main thread.
//
// Parameters:  unsigned int milliseconds - round robin scheduling periond
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

void radThread::Initialize( unsigned int milliseconds )
{
    //
    // First zero the thread table. Don't have to worry about thread protection
    // at this time, since no thread can be created until system is initialized.
    //
    for( unsigned int i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        s_ThreadTable[ i ] = NULL;   
    }

    //
    // Construct the default thread object. It is 
    // statically defined since we do not want to perform memory allocations 
    // as the memory system may require the threading system be initialized.
    // (chicken - egg problem ).
    //
    new( s_theThreadMemory ) radThread( );
    s_RadThreadSystemInitialized = true;// NEW LINE FOR ANDROID 

    (void)milliseconds;
}

//=============================================================================
// Function:    RescheduleThreadEntry
//=============================================================================
// Description: On the Gamecube, round robin scheduling of equal priority 
//              threads does not work as advertized. To get it to work
//              we create a high priority thread that sleeps. When in awakes,
//              lower priory threads are rescheduled when this thread goes
//              back to sleep.
//
// Parameters:  usedData - not used.
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

#ifdef RAD_GAMECUBE

void* RescheduleThreadEntry( void* param )
{
    while( true )
    {
        radThreadSleep( s_RescheduleSleepTime );
    }

    return( NULL );
}

#endif

//=============================================================================
// Function:    radThread::Terminate
//=============================================================================
// Description: This static member is used to shut down the thread stuff. 
//              Clean up 
//
// Parameters:  N/A
//              
// Returns:     N/A
//
// Notes:
//------------------------------------------------------------------------------

void radThread::Terminate( void )
{
    //
    // On the PS2, release the alarm that is performing our round robin 
    // scheduling. Protect this operation as the alarm may expired while 
    // releasing it. This could cause a new one to be created.
    //
#ifdef RAD_PS2

    DI( );
    
    if( s_AlarmId != -1 )
    {
        ReleaseAlarm( s_AlarmId );
    }

    EI( );

#endif

#ifdef RAD_GAMECUBE

    if( s_RescheduleSleepTime != 0 )
    {
        //
        // Must cancel high priority thread which is causing round
        // robin scheduling.
        //
        OSCancelThread( &s_RescheduleThread );
    }

#endif

    //
    // Exlicitly destruct our thread object that represents the main thread.
    //
    ((radThread*)s_theThreadMemory)->~radThread( );

    //
    // Lets make sure all threads have been released. If not assert. 
    //
    for( unsigned int i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        rAssert( s_ThreadTable[ i ] == NULL );
    }
    s_RadThreadSystemInitialized = false; // NEW LINE FOR ANDROID 
}

//=============================================================================
// Function:    radThread::AlarmHandler
//=============================================================================
// Description: On the PS2, this routine is invoked as an interrupt handler.
//              This routine goes rotates threads of each prioity this 
//              system supports. This stuff should be very fast (hopefully).
//
// Parameters:  alarm id, 
//              time
//              userdata
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

#ifdef RAD_PS2

void radThread::AlarmHandler( int id, unsigned short time, void* userData )
{
    (void) id; (void) time; (void) userData;

    //
    // For each prioirty we support, rotate threads of that proirity.
    //
    for( unsigned int i = 0 ; i < (sizeof( s_PriorityMap ) / sizeof( int ) ) ; i++ )
    {
        iRotateThreadReadyQueue( s_PriorityMap[ i ] );
    }

    //
    // Reset the alarm.
    //
    s_AlarmId = iSetAlarm( RADTHREAD_PS2_ROUNDROBINTIME, AlarmHandler, NULL );

}

#endif


//=============================================================================
// Function:    radThread::radThread
//=============================================================================
// Description: This is the default constructor for single thread used to 
//              represent the main thread. All we do is get the current thread 
//              id and add ourself the the active thread table.
//
// Parameters:  N/A
//              
// Returns:     N/A
//
// Notes:       We are reference counted but the object will never be deleted
//              using release.
//------------------------------------------------------------------------------

radThread::radThread( void )
    :
    m_ReferenceCount( 1 ),
    m_IsRunning( true )
    //m_pActiveFiber( &m_Fiber )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThread" );

    m_ThreadId = SDL_ThreadID();

    //
    // Add ourself as the first entry in the thread table. No protection
    // required as no other threads are running yet.
    //
    s_ThreadTable[ 0 ] = this;  

    //
    // Alter the thread proirity to ensure that things are running at the 
    // default proirity.
    //
    SetPriority( IRadThread::PriorityNormal );

    //
    // Set the default ThreadLocalStorage values to NULL
    //
    for ( unsigned int i = 0; i < MAX_THREADLOCALSTORAGE_OBJECTS; i++ )
    {
        m_ThreadLocalStorageValues[i] = NULL;
    }
}

//=============================================================================
// Function:    radThread::radThread
//=============================================================================
// Description: This is construct is used for threads created by the application.
//              Create thread using platform specific functions.
//
// Parameters:  pEntryFunction     thread entry
//              userdata
//              priority           
//              stack size         
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThread::radThread
(   
    RADTHREADENTRY          pEntryFunction,
    void*                   userData, 
    IRadThread::Priority    priority, 
    unsigned int            stackSize
)
    :
        m_ReferenceCount( 1 ),
        m_IsRunning( true ),
        m_EntryFunction( pEntryFunction ),
        m_UserData( userData )
        //m_pActiveFiber( &m_Fiber )
{
    //
    // Lets add ourself to the active thread table. Do so under protection 
    // since other threads may access this table.
    //   
    radThreadInternalLock( );   

    unsigned int i;
    for( i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        if( s_ThreadTable[ i ] == NULL )
        {
            s_ThreadTable[ i ] = this;
            break;
        }
   
    }   

    //
    // Make sure we did not exceed maximun number of threads.
    //
    rAssertMsg( i != MAX_RADTHREADS, "Too many threads created\n");

    //
    // Create thread which then sets its own priority.
    //
    m_Priority = priority;

#if SDL_MAJOR_VERSION < 3
    m_ThreadHandle = SDL_CreateThreadWithStackSize(InternalThreadEntry, /*name*/nullptr, stackSize * 1024, this);
#else
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_THREAD_CREATE_ENTRY_FUNCTION_POINTER, (void*)InternalThreadEntry);
    SDL_SetNumberProperty(props, SDL_PROP_THREAD_CREATE_STACKSIZE_NUMBER, (Sint64)(stackSize * 1024));
    SDL_SetPointerProperty(props, SDL_PROP_THREAD_CREATE_USERDATA_POINTER, this);
    m_ThreadHandle = SDL_CreateThreadWithProperties(props);
    SDL_DestroyProperties(props);
#endif

    //
    // Release our protection.
    //
    radThreadInternalUnlock( );

    //
    // Set the default ThreadLocalStorage values to NULL
    //
    for ( i = 0; i < MAX_THREADLOCALSTORAGE_OBJECTS; i++ )
    {
        m_ThreadLocalStorageValues[i] = NULL;
    }
}

//=============================================================================
// Function:    radThread::radThread( AttachTag )
//=============================================================================
// Wrapper constructor for attaching an EXISTING OS thread (SDLThread, etc).
// ANDROID NEED THIS FOR SDL THREAD 
// IMPORTANT: Must NOT touch s_ThreadTable[0].
//=============================================================================
radThread::radThread( AttachTag )
    :
    m_ReferenceCount( 1 ),
    m_IsRunning( true ),
    m_ReturnCode( 0 ),
    m_EntryFunction( NULL ),
    m_UserData( NULL ),
    m_Priority( IRadThread::PriorityNormal ),
    m_ThreadId( 0 ),
    m_ThreadHandle( NULL ),
    m_pActiveFiber( NULL )
{
    // Identificación en monitor (si está disponible)
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThread(Attached)" );

    // TLS a NULL
    for ( unsigned int i = 0; i < MAX_THREADLOCALSTORAGE_OBJECTS; i++ )
    {
        m_ThreadLocalStorageValues[i] = NULL;
    }
}


//=============================================================================
// Function:    radThread::~radThread
//=============================================================================
// Description: This is the destructor for the mutex object. Simply free the
//              OS resources. If thread is running when we get here, we kill
//              it. This is harsh and should be avoided.
//
// Parameters:  none
//              
// Returns:     N/A
//
// Notes:       We need a special check for the main thread object. We do not
//              kill this one as it always runs.
//------------------------------------------------------------------------------

radThread::~radThread( void )
{
    //
    // No longer any active fibers.
    //
    //m_pActiveFiber = NULL;

    //
    // Check if we are destructing the main thread object. This thread 
    // represents the main thread and was never created by this system.
    //
    if( s_ThreadTable[ 0 ] == this )
    {
        //
        // Just null the table entry and we are done.
        //
        s_ThreadTable[ 0 ] = NULL;
        return;
    }

    //
    // Protect the next series of operations.
    //
    radThreadInternalLock( );   

    //
    // First remove this thread from the active table.
    //
    unsigned int i;
    for( i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        if( s_ThreadTable[ i ] == this )
        {
            s_ThreadTable[ i ] = NULL;
            break;
        }
    }   
    rAssert( i != MAX_RADTHREADS );

    //
    // Here we must check if the thread is still running. This is the bad way
    // of killing a thread and should be avoided. However, we will try to deal with it.
    //
    if( m_IsRunning )
    {
        //
        // Here the thread is running. We will kill the thread. Need to deal with
        // special case where the thread we are killing is the actual running thread.
        //
        if( IsActive( ) )
        {
            //
            // Here we are killing ourself by releaseing the last reference.  This is
            // a very bad way of terminating the thread and will cause leaks.
            // Can release protection now.
            //
            radThreadInternalUnlock( );   

            //
            // We perform platform specific operations and commit suicide. Print
            // warning that memory leaks will may occur as we cannot release all
            // memory or call base class destructor.
            //
            rWarningMsg( false, "radThread: Memory leak has occurred due to bad thread termination\n");
   
            //SDL_KillThread(m_ThreadHandle);
        }
        else
        {
            //
            // Here we are killing a thread that is still running but the calling thread
            // is not the active thread. Perform OS specific terminations. Note on XBOX
            // this cannot be done. 
            //
            //SDL_KillThread(m_ThreadHandle);
            
            //
            // Can release lock. Print warning that this is a bad way to terminate thread
            // as OS may not release all resources.
            //
            radThreadInternalUnlock( );   
                       
            rWarningMsg( false, "radThread: Termination of thread may leak resources.\n");
        }
    }
    else
    {
        //
        // Even though our internal flag indicates that the thread has terminated, we
        // want to make sure it has terminated from the OS perspective since we
        // will be freeing the stack in some cases.
        //
        radThreadInternalUnlock( );   

        //SDL_WaitThread(m_ThreadHandle, nullptr);
    }
}

//=============================================================================
// Function:    radThread::InternalThreadEntry
//=============================================================================
// Description: This is the entry point for threads we create. Needed to 
//              make the users thread entry point platform independant.
//
// Parameters:  param - the thread object
//              
// Returns:     platform specific returns. Not used as we use our own 
//              mechanism.
//
// Notes:
//------------------------------------------------------------------------------

int radThread::InternalThreadEntry( void* param )
{
    //
    // Simply invoke the true entry point. Save return code upon return
    // from callers function.   
    //
    radThread* pThread = (radThread*) param;
    pThread->m_ThreadId = SDL_ThreadID();

    // In SDL, thread priority can only be set on the current thread, so we do it here.
    pThread->SetPriority(pThread->m_Priority);

    //
    // Under windows, convert this thread to a fiber.
    //
#if defined(RAD_WIN32) || defined(RAD_XBOX)
    //pThread->m_Fiber.m_Win32Fiber = ConvertThreadToFiber( NULL );
#endif

    pThread->m_ReturnCode = (pThread->m_EntryFunction)(pThread->m_UserData );

    //
    // Here we consider the thread no longer running.
    //
    pThread->m_IsRunning = false;

    return 0;
}

//=============================================================================
// Function:    radThread::SetPriority
//=============================================================================
// Description: Updates the threads priority
//
// Parameters:  new priority
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThread::SetPriority( Priority priority )
{
    //
    // Save the priority as our current setting and invoke the various OS
    // serives to alter the priority.
    //
    m_Priority = priority;

#if SDL_MAJOR_VERSION < 3
    SDL_SetThreadPriority( s_PriorityMap[ priority ] );
#else
    SDL_SetCurrentThreadPriority( s_PriorityMap[ priority ] );
#endif
}

//=============================================================================
// Function:    radThread::GetPriority
//=============================================================================
// Description: Retrieves current thread priority
//
// Parameters:  
//
// Returns:     priority
//
// Notes:
//------------------------------------------------------------------------------

IRadThread::Priority radThread::GetPriority( void )
{
    return( m_Priority );
}

//=============================================================================
// Function:    radThread::Suspend
//=============================================================================
// Description: Suspends this thread. This member is counted, and number of
//              resumes must equal number of suspends.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThread::Suspend( void )
{ 
    rAssertMsg(false, "SDL does not support suspending/resuming threads\n");
}

//=============================================================================
// Function:    radThread::Resume
//=============================================================================
// Description: Resume this thread. This member is counted, and number of
//              resumes must equal number of suspends.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThread::Resume( void )
{ 
    rAssertMsg(false, "SDL does not support suspending/resuming threads\n");
}

//=============================================================================
// Function:    radThread::IsRunning
//=============================================================================
// Description: Use to check if a thread has terminated. 
//
// Parameters:  Address of where to receive the thread return code. Only
//              valid if thread has terminated.
//
// Returns:     true, thread running. false terminate.
//
// Notes:
//------------------------------------------------------------------------------

bool radThread::IsRunning
( 
    unsigned int* pReturnCode
)
{
    //
    // Check if running. If not return the return code. Care must be taken
    // here as these variables can be altered by the actual thread.
    //
    if( m_IsRunning )
    {
        return( true );
    }

    //
    // Terminated. Return code is valid.
    //
    *pReturnCode = m_ReturnCode;
    
    return( false );
}

//=============================================================================
// Function:    radThread::WaitForTermination
//=============================================================================
// Description: Use to wait until a thread has terminated.
//
// Parameters:  
//
// Returns:     Return value of the entry point
//
// Notes:       Just using a semaphore in the thread entry is not good enough
//              since the entry point might not return right away.
//------------------------------------------------------------------------------

unsigned int radThread::WaitForTermination( void )
{
    int ret;
    SDL_WaitThread(m_ThreadHandle, &ret);
    return ret;
}

//=============================================================================
// Function:    radThread::IsActive
//=============================================================================
// Description: This internal helper function is used to determine if the
//              thread object is currently the running thread.
//
// Parameters:  n/a
//
// Returns:     true if thread is the active one.
//
// Notes:
//------------------------------------------------------------------------------

bool radThread::IsActive( void )
{
    return m_ThreadId == SDL_ThreadID();
}

//=============================================================================
// Function:    radThread::GetActiveThread
//=============================================================================
// Description: This static function is invoked to get the running thread object.
//              We need it to translate OS specific thread ids to our thread
//              object.
//
// Parameters:  n/a
//
// Returns:     IRadThread* - actively running thread.
//
// Notes:
//------------------------------------------------------------------------------

IRadThread* radThread::GetActiveThread( void )
{
    unsigned int i;

    //
    // Tranverse the thread table asking each thread if it is the active.
    // Need protection because threads may terminate and we don't want
    // to vector into outerspace. This will not happen for the active thread
    // as this is who is calling this function.
    //
    radThreadInternalLock( );   
   
    for( i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        if( s_ThreadTable[ i ] != NULL )
        {
            if( s_ThreadTable[ i ]->IsActive( ) )
            {
                break;
            }
        }
    }
    
    radThreadInternalUnlock( );   
                                                
    //
    // Make sure not some logic problem if we failed to find a the active.
    //
    if( MAX_RADTHREADS == i )
    {
        #ifdef RAD_ANDROID
        //LOGE("[radTHREAD] GetActiveThread FAILED tid=%u",
          //   (unsigned int)SDL_ThreadID());
        #endif
        return( NULL );
    }
    #ifdef RAD_ANDROID
    //LOGI("[radTHREAD] GetActiveThread OK tid=%u slot=%u radThread=%p",
      //   (unsigned int)SDL_ThreadID(),
        // i,
         //(void*)s_ThreadTable[i]);
    #endif
    return( s_ThreadTable[ i ] );
}

// FUNCION VERY IMPORTANT PA ANDROID LA VERDAH, fuera bromas esto hace que el SDL thread se enlace con la tabla de memoria donde se controlan todos los hilos, y en principio habrá que revisar pero es poco invasivo, 
// por lo que es bastante bueno para evitar bugs raros en el futuro
radThread* radThread::AttachCurrentThreadIfNeeded( void )
{
#ifdef RAD_ANDROID
    const unsigned long tid = (unsigned long) SDL_ThreadID();

    // 0) Si aún no se inicializó el sistema, no tocamos nada.
    if ( !s_RadThreadSystemInitialized )
    {
        LOGE("[radTHREAD] AttachCurrentThreadIfNeeded called before radThread::Initialize (tid=%lu)", tid);
        return NULL;
    }

    // 1) Fast path: si ya hay active thread, devolvemos.
    {
        IRadThread* active = radThread::GetActiveThread();
        if ( active != NULL )
        {
            return (radThread*) active;
        }
    }

    // 2) Slow path: lock + buscar / crear wrapper
    radThreadInternalLock();

    // 2a) Puede que otro haya registrado justo antes de nuestro lock
    for ( unsigned int i = 0; i < MAX_RADTHREADS; i++ )
    {
        if ( s_ThreadTable[i] && s_ThreadTable[i]->m_ThreadId == tid )
        {
            radThread* found = s_ThreadTable[i];
            radThreadInternalUnlock();
            //LOGI("[radTHREAD] Attach: already attached tid=%lu slot=%u radThread=%p", tid, i, (void*)found);
            return found;
        }
    }

    // 2b) Buscar slot libre en la tabla (NO usar slot 0)
    unsigned int freeSlot = MAX_RADTHREADS;
    for ( unsigned int i = 1; i < MAX_RADTHREADS; i++ )
    {
        if ( s_ThreadTable[i] == NULL )
        {
            freeSlot = i;
            break;
        }
    }

    if ( freeSlot == MAX_RADTHREADS )
    {
        radThreadInternalUnlock();
        LOGE("[radTHREAD] Attach FAILED: no free slot in s_ThreadTable (tid=%lu)", tid);
        return NULL;
    }

    // 2c) Reservar un bloque en el pool
    int poolIndex = -1;
    for ( unsigned int p = 0; p < (MAX_RADTHREADS - 1); p++ )
    {
        if ( !s_attachedThreadUsed[p] )
        {
            s_attachedThreadUsed[p] = true;
            poolIndex = (int)p;
            break;
        }
    }

    if ( poolIndex < 0 )
    {
        radThreadInternalUnlock();
        LOGE("[radTHREAD] Attach FAILED: no free pool blocks (tid=%lu)", tid);
        return NULL;
    }

    // 2d) Construir wrapper sin heap y registrar
    radThread* wrapper = new ( s_attachedThreadMemory[poolIndex] ) radThread( AttachTag{} );

    wrapper->m_ThreadId     = tid;
    wrapper->m_ThreadHandle = NULL;
    wrapper->m_IsRunning    = true;

    // TLS a NULL (redundante con ctor, pero lo dejo por seguridad)
    for ( unsigned int t = 0; t < MAX_THREADLOCALSTORAGE_OBJECTS; t++ )
        wrapper->m_ThreadLocalStorageValues[t] = NULL;

    s_ThreadTable[ freeSlot ] = wrapper;

    radThreadInternalUnlock();

    //LOGI("[radTHREAD] Attach OK tid=%lu slot=%u pool=%d radThread=%p",
      //   tid, freeSlot, poolIndex, (void*)wrapper);

    return wrapper;
#else
    // En otras plataformas normalmente no hace falta.
    return (radThread*)radThread::GetActiveThread();
#endif
}


//=============================================================================
// Function:    radThread::SetLocalStorage
//=============================================================================
// Description: This static function is used to set local storage. Indexs
//              are managed by the thread local storage object.
//
// Parameters:  index 
//              value
//
// Returns:     
//
// Notes:
//------------------------------------------------------------------------------
/*
void radThread::SetLocalStorage
(
     unsigned int index,
     void* value
)
{
     //
     // Get the actively running thread. Set the value
     //
     radThread* thread = (radThread*) GetActiveThread( );

     if( thread != NULL)
     {
        LOGI("el thread no es nulo ");
         thread->m_ThreadLocalStorageValues[ index ] = value;
     }else{
        LOGI("el thread SI es nulo");

     }
}

//=============================================================================
// Function:    radThread::GetLocalStorage
//=============================================================================
// Description: This static function is used to get local storage. Indexs
//              are managed by the thread local storage object.
//
// Parameters:  index 
//
// Returns:     value
//
// Notes:
//------------------------------------------------------------------------------

void* radThread::GetLocalStorage
(
     unsigned int index
)
{
     //
     // Get the actively running thread. Return the value
     //
     radThread* thread = (radThread*) GetActiveThread( );
     if( thread != NULL )
     {
        LOGI("el thread no es nulo ");
         return( thread->m_ThreadLocalStorageValues[ index ] );
     }else{
        LOGI("el thread SI es nulo");

     }
     return( NULL );
}
*/


// NEW FUNCTIONS FOR  IN ANDROID 

void radThread::SetLocalStorage(unsigned int index, void* value)
{
    // 1) Intentar obtener el thread activo (camino rápido)
    radThread* thread = (radThread*)GetActiveThread();

    // 2) Solo si NO existe, attach + reintento
#ifdef RAD_ANDROID
    if (!thread)
    {
        radThread::AttachCurrentThreadIfNeeded();
        thread = (radThread*)GetActiveThread();
    }
#endif

//#ifdef RAD_ANDROID
    // Log opcional, pero sin re-llamar GetActiveThread() otra vez
  //  if (thread)
       // LOGI("[radTLS] SET tid=%lu activeRadThread=%p index=%u value=%p",
         //    (unsigned long)SDL_ThreadID(), (void*)thread, index, value);
    //else
        //LOGE("[radTLS] SET tid=%lu activeRadThread=NULL index=%u value=%p",
          //   (unsigned long)SDL_ThreadID(), index, value);
//#endif

    if (thread)
        thread->m_ThreadLocalStorageValues[index] = value;
}

void* radThread::GetLocalStorage(unsigned int index)
{
    // 1) Camino rápido
    radThread* thread = (radThread*)GetActiveThread();

    // 2) Solo si NO existe, attach + reintento
#ifdef RAD_ANDROID
    if (!thread)
    {
        radThread::AttachCurrentThreadIfNeeded();
        thread = (radThread*)GetActiveThread();
    }
#endif

    void* cur = nullptr;
    if (thread)
        cur = thread->m_ThreadLocalStorageValues[index];

//#ifdef RAD_ANDROID
  //  if (thread)
        //LOGI("[radTLS] GET tid=%lu activeRadThread=%p index=%u value=%p",
          //   (unsigned long)SDL_ThreadID(), (void*)thread, index, cur);
    //else
      //  LOGE("[radTLS] GET tid=%lu activeRadThread=NULL index=%u value=%p",
        //     (unsigned long)SDL_ThreadID(), index, cur);
//#endif

    return cur;
}



//=============================================================================
// Function:    radThread::SetDefaultLocalStorage
//=============================================================================
// Description: This static function is used to reset the local storage object
//              of all threads back to NULL. Indices are managed by the thread
//              local storage object.
//
// Parameters:  index 
//
// Returns:
//
// Notes:
//------------------------------------------------------------------------------

void radThread::SetDefaultLocalStorage 
( 
    unsigned int index 
)
{    //
    // Need protection in case a thread wants to terminate. Traverse all threads
    // and set the TLS back to NULL.
    //
    radThreadInternalLock( );   
   
    for ( unsigned int i = 0 ; i < MAX_RADTHREADS ; i++ )
    {
        if ( s_ThreadTable[ i ] != NULL )
        {
            ( s_ThreadTable[ i ] )->m_ThreadLocalStorageValues[ index ] = NULL;
        }
    }
    
    radThreadInternalUnlock( );
}

#if 0
//=============================================================================
// Function:    radThread::GetActiveFiber
//=============================================================================
// Description: This static function is used to get the active threads, active
//              fiber.
//
// Parameters:  n/a
//
// Returns:     IRadThreadFiber;    
//
// Notes:
//------------------------------------------------------------------------------

IRadThreadFiber* radThread::GetActiveFiber( void )
{
    //
    // Get the actively running threads active fiber. We need to check for
    // null because during shutdown, we may not have an active thread.
    //
    radThread* pThread = (radThread*) GetActiveThread( );

    if( pThread == NULL )
    {
        return( NULL );
    }

    return( pThread->m_pActiveFiber );
}
#endif

//=============================================================================
// Function:    radThread::AddRef
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThread::AddRef
(
	void
)
{
    //
    // Protect this operation with mutex as this is not guarenteed to be thread
    // safe.
    //
    radThreadInternalLock( );
	m_ReferenceCount++;
    radThreadInternalUnlock( );
}

//=============================================================================
// Function:    radThread::Release
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThread::Release
(
	void
)
{
    //
    // Protect this operation with mutex as this is not guarenteed to be thread
    // safe.
    //
    radThreadInternalLock( );

	m_ReferenceCount--;

	if ( m_ReferenceCount == 0 )
	{
        radThreadInternalUnlock( );
		delete this;
	}
    else
    {
        radThreadInternalUnlock( );
    }
}

//=============================================================================
// Function:    radThread::Dump
//=============================================================================
// Description: This member is used to display object info
//
// Parameters:  string buffer and size of buffer
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

#ifdef RAD_DEBUG

void radThread::Dump( char* pStringBuffer, unsigned int bufferSize )
{
    sprintf( pStringBuffer, "Object: [radThread] At Memory Location:[%p]\n", this );
}

#endif

//=============================================================================
// Function:    radThreadLocalStorage::radThreadLocalStorage
//=============================================================================
// Description: Constructor for thread local storage. Find a free index under
//              the protection of the mutex. 
//
// Parameters:  N/A
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThreadLocalStorage::radThreadLocalStorage( void )
    :
    m_ReferenceCount( 1 )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThreadLocalStorage" );
    //
    // Protect the search for a free index.
    //
    radThreadInternalLock( );

    for( m_Index = 0 ; m_Index < MAX_THREADLOCALSTORAGE_OBJECTS ; m_Index++ )
    {
        if( !s_InUseIndexTable[ m_Index ] )
        {
            //
            // We have one. Set it in use and break out.
            //
            s_InUseIndexTable[ m_Index ] = true;
            break;
        }
    }

    radThreadInternalUnlock( );

    //
    // Assert if limit exceeded.
    //
    rAssertMsg( m_Index != MAX_THREADLOCALSTORAGE_OBJECTS, "Too many thread local storage objects created\n");

    //
    // NOTE: we don't need to set a default value here since radThread initializes
    // its values to NULL and our destructor sets the values back to NULL again.
    //
}


//=============================================================================
// Function:    radThreadLocalStorage::~radThreadLocalStorage
//=============================================================================
// Description: Destructor. Just free the local storage index and return the
//              unused entry to NULL.
//
// Parameters:  none
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThreadLocalStorage::~radThreadLocalStorage( void )
{
    radThread::SetDefaultLocalStorage ( m_Index );

    //
    // Don't need to protect as assignments are automic operations.
    //
    s_InUseIndexTable[ m_Index ] = false;
}

//=============================================================================
// Function:    radThreadLocalStorage::GetValue
//=============================================================================
// Description: This is used to obtain the value. The value will be unique to
//              each thread accessing this object.
//
// Parameters:  n/a
//
// Returns:     value
//
// Notes:
//------------------------------------------------------------------------------

void* radThreadLocalStorage::GetValue( void )
{
    return( radThread::GetLocalStorage( m_Index ) );   
}

//=============================================================================
// Function:    radThreadLocalStorage::SetValue
//=============================================================================
// Description: This is used to set the value of this object. Each thread has
//              a unique value for this object.
//
// Parameters:  value
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThreadLocalStorage::SetValue
( 
    void* value
)
{

    radThread::SetLocalStorage( m_Index, value );   
}

//=============================================================================
// Function:    radThreadLocalStorage::AddRef
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThreadLocalStorage::AddRef
(
	void
)
{
    //
    // Protect this operation with mutex as this is not guarenteed to be thread
    // safe.
    //
    radThreadInternalLock( );
	m_ReferenceCount++;
    radThreadInternalUnlock( );
}

//=============================================================================
// Function:    radThreadLocalStorage::Release
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThreadLocalStorage::Release
(
	void
)
{
    //
    // Protect this operation with mutex as this is not guarenteed to be thread
    // safe.
    //
    radThreadInternalLock( );

	m_ReferenceCount--;

	if ( m_ReferenceCount == 0 )
	{
        radThreadInternalUnlock( );
		delete this;
	}
    else
    {
        radThreadInternalUnlock( );
    }
}

//=============================================================================
// Function:    radThreadLocalStorage::Dump
//=============================================================================
// Description: This member is used to display object info
//
// Parameters:  string buffer and size of buffer
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

#ifdef RAD_DEBUG

void radThreadLocalStorage::Dump( char* pStringBuffer, unsigned int bufferSize )
{
    sprintf( pStringBuffer, "Object: [radThreadLocalStorage] At Memory Location:[%p]\n", this );
}

#endif

#if 0
//=============================================================================
// Function:    radThreadFiber::radThreadFiber
//=============================================================================
// Description: Default constructor is invoked when a thread is created. Each
//              thread is a fiber by default.
//
// Parameters:  N/A
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThreadFiber::radThreadFiber( void )
    :
    m_ReferenceCount( 1 ),
    m_StackSize( 0 )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThreadFiber" );
}

//=============================================================================
// Function:    radThreadFiber::radThreadFiber
//=============================================================================
// Description: Constructor for new fiber objects created by a client.
//
// Parameters:  N/A
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThreadFiber::radThreadFiber
( 
    RADFIBERENTRY   pEntryFunction,
    void*           userData,
    unsigned int    stackSize
)
    :
    m_ReferenceCount( 1 ),
    m_pEntryFunction( pEntryFunction ),
    m_StackSize( stackSize ),
    m_Value( userData )
{
    radMemoryMonitorIdentifyAllocation( this, g_nameFTech, "radThreadFiber" );
    rAssert( stackSize != 0 );   

#if defined(RAD_WIN32) || defined(RAD_XBOX)

    m_Win32Fiber = CreateFiber( stackSize, FiberEntry, this );

#endif

#ifdef RAD_PS2

    //
    // One the ps2ee, allocate a stack and set up the entry 
    // point for this fiber.
    //
    m_Stack = radMemoryAllocAligned( GetThisAllocator( ), stackSize + 16, 16 );

    m_CurrentStackPointer = (unsigned int) ((char*) m_Stack + stackSize );
    m_CurrentProgramCounter = (unsigned int) FiberEntry;

#endif

#ifdef RAD_GAMECUBE

    //
    // One the gamecube, allocate a stack and set up the entry 
    // point for this fiber.
    //
    m_Stack = radMemoryAllocAligned( GetThisAllocator( ), stackSize + 16, 16 );

    m_CurrentStackPointer = (unsigned int) ((char*) m_Stack + stackSize );
    m_CurrentProgramCounter = (unsigned int) FiberEntry;

#endif

}

//=============================================================================
// Function:    radThreadFiber::~radThreadFiber
//=============================================================================
// Description: Invoke to destroy a fiber. Cannot be the active fiber.
//
// Parameters:  none
//              
// Returns:     N/A
//
//------------------------------------------------------------------------------

radThreadFiber::~radThreadFiber( void )
{
    rAssertMsg( radThreadGetActiveFiber( ) != this, "Cannot destroy active fiber" );
    
    //
    // If the stack size is zero, it indicates this is the object contained
    // be the parent thread. No need to cleau up.
    //
    if( m_StackSize != 0 )
    {
#if defined(RAD_WIN32) || defined(RAD_XBOX)
        //
        // Under windows, delete the fiber.
        //
        DeleteFiber( m_Win32Fiber );
#endif    

#ifdef RAD_PS2
        radMemoryFreeAligned( GetThisAllocator( ), m_Stack );
#endif

#ifdef RAD_GAMECUBE
        radMemoryFreeAligned( GetThisAllocator( ), m_Stack );
#endif

    }
}

//=============================================================================
// Function:    radThreadFiber::SwitchTo
//=============================================================================
// Description: Member function to switch this fiber.
//
// Parameters:  
//
// Returns:     n/a
//
// Notes:       It is illegal to switch to the actively running fiber. 
//------------------------------------------------------------------------------

void radThreadFiber::SwitchTo( void )
{
    //
    // Verify not the actively running fiber.
    //
    rAssertMsg( radThreadGetActiveFiber( ) != this, "Cannot switch to active fiber" );

    //
    // Perform OS specific switch.
    //
#if defined(RAD_WIN32) || defined(RAD_XBOX)
    
    // 
    // Set this fiber as the calling threads active fiber and switch,
    //
    ((radThread*) radThread::GetActiveThread( ))->m_pActiveFiber = this;

    SwitchToFiber( m_Win32Fiber );        

#endif

#ifdef RAD_PS2

    //
    // Get the active fiber. Sets its program counter and stack to where we 
    // are. Then switch to new one.
    //
    radThreadFiber* oldFiber = (radThreadFiber*) radThreadGetActiveFiber( );
    ((radThread*) radThread::GetActiveThread( ))->m_pActiveFiber = this;

    PS2SwitchToFiber( &(oldFiber->m_CurrentStackPointer), &(oldFiber->m_CurrentProgramCounter),
                      m_CurrentStackPointer, m_CurrentProgramCounter );

#endif


#ifdef RAD_GAMECUBE

    //
    // Get the active fiber. Sets its program counter and stack to where we 
    // are. Then switch to need one.
    //
    radThreadFiber* oldFiber = (radThreadFiber*) radThreadGetActiveFiber( );
    ((radThread*) radThread::GetActiveThread( ))->m_pActiveFiber = this;

    GCNSwitchToFiber( &(oldFiber->m_CurrentStackPointer), &(oldFiber->m_CurrentProgramCounter),
                      m_CurrentStackPointer, m_CurrentProgramCounter );
   
#endif

}

//=============================================================================
// Function:    radThreadFiber::GetValue
//=============================================================================
// Description: Just a helper function whereby client can associate a value
//              with the fiber.
//
// Parameters:  n/a
//
// Returns:     value
//
// Notes:
//------------------------------------------------------------------------------

void* radThreadFiber::GetValue( void )
{
    return( m_Value );   
}

//=============================================================================
// Function:    radThreadFiber::SetValue
//=============================================================================
// Description: Just a helper function whereby client can associate a value
//              with the fiber.
//
// Parameters:  value
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThreadFiber::SetValue
( 
    void* value
)
{
    m_Value = value;
}

//=============================================================================
// Function:    radThreadFiber::AddRef
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:       Fiber objects should not be accessed by more than one thread,
//              hence no need for protection.
//------------------------------------------------------------------------------

void radThreadFiber::AddRef
(
	void
)
{
	m_ReferenceCount++;
}

//=============================================================================
// Function:    radThreadFiber::Release
//=============================================================================
// Description: Reference Management.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

void radThreadFiber::Release
(
	void
)
{
	m_ReferenceCount--;

	if ( m_ReferenceCount == 0 )
	{
		delete this;
	}
}

//=============================================================================
// Function:    radThreadFiber::Dump
//=============================================================================
// Description: This member is used to display object info
//
// Parameters:  string buffer and size of buffer
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

#ifdef RAD_DEBUG

void radThreadFiber::Dump( char* pStringBuffer, unsigned int bufferSize )
{
    sprintf( pStringBuffer, "Object: [radThreadFiber] At Memory Location:[%p]\n", this );
}

#endif

//=============================================================================
// Function:    radThreadFiber::FiberEntry
//=============================================================================
// Description: This member is our internal entry point. We use it to 
//              normalize fiber entry
//
// Parameters:  
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

#if defined(RAD_WIN32) || defined(RAD_XBOX)
void CALLBACK radThreadFiber::FiberEntry( void* param )
{
    (void) param;
#endif
#ifdef RAD_GAMECUBE
void radThreadFiber::FiberEntry( void )
{
#endif
#ifdef RAD_PS2
void radThreadFiber::FiberEntry( void )
{
#endif
    //
    // Get the active thread and invoke callers function with user data.
    //
    radThreadFiber* pFiber = (radThreadFiber*) radThreadGetActiveFiber( );

    (pFiber->m_pEntryFunction)( pFiber->m_Value );

    //
    // If we ever get here things are screwed up.
    //
    rAssertMsg( false, "Fibers are not allowed to return\n");

}

//=============================================================================
// Function:    radThreadFiber::PS2SwitchToFiber
//=============================================================================
// Description: This static is used on the PS2 to switch fibers.
//
// Parameters:  Where to copy old SP and old PC
//              new values for SP and PC
//
// Returns:     n/a
//
// Notes:
//------------------------------------------------------------------------------

#ifdef RAD_PS2

#ifndef RAD_MW

void radThreadFiber::PS2SwitchToFiber( unsigned int* oldSp, unsigned int* oldPc, unsigned int newSp, unsigned int newPc )
{
    asm( "addiu $29, 0xfff8" );         // Make room on stack to store frame pointer and return address
    asm( "sw $31, 0($29)" );            // Save return address on stack
    asm( "sw $30, 4($29)" );            // Save frame pointer on stack
    asm( "sw $29, 0($4 )" );            // Save the current stack pointer to *oldSp 
    asm( "la $8, returnpoint" );        // Load return address in temp register
    asm( "sw $8, 0($5)" );              // Save temp register into *oldPc
    asm( "add $29,$6,$0" );             // Set the stack pointer to newSP
    asm( "jr $7 " );                    // Set PC to newCP

    asm( "returnpoint: nop" );          // Were we return to
    asm( "lw $30,4($29)" );             // Restore frame pointer
    asm( "lw $31,0($29)" );             // Restore return address
    asm( "addiu $29, 0x8" );            // Balance stack
}

#else

//
// Metrowerks verions is a little different    
//
asm void radThreadFiber::PS2SwitchToFiber( unsigned int* oldSp, unsigned int* oldPc, unsigned int newSp, unsigned int newPc )
{
    addiu $29, -8           // Make room on stack to store frame pointer and return address
    sw $31, 0($29)              // Save return address on stack
    sw $30, 4($29)              // Save frame pointer on stack
    sw $29, 0($4 )              // Save the current stack pointer to *oldSp 
    
    jal next                    // load retrun address to point to next
    nop
next:
    add $8,$31, 16              // Temp now has the address of return point 
    sw $8, 0($5)                // Save temp register into *oldPc
    add $29,$6,$0               // Set the stack pointer to newSP
    jr $7                       // Set PC to newCP

    returnpoint: nop            // Were we return to
    lw $30,4($29)               // Restore frame pointer
    lw $31,0($29)               // Restore return address
    addiu $29, 0x8              // Balance stack
    jr  $31
    nop
}

#endif

#endif


//=============================================================================
// Function:    radThreadFiber::GCNSwitchToFiber
//=============================================================================
// Description: This static is used on the GameCube to switch fibers.
//
// Parameters:  Where to copy old SP and old PC
//              new values for SP and PC
//
// Returns:     n/a
//
// Notes:       Static function, no this pointer
//------------------------------------------------------------------------------

#ifdef RAD_GAMECUBE

asm void radThreadFiber::GCNSwitchToFiber( unsigned int* oldSp, unsigned int* oldPc, unsigned int newSp, unsigned int newPc )
{
    nofralloc

    mflr    r7          // save link reg ( return address ) in temp reg r7
    stwu    r1,-8(r1)   // make space on stack and save sp
    stw     r7,4(r1)    // store link register
    stw     r1,0(r3)    // save sp in oldSp
    bl      next        // load the link register with the address of next.    
next:
    mflr    r7          // Save link register in temporary register
    addi    r7,r7,6 * 4 // Add 6 instruction to the temp register pointing us to the return point
    stw     r7, 0(r4)   // save this as oldPc
    mr      r1, r5      // load newSp into stack pointer
    mtlr    r6          // load newPc into link register
    blr                 // branch to link register
returnpoint:
    lwz     r7, 4(r1)   // get return address    
    mtlr    r7          // move into link register
    addi    r1, r1, 8   // balance stack
    blr                 // return
}

#endif

#endif // if 0

















