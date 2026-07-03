#include <input/touch/touchcontextresolver.h>

#include <contexts/contextenum.h>
#include <input/controller.h>
#include <presentation/presentation.h>
#include <presentation/fmvplayer/fmvplayer.h>
#include <mission/gameplaymanager.h>
#include <mission/mission.h>
#include <mission/missionstage.h>
#include <mission/objectives/missionobjective.h>

// includes para contextos del minijuego
#include <gameflow/gameflow.h>
#include <contexts/supersprint/supersprintcontext.h>
//=============================================================================
// Singleton access
//=============================================================================

TouchContextResolver& TouchContextResolver::GetInstance()
{
    static TouchContextResolver sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchContextResolver::TouchContextResolver()
:
mContextArea( TOUCH_CONTEXT_AREA_HIDDEN ),
mGameplayProfile( TOUCH_PROFILE_CHARACTER ),
mHasForcedProfile( false ),
mForcedProfile( TOUCH_PROFILE_HIDDEN ),
mMissionBriefingActive( false ),
mGameplayConversationActive( false ),
mPurchaseRewardConversationActive( false ),
mLanguageSelectionActive( false ),
mSuspendedSuperSprintProfile( TOUCH_PROFILE_FRONTEND )
{
}

TouchContextResolver::~TouchContextResolver()
{
}

//=============================================================================
// Public API
//=============================================================================

void TouchContextResolver::Reset()
{
    mContextArea = TOUCH_CONTEXT_AREA_HIDDEN;
    mGameplayProfile = TOUCH_PROFILE_CHARACTER;
    mHasForcedProfile = false;
    mForcedProfile = TOUCH_PROFILE_HIDDEN;
    mMissionBriefingActive = false;
    mGameplayConversationActive = false;
    mPurchaseRewardConversationActive = false;
    mLanguageSelectionActive = false;
    mSuspendedSuperSprintProfile = TOUCH_PROFILE_FRONTEND;
}

TouchProfile TouchContextResolver::Resolve() const
{
    /*
     * Forced profile keeps maximum priority.
     *
     * After that, resolution order is:
     * 0. Check forced Profile 
     * 1. Normal gameplay.
     * 2. SuperSprint / minigame.
     * 3. Language selection screen.
     * 4. Other context areas.
     */

    if ( mHasForcedProfile )
    {
        return mForcedProfile;
    }

    GameFlow* gameFlow = GetGameFlow();

    int currentContext = -1;

    if ( gameFlow != 0 )
    {
        currentContext = gameFlow->GetCurrentContext();
    }

    TouchContextArea resolveCase = TOUCH_CONTEXT_AREA_HIDDEN;

    if
    (
        mContextArea == TOUCH_CONTEXT_AREA_GAMEPLAY &&
        currentContext != CONTEXT_SUPERSPRINT &&
        currentContext != CONTEXT_SUPERSPRINT_FE
    )
    {
        resolveCase = TOUCH_CONTEXT_AREA_GAMEPLAY;
    }
    else if ( currentContext == CONTEXT_SUPERSPRINT_FE )
    {
        resolveCase = TOUCH_CONTEXT_AREA_SUPERSPRINT_FE;
    }
    else if ( currentContext == CONTEXT_SUPERSPRINT )
    {
        resolveCase = TOUCH_CONTEXT_AREA_SUPERSPRINT;
    }
    else if ( IsLanguageSelectionActive() )
    {
        resolveCase = TOUCH_CONTEXT_AREA_LANGUAGE_SELECTION;
    }
    else
    {
        switch ( mContextArea )
        {
            case TOUCH_CONTEXT_AREA_HIDDEN:
            {
                resolveCase = TOUCH_CONTEXT_AREA_HIDDEN;
                break;
            }

            case TOUCH_CONTEXT_AREA_FRONTEND:
            {
                resolveCase = TOUCH_CONTEXT_AREA_FRONTEND;
                break;
            }

            case TOUCH_CONTEXT_AREA_GAMEPLAY:
            {
                resolveCase = TOUCH_CONTEXT_AREA_GAMEPLAY;
                break;
            }

            case TOUCH_CONTEXT_AREA_CINEMATIC:
            {
                resolveCase = TOUCH_CONTEXT_AREA_CINEMATIC;
                break;
            }

            case TOUCH_CONTEXT_AREA_SPECIAL:
            {
                resolveCase = TOUCH_CONTEXT_AREA_SPECIAL;
                break;
            }

            default:
            {
                resolveCase = TOUCH_CONTEXT_AREA_HIDDEN;
                break;
            }
        }
    }

    switch ( resolveCase )
    {
        case TOUCH_CONTEXT_AREA_GAMEPLAY:
        {
            if ( IsValidGameplayProfile( mGameplayProfile ) )
            {
                return mGameplayProfile;
            }

            return TOUCH_PROFILE_CHARACTER;
        }

        case TOUCH_CONTEXT_AREA_SUPERSPRINT_FE:
        {
            return TOUCH_PROFILE_FRONTEND;
        }

        case TOUCH_CONTEXT_AREA_SUPERSPRINT:
        {
            SuperSprintContext* superSprintContext =
                static_cast<SuperSprintContext*>( gameFlow->GetContext( CONTEXT_SUPERSPRINT ) );

            if ( superSprintContext != 0 && superSprintContext->IsSuspended() )
            {
                return mSuspendedSuperSprintProfile;
            }

            return TOUCH_PROFILE_MINIGAME_VEHICLE;
        }

        case TOUCH_CONTEXT_AREA_LANGUAGE_SELECTION:
        {
            return TOUCH_PROFILE_FRONTEND;
        }

        case TOUCH_CONTEXT_AREA_HIDDEN:
        {
            return TOUCH_PROFILE_HIDDEN;
        }

        case TOUCH_CONTEXT_AREA_FRONTEND:
        {
            return TOUCH_PROFILE_FRONTEND;
        }

        case TOUCH_CONTEXT_AREA_CINEMATIC:
        {
            return TOUCH_PROFILE_CINEMATIC;
        }

        case TOUCH_CONTEXT_AREA_SPECIAL:
        {
            return TOUCH_PROFILE_SPECIAL;
        }

        default:
        {
            return TOUCH_PROFILE_HIDDEN;
        }
    }
}

void TouchContextResolver::UpdateFromGameState
(
    int gameContext,
    unsigned inputGameState,
    bool avatarInVehicle
)
{
    /*
     * Animated camera / world-load intro / focus camera.
     *
     * We hide touch controls here because this state is not always a real
     * skippable cinematic. It can also happen when the level loads and the
     * camera focuses the player.
     */
    if ( inputGameState == Input::ACTIVE_ANIM_CAM )
    {
        SetHiddenContext();
        return;
    }

#if defined(RAD_ANDROID)
    /*
     * Real FMV/cinematic.
     *
     * This uses the cinematic touch profile, where we render only the
     * cinematic skip button from RenderManager.
     */
    PresentationManager* pm = GetPresentationManager();

    if ( pm != 0 && pm->GetFMVPlayer() != 0 )
    {
        if ( pm->GetFMVPlayer()->IsPlaying() )
        {
            SetCinematicContext();
            return;
        }
    }

    /*
     * Gameplay frontend-like states.
     *
     * Some scenes keep CONTEXT_GAMEPLAY, but the player does not have free
     * character control:
     *
     * - mission dialogue objectives
     * - mission briefing / race briefing screens
     * - bonus mission conversations
     *
     * In those cases we use the frontend/menu touch layout instead of the
     * character layout.
     */
    if ( gameContext == CONTEXT_GAMEPLAY && IsGameplayFrontendLikeState() )
    {
        SetFrontendContext();
        return;
    }
    
#endif

    switch ( gameContext )
    {
        case CONTEXT_FRONTEND:
        case CONTEXT_PAUSE:
        case CONTEXT_SUPERSPRINT_FE:
        {
            SetFrontendContext();
            break;
        }

        case CONTEXT_GAMEPLAY:
        {
            if ( avatarInVehicle )
            {
                SetVehicleContext();
            }
            else
            {
                SetCharacterContext();
            }

            break;
        }

        case CONTEXT_SUPERSPRINT:
        {
            SetSpecialContext();
            break;
        }

        case CONTEXT_ENTRY:
        case CONTEXT_BOOTUP:
        case CONTEXT_LOADING_DEMO:
        case CONTEXT_DEMO:
        case CONTEXT_LOADING_SUPERSPRINT:
        case CONTEXT_LOADING_GAMEPLAY:
        case CONTEXT_EXIT:
        default:
        {
            SetHiddenContext();
            break;
        }
    }
}

void TouchContextResolver::SetSuspendedSuperSprintProfile( TouchProfile profile )
{
    mSuspendedSuperSprintProfile = profile;
}

void TouchContextResolver::ClearSuspendedSuperSprintProfile()
{
    mSuspendedSuperSprintProfile = TOUCH_PROFILE_FRONTEND;
}

void TouchContextResolver::SetMissionBriefingActive( bool active )
{
    mMissionBriefingActive = active;
}

void TouchContextResolver::SetGameplayConversationActive( bool active )
{
    mGameplayConversationActive = active;
}

void TouchContextResolver::SetLanguageSelectionActive( bool active )
{
    mLanguageSelectionActive = active;
}

bool TouchContextResolver::IsLanguageSelectionActive() const
{
    return mLanguageSelectionActive;
}

void TouchContextResolver::SetPurchaseRewardConversationActive( bool active )
{
    mPurchaseRewardConversationActive = active;
}

bool TouchContextResolver::IsPurchaseRewardConversationActive() const
{
    return mPurchaseRewardConversationActive;
}

bool TouchContextResolver::IsGameplayFrontendLikeState() const
{
    // Agrupamos dialogo mision, dialogo mision bonus, dialogo conversacion de carrera y dialogo compra de vehiculo
    return IsMissionDialogueObjectiveActive() ||
           IsMissionBriefingActive() ||
           IsGameplayConversationActive()||  IsPurchaseRewardConversationActive();
}

bool TouchContextResolver::IsMissionDialogueObjectiveActive() const
{
    if ( GetGameplayManager() == 0 )
    {
        return false;
    }

    Mission* mission = GetGameplayManager()->GetCurrentMission();

    if ( mission == 0 )
    {
        return false;
    }

    MissionStage* stage = mission->GetCurrentStage();

    if ( stage == 0 )
    {
        return false;
    }

    MissionObjective* objective = stage->GetObjective();

    if ( objective == 0 )
    {
        return false;
    }

    return objective->GetObjectiveType() == MissionObjective::OBJ_DIALOGUE;
}

bool TouchContextResolver::IsMissionBriefingActive() const
{
    return mMissionBriefingActive;
}

bool TouchContextResolver::IsGameplayConversationActive() const
{
    return mGameplayConversationActive;
}

void TouchContextResolver::SetHiddenContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_HIDDEN;
}

void TouchContextResolver::SetFrontendContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_FRONTEND;
}

void TouchContextResolver::SetCharacterContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_GAMEPLAY;
    mGameplayProfile = TOUCH_PROFILE_CHARACTER;
}

void TouchContextResolver::SetVehicleContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_GAMEPLAY;
    mGameplayProfile = TOUCH_PROFILE_VEHICLE;
}

void TouchContextResolver::SetCinematicContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_CINEMATIC;
}

void TouchContextResolver::SetSpecialContext()
{
    mContextArea = TOUCH_CONTEXT_AREA_SPECIAL;
}

void TouchContextResolver::SetContextArea( TouchContextArea area )
{
    if ( IsValidContextArea( area ) )
    {
        mContextArea = area;
    }
    else
    {
        mContextArea = TOUCH_CONTEXT_AREA_HIDDEN;
    }
}

TouchContextArea TouchContextResolver::GetContextArea() const
{
    return mContextArea;
}

void TouchContextResolver::SetGameplayProfile( TouchProfile profile )
{
    if ( IsValidGameplayProfile( profile ) )
    {
        mGameplayProfile = profile;
    }
}

TouchProfile TouchContextResolver::GetGameplayProfile() const
{
    return mGameplayProfile;
}

void TouchContextResolver::SetForcedProfile( TouchProfile profile )
{
    if ( IsValidProfile( profile ) )
    {
        mForcedProfile = profile;
        mHasForcedProfile = true;
    }
    else
    {
        ClearForcedProfile();
    }
}

void TouchContextResolver::ClearForcedProfile()
{
    mHasForcedProfile = false;
    mForcedProfile = TOUCH_PROFILE_HIDDEN;
}

bool TouchContextResolver::HasForcedProfile() const
{
    return mHasForcedProfile;
}

TouchProfile TouchContextResolver::GetForcedProfile() const
{
    return mForcedProfile;
}

bool TouchContextResolver::IsHidden() const
{
    return Resolve() == TOUCH_PROFILE_HIDDEN;
}

bool TouchContextResolver::IsFrontend() const
{
    return Resolve() == TOUCH_PROFILE_FRONTEND;
}

bool TouchContextResolver::IsCharacter() const
{
    return Resolve() == TOUCH_PROFILE_CHARACTER;
}

bool TouchContextResolver::IsVehicle() const
{
    return Resolve() == TOUCH_PROFILE_VEHICLE;
}

bool TouchContextResolver::IsCinematic() const
{
    return Resolve() == TOUCH_PROFILE_CINEMATIC;
}

bool TouchContextResolver::IsSpecial() const
{
    return Resolve() == TOUCH_PROFILE_SPECIAL;
}

//=============================================================================
// Validation helpers
//=============================================================================

bool TouchContextResolver::IsValidProfile( TouchProfile profile ) const
{
    switch ( profile )
    {
        case TOUCH_PROFILE_HIDDEN:
        case TOUCH_PROFILE_FRONTEND:
        case TOUCH_PROFILE_CHARACTER:
        case TOUCH_PROFILE_VEHICLE:
        case TOUCH_PROFILE_CINEMATIC:
        case TOUCH_PROFILE_SPECIAL:
        case TOUCH_PROFILE_MINIGAME_VEHICLE:
        case TOUCH_PROFILE_MINIGAME_SUMMARY:
        case TOUCH_PROFILE_SCRAPBOOK_CONTENTS:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

bool TouchContextResolver::IsValidContextArea( TouchContextArea area ) const
{
    switch ( area )
    {
        case TOUCH_CONTEXT_AREA_HIDDEN:
        case TOUCH_CONTEXT_AREA_FRONTEND:
        case TOUCH_CONTEXT_AREA_GAMEPLAY:
        case TOUCH_CONTEXT_AREA_CINEMATIC:
        case TOUCH_CONTEXT_AREA_SPECIAL:
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}

bool TouchContextResolver::IsValidGameplayProfile( TouchProfile profile ) const
{
    return profile == TOUCH_PROFILE_CHARACTER ||
           profile == TOUCH_PROFILE_VEHICLE;
}