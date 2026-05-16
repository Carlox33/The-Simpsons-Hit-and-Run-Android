//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   CGuiScreenPauseMission
//
// Description: Implementation of the CGuiScreenPauseMission class.
//
// Authors:     Darwin Chau
//              Tony Chu
//
// Revisions		Date			Author	    Revision
//                  2000/11/20		DChau		Created
//                  2002/06/06      TChu        Modified for SRR2
//
//===========================================================================

//===========================================================================
// Includes
//===========================================================================
#include <presentation/gui/ingame/guiscreenmissionload.h>
#include <presentation/gui/ingame/guiscreenpausemission.h>
#include <presentation/gui/ingame/guimanageringame.h>
#include <presentation/gui/ingame/guiscreenhud.h>
#include <presentation/gui/guimenu.h>
#include <presentation/gui/guiscreenprompt.h>
#include <presentation/gui/guisystem.h>
#include <events/eventmanager.h>
#include <memory/srrmemory.h>
#include <mission/gameplaymanager.h>
#include <mission/mission.h>
#include <worldsim/hitnrunmanager.h>
#include <worldsim/avatarmanager.h>
#include <raddebug.hpp> // Foundation
#include <Group.h>
#include <Page.h>
#include <Screen.h>
#include <Text.h>
#include <Sprite.h>

//===========================================================================
// Global Data, Local Data, Local Classes
//===========================================================================

enum eMenuItem
{
    MENU_ITEM_CONTINUE,
//    MENU_ITEM_VIEW_MAP,
    MENU_ITEM_RESTART_MISSION,
    MENU_ITEM_ABORT_MISSION,
    MENU_ITEM_OPTIONS,
    MENU_ITEM_QUIT_GAME,
#ifdef RAD_PC
    MENU_ITEM_EXIT_GAME,
#endif

    NUM_MENU_ITEMS
};

static const char* MENU_ITEMS[] = 
{
    "Continue",
//    "ViewMap",
    "RestartMission",
    "AbortMission",
    "Options",
    "QuitGame"
#ifdef RAD_PC
    ,"ExitToSystem"
#endif
};

//===========================================================================
// Public Member Functions
//===========================================================================

//===========================================================================
// CGuiScreenPauseMission::CGuiScreenPauseMission
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
CGuiScreenPauseMission::CGuiScreenPauseMission
(
	Scrooby::Screen* pScreen,
	CGuiEntity* pParent
)
:	
	CGuiScreenPause( pScreen, pParent, GUI_SCREEN_ID_PAUSE_MISSION )
{
MEMTRACK_PUSH_GROUP( "CGUIScreenPauseMission" );
    // Create a menu.
    //
    m_pMenu = new(GMA_LEVEL_HUD) CGuiMenu( this, NUM_MENU_ITEMS );
    rAssert( m_pMenu != NULL );

    // Retrieve the Scrooby drawing elements.
    //
    Scrooby::Page* pPage = m_pScroobyScreen->GetPage( "PauseMission" );
    rAssert( pPage != NULL );

    // Add menu items
    //
    Scrooby::Group* menu = pPage->GetGroup( "Menu" );
    rAssert( menu != NULL );
    Scrooby::Text* pText = NULL;
    for( int i = 0; i < NUM_MENU_ITEMS; i++ )
    {
        pText = menu->GetText( MENU_ITEMS[ i ] );
#ifdef RAD_PC
        rAssert( pText || i == MENU_ITEM_EXIT_GAME );
        if( !pText )
            continue;
#else
        rAssert( pText );
#endif

        m_pMenu->AddMenuItem( pText );
    }

#ifndef RAD_PC
    pText = menu->GetText( "ExitToSystem" );
    if( pText )
        pText->SetVisible( false );

    // re-center menu items
    //
    menu->ResetTransformation();
    menu->Translate( 0, -20 );
#endif
MEMTRACK_POP_GROUP("CGUIScreenPauseMission" );
}


//===========================================================================
// CGuiScreenPauseMission::~CGuiScreenPauseMission
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
CGuiScreenPauseMission::~CGuiScreenPauseMission()
{
}


//===========================================================================
// CGuiScreenPauseMission::HandleMessage
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
void CGuiScreenPauseMission::HandleMessage
(
	eGuiMessage message, 
	unsigned int param1,
	unsigned int param2 
)
{
    if( message == GUI_MSG_MENU_PROMPT_RESPONSE )
    {
        if( param1 == PROMPT_CONFIRM_RESTART )
        {
            switch( param2 )
            {
                case (CGuiMenuPrompt::RESPONSE_YES):
                {
                    CGuiScreenMissionLoad::ReplaceBitmap();
                    this->HandleResumeGame( ON_HUD_ENTER_RESTART_MISSION );
                    break;
                }

                case (CGuiMenuPrompt::RESPONSE_NO):
                {
                    this->ReloadScreen();

                    break;
                }

                default:
                {
                    rAssertMsg( 0, "WARNING: *** Invalid prompt response!\n" );

                    break;
                }
            }
        }
        else if( param1 == PROMPT_CONFIRM_ABORT )
        {
            switch( param2 )
            {
                case (CGuiMenuPrompt::RESPONSE_YES):
                {
                    this->HandleResumeGame( ON_HUD_ENTER_ABORT_MISSION );
                    GetEventManager()->TriggerEvent(EVENT_USER_CANCEL_PAUSE_MENU);              

                    break;
                }

                case (CGuiMenuPrompt::RESPONSE_NO):
                {
                    this->ReloadScreen();

                    break;
                }

                default:
                {
                    rAssertMsg( 0, "WARNING: *** Invalid prompt response!\n" );

                    break;
                }
            }
        }
    }

    if( m_state == GUI_WINDOW_STATE_RUNNING )
    {
        switch( message )
        {
            case GUI_MSG_MENU_SELECTION_MADE:	
            {
                switch( param1 )
                {
                    case MENU_ITEM_CONTINUE:
                    {
                        this->HandleResumeGame();

                        break;
                    }
/*
                    case MENU_ITEM_VIEW_MAP:
                    {
                        m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_HUD_MAP );

                        break;
                    }
*/
                    case MENU_ITEM_RESTART_MISSION:
                    {
                        rAssert( m_guiManager );
                        m_guiManager->DisplayPrompt( PROMPT_CONFIRM_RESTART, this );

//                        this->HandleResumeGame( ON_HUD_ENTER_RESTART_MISSION );

                        break;
                    }
                    case MENU_ITEM_ABORT_MISSION:
                    {
                        rAssert( m_guiManager );
                        m_guiManager->DisplayPrompt( PROMPT_CONFIRM_ABORT, this );

//                        this->HandleResumeGame( ON_HUD_ENTER_ABORT_MISSION );

                        break;
                    }
                    case MENU_ITEM_OPTIONS:
                    {
                        // Linea original
                        m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_OPTIONS );
                        // fin linea original

                        
                        break;
                    }
                    case MENU_ITEM_QUIT_GAME:
                    {
                        this->HandleQuitGame();

                        break;
                    }
#ifdef RAD_PC
                    case MENU_ITEM_EXIT_GAME:
                    {
                        this->HandleQuitToSystem();

                        break;
                    }
#endif
                    default:
                    {
                        rAssertMsg( 0, "WARNING: Invalid case for switch statement!\n" );
                        break;
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }
    }

	// Propogate the message up the hierarchy.
	//
	CGuiScreenPause::HandleMessage( message, param1, param2 );
}


//===========================================================================
// CGuiScreenPauseMission::InitIntro
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
void CGuiScreenPauseMission::InitIntro()
{
    CGuiScreenPause::InitIntro();

    // enable 'restart mission' and 'abort mission' only if current mission
    // isn't completed yet; otherwise, disable these selections
    //
    Mission* currentMission = GetGameplayManager()->GetCurrentMission();
    rAssert( currentMission != NULL );

    HitnRunManager* hnrm = GetHitnRunManager();
    rAssert( hnrm != NULL );

    AvatarManager* pAvatarManager= GetAvatarManager();
    rAssert( pAvatarManager != NULL );

    CGuiManagerInGame* guiManagerIngame = static_cast<CGuiManagerInGame*>( m_pParent );
    rAssert( guiManagerIngame != NULL );

    bool isRestartAndAbortAllowed = !currentMission->IsSundayDrive() &&
                                    !currentMission->IsComplete() &&
                                    !hnrm->BustingPlayer() &&
                                    !pAvatarManager->IsAvatarGettingInOrOutOfCar(0) &&
                                    (guiManagerIngame->GetResumeGameScreenID() != GUI_SCREEN_ID_TUTORIAL);

    rAssert( m_pMenu != NULL );
    m_pMenu->SetMenuItemEnabled( MENU_ITEM_RESTART_MISSION, isRestartAndAbortAllowed );
    m_pMenu->SetMenuItemEnabled( MENU_ITEM_ABORT_MISSION, isRestartAndAbortAllowed );
}


//===========================================================================
// CGuiScreenPauseMission::InitRunning
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
void CGuiScreenPauseMission::InitRunning()
{
    CGuiScreenPause::InitRunning();
}


//===========================================================================
// CGuiScreenPauseMission::InitOutro
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
void CGuiScreenPauseMission::InitOutro()
{
    CGuiScreenPause::InitOutro();
}


//---------------------------------------------------------------------
// Private Functions
//---------------------------------------------------------------------
