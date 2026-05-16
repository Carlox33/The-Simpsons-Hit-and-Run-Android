//===========================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   Win32Platform   
//
// Description: Abstracts the differences for setting up and shutting down
//              the different platforms.
//
// History:     + Stolen and cleaned up from Penthouse -- Darwin Chau
//
//===========================================================================

//========================================
// System Includes
//========================================
#include <SDL.h>
// Standard Lib
#include <stdlib.h>
#include <string.h>
// Pure 3D
#include <p3d/anim/compositedrawable.hpp>
#include <p3d/anim/expression.hpp>
#include <p3d/anim/multicontroller.hpp>
#include <p3d/anim/polyskin.hpp>
#include <p3d/anim/sequencer.hpp>
#include <p3d/anim/skeleton.hpp>
#include <p3d/camera.hpp>
#include <p3d/gameattr.hpp>
#include <p3d/image.hpp>
#include <p3d/imagefont.hpp>
#include <p3d/light.hpp>
#include <p3d/locator.hpp>
#include <p3d/platform.hpp>
#include <p3d/scenegraph/scenegraph.hpp>
#include <p3d/sprite.hpp>
#include <p3d/utility.hpp>
#include <p3d/texture.hpp>
#include <p3d/file.hpp>
#include <p3d/shader.hpp>
#include <p3d/matrixstack.hpp>
#include <p3d/memory.hpp>
#include <p3d/bmp.hpp>
#include <p3d/png.hpp>
#include <p3d/targa.hpp>
#include <p3d/font.hpp>
#include <p3d/texturefont.hpp>
#include <p3d/unicode.hpp>
// Pure 3D: Loader-specific
#include <render/Loaders/GeometryWrappedLoader.h>
#include <render/Loaders/StaticEntityLoader.h>
#include <render/Loaders/StaticPhysLoader.h>
#include <render/Loaders/TreeDSGLoader.h>
#include <render/Loaders/FenceLoader.h>
#include <render/Loaders/IntersectLoader.h>
#include <render/Loaders/AnimCollLoader.h>
#include <render/Loaders/AnimDSGLoader.h>
#include <render/Loaders/DynaPhysLoader.h>
#include <render/Loaders/InstStatPhysLoader.h>
#include <render/Loaders/InstStatEntityLoader.h>
#include <render/Loaders/WorldSphereLoader.h>
#include <loading/roaddatasegmentloader.h>
#include <render/Loaders/BillboardWrappedLoader.h>
#include <render/Loaders/instparticlesystemloader.h>
#include <render/Loaders/breakableobjectloader.h>
#include <render/Loaders/AnimDynaPhysLoader.h>
#include <render/Loaders/LensFlareLoader.h>
#include <p3d/shadow.hpp>
#include <p3d/anim/animatedobject.hpp>
#include <p3d/effects/particleloader.hpp>
#include <p3d/effects/opticloader.hpp>
#include <p3d/anim/vertexanimkey.hpp>
#include <stateprop/statepropdata.hpp>

// Foundation Tech
#include <raddebug.hpp>
#include <radthread.hpp>
#include <radplatform.hpp>
#include <radtime.hpp>
#include <radmemorymonitor.hpp>
#include <raddebugcommunication.hpp>
#include <raddebugwatch.hpp>
#include <radfile.hpp>
#include <radmovie2.hpp>

//This is so we can get the name of the file that's failing.
#include <../src/radfile/common/requests.hpp>

// sim - for InstallSimLoaders
#include <simcommon/simutility.hpp>

//========================================
// Project Includes
//========================================
#include <input/inputmanager.h>
#include <main/win32platform.h>
#include <main/commandlineoptions.h>
#include <main/game.h>
#include <render/RenderManager/RenderManager.h>
#include <render/RenderFlow/renderflow.h>
#include <render/Loaders/AllWrappers.h>
#include <memory/srrmemory.h>

#include <loading/locatorloader.h>
#include <loading/cameradataloader.h>
#include <loading/roadloader.h>
#include <loading/pathloader.h>
#include <loading/intersectionloader.h>
#include <loading/roaddatasegmentloader.h>
#include <atc/atcloader.h>
#include <data/gamedatamanager.h>
#ifdef RAD_PC
#include <data/config/gameconfigmanager.h>
#endif
#include <debug/debuginfo.h>
#include <constants/srrchunks.h>
#include <gameflow/gameflow.h>
#include <sound/soundmanager.h>
#include <presentation/presentation.h>
#include <presentation/gui/guitextbible.h>
#include <cheats/cheatinputsystem.h>
#include <mission/gameplaymanager.h>

#ifdef __SWITCH__
#include <switch.h>
#endif


#ifdef RAD_ANDROID
#include <android/log.h>
#endif


#include <radload/radload.hpp>

#include <main/errorsWIN32.h>

#define _stricmp SDL_strcasecmp
#define WIN32_SECTION "WIN32_SECTION"
#define TIMER_LEAVE 1


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


//#define PRINT_WINMESSAGES

//******************************************************************************
//
// Global Data, Local Data, Local Classes
//
//******************************************************************************

// Static pointer to instance of singleton.
Win32Platform* Win32Platform::spInstance = NULL;

// Other static members.
SDL_Window* Win32Platform::mWnd = NULL;
#ifdef WIN32
#include <Windows.h>
void* Win32Platform::mhMutex = NULL;
#endif
bool Win32Platform::mShowCursor = true;

//The Adlib font.  <sigh>
unsigned char gFont[] = 
#include <font/defaultfont.h>

//
// Define the starting resolution.
//
static const Win32Platform::Resolution StartingResolution = Win32Platform::Res_800x600;
static const int StartingBPP = 32;

// This specifies the PDDI DLL to use.
#ifdef RAD_DEBUG
static const char pddiLibraryName[] = "pddi%sd.dll";
#endif
#ifdef RAD_TUNE
static const char pddiLibraryName[] = "pddi%st.dll";
#endif
#ifdef RAD_RELEASE
static const char pddiLibraryName[] = "pddi%sr.dll";
#endif

// Name of the application.  This is the string that appears in the Window's
// title bar.
static const char ApplicationName[] = "The Simpsons: Hit & Run";

// The gamma of the desktop.. needed to reset it on alt-tabs.
static Uint16 DesktopGammaRamp[ 3 ][ 256 ] = { 0 };

void LoadMemP3DFile( unsigned char* buffer, unsigned int size, tEntityStore* store )
{
    tFileMem* file = new tFileMem(buffer,size);
    file->AddRef();
    file->SetFilename("memfile.p3d");
    p3d::loadManager->GetP3DHandler()->Load( file, p3d::inventory );
    file->Release();
}

//******************************************************************************
//
// Public Member Functions
//
//******************************************************************************

//==============================================================================
// Win32Platform::CreateInstance
//==============================================================================
//
// Description: Creates the Win32Platform.
//
// Parameters:	win32 parameters.
//
// Return:      Pointer to the Win32Platform.
//
// Constraints: This is a singleton so only one instance is allowed.
//
//==============================================================================
Win32Platform* Win32Platform::CreateInstance()
{
MEMTRACK_PUSH_GROUP( "Win32Platform" );
    rAssert( spInstance == NULL );

    spInstance = new(GMA_PERSISTENT) Win32Platform();
    rAssert( spInstance );
MEMTRACK_POP_GROUP( "Win32Platform" );

    return spInstance;
}

//==============================================================================
// Win32Platform::GetInstance
//==============================================================================
//
// Description: - Access point for the Win32Platform singleton.  
//
// Parameters:	None.
//
// Return:      Pointer to the Win32Platform.
//
// Constraints: This is a singleton so only one instance is allowed.
//
//==============================================================================
Win32Platform* Win32Platform::GetInstance()
{
    rAssert( spInstance != NULL );

    return spInstance;
}


//==============================================================================
// Win32Platform::DestroyInstance
//==============================================================================
//
// Description: Destroy the Win32Platform.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void Win32Platform::DestroyInstance()
{
    rAssert( spInstance != NULL );

    delete( GMA_PERSISTENT, spInstance );
    spInstance = NULL;
}



//==============================================================================
// Win32Platform::InitializeWindow
//==============================================================================
// Description: Creates the window class and window instance for the application.
//              We must do this before initializing the platform.
//
// Parameters:	hInstance - the handle to the instance.
//
// Return:      true if successful and the program can run.
//              false if another simpsons window already exists and this
//                instance should terminate.
//
// Constraints: Must be initialized before the platform.
//
//==============================================================================
bool Win32Platform::InitializeWindow()
{
#ifdef WIN32
    // check to see if another instance is running...
    mhMutex = CreateMutex(NULL, 0, ApplicationName);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // simpsons is already running, so lets find the window and give it focus
        HWND hwnd = FindWindow(ApplicationName, NULL);
        if (hwnd != NULL)
        {
            // if window is minimized, restore it
            WINDOWPLACEMENT wndpl;
            if (GetWindowPlacement(hwnd, &wndpl) != 0)
            {
                if ((wndpl.showCmd == SW_MINIMIZE) ||
                    (wndpl.showCmd == SW_SHOWMINIMIZED))
                {
                    ShowWindow(hwnd, SW_RESTORE);
                }
            }

            // activate the window
            SetForegroundWindow(hwnd);

            return false;
        }
    }
#endif

    // These attributes must be set prior to creating the first window

#if RAD_GLES

    // ------------------------------------------------------------
    // Android: Prefer GLES3, fallback to GLES2 (runtime)
    // ------------------------------------------------------------
#ifdef RAD_ANDROID

    // Attempt #1 : GLES3
#if SDL_MAJOR_VERSION < 3
    SDL_GL_ResetAttributes();
#endif
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

#else // !RAD_ANDROID

    // ------------------------------------------------------------
    // Existing behavior for non-Android GLES builds
    // ------------------------------------------------------------
#if RAD_CG
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
#else
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, RAD_GLES_VERSION );
#endif
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

#endif // RAD_ANDROID

#else  // !RAD_GLES

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

#endif // RAD_GLES

#ifdef __SWITCH__
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
#else
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
#endif
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );

    int flags = 0;
#ifndef RAD_VITA
    flags |= SDL_WINDOW_OPENGL;
#endif
#ifdef __SWITCH__
    // Support switching between docked and handheld mode
    flags |= SDL_WINDOW_RESIZABLE;
#endif
//#ifdef RAD_ANDROID
    // Optional: on Android you typically want fullscreen from the start.
    // If you want to keep fullscreen handling only in InitializePlatform(),
    // you can remove this line.
    //flags |= SDL_WINDOW_FULLSCREEN; // lo comento por ahora julioh 
//#endif

    int w, h;
    TranslateResolution( StartingResolution, w, h );

#if SDL_MAJOR_VERSION < 3
    mWnd = SDL_CreateWindow( ApplicationName,
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             w, h, flags );
#else
    mWnd = SDL_CreateWindow( ApplicationName, w, h, flags );
#endif

#ifdef RAD_ANDROID
    // If GLES3 window/context setup fails, fallback to GLES2 and retry once.
    if( mWnd == NULL )
    {
        rDebugPrintf( "SDL_CreateWindow GLES3 failed: %s\n", SDL_GetError() );

        // Attempt #2 : GLES2
#if SDL_MAJOR_VERSION < 3
        SDL_GL_ResetAttributes();
#endif
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );

#ifdef __SWITCH__
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
#else
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
#endif
        SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );

#if SDL_MAJOR_VERSION < 3
        mWnd = SDL_CreateWindow( ApplicationName,
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 w, h, flags );
#else
        mWnd = SDL_CreateWindow( ApplicationName, w, h, flags );
#endif
    }
#endif // RAD_ANDROID

    rAssertMsg( mWnd != NULL, SDL_GetError() );

#if defined( PRINT_WINMESSAGES ) && defined( RAD_DEBUG )
    SDL_SetHint( SDL_HINT_EVENT_LOGGING, "1" );
#endif
    SDL_SetEventFilter( WndProc, mWnd );

    ShowTheCursor( false );

#if SDL_MAJOR_VERSION < 3
    SDL_GetWindowGammaRamp( mWnd, DesktopGammaRamp[0], DesktopGammaRamp[1], DesktopGammaRamp[2] );
#endif

    SDL_DisableScreenSaver();

    return true;
}


//==============================================================================
// Win32Platform::InitializeFoundation
//==============================================================================
// Description: FTech must be setup first so that all the memory services
//              are ready to go before we begin allocating anything.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be initialized in a particular order.
//              Consult their documentation before changing.
//
//==============================================================================
void Win32Platform::InitializeFoundation() 
{
    //
    // Initialize the memory heaps
    // obsolete now.. the heaps initialize memory.
    //
    //InitializeMemory();
#ifdef RAD_ANDROID
  InitializeMemory();
#endif


    //
    // Register an out-of-memory display handler in case something goes bad
    // while allocating the heaps
    //
    ::radMemorySetOutOfMemoryCallback( PrintOutOfMemoryMessage, NULL );

    //
    // Initialize memory monitor by JamesCo. TM.
    //
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        const int KB = 1024;
        ::radMemoryMonitorInitialize( 64 * KB, GMA_DEBUG );
    }

    // Setup the memory heaps
    //
    HeapMgr()->PrepareHeapsStartup ();

    // Seed the heap stack
    //
    HeapMgr()->PushHeap (GMA_PERSISTENT);

    //
    // Initilalize the platform system
    // 
    ::radPlatformInitialize( mWnd );

    //
    // Initialize the timer system
    //
    ::radTimeInitialize();

    //
    // Initialize the debug communication system.
    //
    ::radDbgComTargetInitialize( WinSocket, 
        radDbgComDefaultPort, // Default
        NULL,                 // Default
        GMA_DEBUG );


    //
    // Initialize the Watcher.
    //
    ::radDbgWatchInitialize( "SRR2",
                             32 * 16384, // 2 * Default
                             GMA_DEBUG );

    //
    // Initialize the file system.
    //
    ::radFileInitialize( 50, // Default
        32, // Default
        GMA_PERSISTENT );

    ::radLoadInitialize();
    //radLoad->SetSyncLoading( true );	

    ::radDriveMount( NULL, GMA_PERSISTENT);

    //
    // Initialize the new movie player
    //
    ::radMovieInitialize2( GMA_PERSISTENT );

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}

//==============================================================================
// Win32Platform::InitializeMemory
//==============================================================================
//
// Description: 
//
// Parameters:  
//
// Return:      
//
//==============================================================================
void Win32Platform::InitializeMemory()
{
    //
    // Only do this once!
    //
    if( gMemorySystemInitialized == true )
    {
        return;
    }

    gMemorySystemInitialized = true;

    //
    // Initialize the thread system.
    //
    ::radThreadInitialize();

    //
    // Initialize the memory system.
    //
    ::radMemoryInitialize();
}

//==============================================================================
// Win32Platform::ShutdownMemory
//==============================================================================
//
// Description: 
//
// Parameters:  
//
// Return:      
//
//==============================================================================
void Win32Platform::ShutdownMemory()
{
    if( gMemorySystemInitialized )
    {
        gMemorySystemInitialized = false;

        // No shutdown the memory.  This leads to bad errors when destroying 
        // static variables sprinkled here and there.
        //::radMemoryTerminate();
        
        ::radThreadTerminate();
    }
}

//==============================================================================
// Win32Platform::InitializePlatform
//==============================================================================
// Description: Get the Win32 ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void Win32Platform::InitializePlatform() 
{
    HeapMgr()->PushHeap (GMA_PERSISTENT);

#ifdef RAD_PC
    //
    // Register with the game config manager
    //
    GetGameConfigManager()->RegisterConfig(this);
    GetGameConfigManager()->LoadConfigFile();
#endif

    //
    // Rendering is good.
    //
    InitializePure3D();

    //
    // Add anything here that needs to be before the drive is opened.
    //
    DisplaySplashScreen( Error ); // blank screen
	
#ifdef RAD_ANDROID
	mFullscreen = true; // ESTO ES IMPORTANTE TENERLO EN ANDROID DE OTRA FORMA LAS BARRAS DEL MENU DEL MOVIL COMO LA HORA Y EL VOLVER HACIA ATRAS VIRTUAL TAPAN PARTES DE LA PANTALLA
     SDL_SetWindowFullscreen( mWnd, mFullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
#endif

#ifndef __SWITCH__
    //
    // Show in fullscreen if fullscreen flag is set.
    //
    SDL_SetWindowFullscreen( mWnd, mFullscreen ? SDL_WINDOW_FULLSCREEN : 0 );
#endif

    //
    // Opening the drive is SLOW...
    //
    InitializeFoundationDrive();    

    //
    // Initialize the controller.
    //
    GetInputManager()->Init();

    HeapMgr()->PopHeap (GMA_PERSISTENT);
}


//==============================================================================
// Win32Platform::ShutdownPlatform
//==============================================================================
// Description: Shut down the PS2.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void Win32Platform::ShutdownPlatform()
{
    ShutdownPure3D();
    ShutdownFoundation();
}

//=============================================================================
// Win32Platform::LaunchDashboard
//=============================================================================
// Description: We use this a the emergency exit from the game if we arent in a context that suppose the transition 
//                    to the CONTEXT_EXIT  
// Parameters:  ()
//
// Return:      void 
//
//=============================================================================
void Win32Platform::LaunchDashboard()
{   

    {
        //chuck I copied and pasted from the other platform's implementations

        GetLoadingManager()->CancelPendingRequests();
           //TODO: Make sure sounds shut down too.
        GetSoundManager()->SetMasterVolume( 0.0f );

       // DisplaySplashScreen( FadeToBlack );

        GetPresentationManager()->StopAll();

        //Shouldn't need to do this since, this singleton and the others should get destroyed once we 
        //retrun the main loop
        //GameDataManager::DestroyInstance();  //Get rid of memcards

        p3d::loadManager->CancelAll();

        GetSoundManager()->StopForMovie();

        //Shouldnt need the early destruction of this singleton either
        //SoundManager::DestroyInstance();
        
        //Dont want to shutdown platform early either.
        //ShutdownPlatform();
        //rAssertMsg( false, "Doesn't make sense for win32." );
    }
}

//=============================================================================
// Win32Platform::ResetMachine
//=============================================================================
// Description: Comment
//
// Parameters:  ()
//
// Return:      void 
//
//=============================================================================
void Win32Platform::ResetMachine()
{
    rAssertMsg( false, "Doesn't make sense for win32." );
}

//=============================================================================
// Win32Platform::DisplaySplashScreen
//=============================================================================
// Description: Comment
//
// Parameters:  ( SplashScreen screenID, 
//                const char* overlayText = NULL, 
//                float fontScale = 1.0f, 
//                float textPosX = 0.0f, 
//                float textPosY = 0.0f,
//                tColour textColour,
//                int fadeFrames = 3 )
//
// Return:      void 
//
//=============================================================================
void Win32Platform::DisplaySplashScreen( SplashScreen screenID, 
                                       const char* overlayText, 
                                       float fontScale, 
                                       float textPosX, 
                                       float textPosY,
                                       tColour textColour,
                                       int fadeFrames )
{
    HeapMgr()->PushHeap( GMA_TEMP );

    p3d::inventory->PushSection();
    p3d::inventory->AddSection( WIN32_SECTION );
    p3d::inventory->SelectSection( WIN32_SECTION );

    P3D_UNICODE unicodeText[256];

    // Save the current Projection mode so I can restore it later
    pddiProjectionMode pm = p3d::pddi->GetProjectionMode();
    p3d::pddi->SetProjectionMode(PDDI_PROJECTION_DEVICE);

    pddiCullMode cm = p3d::pddi->GetCullMode();
    p3d::pddi->SetCullMode(PDDI_CULL_NONE);


    //CREATE THE FONT
    tTextureFont* thisFont = NULL;

    // Convert memory buffer into a texturefont.
    //
    //p3d::load(gFont, DEFAULTFONT_SIZE, GMA_TEMP);
    LoadMemP3DFile( gFont, DEFAULTFONT_SIZE, p3d::inventory );

    thisFont = p3d::find<tTextureFont>("adlibn_20");
    rAssert( thisFont );

    thisFont->AddRef();
    tShader* fontShader = thisFont->GetShader();
    //fontShader->SetInt( )


    p3d::AsciiToUnicode( overlayText, unicodeText, 256 );

    // Make the missing letter into somthing I can see
    //
    thisFont->SetMissingLetter(p3d::ConvertCharToUnicode('j'));

    int a = 0;

    do
    {
        p3d::pddi->SetColourWrite(true, true, true, true);
        p3d::pddi->SetClearColour( pddiColour(0,0,0) );
        p3d::pddi->BeginFrame();
        p3d::pddi->Clear(PDDI_BUFFER_COLOUR);

        //This is for fading in the font and shaders.
        int bright = 255;
        if (a < fadeFrames) bright = (a * 255) / fadeFrames;
        if ( bright > 255 ) bright = 255;
        tColour c(bright, bright, bright, 255);

        //Display font
        if (overlayText != NULL)
        {
            tColour colour = textColour;
            colour.SetAlpha( bright );

            thisFont->SetColour( colour );

            p3d::pddi->SetProjectionMode(PDDI_PROJECTION_ORTHOGRAPHIC);
            p3d::stack->Push();
            p3d::stack->LoadIdentity();

            p3d::stack->Translate( textPosX, textPosY, 1.0f);
            float scaleSize = 1.0f / 480.0f;  //This is likely good for 528 also.
            p3d::stack->Scale(scaleSize * fontScale, scaleSize* fontScale , 1.0f);

            if ( textPosX != 0.0f || textPosY != 0.0f )
            {
                thisFont->DisplayText( unicodeText );
            }
            else
            {
                thisFont->DisplayText( unicodeText, 3 );
            }

            p3d::stack->Pop();
        }

        p3d::pddi->EndFrame();
        p3d::context->SwapBuffers();

        ++a;

    } while (a <= fadeFrames + 1);

    p3d::pddi->SetCullMode(cm);
    p3d::pddi->SetProjectionMode(pm);

    //Should do this after a vsync.
    thisFont->Release();

    p3d::inventory->RemoveSectionElements(WIN32_SECTION);
    p3d::inventory->DeleteSection(WIN32_SECTION);
    p3d::inventory->PopSection();

    HeapMgr()->PopHeap( GMA_TEMP );
}


//=============================================================================
// Win32Platform::DisplaySplashScreen
//=============================================================================
// Description: Comment
//
// Parameters:  ( const char* textureName, 
//                const char* overlayText = NULL, 
//                float fontScale = 1.0f, 
//                float textPosX = 0.0f, 
//                float textPosY = 0.0f, 
//                tColour textColour,
//                int fadeFrames = 3 )
//
// Return:      void 
//
//=============================================================================
void Win32Platform::DisplaySplashScreen( const char* textureName,
                                       const char* overlayText, 
                                       float fontScale, 
                                       float textPosX, 
                                       float textPosY, 
                                       tColour textColour,
                                       int fadeFrames )
{
}

// FUNCION ORIGINAL

void Win32Platform::OnControllerError(const char *msg)
{
    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    mErrorState = CTL_ERROR;
    mPauseForError = true;

}


// NUEVA FUNCION MISMO COMPORTAMIENTO CON LOGI 
/*
void Win32Platform::OnControllerError(const char *msg)
{
    LOGI("[Platform]: OnControllerError ENTER msg=%s", msg ? msg : "(null)");

    LOGI("[Platform]: Before DisplaySplashScreen");
    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    LOGI("[Platform]: After DisplaySplashScreen");

    LOGI("[Platform]: Before mErrorState = CTL_ERROR");
    mErrorState = CTL_ERROR;
    LOGI("[Platform]: After mErrorState = CTL_ERROR");

    LOGI("[Platform]: Before mPauseForError = true");
    mPauseForError = true;
    LOGI("[Platform]: After mPauseForError = true");
}
*/
/*


// PRUEBA A 
void Win32Platform::OnControllerError(const char *msg)
{
    LOGI("[Platform][TEST A]: ENTER");
    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    LOGI("[Platform][TEST A]: EXIT");
}
*/

/*
// PRUEBA B
void Win32Platform::OnControllerError(const char *msg)
{
    LOGI("[Platform][TEST B]: ENTER");
    mErrorState = CTL_ERROR;
    mPauseForError = true;
    LOGI("[Platform][TEST B]: EXIT");
}
*/
/*
void Win32Platform::OnControllerError(const char *msg)
{
    LOGI("[Platform][TEST ORDER]: ENTER");

    DisplaySplashScreen( Error, msg, 0.7f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
    LOGI("[Platform][TEST ORDER]: After DisplaySplashScreen");

   

    LOGI("[Platform][TEST ORDER]: Before mErrorState / mPauseForError");
    mErrorState = CTL_ERROR;
    mPauseForError = true;
    LOGI("[Platform][TEST ORDER]: After mErrorState / mPauseForError");
}
*/
//=============================================================================
// Win32Platform::OnDriveError
//=============================================================================
// Description: Comment
//
// Parameters:  ( radFileError error, const char* pDriveName, void* pUserData )
//
// Return:      bool 
//
//=============================================================================
bool Win32Platform::OnDriveError( radFileError error, const char* pDriveName, void* pUserData )
{
    // First check if the error is related to loading/saving games.
    // We do this here because windows has one drive for all operations.
    // If the game data manager is using the drive, it handles the error.
    GameDataManager* gm = GetGameDataManager();
    if( gm->IsUsingDrive() )
    {
        return gm->OnDriveError( error, pDriveName, pUserData );
    }

    switch ( error )
    {
    case Success:
        {
            if ( mErrorState != NONE )
            {
                DisplaySplashScreen( FadeToBlack );
                mErrorState = NONE;
                mPauseForError = false;
            }

            return true;
            break;
        }
    case FileNotFound:
        {
            rAssert( pUserData != NULL );

            radFileRequest* request = static_cast<radFileRequest*>( pUserData );
            const char* fileName = request->GetFilename();

            //Get rid of the slashes.
            unsigned int i;
            unsigned int lastIndex = 0;
            for ( i = 0; i < strlen( fileName ); ++i )
            {
                if ( fileName[ i ] == '\\' )
                {
                    lastIndex = i;
                }
            }

            unsigned int adjustedIndex = lastIndex == 0 ? lastIndex : lastIndex + 1;

            char adjustedName[32];
            strncpy( adjustedName, &fileName[adjustedIndex], ( strlen( fileName ) - lastIndex ) );
            adjustedName[ strlen( fileName ) - lastIndex ] = '\0';

#ifdef RAD_PC
            if( strcmp( fileName, GameConfigManager::ConfigFilename ) == 0 )
            {
                return false;
            }
#endif

            char errorString[256];
            sprintf( errorString, "%s:\n%s", ERROR_STRINGS[error], adjustedName );
            fprintf(stderr, "error: %s\n", errorString);
            DisplaySplashScreen( Error, errorString, 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
            mErrorState = P_ERROR;
            mPauseForError = true;

            return true;
        }
    case NoMedia:
    case MediaNotFormatted:
    case MediaCorrupt:
    case NoFreeSpace:
    case HardwareFailure:
        {
            //This could be the wrong disc.
            fprintf(stderr, "error: %s\n", ERROR_STRINGS[error]);
            DisplaySplashScreen( Error, ERROR_STRINGS[error], 1.0f, 0.0f, 0.0f, tColour(255, 255, 255), 0 );
            mErrorState = P_ERROR;
            mPauseForError = true;

            return true;
        }
    default:
        {
            //Others are not supported.
            rAssert( false );
        }
    }

    return false;
}

//=============================================================================
// Win32Platform::SetResolution
//=============================================================================
// Description: Sets the screen resolution
//
// Parameters:  res - desired resolution
//
// Returns:     true if successful
//              false if not supported
//
// Notes:
//=============================================================================

bool Win32Platform::SetResolution( Resolution res, int bpp, bool fullscreen )
{
    // Check if resolution is supported.
    if( !mpContext || !IsResolutionSupported( res, bpp ) )
    {
        return false;
    }

    // Set up the new properties
    mResolution = res;
    mbpp = bpp;
    mFullscreen = fullscreen;

    // Reinitialize the d3d context.
    InitializeContext();

    // Resize the window for the new resolution
    ResizeWindow();

    return true;
}

//=============================================================================
// Win32Platform::GetResolution
//=============================================================================
// Description: Returns the current resolution
//
// Parameters:  n/a
//
// Returns:     resolution
//
// Notes:
//=============================================================================

Win32Platform::Resolution Win32Platform::GetResolution() const
{
    return mResolution;
}

//=============================================================================
// Win32Platform::GetBPP
//=============================================================================
// Description: Returns the current bit depth.
//
// Parameters:  n/a
//
// Returns:     bit depth
//
// Notes:
//=============================================================================

int Win32Platform::GetBPP() const
{
    return mbpp;
}

//=============================================================================
// Win32Platform::IsFullscreen
//=============================================================================
// Description: Returns true if currently in full screen mode
//
// Parameters:  n/a
//
// Returns:     true if in full screen, false if in window
//
// Notes:
//=============================================================================

bool Win32Platform::IsFullscreen() const
{
    return mFullscreen;
}

//=============================================================================
// Win32Platform::GetConfigName
//=============================================================================
// Description: Returns the name of the win32 platform's config
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================
#ifdef RAD_PC
const char* Win32Platform::GetConfigName() const
{
    return "System";
}

//=============================================================================
// Win32Platform::GetNumProperties
//=============================================================================
// Description: Returns the number of config properties
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

int Win32Platform::GetNumProperties() const
{
    return 4;
}

//=============================================================================
// Win32Platform::LoadDefaults
//=============================================================================
// Description: Loads the default configuration for the system.
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================

void Win32Platform::LoadDefaults()
{
#ifdef RAD_PC

#ifdef RAD_DEBUG
    SetResolution( StartingResolution, StartingBPP, !CommandLineOptions::Get( CLO_WINDOW_MODE ) );
#else
    SetResolution( StartingResolution, StartingBPP, true );
#endif

#else
    SetResolution( StartingResolution, StartingBPP, false );
#endif
    

    GetRenderFlow()->SetGamma( 1.0f );
}

//=============================================================================
// Win32Platform::LoadConfig
//=============================================================================
// Description: Loads the platforms configuration
//
// Parameters:  n/a
//
// Returns:     
//
// Notes:
//=============================================================================
void Win32Platform::LoadConfig( ConfigString& config )
{
    char property[ ConfigString::MaxLength ];
    char value[ ConfigString::MaxLength ];

    while ( config.ReadProperty( property, value ) )
    {
        if( _stricmp( property, "display" ) == 0 )
        {
            if( _stricmp( value, "window" ) == 0 )
            {
                mFullscreen = false;
            }
            else if( _stricmp( value, "fullscreen" ) == 0 )
            {
                mFullscreen = true;
            }
        }
        else if( _stricmp( property, "resolution" ) == 0 )
        {
            if( strcmp( value, "640x480" ) == 0 )
            {
                mResolution = Res_640x480;
            }
            else if( strcmp( value, "800x600" ) == 0 )
            {
                mResolution = Res_800x600;
            }
            else if( strcmp( value, "1024x768" ) == 0 )
            {
                mResolution = Res_1024x768;
            }
            else if( strcmp( value, "1152x864" ) == 0 )
            {
                mResolution = Res_1152x864;
            }
            else if( strcmp( value, "1280x1024" ) == 0 )
            {
                mResolution = Res_1280x1024;
            }
            else if( strcmp( value, "1600x1200" ) == 0 )
            {
                mResolution = Res_1600x1200;
            }
        }
        else if( _stricmp( property, "bpp" ) == 0 )
        {
            if( strcmp( value, "16" ) == 0 )
            {
                mbpp = 16;
            }
            else if( strcmp( value, "32" ) == 0 )
            {
                mbpp = 32;
            }
        }
        else if( _stricmp( property, "gamma" ) == 0 )
        {
            float val = (float) atof( value );
            if( val > 0 )
            {
                GetRenderFlow()->SetGamma( val );
            }
        }
        else if (_stricmp(property, "renderer") == 0)
        {
            strncpy(mRenderer, value, ConfigString::MaxLength);
        }
    }

    // apply the new settings.
    SetResolution( mResolution, mbpp, mFullscreen );
}

//=============================================================================
// Win32Platform::SaveConfig
//=============================================================================
// Description: Saves the system configuration to the config string.
//
// Parameters:  config string to save to
//
// Returns:     
//
// Notes:
//=============================================================================

void Win32Platform::SaveConfig( ConfigString& config )
{
    config.WriteProperty( "display", mFullscreen ? "fullscreen" : "window" );

    const char* res = "800x600";
    switch( mResolution )
    {
        case Res_640x480:
        {
            res = "640x480";
            break;
        }
        case Res_800x600:
        {
            res = "800x600";
            break;
        }
        case Res_1024x768:
        {
            res = "1024x768";
            break;
        }
        case Res_1152x864:
        {
            res = "1152x864";
            break;
        }
        case Res_1280x1024:
        {
            res = "1280x1024";
            break;
        }
        case Res_1600x1200:
        {
            res = "1600x1200";
            break;
        }
        default:
        {
            rAssert( false );
        }
    }

    config.WriteProperty( "resolution", res );

    config.WriteProperty( "bpp", mbpp == 16 ? "16" : "32" );

    char gamma[20];
    sprintf( gamma, "%f", GetRenderFlow()->GetGamma() );
    config.WriteProperty( "gamma", gamma );

    config.WriteProperty("renderer", mRenderer);
}
#endif

//******************************************************************************
//
// Private Member Functions
//
//******************************************************************************

//==============================================================================
// Win32Platform::Win32Platform
//==============================================================================
// Description: Constructor.
//
// Parameters:	None.
//
// Return:      N/A.
//
//==============================================================================
Win32Platform::Win32Platform() :
    mpPlatform( NULL ),
    mpContext( NULL ),
    mResolution( StartingResolution ),
    mbpp( StartingBPP ),
    mRenderer( "dx8" )
{
    mFullscreen = false;
}


//==============================================================================
// Win32Platform::~Win32Platform
//==============================================================================
// Description: Destructor.
//
// Parameters:	None.
//
// Return:      N/A.
//
//==============================================================================
Win32Platform::~Win32Platform()
{
    HeapManager::DestroyInstance();
#ifdef WIN32
    CloseHandle( mhMutex );
#endif
}

//==============================================================================
// Win32Platform::InitializeFoundationDrive
//==============================================================================
// Description: Get FTech ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be initialized in a particular order.
//              Consult their documentation before changing.
//
//==============================================================================
void Win32Platform::InitializeFoundationDrive() 
{
    //
    // Get the default drive and hold it open for the life of the game.
    // This is a costly operation so we only want to do it once.
    //

    char defaultDrive[ radFileDrivenameMax + 1 ];

    ::radGetDefaultDrive( defaultDrive );

    ::radDriveOpenSync( &mpIRadDrive, 
                        defaultDrive,
                        NormalPriority, // Default
                        GMA_PERSISTENT );

    rAssert( mpIRadDrive != NULL );

    mpIRadDrive->RegisterErrorHandler( this, NULL );
}


//==============================================================================
// Win32Platform::ShutdownFoundation
//==============================================================================
// Description: Shut down Foundation Tech.
//
// Parameters:	None.
//
// Return:      None.
//
// Constraints: The FTech systems must be terminated in the reverse order that
//              they were initialized in.
//
//==============================================================================
void Win32Platform::ShutdownFoundation()
{
    //
    // Release the drive we've held open since the begining.
    //
    mpIRadDrive->Release();
    mpIRadDrive = NULL;

    //
    // Shutdown the systems in the reverse order.
    //
    ::radMovieTerminate2();
    ::radDriveUnmount( NULL );
    ::radLoadTerminate();
    ::radFileTerminate();
    ::radDbgWatchTerminate();
    if( CommandLineOptions::Get( CLO_MEMORY_MONITOR ) )
    {
        ::radMemoryMonitorTerminate();
    }
    ::radDbgComTargetTerminate();
    ::radTimeTerminate();
    ::radPlatformTerminate();
}


//==============================================================================
// Win32Platform::InitializePure3D
//==============================================================================
// Description: Get Pure3D ready to go.
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void Win32Platform::InitializePure3D() 
{
MEMTRACK_PUSH_GROUP( "Win32Platform" );
    //    p3d::SetMemAllocator( p3d::ALLOC_DEFAULT, GMA_PERSISTENT );
    //    p3d::SetMemAllocator( p3d::ALLOC_LOADED, GMA_LEVEL );

    //
    // Initialise Pure3D platform object.
    // This call differs between different platforms.  The Win32 version,
    // for example requires the application instance to be passed in.
    //
    mpPlatform = tPlatform::Create( mWnd );
    rAssert( mpPlatform != NULL );

    //
    // Initialize the d3d context.
    //
    InitializeContext();

    //
    // This call installs chunk handlers for all the primary chunk types that
    // Pure3D supports.  This includes textures, materials, geometries, and the
    // like.
    //
    //    p3d::InstallDefaultLoaders();
    P3DASSERT(p3d::context);
    tP3DFileHandler* p3d = new(GMA_PERSISTENT) tP3DFileHandler;
    //    p3d::loadManager->AddHandler(p3d, "p3d");
    p3d::context->GetLoadManager()->AddHandler(p3d, "p3d");
    p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "png");

    if( CommandLineOptions::Get( CLO_FE_UNJOINED ) )
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "bmp");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "tga");
    }
    else
    {
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tBMPHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tPNGHandler, "p3d");
        p3d::context->GetLoadManager()->AddHandler(new(GMA_PERSISTENT) tTargaHandler, "p3d");
    }

    //    p3d->AddHandler(new tGeometryLoader);
    //    GeometryWrappedLoader* pGWL = new GeometryWrappedLoader;
    GeometryWrappedLoader* pGWL = 
        (GeometryWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msGeometry );
    pGWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pGWL );

    StaticEntityLoader* pSEL = 
        (StaticEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticEntity );
    pSEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSEL );

    StaticPhysLoader* pSPL = 
        (StaticPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msStaticPhys );
    pSPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pSPL );

    TreeDSGLoader* pTDL = 
        (TreeDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msTreeDSG );
    pTDL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pTDL );

    FenceLoader* pFL = 
        (FenceLoader*)GetAllWrappers()->mpLoader( AllWrappers::msFenceEntity );
    pFL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pFL );

    IntersectLoader* pIL = 
        (IntersectLoader*)GetAllWrappers()->mpLoader( AllWrappers::msIntersectDSG );
    pIL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pIL );

    AnimCollLoader* pACL = 
        (AnimCollLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimCollEntity );
    pACL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pACL );

    AnimDSGLoader* pAnimDSGLoader = 
        (AnimDSGLoader*)GetAllWrappers()->mpLoader( AllWrappers::msAnimEntity );
    pAnimDSGLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDSGLoader );


    DynaPhysLoader* pDPL = 
        (DynaPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msDynaPhys );
    pDPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pDPL );

    InstStatPhysLoader* pISPL = 
        (InstStatPhysLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatPhys );
    pISPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISPL );

    InstStatEntityLoader* pISEL = 
        (InstStatEntityLoader*)GetAllWrappers()->mpLoader( AllWrappers::msInstStatEntity );
    pISEL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pISEL );

    LocatorLoader* pLL = 
        (LocatorLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLocator);
    pLL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLL );

    RoadLoader* pRL = 
        (RoadLoader*)GetAllWrappers()->mpLoader( AllWrappers::msRoadSegment);
    pRL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pRL );

    PathLoader* pPL = 
        (PathLoader*)GetAllWrappers()->mpLoader( AllWrappers::msPathSegment);
    pPL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pPL );

    WorldSphereLoader* pWSL = 
        (WorldSphereLoader*)GetAllWrappers()->mpLoader( AllWrappers::msWorldSphere);
    pWSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pWSL );

    LensFlareLoader* pLSL = 
        (LensFlareLoader*)GetAllWrappers()->mpLoader( AllWrappers::msLensFlare);
    pLSL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pLSL );

    BillboardWrappedLoader* pBWL = 
        (BillboardWrappedLoader*)GetAllWrappers()->mpLoader( AllWrappers::msBillboard);
    pBWL->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBWL );


    InstParticleSystemLoader* pInstParticleSystemLoader = 
        (InstParticleSystemLoader*) GetAllWrappers()->mpLoader( AllWrappers::msInstParticleSystem);
    pInstParticleSystemLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pInstParticleSystemLoader );

    BreakableObjectLoader* pBreakableObjectLoader = 
        (BreakableObjectLoader*) GetAllWrappers()->mpLoader( AllWrappers::msBreakableObject);
    pBreakableObjectLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pBreakableObjectLoader );

    AnimDynaPhysLoader*	pAnimDynaPhysLoader = 
        (AnimDynaPhysLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhys);
    pAnimDynaPhysLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimDynaPhysLoader );

    AnimDynaPhysWrapperLoader* pAnimWrapperLoader = 
        (AnimDynaPhysWrapperLoader*) GetAllWrappers()->mpLoader( AllWrappers::msAnimDynaPhysWrapper);
    pAnimWrapperLoader->SetRegdListener( GetRenderManager(), 0 );
    p3d->AddHandler( pAnimWrapperLoader );

    p3d->AddHandler(new(GMA_PERSISTENT) tTextureLoader);
    p3d->AddHandler( new(GMA_PERSISTENT) tSetLoader );
    p3d->AddHandler(new(GMA_PERSISTENT) tShaderLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCameraLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tGameAttrLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightLoader);

    p3d->AddHandler(new(GMA_PERSISTENT) tLocatorLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLightGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tTextureFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tImageFontLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSpriteLoader);
    //p3d->AddHandler(new(GMA_PERSISTENT) tBillboardQuadGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tSkeletonLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tPolySkinLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tCompositeDrawableLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tVertexAnimKeyLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimationLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tFrameControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tMultiControllerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tAnimatedObjectLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemFactoryLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tParticleSystemLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tLensFlareGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) sg::Loader);

    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionGroupLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionMixerLoader);
    p3d->AddHandler(new(GMA_PERSISTENT) tExpressionLoader);

    //ATCloader, hope this doesnt blow up
    p3d->AddHandler(new(GMA_PERSISTENT) ATCLoader);

    //p3d->AddHandler(new p3d::tIgnoreLoader);

    tSEQFileHandler* sequencerFileHandler = new(GMA_PERSISTENT) tSEQFileHandler;
    p3d::loadManager->AddHandler(sequencerFileHandler, "seq");

    // sim lib
    sim::InstallSimLoaders();

    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::FOLLOWCAM);    
    p3d->AddHandler(new(GMA_PERSISTENT) CameraDataLoader, SRR2::ChunkID::WALKERCAM);    
    p3d->AddHandler(new(GMA_PERSISTENT) IntersectionLoader);    
    //p3d->AddHandler(new(GMA_PERSISTENT) RoadLoader);    
    p3d->AddHandler(new(GMA_PERSISTENT) RoadDataSegmentLoader);    
    p3d->AddHandler(new(GMA_PERSISTENT) CStatePropDataLoader);
MEMTRACK_POP_GROUP( "Win32Platform" );
}


//==============================================================================
// Win32Platform::ShutdownPure3D
//==============================================================================
// Description: Clean up and shut down Pure3D.
//
// Parameters:	None.
// 
// Return:      None.
//
//==============================================================================
void Win32Platform::ShutdownPure3D()
{
    //
    // Clean-up the Pure3D Inventory
    //
    p3d::inventory->RemoveAllElements();
    p3d::inventory->DeleteAllSections();

    //
    // Clean-up the space taken by the Pure 3D context.
    //
    if( mpContext != NULL )
    {
        mpPlatform->DestroyContext( mpContext );
        mpContext = NULL;
    }

    //
    // Clean-up the space taken by the Pure 3D platform.
    //
    if( mpPlatform != NULL )
    {
        tPlatform::Destroy( mpPlatform );
        mpPlatform = NULL;
    }
}

//==============================================================================
// Win32Platform::InitializeContext
//==============================================================================
// Description: Initializes the d3d context for this application according to
//              the class' display settings - resolution, bpp, fullscreen.
//
// Parameters:	n/a
//
// Return:      n/a
//
//==============================================================================

void Win32Platform::InitializeContext()
{
    tContextInitData init;

    //
    // This is the window we want to render into.
    //
    init.window = mWnd;

    //
    // Set the fullscreen/window mode.
    //
    init.displayMode = mFullscreen ? PDDI_DISPLAY_FULLSCREEN : PDDI_DISPLAY_WINDOW;

    //
    // All applications should supply PDDI_BUFFER_COLOUR.  PDDI_BUFFER_DEPTH
    // specifies that we also want to allocate a Z-buffer.
    //
    init.bufferMask = PDDI_BUFFER_COLOUR | PDDI_BUFFER_DEPTH;
    init.enableSnapshot = false;

    //
    // These values only take effect in fullscreen mode.  In windowed mode, the
    // dimensions of the window define the rendering area.  We'll define them
    // anyway for completeness sake.
    //
    TranslateResolution( mResolution, init.xsize, init.ysize );
	
	#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
    rDebugPrintf("[RES] requested init.xsize/init.ysize = %dx%d (mResolution=%d)\n",
                 init.xsize, init.ysize, (int)mResolution);
	#endif
	

    //
    // Depth of the rendering buffer.  Again, this value only works in
    // fullscreen mode.  In window mode, the depth of the desktop is used.
    // This value should be either 16 or 32.
    //
    init.bpp = mbpp;

    init.lockToVsync = false;

    if( mpContext == NULL )
    {
        //
        // This the name of the PDDI we will be using for rendering
        //
        snprintf(init.PDDIlib, 128, pddiLibraryName, mRenderer);

        // Create the context
        mpContext = mpPlatform->CreateContext( &init );
        rAssert( mpContext != NULL );

        //
        // Assign this context to the platform.
        //
        mpPlatform->SetActiveContext( mpContext );
        p3d::pddi->EnableZBuffer( true );
        #if defined(RAD_ANDROID)
        // VSYNC on + habilita cap a 60 (m_only60)
        mpContext->GetDisplay()->SetForceVSync(true, true);
        // opcional (1 vez): pedir VSYNC al driver
        SDL_GL_SetSwapInterval(1);
		
		// ESTO ES PARA VER LA RESOLUCION ACTUIAL 
		int realW = mpContext->GetDisplay()->GetWidth();
        int realH = mpContext->GetDisplay()->GetHeight();
        rDebugPrintf("[RES] REAL display/backbuffer = %dx%d\n", realW, realH);
        #endif
    }
    else
    {
        // Update the display settings.
        mpContext->GetDisplay()->InitDisplay( &init );
        #if defined(RAD_ANDROID)
        // por si InitDisplay recrea / resetea algo
        mpContext->GetDisplay()->SetForceVSync(true, true);
        SDL_GL_SetSwapInterval(1);
		
		
		// aqui lo repito para ver resolucion real
		int realW = mpContext->GetDisplay()->GetWidth();
        int realH = mpContext->GetDisplay()->GetHeight();
        rDebugPrintf("[RES] REAL display/backbuffer = %dx%d\n", realW, realH);
		
        #endif
    }
}

//==============================================================================
// Win32Platform::TranslateResolution
//==============================================================================
// Description: translates resolution enums to x and y
//
// Parameters:	resolution - the res enum
//              x - corresponding width
//              y - corresponding height
//
// Return:      N/A.
//
//==============================================================================

void Win32Platform::TranslateResolution( Resolution res, int&x, int&y )
{
#ifdef __SWITCH__
    AppletOperationMode operationMode = appletGetOperationMode();
    if( operationMode == AppletOperationMode_Handheld )
    {
        x = 1280;
        y = 720;
    }
    else
    {
        x = 1920;
        y = 1080;
    }
#elif defined(RAD_VITA)
    x = 960;
    y = 544;
	
#elif defined(RAD_ANDROID)
    // Android: fixed render size for a first working port (720p target).
    // This keeps behavior predictable across devices and matches the "fixed res per platform" style
    // already used for Vita and Switch.
    x = 1280;
    y = 720;

#else
    switch( res )
    {
        case Res_640x480:
        {
            x = 640;
            y = 480;
            break;
        }
        case Res_800x600:
        {
            x = 800;
            y = 600;
            break;
        }
        case Res_1024x768:
        {
            x = 1024;
            y = 768;
            break;
        }
        case Res_1152x864:
        {
            x = 1152;
            y = 864;
            break;
        }
        case Res_1280x1024:
        {
            x = 1280;
            y = 1024;
            break;
        }
        case Res_1600x1200:
        {
            x = 1600;
            y = 1200;
            break;
        }
        default:
        {
            rAssert( false );
        }
    }
#endif
}

//==============================================================================
// Win32Platform::IsResolutionSupported
//==============================================================================
// Description: Determines if a resolution is supported on this pc
//
// Parameters:	resolution - the res enum
//
// Return:      true if supported.
//
//==============================================================================

bool Win32Platform::IsResolutionSupported( Resolution res, int bpp ) const
{
    int x,y;

    TranslateResolution( res, x, y );

    // Get the display info for the device
    pddiDisplayInfo* displays = NULL;
    int num_adapters = mpContext->GetDevice()->GetDisplayInfo( &displays );
    rAssert( num_adapters > 0 );

    // Go through the supported modes and see if we can do it.
    // Ignore the refresh rate - directx uses default.
    for( int i = 0; i < displays[0].nDisplayModes; i++ )
    {
        if( displays[0].modeInfo[i].width == x &&
            displays[0].modeInfo[i].height == y &&
            displays[0].modeInfo[i].bpp == bpp )
        {
            return true;
        }
    }

    return false;
}

//=============================================================================
// Win32Platform::ResizeWindow
//=============================================================================
// Description: Resizes the app's window based on the current resolution.
//
// Parameters:  n/a
//
// Returns:     n/a
//
// Notes:
//=============================================================================

void Win32Platform::ResizeWindow()
{
    // If fullscreen, no need to change the window size.
    if( mFullscreen )
    {
        return;
    }

    int w,h;
    TranslateResolution( mResolution, w, h );

    SDL_SetWindowSize( mWnd, w, h );
    SDL_SetWindowPosition( mWnd, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
    SDL_ShowWindow( mWnd );
}

//=============================================================================
// Win32Platform::ShowTheCursor
//=============================================================================
// Description: Shows or hides the cursor.  Wrapper for the windows ShowCursor
//              function, except it doesn't keep a counter for the number of
//              shows/hides.
//
// Parameters:  show - show cursor
//
// Returns:     n/a
//
// Notes:
//=============================================================================

void Win32Platform::ShowTheCursor( bool show )
{
#ifdef RAD_ANDROID
    //
    // Android has no traditional mouse cursor.
    // Keep internal state in sync but don't call SDL cursor APIs.
    //
    mShowCursor = show;
    return;
#endif
    if( mShowCursor != show )
    {        
        mShowCursor = show;
#if SDL_MAJOR_VERSION < 3
        SDL_ShowCursor( mShowCursor ? SDL_ENABLE : SDL_DISABLE );
#else
        if( mShowCursor )
            SDL_ShowCursor();
        else
            SDL_HideCursor();
#endif
    }
}

//=============================================================================
// Win32Platform::WndProc
//=============================================================================
// Description: The windows os messaging callback for the game.
//              Routes messages to pure3d.
//
// Parameters:  hwnd - handle for window
//                message - message ID
//                wParam - word parameter
//                lParam - long parameter
//
// Returns:     windows result
//
// Notes:
//=============================================================================

#if SDL_MAJOR_VERSION < 3
int SDLCALL Win32Platform::WndProc( void * userdata, SDL_Event * event )
#else
bool SDLCALL Win32Platform::WndProc( void * userdata, SDL_Event * event )
#endif
{
    SDL_Window * wnd = (SDL_Window *)userdata;

    switch(event->type)
    {
#if SDL_MAJOR_VERSION < 3
    case SDL_WINDOWEVENT: // WM_ACTIVATEAPP
#else
    default: // WM_ACTIVATEAPP
        if( event->type >= SDL_EVENT_WINDOW_FIRST && event->type <= SDL_EVENT_WINDOW_LAST )
#endif
        {
            //
            // Under Win32, Pure3D needs to get a crack at the Windows messages so
            // it can detect window moving, resizing, and activation.
            //
            if (p3d::platform != NULL)
                p3d::platform->ProcessWindowsMessage( wnd, &event->window );

            if( spInstance != NULL && spInstance->mpContext != NULL )
            {
                InputManager* pInputManager = GetInputManager();
#if SDL_MAJOR_VERSION < 3
                switch( event->window.event )
                {
                case SDL_WINDOWEVENT_FOCUS_GAINED: // Window is being shown (in focus)
                    {
                        RenderFlow* rf = GetRenderFlow();

                        rf->SetGamma( rf->GetGamma() );
                        if( pInputManager )
                        {
                            //GetInputManager()->SetRumbleForDevice(0, true);
                            //rDebugPrintf("Force Effects Started!!! \n");
                        }
                    }
                    break;

                case SDL_WINDOWEVENT_FOCUS_LOST:  // Window is being hidden (not in focus)
#ifndef RAD_ANDROID               
					SDL_SetWindowGammaRamp( wnd,
                        DesktopGammaRamp[0],
                        DesktopGammaRamp[1],
                        DesktopGammaRamp[2] );
#endif
                    if( pInputManager )
                    {
                        //GetInputManager()->SetRumbleForDevice(0, false);
                        //rDebugPrintf("Force Effects Stopped!!! \n");
                    }
                    break;

#ifdef RAD_PC
                case SDL_WINDOWEVENT_LEAVE:
                    GetInputManager()->GetFEMouse()->getCursor()->SetVisible( false );
                    break;
#endif
                }

                ShowTheCursor( event->window.event == SDL_WINDOWEVENT_FOCUS_LOST );
#else
                switch( event->window.type )
                {
                case SDL_EVENT_WINDOW_FOCUS_GAINED: // Window is being shown (in focus)
                    {
                        RenderFlow* rf = GetRenderFlow();

                        rf->SetGamma( rf->GetGamma() );
                        if( pInputManager )
                        {
                            //GetInputManager()->SetRumbleForDevice(0, true);
                            //rDebugPrintf("Force Effects Started!!! \n");
                        }
                    }
#ifndef RAD_ANDROID
                    SDL_HideCursor();
#endif
                    break;

                case SDL_EVENT_WINDOW_FOCUS_LOST:  // Window is being hidden (not in focus)
                    if( pInputManager )
                    {
                        //GetInputManager()->SetRumbleForDevice(0, false);
                        //rDebugPrintf("Force Effects Stopped!!! \n");
                    }
#ifndef RAD_ANDROID	
                    SDL_ShowCursor();
#endif
                    break;

#ifdef RAD_PC
                case SDL_EVENT_WINDOW_LEAVE:
                    GetInputManager()->GetFEMouse()->getCursor()->SetVisible( false );
                    break;
#endif
                }
#endif
            }

            break;
        }

#if SDL_MAJOR_VERSION < 3
    case SDL_KEYDOWN: // WM_SYSKEYDOWN
    case SDL_KEYUP:   // WM_SYSKEYUP
#else
    case SDL_EVENT_KEY_DOWN: // WM_SYSKEYDOWN
    case SDL_EVENT_KEY_UP:   // WM_SYSKEYUP
#endif
        {
            //Ignore Alt and F10 keys.
#if SDL_MAJOR_VERSION < 3
            switch( event->key.keysym.sym )
#else
            switch( event->key.key )
#endif
            {
            case SDLK_LALT:
            case SDLK_RALT:
            	return 0;
            case SDLK_F10:
            	return 0;
            default: break;
            }
        }

#if SDL_MAJOR_VERSION < 3
    case SDL_MOUSEMOTION:
#else
    case SDL_EVENT_MOUSE_MOTION:
#endif
        {
#ifdef RAD_PC
            // For some reason beyond my comprehension WM_MOUSEMOVE seems to be getting called regardless if the
            // mouse moved or not. So let the FEMouse determine if we moved.
            FEMouse* pFEMouse = GetInputManager()->GetFEMouse();
            if( pFEMouse->DidWeMove( event->motion.x, event->motion.y ) )
            {
                int w, h;
                SDL_GetWindowSize( wnd, &w, &h );
                pFEMouse->Move( event->motion.x, event->motion.y, w, h );
            }
#endif

            ShowTheCursor( false );

            break;
        }

#ifdef RAD_PC
    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
            GetInputManager()->GetFEMouse()->ButtonDown( BUTTON_LEFT );
    //        rDebugPrintf("LEFT MOUSE BUTTON PRESSED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
        break;

    case SDL_MOUSEBUTTONUP:
        if(event->button.button == SDL_BUTTON_LEFT)
            GetInputManager()->GetFEMouse()->ButtonUp( BUTTON_LEFT );
        break;
#endif

        // PDDI will sent this message to enable or disable rendering in response to an
        // application level window event.  For example, if the user clicks away from
        // the rendering window, or uses ALT-TAB to select another application, PDDI
        // will tell sent a WM_PDDI_DRAW_ENABLE(0) message.  When the application
        // regains focus, WM_PDDI_DRAW_ENABLE(1) will be sent.
    //case WM_PDDI_DRAW_ENABLE:
        //GetApplication()->EnableRendering(wParam == 1);
        break;
    }

    return 1;
}

