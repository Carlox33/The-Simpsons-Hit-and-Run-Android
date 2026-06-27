#ifndef TOUCHCONTEXTRESOLVER_H_
#define TOUCHCONTEXTRESOLVER_H_

//=============================================================================
// TouchContextResolver
//
// Resolves which touch profile should be active at any moment.
//
// This class intentionally does not talk directly to gameplay, AvatarManager,
// HUD, SDL or rendering yet. It only stores and resolves context state.
//
// Phase 5A goal:
// - Compile safely.
// - Support all profiles: Hidden, Frontend, Character, Vehicle, Special.
// - Prepare future connection to GameFlow and Avatar::IsInCar().
//=============================================================================

#include <input/touch/touchtypes.h>

enum TouchContextArea
{
    TOUCH_CONTEXT_AREA_HIDDEN = 0,
    TOUCH_CONTEXT_AREA_FRONTEND,
    TOUCH_CONTEXT_AREA_GAMEPLAY,
    TOUCH_CONTEXT_AREA_CINEMATIC,
    TOUCH_CONTEXT_AREA_SPECIAL
};

inline const char* TouchContextAreaToString( TouchContextArea area )
{
    switch ( area )
    {
        case TOUCH_CONTEXT_AREA_HIDDEN:
            return "Hidden";
        case TOUCH_CONTEXT_AREA_FRONTEND:
            return "Frontend";
        case TOUCH_CONTEXT_AREA_GAMEPLAY:
            return "Gameplay";
        case TOUCH_CONTEXT_AREA_SPECIAL:
            return "Special";
        default:
            return "Unknown";
    }
}

class TouchContextResolver
{
public:
    static TouchContextResolver& GetInstance();

    void Reset();

    // Main query used by future TouchHudSystem.
    TouchProfile Resolve() const;

    // Updates the resolver using real game state information.
    // This does not query GameFlow, InputManager or AvatarManager directly.
    // The caller provides the current values to keep this class isolated.
    void UpdateFromGameState( int gameContext, unsigned inputGameState, bool avatarInVehicle );
    
    // High-level context setters.
    void SetHiddenContext();
    void SetFrontendContext();
    void SetCharacterContext();
    void SetVehicleContext();
    void SetCinematicContext();
    void SetSpecialContext();
    void SetMissionBriefingActive( bool active );
    void SetGameplayConversationActive( bool active );

    // Lower-level setters for future integration.
    void SetContextArea( TouchContextArea area );
    TouchContextArea GetContextArea() const;

    void SetGameplayProfile( TouchProfile profile );
    TouchProfile GetGameplayProfile() const;

    // Debug/testing override. When forced profile is enabled, Resolve()
    // always returns that profile.
    void SetForcedProfile( TouchProfile profile );
    void ClearForcedProfile();
    bool HasForcedProfile() const;
    TouchProfile GetForcedProfile() const;

    // Minijuego
    void SetSuspendedSuperSprintProfile( TouchProfile profile );
    void ClearSuspendedSuperSprintProfile();

// Para detectar dialogos de personajes que te venden 1 coche por nivel 
    void SetPurchaseRewardConversationActive( bool active );
    bool IsPurchaseRewardConversationActive() const;

    // para detectar la pantalla de idioma inicial(guimanagerlanguage.cpp) en caso de no coincidir ningún idioma(language.cpp). 
    //Ejemplo brasileño quiere jugar en ingles.

    void SetLanguageSelectionActive( bool active );
    bool IsLanguageSelectionActive() const;

    bool IsHidden() const;
    bool IsFrontend() const;
    bool IsCharacter() const;
    bool IsVehicle() const;
    bool IsCinematic() const;
    bool IsSpecial() const;
    

private:
    TouchContextResolver();
    ~TouchContextResolver();

    // Intentionally not implemented.
    TouchContextResolver( const TouchContextResolver& );
    TouchContextResolver& operator=( const TouchContextResolver& );

    bool IsValidProfile( TouchProfile profile ) const;
    bool IsValidContextArea( TouchContextArea area ) const;
    bool IsValidGameplayProfile( TouchProfile profile ) const;
    

    bool IsGameplayFrontendLikeState() const;
    bool IsMissionDialogueObjectiveActive() const;
    bool IsMissionBriefingActive() const;
    bool IsGameplayConversationActive() const;
    bool mPurchaseRewardConversationActive;
    bool mLanguageSelectionActive;
private:
    TouchContextArea mContextArea;

    TouchProfile mGameplayProfile;

    bool mHasForcedProfile;
    TouchProfile mForcedProfile;

    TouchProfile mSuspendedSuperSprintProfile;

    bool mMissionBriefingActive;
    bool mGameplayConversationActive;

   
};

#endif // TOUCHCONTEXTRESOLVER_H_