//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   CGuiScreenPauseOptions
//
// Description: Implementation of the CGuiScreenPauseOptions class.
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
#include <presentation/gui/ingame/guiscreenpauseoptions.h>
#include <presentation/gui/guimenu.h>

#include <cheats/cheatinputsystem.h>
#include <memory/srrmemory.h>

#include <raddebug.hpp> // Foundation
#include <Group.h>
#include <Screen.h>
#include <Page.h>
#include <Text.h>

//===========================================================================
// Global Data, Local Data, Local Classes
//===========================================================================

enum ePauseMenuItem
{
#ifdef RAD_PC
    MENU_ITEM_DISPLAY,
#endif
    MENU_ITEM_CONTROLLER,
    MENU_ITEM_SOUND,

#ifndef RAD_ANDROID
    MENU_ITEM_SETTINGS,
#endif
//    MENU_ITEM_CAMERA,

    NUM_PAUSE_MENU_ITEMS
};


static const char* PAUSE_MENU_ITEMS[] =
{
#ifdef RAD_PC
    "Display",
#endif
    "Controller",
    "Sound",

#ifndef RAD_ANDROID
    "Settings",
#endif
//    "Camera",

    ""
};

//===========================================================================
// Public Member Functions
//===========================================================================

//===========================================================================
// CGuiScreenPauseOptions::CGuiScreenPauseOptions
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
CGuiScreenPauseOptions::CGuiScreenPauseOptions
(
    Scrooby::Screen* pScreen,
    CGuiEntity* pParent
)
:   CGuiScreen( pScreen, pParent, GUI_SCREEN_ID_OPTIONS ),
    m_pMenu( NULL )
{
MEMTRACK_PUSH_GROUP( "CGUIScreenPauseOptions" );
    // Retrieve the Scrooby drawing elements.
    //
    Scrooby::Page* pPage;
	pPage = m_pScroobyScreen->GetPage( "PauseOptions" );
	rAssert( pPage );

    // Create a menu.
    //
    m_pMenu = new(GMA_LEVEL_HUD) CGuiMenu( this, NUM_PAUSE_MENU_ITEMS );
    rAssert( m_pMenu != NULL );

    // Add menu items
    //
    Scrooby::Group* menu = pPage->GetGroup( "Menu" );
    rAssert( menu != NULL );
    char itemName[ 32 ];
    for( int i = 0; i < NUM_PAUSE_MENU_ITEMS; i++ )
    {
        sprintf( itemName, "%s_Value", PAUSE_MENU_ITEMS[ i ] );
        Scrooby::Text* pTextValue = pPage->GetText( itemName );

        sprintf( itemName, "%s_LArrow", PAUSE_MENU_ITEMS[ i ] );
        Scrooby::Sprite* pLArrow = pPage->GetSprite( itemName );

        sprintf( itemName, "%s_RArrow", PAUSE_MENU_ITEMS[ i ] );
        Scrooby::Sprite* pRArrow = pPage->GetSprite( itemName );

        m_pMenu->AddMenuItem( menu->GetText( PAUSE_MENU_ITEMS[ i ] ),
                              pTextValue,
                              NULL,
                              NULL,
                              pLArrow,
                              pRArrow );
    }

    // NUEVO BLOQUE PARA ELIMINAR opciones->configuracion para android y asi evitar un crash accidental
#ifdef RAD_ANDROID
    Scrooby::Text* pSettings = menu->GetText( "Settings" );
    if( pSettings != NULL )
    {
        pSettings->SetVisible( false );
    }

    Scrooby::Text* pSettingsValue = pPage->GetText( "Settings_Value" );
    if( pSettingsValue != NULL )
    {
        pSettingsValue->SetVisible( false );
    }

    Scrooby::Sprite* pSettingsLArrow = pPage->GetSprite( "Settings_LArrow" );
    if( pSettingsLArrow != NULL )
    {
        pSettingsLArrow->SetVisible( false );
    }

    Scrooby::Sprite* pSettingsRArrow = pPage->GetSprite( "Settings_RArrow" );
    if( pSettingsRArrow != NULL )
    {
        pSettingsRArrow->SetVisible( false );
    }
#endif

    // FIN NUEVO BLOQUE 

#ifndef RAD_PC
    Scrooby::Text* pText = menu->GetText( "Display" );
    if( pText )
        pText->SetVisible( false );

    // re-center menu items
    //
    menu->ResetTransformation();
    menu->Translate( 0, 50 );
#endif

    // TC: [TEMP] disable controller screen for now to free up some memory for HUD map
    //
#ifndef RAD_PC
    m_pMenu->SetMenuItemEnabled( MENU_ITEM_CONTROLLER, false, true );
#endif

#ifdef RAD_E3
    // disable pause menu settings for E3 build
    //
    m_pMenu->SetMenuItemEnabled( MENU_ITEM_SETTINGS, false );
#endif
MEMTRACK_POP_GROUP("CGUIScreenPauseOptions");
}


//===========================================================================
// CGuiScreenPauseOptions::~CGuiScreenPauseOptions
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
CGuiScreenPauseOptions::~CGuiScreenPauseOptions()
{
    if( m_pMenu != NULL )
    {
        delete m_pMenu;
        m_pMenu = NULL;
    }
}


//===========================================================================
// CGuiScreenPauseOptions::HandleMessage
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
void CGuiScreenPauseOptions::HandleMessage
(
	eGuiMessage message, 
	unsigned int param1,
	unsigned int param2 
)
{
    if( this->IsControllerMessage( message ) &&
        GetCheatInputSystem()->IsActivated( param1 ) )
    {
        // ignore all controller inputs when cheat input system is activated
        return;
    }

    if( m_state == GUI_WINDOW_STATE_RUNNING )
    {
        switch( message )
        {
            case GUI_MSG_CONTROLLER_START:
            {
                if( !m_pMenu->HasSelectionBeenMade() )
                {
                    // resume game
                    m_pParent->HandleMessage( GUI_MSG_UNPAUSE_INGAME );
                }

                break;
            }

            case GUI_MSG_MENU_SELECTION_MADE:
            {
                if( param1 == MENU_ITEM_CONTROLLER )
                {
                    m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_CONTROLLER );
                }
                else if( param1 == MENU_ITEM_SOUND )
                {
                    m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_SOUND );
                }
                #ifndef RAD_ANDROID
                else if( param1 == MENU_ITEM_SETTINGS )
                {
                     m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_SETTINGS );
                }
                #endif
#ifdef RAD_PC
                else if( param1 == MENU_ITEM_DISPLAY )
                {
                    m_pParent->HandleMessage( GUI_MSG_GOTO_SCREEN, GUI_SCREEN_ID_DISPLAY );
                }
#endif
                else
                {
                    rAssertMsg( false, "Invalid menu selection!" );
                }

                break;
            }
/*
            case GUI_MSG_MENU_SELECTION_CHANGED:
            {
                this->SetButtonVisible( BUTTON_ICON_ACCEPT, (param1 != MENU_ITEM_CAMERA) );

                break;
            }
*/
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


//===========================================================================
// CGuiScreenPauseOptions::InitIntro
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
void CGuiScreenPauseOptions::InitIntro()
{
//    this->SetButtonVisible( BUTTON_ICON_ACCEPT, (m_pMenu->GetSelection() != MENU_ITEM_CAMERA) );

#ifndef RAD_E3
    GetCheatInputSystem()->SetEnabled( true );
#endif
}


//===========================================================================
// CGuiScreenPauseOptions::InitRunning
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
void CGuiScreenPauseOptions::InitRunning()
{
}


//===========================================================================
// CGuiScreenPauseOptions::InitOutro
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
void CGuiScreenPauseOptions::InitOutro()
{
#ifndef RAD_E3
    GetCheatInputSystem()->SetEnabled( false );
#endif
}


//---------------------------------------------------------------------
// Private Functions
//---------------------------------------------------------------------

