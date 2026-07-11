//=============================================================================
// Copyright (C) 2002 Radical Entertainment Ltd.  All rights reserved.
//
// File:        game.cpp
//
// Description: The game loop
//
// History:     + Stolen and cleaned up from Penthouse -- Darwin Chau
//
//=============================================================================

//========================================
// System Includes
//========================================
// Standard Library
#include <stdlib.h>
#include <string.h>
// Foundation Tech
#include <raddebug.hpp>
#include <raddebugcommunication.hpp>
#include <raddebugconsole.hpp>
#include <raddebugwatch.hpp>
#include <radfile.hpp>
#include <radmemorymonitor.hpp>
#include <radtime.hpp>
// Pure3D
#include <p3d/loadmanager.hpp>
#include <p3d/utility.hpp>
#ifdef RAD_WIN32
#include <SDL.h>  // for SDL_PollEvent...
#endif

//========================================
// Project Includes
//========================================
#include <contexts/contextenum.h>
#include <debug/profiler.h>
#include <gameflow/gameflow.h>
#include <presentation/gui/ingame/guiscreenmissionload.h>
#include <main/commandlineoptions.h>
#include <main/game.h>
#include <main/platform.h>

#ifdef RAD_GAMECUBE
#include <main/gamecube_extras/gcmanager.h>
#endif

#include <memory/srrmemory.h>
#include <memory/memoryutilities.h>

#include <render/RenderFlow/renderflow.h>
#include <sound/soundmanager.h>
#include <input/inputmanager.h>
#ifdef RAD_WIN32
#include <input/touch/touchinputmodemanager.h>
#include <input/touch/touchinputadapter.h>
#endif
#ifdef RAD_ANDROID
#include <input/touch/touchcontextresolver.h>
#include <input/touch/touchhudsystem.h>
#include <input/touch/touchinputadapter.h>
#include <worldsim/avatarmanager.h>
#include <input/touch/touchassetextractor.h>
#include <input/touch/touchassetmanager.h>
#include <input/touch/touchhudrenderer.h>
#include <data/config/androidconfigurationmanager.h>
#include <input/touch/touchcontrolsconfigurationmanager.h>
#endif

#if defined(RAD_ANDROID)
  #include <android/log.h>
  #define LOG_TAG "SimpsonsHitAndRun"
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif
//******************************************************************************
//
// Global Data, Local Data, Local Classes
//
//******************************************************************************

//
// Static pointer to instance of this singleton.
//
Game* Game::spInstance = NULL;

bool g_inDemoMode = false;

#ifdef RAD_ANDROID

static bool GetTouchAvatarInVehicleForPlayer0()
{
    /*
     * Keep this helper conservative.
     *
     * It is only queried when the current context is gameplay.
     * AvatarManager is the correct long-term source for detecting whether
     * player 0 is currently inside a vehicle.
     */
    Avatar* avatar = GetAvatarManager()->GetAvatarForPlayer( 0 );

    if ( avatar == NULL )
    {
        return false;
    }

    return avatar->IsInCar();
}

static void UpdateTouchContextResolverFromGame()
{
    if ( GetGameFlow() == NULL || InputManager::GetInstance() == NULL )
    {
        return;
    }

    const int currentContext = GetGameFlow()->GetCurrentContext();
    const unsigned inputGameState = InputManager::GetInstance()->GetGameState();

    bool avatarInVehicle = false;

    /*
     * Only query AvatarManager while actually in gameplay.
     * This avoids touching avatar systems during boot, frontend, loading or exit.
     */
    if ( currentContext == CONTEXT_GAMEPLAY )
    {
        avatarInVehicle = GetTouchAvatarInVehicleForPlayer0();
    }

    TouchContextResolver::GetInstance().UpdateFromGameState(
        currentContext,
        inputGameState,
        avatarInVehicle
    );
}

static void UpdateTouchHudSystemFromSDLEvent( const SDL_Event& msg )
{
#if SDL_MAJOR_VERSION < 3

    switch ( msg.type )
    {
        case SDL_FINGERDOWN:
        {
            TouchInputAdapter::GetInstance().SetEnabled( true );
            TouchInputAdapter::GetInstance().SetTargetControllerIndex( 0 );

            TouchHudSystem::GetInstance().HandleFingerDown(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerId ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        case SDL_FINGERMOTION:
        {
            TouchHudSystem::GetInstance().HandleFingerMove(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerId ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        case SDL_FINGERUP:
        {
            TouchHudSystem::GetInstance().HandleFingerUp(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerId ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        default:
        {
            break;
        }
    }

#else

    switch ( msg.type )
    {
        case SDL_EVENT_FINGER_DOWN:
        {
            TouchInputAdapter::GetInstance().SetEnabled( true );
            TouchInputAdapter::GetInstance().SetTargetControllerIndex( 0 );

            TouchHudSystem::GetInstance().HandleFingerDown(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerID ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        case SDL_EVENT_FINGER_MOTION:
        {
            TouchHudSystem::GetInstance().HandleFingerMove(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerID ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        case SDL_EVENT_FINGER_UP:
        {
            TouchHudSystem::GetInstance().HandleFingerUp(
                static_cast<TouchHudFingerId>( msg.tfinger.fingerID ),
                msg.tfinger.x,
                msg.tfinger.y,
                msg.tfinger.pressure
            );

            break;
        }

        default:
        {
            break;
        }
    }

#endif
}

#endif // RAD_ANDROID

#ifdef RAD_WIN32

static void UpdateTouchInputModeFromSDLEvent( const SDL_Event& msg )
{
#if SDL_MAJOR_VERSION < 3

    switch ( msg.type )
    {
        case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        case SDL_FINGERUP:
        {
            TouchInputModeManager::GetInstance().NotifyTouchInput();
            break;
        }

        case SDL_CONTROLLERDEVICEADDED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadConnected();
            break;
        }

        case SDL_CONTROLLERDEVICEREMOVED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadDisconnected();
            break;
        }

        case SDL_CONTROLLERBUTTONDOWN:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadInput();
            break;
        }

        case SDL_CONTROLLERAXISMOTION:
        {
            if ( msg.caxis.value > 8000 || msg.caxis.value < -8000 )
            {
                TouchInputModeManager::GetInstance().NotifyGamepadInput();
            }
            break;
        }

        case SDL_JOYDEVICEADDED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadConnected();
            break;
        }

        case SDL_JOYDEVICEREMOVED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadDisconnected();
            break;
        }

        case SDL_JOYBUTTONDOWN:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadInput();
            break;
        }

        case SDL_JOYAXISMOTION:
        {
            if ( msg.jaxis.value > 8000 || msg.jaxis.value < -8000 )
            {
                TouchInputModeManager::GetInstance().NotifyGamepadInput();
            }
            break;
        }

        default:
        {
            break;
        }
    }

#else

    switch ( msg.type )
    {
        case SDL_EVENT_FINGER_DOWN:
        case SDL_EVENT_FINGER_MOTION:
        case SDL_EVENT_FINGER_UP:
        {
            TouchInputModeManager::GetInstance().NotifyTouchInput();
            break;
        }

        case SDL_EVENT_GAMEPAD_ADDED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadConnected();
            break;
        }

        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadDisconnected();
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadInput();
            break;
        }

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        {
            if ( msg.gaxis.value > 8000 || msg.gaxis.value < -8000 )
            {
                TouchInputModeManager::GetInstance().NotifyGamepadInput();
            }
            break;
        }

        case SDL_EVENT_JOYSTICK_ADDED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadConnected();
            break;
        }

        case SDL_EVENT_JOYSTICK_REMOVED:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadDisconnected();
            break;
        }

        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
        {
            TouchInputModeManager::GetInstance().NotifyGamepadInput();
            break;
        }

        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
        {
            if ( msg.jaxis.value > 8000 || msg.jaxis.value < -8000 )
            {
                TouchInputModeManager::GetInstance().NotifyGamepadInput();
            }
            break;
        }

        default:
        {
            break;
        }
    }

#endif
}



#endif // RAD_WIN32

//#define DEMO_MODE_PROFILER

#ifdef DEMO_MODE_PROFILER
    #define DEMOPROFILE(X) X
#else
    #define DEMOPROFILE(X)
#endif

#ifdef DEMO_MODE_PROFILER

#ifdef RAD_PS2
#include <libgraph.h>
#endif

DemoProfiler::DemoProfiler(unsigned mf) :
    recording(false), maxFrames(mf), nChannel(0), currentFrame(0),
    alertStatus(PROFILER_ALERT_GREEN),
    numFramesBelow_20(0), numFramesBetween_20_30(0),numFramesBetween_30_40(0), numFramesAbove_40(0)
{
    for( int i=0; i < MAX_CHANNEL; i++)
    {
        channel[i] = NULL;
    }
}
    
void DemoProfiler::AddChannel(unsigned c, const char* name)
{
    rReleaseAssert(c < MAX_CHANNEL);
    channel[c] = new Channel;
    channel[c]->samples = new unsigned[maxFrames];
    memset(channel[c]->samples, 0, maxFrames*sizeof(unsigned));
    channel[c]->t0 = 0;
    strncpy(channel[c]->name, name, 254);
    nChannel++;
}

void DemoProfiler::Start(unsigned c)
{
    if(!channel[c]) return;
    if( recording )
    {
        channel[c]->t0 = radTimeGetMicroseconds64();
    }
}


void DemoProfiler::Stop(unsigned c)
{
    if(!channel[c]) return;
    if( recording )
    {
        radTime64 elapsed = radTimeGetMicroseconds64() - channel[c]->t0;
        channel[c]->samples[currentFrame] += elapsed;

        if(c == 0)
        {
            alertStatus = PROFILER_ALERT_GREEN;
            if( elapsed >= 50000 )
            {
                numFramesBelow_20++;
                alertStatus = PROFILER_ALERT_RED;
            }
            else if( (elapsed < 50000) && (elapsed >= 33333) )
            {
                numFramesBetween_20_30++;
                alertStatus = PROFILER_ALERT_YELLOW;
            }
            else if( (elapsed < 33333) && (elapsed >= 25000) )
            {
                numFramesBetween_30_40++;
            }
            else
            {
                numFramesAbove_40++;
            }
        }
    }
}

void DemoProfiler::Set(unsigned c, unsigned val)
{
    if(!channel[c]) return;
    if( recording )
    {
        channel[c]->samples[currentFrame] = val;
    }
}

void DemoProfiler::StartRecording()
{
    recording = true;
}

bool DemoProfiler::IsRecording()
{
    return recording;
}

unsigned DemoProfiler::GetSample(unsigned c)
{
    return (recording && channel[c]) ? channel[c]->samples[currentFrame] : 0;
}

unsigned DemoProfiler::GetCurrentFrame()
{
    return currentFrame;
}

void DemoProfiler::Accumulate(unsigned c, unsigned val)
{
    if(!channel[c]) return;
    if( recording )
    {
        channel[c]->samples[currentFrame] += val;
    }
}

void DemoProfiler::NextFrame()
{
    if( recording )
    {
        currentFrame++;
        if(currentFrame >= maxFrames)
        {
            Dump();
            recording = false;
        }
    }
}

DemoProfiler::AlertStatus DemoProfiler::GetAlertStatus()
{
    return alertStatus;
}

void DemoProfiler::Dump()
{
    rReleasePrintf("\n\n~~~~~~~~~~~~~~~~~~~~~ PROFILER STATS ~~~~~~~~~~~~~~~~~~~~~\n");
    rReleasePrintf(    "Total Frames: %d\n"
                       "< 20 fps: %d (%.2f%%)\n"
                       "20-30 fps: %d (%.2f%%)\n"
                       "30-40 fps: %d (%.2f%%)\n"
                       ">40 fps: %d (%.2f%%)\n",
                       maxFrames,
                       numFramesBelow_20, 100.0f * (float)numFramesBelow_20 / (float)maxFrames,
                       numFramesBetween_20_30, 100.0f * (float)numFramesBetween_20_30 / (float)maxFrames,
                       numFramesBetween_30_40, 100.0f * (float)numFramesBetween_30_40 / (float)maxFrames,
                       numFramesAbove_40, 100.0f * (float)numFramesAbove_40 / (float)maxFrames );

    for( unsigned i=0; i < MAX_CHANNEL; i++)
    {
        if(channel[i])
        {
            rReleasePrintf("%s\t", channel[i]->name);
        }
    }
    rReleasePrintf("\n");

    for( unsigned i=0; i < maxFrames; i++ )
    {
        for( unsigned j=0; j < MAX_CHANNEL; j++ )
        {
            if(channel[j])
            {
                rReleasePrintf("%.1f\t", (float)channel[j]->samples[i] * 0.001f);
            }
        }
        rReleasePrintf("\n");

        if( !(i % 10) )
        {
            rmt::Sin(0.0f);
#ifdef RAD_PS2
            sceGsSyncV(0);
#endif
        }
    }

    while(1)
    {
        rmt::Sin(0.0f);
    }
}


DemoProfiler g_DemoProfiler( 1850 );  // about a minute at 30 fps

#include <pddi/pddi.hpp>

static bool g_DemoProfiler_Started = false;
static const int g_DemoProfiler_StartFrame = 150;
static int g_DemoProfiler_CurrentFrame = 0;

#endif


//******************************************************************************
//
// Public Member Functions
//
//******************************************************************************

//==============================================================================
// Game::CreateInstance
//==============================================================================
// Description: Create the game
//
// Parameters:	platform - the platform that the game is to be created on
//
// Return:      pointer to the created game
//
// Constraints: This is a singleton so only one instance is allowed.
//
//==============================================================================
Game* Game::CreateInstance( Platform* platform )
{
    rAssert( platform != NULL );

    rAssertMsg( (spInstance == NULL), "Trying to create more than one instance of the game!" );

MEMTRACK_PUSH_GROUP( "Game" );
    if( spInstance == NULL )
    {
        spInstance = new(GMA_PERSISTENT) Game( platform );
        rAssert( spInstance != NULL );
    }
MEMTRACK_POP_GROUP( "Game" );
    
    return spInstance;
}


//==============================================================================
// Game::DestroyInstance
//==============================================================================
// Description: Destroy the game
//
// Parameters:	None.
//
// Return:      None.
//
//==============================================================================
void Game::DestroyInstance()
{
    delete( GMA_PERSISTENT, spInstance );
    spInstance = NULL;
}


//==============================================================================
// Game::GetInstance
//==============================================================================
// Synopsis:    Get an instance of the game
//
// Parameters:  None.
//
// Returns:     a poitner to the game
//
// Constraints: Game must be created before this is called
//
//==============================================================================
Game* Game::GetInstance()
{
    rAssertMsg((spInstance != NULL), "Trying to get an instance of the game before it is created!");
    
    return spInstance;
}

//=============================================================================
// Game::GetPlatform
//=============================================================================
// Description: Comment
//
// Parameters:  ()
//
// Return:      Platform
//
//=============================================================================
Platform* Game::GetPlatform()
{
    return mpPlatform;
}


//==============================================================================
// Game::Initialize
//==============================================================================
// Synopsis:    Initialize the game
//
// Parameters:  None.
//
// Returns:     None.
//
//==============================================================================
void Game::Initialize()
{
    rAssert( mpPlatform != NULL );

    //
    // Initialize the platform and core systems.
    //
    mpPlatform->InitializePlatform();

    // Extraemos el layout tactil contenido en el apk en archivos leibles para que el pure3d mas adelante lo pueda renderizar en el juego 
    //APK->com.c4rlox.simpsons/files/touch_controls/icono.png
    #ifdef RAD_ANDROID
    TouchAssetExtractor::GetInstance().EnsureAssetsExtracted();
    // Realizamos la carga con pure3d de los iconos extraidos anteriormente 
    TouchAssetManager::GetInstance().Initialize();
    // Cargamos la configuración personalizada de controles táctiles del jugador (en caso de que exista) antes de inicializar el sistema de renderizado de iconos 
    TouchControlsConfigurationManager::GetInstance().Initialize();

    // Realizamos inicializacion del render, pero aqui no es donde solicitamos el renderizado,solo iniciamos el sistema de render
    TouchHudRenderer::GetInstance().Initialize();

    // hacemos que se utilice en el ciclo del juego al arrancar la configuración del archivo Simpsons_configuration.txt
    GetAndroidConfigurationManager()->Initialize();


    #endif

    //
    // Initialize the sound manager.
    //
    SoundManager::GetInstance()->Initialize();
    
    //
    // Initialize the timer system
    //
    ::radTimeCreateList( &mpTimerList,
                         16, // Default
                         GMA_PERSISTENT );

    rAssert( mpTimerList != NULL );

    //
    // Create the GameFlow & Couple the RenderFlow
    //
    mpGameFlow = GameFlow::CreateInstance();
    mpRenderFlow = RenderFlow::GetInstance();
    mpRenderFlow->DoAllRegistration();

    CGuiScreenMissionLoad::InitializePermanentVariables();

#ifdef RAD_E3
    rReleasePrintf( "\n----------=[  SIMPSONS HIT & RUN - E3 BUILD  ]=----------\n\n" );
#endif

    //
    // Set the starting context
    //
    mpGameFlow->SetContext( CONTEXT_BOOTUP );
}


//==============================================================================
// Game::Terminate
//==============================================================================
// Synopsis:    Clean up and shut down.
//
// Parameters:  None.
//
// Returns:     None.
//
//==============================================================================
void Game::Terminate() 
{
    
    rAssert( mpGameFlow != NULL );
    rAssert( mpRenderFlow != NULL );
    rAssert( mpTimerList != NULL );
    rAssert( mpPlatform != NULL );

    #ifdef RAD_ANDROID
    // Primero apagamos el render y despues liberamos los assets(iconos tactiles)
    TouchHudRenderer::GetInstance().Shutdown();
    TouchAssetManager::GetInstance().Shutdown();
    #endif

    //
    // Kill the flow servers.
    //
    mpGameFlow->DestroyInstance();
    mpGameFlow = NULL;
    
    // Render flow destroyed by singletons.cpp
    //mpRenderFlow->DestroyInstance();
    mpRenderFlow = NULL;
    
    //
    // Release the game's references to the timer list.
    //
    mpTimerList->Release();
    mpTimerList = NULL;
}


//==============================================================================
// Game::Run
//==============================================================================
// Synopsis:    This is where game loop spins.  It exits after Stop() is called.
//
// Parameters:  None.
//
// Returns:     None.
//
//==============================================================================
const unsigned PROFILE_CHANNEL_ALL = 0;
const unsigned PROFILE_CHANNEL_AI = 1;
const unsigned PROFILE_CHANNEL_RENDER = 2;
const unsigned PROFILE_CHANNEL_LOAD = 3;

void Game::Run() 
{
    extern bool g_AllowDebugOutput;
    
#ifdef DEMO_MODE_PROFILER
    g_AllowDebugOutput = false;
    g_DemoProfiler.AddChannel(0, "All");
    g_DemoProfiler.AddChannel(1, "Ai");
    g_DemoProfiler.AddChannel(2, "Render");
    g_DemoProfiler.AddChannel(3, "Load");
    g_DemoProfiler.AddChannel(4, "Opaque");
    g_DemoProfiler.AddChannel(5, "Translucent");
    g_DemoProfiler.AddChannel(6, "World Sphere");
    g_DemoProfiler.AddChannel(7, "Anim Entity");
    g_DemoProfiler.AddChannel(8, "Breakable");
    g_DemoProfiler.AddChannel(9, "InstAnimDynaPhys");
    g_DemoProfiler.AddChannel(10, "InstDynaPhys");
    g_DemoProfiler.AddChannel(11, "InstStat");
    g_DemoProfiler.AddChannel(12, "InstStatPhys");
    g_DemoProfiler.AddChannel(13,"Lens Flare");
    g_DemoProfiler.AddChannel(14, "State Prop");
    g_DemoProfiler.AddChannel(15, "Static");
    g_DemoProfiler.AddChannel(16, "Tristrip");
    g_DemoProfiler.AddChannel(17, "AnimCollision");
    g_DemoProfiler.AddChannel(18, "Cars");
    g_DemoProfiler.AddChannel(19, "Characters");
    g_DemoProfiler.AddChannel(20, "ABH");
    g_DemoProfiler.AddChannel(21, "Actor");
    g_DemoProfiler.AddChannel(22, "AnimatedIcon");
    g_DemoProfiler.AddChannel(23, "ParticleSystem");

#endif

    unsigned time = radTimeGetMilliseconds();
    while( !mExitNow )
    {
        DEMOPROFILE( g_DemoProfiler.Start(PROFILE_CHANNEL_ALL); )

        BEGIN_PROFILER_FRAME();

        BEGIN_PROFILE( "GameLoop" )

        unsigned newTime =  radTimeGetMilliseconds();
        unsigned elapsed = newTime - time;
        time = newTime;
        #ifdef RAD_ANDROID
        UpdateTouchContextResolverFromGame();
        TouchHudSystem::GetInstance().Update( elapsed );
        #endif
       

        #ifdef RAD_WIN32
            TouchInputModeManager::GetInstance().Update( elapsed );
        #endif
        //
        // Service the windows message loop.
        //
#ifdef RAD_WIN32
        SDL_Event msg;
        while( SDL_PollEvent( &msg ) )
        {
            UpdateTouchInputModeFromSDLEvent( msg );
            #ifdef RAD_ANDROID
                UpdateTouchHudSystemFromSDLEvent( msg );
            #endif
#if SDL_MAJOR_VERSION < 3
            if( msg.type == SDL_QUIT )
#else
            if( msg.type == SDL_EVENT_QUIT )
#endif
            {
                //Chuck someone closed the Window we are going to try to exit the game 
                //if the game isnt in a context that can easily transition to the EXIT context we 
                // are going to call the Launch Dashboard and return to the main loop and shutdown
                if ( GetGameFlow()->GetCurrentContext() != CONTEXT_FRONTEND &&
                     GetGameFlow()->GetCurrentContext() != CONTEXT_GAMEPLAY && 
                     GetGameFlow()->GetCurrentContext() != CONTEXT_PAUSE )
                {
                    GetGame()->GetPlatform()->LaunchDashboard();
                    //return to the winmain and shutdown
                    return;
                }
                //we are in a context that will transition nicely to the Exit context.
                else
                {
                    mpGameFlow->SetContext( CONTEXT_EXIT );
                }               
            }
        }
#endif // RAD_WIN32

        //
        // Service the GameFlow and RenderFlow.
        //

        if ( !mpPlatform->PausedForErrors() )
        {
            DEMOPROFILE( g_DemoProfiler.Start(PROFILE_CHANNEL_AI); )
            mpTimerList->Service(); //nv
            mpGameFlow->OnTimerDone(elapsed, NULL);
            DEMOPROFILE( g_DemoProfiler.Stop(PROFILE_CHANNEL_AI); )

            if( !mExitNow )
            {
                DEMOPROFILE( g_DemoProfiler.Start(PROFILE_CHANNEL_RENDER); )
                mpRenderFlow->OnTimerDone(elapsed, NULL);
                DEMOPROFILE( g_DemoProfiler.Stop(PROFILE_CHANNEL_RENDER); )
            }
        }
        else if (mpPlatform->IsControllerError()) // if controller unplugged
        {                                         // need to update input manager
            if (InputManager::GetInstance())
            {
                InputManager::GetInstance()->Update( elapsed );
            }
        }
        else
        {
#ifdef RAD_GAMECUBE
            GCManager::GetInstance()->OnTimerDone( elapsed, NULL );
#endif
        }

        //
        // Service FTech subsystems.
        //
        ::radFileService();
        ::radDbgComService();
        ::radDebugConsoleService();
        
        if( CommandLineOptions::Get( CLO_MEMORY_MONITOR) )
        {
            ::radMemoryMonitorService();
        }

        //
        // Service the sound renderer.
        //
        SoundManager::GetInstance()->Update();

        if ( mpPlatform->PausedForErrors() )
        {
            //
            // [ps] We update sound without a valid context or elapsed time. 
            // We use 0 and NUM_CONTEXTS, since these values are unlikely to
            // happen in the real game.
            //
            SoundManager::GetInstance()->UpdateOncePerFrame( 0, NUM_CONTEXTS, false, true );
        }

        //
        // Spin Pure3D async loading.
        //
        DEMOPROFILE( g_DemoProfiler.Start(PROFILE_CHANNEL_LOAD); )
        p3d::loadManager->SwitchTask();
        DEMOPROFILE( g_DemoProfiler.Stop(PROFILE_CHANNEL_LOAD); )

        ++mFrameCount;

        DEMOPROFILE( g_DemoProfiler.Stop(PROFILE_CHANNEL_ALL); )

#ifdef DEMO_MODE_PROFILER
        g_AllowDebugOutput = true;
        if(g_inDemoMode)
        {
            if( !g_DemoProfiler_Started && (g_DemoProfiler_CurrentFrame == g_DemoProfiler_StartFrame) )
            {
                g_DemoProfiler_Started = true;
                g_DemoProfiler.StartRecording();
                rReleasePrintf("Beginning demo profile run\n");
            }

            if(g_DemoProfiler.IsRecording())
            {
                pddiColour colour(255,255,255);
                if( g_DemoProfiler.GetAlertStatus() == DemoProfiler::PROFILER_ALERT_YELLOW)
                    colour.Set(255,255,0);
                else
                if( g_DemoProfiler.GetAlertStatus() == DemoProfiler::PROFILER_ALERT_RED)
                    colour.Set(255,0,0);

                char duff[255];
                sprintf(duff, "%d %d", g_DemoProfiler.GetCurrentFrame(), (g_DemoProfiler.GetSample(0) + 500) / 1000);
                p3d::pddi->DrawString( duff, 10, 400, colour );
            }
            
            g_DemoProfiler.NextFrame();
            g_DemoProfiler_CurrentFrame++;
        }

        g_AllowDebugOutput = false;
#endif // DEMO_MODE_PROFILER

        END_PROFILE( "GameLoop" )

        END_PROFILER_FRAME();
    }
}


//==============================================================================
// Game::Stop
//==============================================================================
// Synopsis:    Sets the flag to break us out of the game loop.
//
// Parameters:  None.
//
// Returns:     None.
//
//==============================================================================
void Game::Stop() 
{
    //
    // Stop any further rendering from happening.
    //
    mExitNow = true;
}


unsigned Game::GetRandomSeed ()
{
    radDate date;
    ::radTimeGetDate (&date);
    return ( ( date.m_Year << 16 ) | (  date.m_Month << 8 ) | ( date.m_Day ) ) ^ ( ( date.m_Second << 24 ) | ( date.m_Minute << 8 ) | ( date.m_Hour ) );
}



//******************************************************************************
//
// Private Member Functions
//
//******************************************************************************

//==============================================================================
// Game::Game
//==============================================================================
// Synopsis:    Constructor.
//
// Parameters:  platform - platform on which game should be created
//
// Returns:     N/A.
//
//==============================================================================
Game::Game( Platform* platform ) :
    mpPlatform( platform ),
    mpTimerList( NULL ),
    mpGameFlow( NULL ),
    mpRenderFlow( NULL ),
    mFrameCount( 0 ),
    mExitNow( false ),
    mDemoCount( 0 ),
    mTimeMS( 0 )
{
}


//==============================================================================
// Game::~Game
//==============================================================================
// Synopsis:    Destructor.
//
// Parameters:  None.
//
// Returns:     N/A.
//
//==============================================================================
Game::~Game()
{
}
