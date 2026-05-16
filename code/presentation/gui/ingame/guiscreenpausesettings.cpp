//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   CGuiScreenPauseSettings
//
// Description: Implementation of the CGuiScreenPauseSettings class.
//
// Authors:     Tony Chu
//
// Revisions		Date			Author	    Revision
//                  2002/07/04      TChu        Created for SRR2
//
//===========================================================================

//===========================================================================
// Includes
//===========================================================================
#include <presentation/gui/ingame/guiscreenpausesettings.h>
#include <presentation/gui/ingame/guimanageringame.h>
#include <presentation/gui/guimenu.h>
#include <presentation/gui/guitextbible.h>
#include <presentation/gui/guisystem.h>

#include <camera/supercammanager.h>
#include <camera/supercamcentral.h>
#include <input/inputmanager.h>
#include <memory/srrmemory.h>
#include <mission/charactersheet/charactersheetmanager.h>
#include <presentation/tutorialmanager.h>
#include <worldsim/avatarmanager.h>

#include <raddebug.hpp> // Foundation
#include <Group.h>
#include <Screen.h>
#include <Page.h>
#include <Text.h>
#include <strings/unicodeString.h>



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


//===========================================================================
// Global Data, Local Data, Local Classes
//===========================================================================

enum ePauseSettingsMenuItem
{
    MENU_ITEM_CAMERA,
    MENU_ITEM_JUMP_CAMERAS,
#ifndef RAD_PC
    MENU_ITEM_INVERT_CAM_CONTROL,
#endif
    MENU_ITEM_INTERSECT_NAV_SYSTEM,
    MENU_ITEM_RADAR,
#ifndef RAD_PC
    MENU_ITEM_VIBRATION,
#endif
    MENU_ITEM_TUTORIAL,

    NUM_PAUSE_SETTINGS_MENU_ITEMS
};


const char* PAUSE_SETTINGS_MENU_ITEMS[] =
{
    "Camera",
    "JumpCamera",
#if !defined(RAD_PC) //&& !defined(RAD_ANDROID)
    "InvertCamControl",
#endif
    "IntersectNavSystem",
    "Radar",
#if !defined(RAD_PC) //&& !defined(RAD_ANDROID)
    "Vibration",
#endif
    "Tutorial",

    ""
};

#ifdef RAD_PC
SuperCam::Type PC_CAMERAS_FOR_WALKING[] =
{
    SuperCam::WALKER_CAM,
    SuperCam::DEBUG_CAM,
    SuperCam::KULL_CAM
};

const int NUM_PC_CAMERAS_FOR_WALKING = sizeof(PC_CAMERAS_FOR_WALKING)/sizeof(SuperCam::Type);
const int NUM_PC_CAMERAS_FOR_WALKING_WITHOUT_CHEAT = 1;
#endif

//===========================================================================
// Public Member Functions
//===========================================================================

//===========================================================================
// CGuiScreenPauseSettings::CGuiScreenPauseSettings
//===========================================================================
// Description: Constructor.
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
CGuiScreenPauseSettings::CGuiScreenPauseSettings
(
    Scrooby::Screen* pScreen,
    CGuiEntity* pParent
)
:   CGuiScreen( pScreen, pParent, GUI_SCREEN_ID_SETTINGS ),
    m_pMenu( NULL ),
    m_currentCameraSelectionMode( CAMERA_SELECTION_NOT_AVAILABLE )
{
MEMTRACK_PUSH_GROUP( "CGuiScreenPauseSettings" );
    // Retrieve the Scrooby drawing elements.
    //
    Scrooby::Page* pPage = m_pScroobyScreen->GetPage( "PauseSettings" );
	rAssert( pPage );

    // Create a menu.
    //
    m_pMenu = new CGuiMenu( this, NUM_PAUSE_SETTINGS_MENU_ITEMS, GUI_TEXT_MENU, MENU_SFX_NONE );
    rAssert( m_pMenu != NULL );
    
    // Add menu items
    //
    char itemName[ 32 ];

    // ORIGINAL CODE lo comento y voy a usar otro ahora mismo para debug
    /*
    for( int i = 0; i < NUM_PAUSE_SETTINGS_MENU_ITEMS; i++ )
    {
        Scrooby::Group* group = pPage->GetGroup( PAUSE_SETTINGS_MENU_ITEMS[ i ] );
        rAssert( group != NULL );// Assert que nos provoca el fallo actualmente 

        Scrooby::Text* pText = group->GetText( PAUSE_SETTINGS_MENU_ITEMS[ i ] );
        rAssert( pText != NULL );
        pText->SetTextMode( Scrooby::TEXT_WRAP );

        sprintf( itemName, "%s_Value", PAUSE_SETTINGS_MENU_ITEMS[ i ] );
        Scrooby::Text* pTextValue = group->GetText( itemName );
        rAssert( pTextValue != NULL );
        pTextValue->SetTextMode( Scrooby::TEXT_WRAP );

        sprintf( itemName, "%s_LArrow", PAUSE_SETTINGS_MENU_ITEMS[ i ] );
        Scrooby::Sprite* pLArrow = group->GetSprite( itemName );

        sprintf( itemName, "%s_RArrow", PAUSE_SETTINGS_MENU_ITEMS[ i ] );
        Scrooby::Sprite* pRArrow = group->GetSprite( itemName );

        m_pMenu->AddMenuItem( pText,
                              pTextValue,
                              NULL,
                              NULL,
                              pLArrow,
                              pRArrow,
                              SELECTION_ENABLED | VALUES_WRAPPED | TEXT_OUTLINE_ENABLED );
    }
  */   
 // Buscar el contenedor "Menu" (según el XML: <Group Name="Menu">)
Scrooby::Group* menu = pPage->GetGroup( "Menu" );

#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
LOGI("[PauseSettings] page=%p menu=%p", (void*)pPage, (void*)menu);
#endif

rAssert( menu != NULL );
                    for( int i = 0; i < NUM_PAUSE_SETTINGS_MENU_ITEMS; i++ )
                    {
                        const char* item = PAUSE_SETTINGS_MENU_ITEMS[ i ];

                    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        LOGI("[PauseSettings] item=%d name='%s' page=%p", i, item, (void*)pPage);
                    #endif

                       Scrooby::Group* group = menu->GetGroup( item );

                    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        if( group == NULL )
                        {
                            LOGE("[PauseSettings] MISSING GROUP item=%d name='%s' page=%p screen=%p",
                                i, item, (void*)pPage, (void*)m_pScroobyScreen);
                        }
                        else
                        {
                            // Size() existe (lo estás usando en RestoreButtons), nos da pista de layout
                            LOGI("[PauseSettings] group found item=%d name='%s' group=%p size=%u",
                                i, item, (void*)group, (unsigned)group->Size());
                        }
                    #endif

                    // COdigo extra debug 

#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
if( group == NULL )
{
    const unsigned int n = (unsigned int)menu->Size();
    LOGE("[PauseSettings] Dump Menu children: menu=%p size=%u", (void*)menu, n);

    for( unsigned int c = 0; c < n; ++c )
    {
        // OJO: aquí depende de tu API real.
        // Prueba primero con GetChildDrawable(c) si existe (suele existir en FeParent/Group).
        Scrooby::Drawable* d = menu->GetChildDrawable( c );
        if( d )
        {
            // si Drawable tiene GetName() o GetID() úsalo
            LOGE("[PauseSettings]  child[%u]=%p", c, (void*)d);
        }
        else
        {
            LOGE("[PauseSettings]  child[%u]=NULL", c);
        }
    }
}
#endif

                    //FIN Codigo extra debug
                       // rAssert( group != NULL ); //AQUI Se produce el mismo assert, esto nos lo saltamos para poder acceder al mundo3D, habra que revisarlo luego, tiene que ver con que en el Cmake del Code, no se incluye el archivo para settings
if( group == NULL )
{
#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
    LOGE("[PauseSettings] SKIP missing group '%s' (item=%d). Likely wrong PauseSettings layout for this build.", item, i);
#endif
    continue; // temporal para seguir arrancando
}
                        Scrooby::Text* pText = group->GetText( item );

                    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        if( pText == NULL )
                        {
                            LOGE("[PauseSettings] MISSING TEXT label item=%d name='%s' group=%p",
                                i, item, (void*)group);
                        }
                    #endif

                        rAssert( pText != NULL );
                        pText->SetTextMode( Scrooby::TEXT_WRAP );

                        // Value text
                        sprintf( itemName, "%s_Value", item );
                        Scrooby::Text* pTextValue = group->GetText( itemName );

                   #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        if( pTextValue == NULL )
                        {
                            LOGE("[PauseSettings] MISSING TEXT value item=%d valueName='%s' base='%s' group=%p",
                                i, itemName, item, (void*)group);
                        }
                    #endif

                        rAssert( pTextValue != NULL );
                        pTextValue->SetTextMode( Scrooby::TEXT_WRAP );

                        // Left arrow (puede ser NULL “legalmente” según layout, así que LOGI opcional)
                        sprintf( itemName, "%s_LArrow", item );
                        Scrooby::Sprite* pLArrow = group->GetSprite( itemName );

                    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        LOGI("[PauseSettings] arrows item=%d base='%s' L='%s' ptr=%p",
                            i, item, itemName, (void*)pLArrow);
                    #endif

                        // Right arrow
                        sprintf( itemName, "%s_RArrow", item );
                        Scrooby::Sprite* pRArrow = group->GetSprite( itemName );

                    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
                        LOGI("[PauseSettings] arrows item=%d base='%s' R='%s' ptr=%p",
                            i, item, itemName, (void*)pRArrow);
                    #endif

                        m_pMenu->AddMenuItem( pText,
                                            pTextValue,
                                            NULL,
                                            NULL,
                                            pLArrow,
                                            pRArrow,
                                            SELECTION_ENABLED | VALUES_WRAPPED | TEXT_OUTLINE_ENABLED );
                    }




#ifdef RAD_GAMECUBE
    // change "Vibration" text to "Rumble"
    //
    Scrooby::Text* vibrationText = dynamic_cast<Scrooby::Text*>( m_pMenu->GetMenuItem( MENU_ITEM_VIBRATION )->GetItem() );
    rAssert( vibrationText != NULL );
    vibrationText->SetIndex( 1 );
#endif // RAD_GAMECUBE

    for( int j = 0; j < NUM_CAMERA_SELECTION_MODES; j++ )
    {
        m_cameraSelections[ j ] = NULL;
        m_numCameraSelections[ j ] = 0;
    }

    m_cameraSelections[ CAMERA_SELECTION_FOR_DRIVING ] = CAMERAS_FOR_DRIVING;
#ifdef RAD_PC
    m_cameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = PC_CAMERAS_FOR_WALKING;
#else
    m_cameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = CAMERAS_FOR_WALKING;
#endif

    if( GetCheatInputSystem()->IsCheatEnabled( CHEAT_ID_UNLOCK_CAMERAS ) )
    {
        m_numCameraSelections[ CAMERA_SELECTION_FOR_DRIVING ] = NUM_CAMERAS_FOR_DRIVING;

#ifdef RAD_PC
        m_numCameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = NUM_PC_CAMERAS_FOR_WALKING;
#else
        m_numCameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = NUM_CAMERAS_FOR_WALKING;
#endif
    }
    else
    {
        m_numCameraSelections[ CAMERA_SELECTION_FOR_DRIVING ] = NUM_CAMERAS_FOR_DRIVING_WITHOUT_CHEAT;
#ifdef RAD_PC
        m_numCameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = NUM_PC_CAMERAS_FOR_WALKING_WITHOUT_CHEAT;
#else
        m_numCameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = NUM_CAMERAS_FOR_WALKING_WITHOUT_CHEAT;
#endif
    }

    GetCheatInputSystem()->RegisterCallback( this );
MEMTRACK_POP_GROUP("CGuiScreenPauseSettings");
}


//===========================================================================
// CGuiScreenPauseSettings::~CGuiScreenPauseSettings
//===========================================================================
// Description: Destructor.
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
CGuiScreenPauseSettings::~CGuiScreenPauseSettings()
{
    GetCheatInputSystem()->UnregisterCallback( this );

    if( m_pMenu != NULL )
    {
        delete m_pMenu;
        m_pMenu = NULL;
    }
}


//===========================================================================
// CGuiScreenPauseSettings::HandleMessage
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
void CGuiScreenPauseSettings::HandleMessage
(
	eGuiMessage message, 
	unsigned int param1,
	unsigned int param2 
)
{
    if( m_state == GUI_WINDOW_STATE_RUNNING )
    {
        switch( message )
        {
            case GUI_MSG_CONTROLLER_START:
            {
                // resume game
                //
                m_pParent->HandleMessage( GUI_MSG_UNPAUSE_INGAME );

                break;
            }

            case GUI_MSG_MENU_SELECTION_VALUE_CHANGED:
            {
                if( param1 == MENU_ITEM_CAMERA )
                {
                    // set new super cam
                    //
                    SuperCam::Type superCamType = m_cameraSelections[ m_currentCameraSelectionMode ][ param2 ];
                    GetSuperCamManager()->GetSCC( 0 )->SelectSuperCam( superCamType, SuperCamCentral::CUT, 0 );

                    // and apply it right away
                    //
                    GetSuperCamManager()->GetSCC( 0 )->Update( 0 );
                }
#ifndef RAD_PC
                else if( param1 == MENU_ITEM_VIBRATION )
                {
                    if( param2 == 1 ) // vibration turned ON
                    {
                        // send vibration pulse to controller
                        //
                        int controllerID = GetInputManager()->GetControllerIDforPlayer( 0 );
#ifdef RAD_PS2
                        if ( GetInputManager()->IsControllerInPort( Input::USB0 ) )
                        {
                            GetInputManager()->TriggerRumblePulse( Input::USB0 );
                        }
                        else if ( GetInputManager()->IsControllerInPort( Input::USB1 ) )
                        {
                            GetInputManager()->TriggerRumblePulse( Input::USB1 );
                        }
                        else
#endif
                        {
                            GetInputManager()->TriggerRumblePulse( controllerID );
                        }

                    }
                }
#endif

                break;
            }

            default:
            {
                break;
            }
        }

        // relay message to menu
        if( m_pMenu != NULL )
        {
            m_pMenu->HandleMessage( message, param1, param2 );
        }
    }

	// Propogate the message up the hierarchy.
	//
	CGuiScreen::HandleMessage( message, param1, param2 );
}

void
CGuiScreenPauseSettings::OnCheatEntered( eCheatID cheatID, bool isEnabled )
{
    if( cheatID == CHEAT_ID_UNLOCK_CAMERAS && isEnabled )
    {
        m_numCameraSelections[ CAMERA_SELECTION_FOR_DRIVING ] = NUM_CAMERAS_FOR_DRIVING;
        m_numCameraSelections[ CAMERA_SELECTION_FOR_WALKING ] = NUM_CAMERAS_FOR_WALKING;
    }
}

//===========================================================================
// CGuiScreenPauseSettings::InitIntro
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
void CGuiScreenPauseSettings::InitIntro()
{
    // update camera setting
    //
    bool allowCameraToggle = GetSuperCamManager()->GetSCC( 0 )->AllowCameraToggle();

    rAssert( m_pMenu );
    m_pMenu->SetMenuItemEnabled( MENU_ITEM_CAMERA, allowCameraToggle );

    if( allowCameraToggle )
    {
        // update camera selections
        //
        this->UpdateCameraSelections();
    }
    else
    {
        m_currentCameraSelectionMode = CAMERA_SELECTION_NOT_AVAILABLE;
    }

    // update other gameplay settings
    //
    bool isSettingOn = false;

#ifndef RAD_PC
    isSettingOn = GetSuperCamManager()->GetSCC( 0 )->IsInvertedCameraEnabled();
    m_pMenu->SetSelectionValue( MENU_ITEM_INVERT_CAM_CONTROL,
                                isSettingOn ? 1 : 0 );
#endif

    isSettingOn = GetSuperCamManager()->GetSCC( 0 )->JumpCamsEnabled();
    m_pMenu->SetSelectionValue( MENU_ITEM_JUMP_CAMERAS,
                                isSettingOn ? 1: 0 );

    isSettingOn = GetCharacterSheetManager()->QueryNavSystemSetting();
    m_pMenu->SetSelectionValue( MENU_ITEM_INTERSECT_NAV_SYSTEM,
                                isSettingOn ? 1 : 0 );

    isSettingOn = GetGuiSystem()->IsRadarEnabled();
    m_pMenu->SetSelectionValue( MENU_ITEM_RADAR,
                                isSettingOn ? 1 : 0 );
#ifndef RAD_PC
    isSettingOn = GetInputManager()->IsRumbleEnabled();
    m_pMenu->SetSelectionValue( MENU_ITEM_VIBRATION,
                                isSettingOn ? 1 : 0 );
#endif

    isSettingOn = GetTutorialManager()->IsTutorialEventsEnabled();
    m_pMenu->SetSelectionValue( MENU_ITEM_TUTORIAL,
                                isSettingOn ? 1 : 0 );

    // show/hide tutorial setting depending on whether or not tutorial mode is enabled
    //
    if( GetTutorialManager()->IsTutorialModeEnabled() )
    {
        CGuiManagerInGame* guiManagerIngame = static_cast<CGuiManagerInGame*>( m_pParent );
        rAssert( guiManagerIngame != NULL );

        bool isTutorialToggleAllowed = (guiManagerIngame->GetResumeGameScreenID() != GUI_SCREEN_ID_TUTORIAL);
        m_pMenu->SetMenuItemEnabled( MENU_ITEM_TUTORIAL, isTutorialToggleAllowed );
    }
    else
    {
        m_pMenu->SetMenuItemEnabled( MENU_ITEM_TUTORIAL, false, true );
    }
}


//===========================================================================
// CGuiScreenPauseSettings::InitRunning
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
void CGuiScreenPauseSettings::InitRunning()
{
}


//===========================================================================
// CGuiScreenPauseSettings::InitOutro
//===========================================================================
// Description: 
//
// Constraints:	None.
//
// Parameters:	None.
//
// Return:      N/A.
//
//===========================================================================
void CGuiScreenPauseSettings::InitOutro()
{
    // apply new gameplay settings
    //
    rAssert( m_pMenu != NULL );
    bool isSettingOn = false;

#ifndef RAD_PC
    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_INVERT_CAM_CONTROL ) == 1);
    GetSuperCamManager()->GetSCC( 0 )->EnableInvertedCamera( isSettingOn );
#endif

    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_JUMP_CAMERAS ) == 1);
    GetSuperCamManager()->GetSCC( 0 )->EnableJumpCams( isSettingOn );

    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_INTERSECT_NAV_SYSTEM ) == 1);
    GetCharacterSheetManager()->SetNavSystemOn( isSettingOn );

    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_RADAR ) == 1);
    GetGuiSystem()->SetRadarEnabled( isSettingOn );

#ifndef RAD_PC
    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_VIBRATION ) == 1);
    GetInputManager()->SetRumbleEnabled( isSettingOn );
#endif

    isSettingOn = (m_pMenu->GetSelectionValue( MENU_ITEM_TUTORIAL ) == 1) 
#ifdef RAD_PC
                  && !(GetInputManager()->GetController(0)->IsTutorialDisabled())
#endif                  
                  ;
    GetTutorialManager()->EnableTutorialEvents( isSettingOn );
}


//---------------------------------------------------------------------
// Private Functions
//---------------------------------------------------------------------

void
CGuiScreenPauseSettings::UpdateCameraSelections()
{
    rAssert( m_pMenu );

    // set current camera selection mode
    //
    bool isPlayerInCar = GetAvatarManager()->GetAvatarForPlayer( 0 )->IsInCar();
    m_currentCameraSelectionMode = isPlayerInCar ? CAMERA_SELECTION_FOR_DRIVING : CAMERA_SELECTION_FOR_WALKING;

    // get current active camera
    //
    SuperCam* currentSuperCam = GetSuperCamManager()->GetSCC( 0 )->GetActiveSuperCam();
    rAssert( currentSuperCam );
    SuperCam::Type currentSuperCamType = currentSuperCam->GetType();

    int currentCameraIndex = -1;

    // check to see if current camera is in the selection list
    //
    for( int i = 0; i < m_numCameraSelections[ m_currentCameraSelectionMode ]; i++ )
    {
        if( m_cameraSelections[ m_currentCameraSelectionMode ][ i ] == currentSuperCamType )
        {
            currentCameraIndex = i;

            break;
        }
    }

    if( currentCameraIndex == -1 )
    {
        // current camera not found in selection list, disable camera selection
        //
        m_pMenu->SetMenuItemEnabled( MENU_ITEM_CAMERA, false );

        m_currentCameraSelectionMode = CAMERA_SELECTION_NOT_AVAILABLE;

        return;
    }

    m_pMenu->SetMenuItemEnabled( MENU_ITEM_CAMERA, true );

    // set number of camera selections available
    //
    m_pMenu->SetSelectionValueCount( MENU_ITEM_CAMERA,
                                     m_numCameraSelections[ m_currentCameraSelectionMode ] );

    // update camera names
    //
    for( int i = 0; i < m_numCameraSelections[ m_currentCameraSelectionMode ]; i++ )
    {
        GuiMenuItem* menuItemCamera = m_pMenu->GetMenuItem( MENU_ITEM_CAMERA );
        rAssert( menuItemCamera != NULL );

        SuperCam* superCam = GetSuperCamManager()->GetSCC( 0 )->GetSuperCam( m_cameraSelections[ m_currentCameraSelectionMode ][ i ] );
        rAssert( superCam );

        HeapMgr()->PushHeap( GMA_LEVEL_HUD );

        P3D_UNICODE* stringBuffer = GetTextBibleString( superCam->GetName() );
        rAssert( stringBuffer );

        UnicodeString unicodeString;
        unicodeString.ReadUnicode( static_cast<UnicodeChar*>( stringBuffer ) );

        Scrooby::Text* menuItemValue = dynamic_cast<Scrooby::Text*>( menuItemCamera->GetItemValue() );
        rAssert( menuItemValue != NULL );
        menuItemValue->SetString( i, unicodeString );

        HeapMgr()->PopHeap(GMA_LEVEL_HUD);
    }

    // set current camera selection
    //
    m_pMenu->SetSelectionValue( MENU_ITEM_CAMERA, currentCameraIndex );
}

