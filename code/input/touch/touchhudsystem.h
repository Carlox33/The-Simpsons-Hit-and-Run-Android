#ifndef TOUCHHUDSYSTEM_H_
#define TOUCHHUDSYSTEM_H_

//=============================================================================
// TouchHudSystem
//=============================================================================

#include <input/touch/touchtypes.h>

typedef long long TouchHudFingerId;

enum TouchHudFingerRole
{
    TOUCH_HUD_FINGER_ROLE_NONE = 0,
    TOUCH_HUD_FINGER_ROLE_MOVEMENT,
    TOUCH_HUD_FINGER_ROLE_CAMERA,
    TOUCH_HUD_FINGER_ROLE_BUTTON,
    TOUCH_HUD_FINGER_ROLE_EDIT_CONTROL
};

enum TouchHudControlId
{
    TOUCH_HUD_CONTROL_NONE = 0,

    // Character
    TOUCH_HUD_CONTROL_CHARACTER_MOVEMENT_ZONE,
    TOUCH_HUD_CONTROL_CHARACTER_CAMERA_ZONE,
    TOUCH_HUD_CONTROL_CHARACTER_JUMP,
    TOUCH_HUD_CONTROL_CHARACTER_SPRINT,
    TOUCH_HUD_CONTROL_CHARACTER_ATTACK,

    // Gameplay Start/Pause button.
    // Same visual slot as Cinematic Skip, but used while playing.
    TOUCH_HUD_CONTROL_CHARACTER_START,

    // Main contextual slot. For now it sends DoAction.
    // Future: mission/interior/phone booth/car prompt icon.
    TOUCH_HUD_CONTROL_CHARACTER_CONTEXT_ACTION,

    // Secondary contextual slot. Also DoAction for now.
    // Useful if we want to keep a door/car icon separated from the generic prompt.
    TOUCH_HUD_CONTROL_CHARACTER_SECONDARY_CONTEXT_ACTION,

    // Vehicle
    TOUCH_HUD_CONTROL_VEHICLE_STEER_LEFT,
    TOUCH_HUD_CONTROL_VEHICLE_STEER_RIGHT,
    TOUCH_HUD_CONTROL_VEHICLE_ACCELERATE,
    TOUCH_HUD_CONTROL_VEHICLE_REVERSE,
    TOUCH_HUD_CONTROL_VEHICLE_HAND_BRAKE,
    TOUCH_HUD_CONTROL_VEHICLE_GET_OUT,
    TOUCH_HUD_CONTROL_VEHICLE_HORN,
    TOUCH_HUD_CONTROL_VEHICLE_RESET,
    TOUCH_HUD_CONTROL_VEHICLE_START,
    TOUCH_HUD_CONTROL_VEHICLE_CAMERA_TOGGLE,

    // Frontend
    TOUCH_HUD_CONTROL_FRONTEND_MOVE_UP,
    TOUCH_HUD_CONTROL_FRONTEND_MOVE_DOWN,
    TOUCH_HUD_CONTROL_FRONTEND_MOVE_LEFT,
    TOUCH_HUD_CONTROL_FRONTEND_MOVE_RIGHT,
    TOUCH_HUD_CONTROL_FRONTEND_SELECT,
    TOUCH_HUD_CONTROL_FRONTEND_BACK,
    TOUCH_HUD_CONTROL_FRONTEND_START,

    

    // Cinematic
    TOUCH_HUD_CONTROL_CINEMATIC_SKIP,

    // Minigame 

    //Minimage vehiculo
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_LEFT,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_RIGHT,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_ACCELERATE,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_REVERSE,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HAND_BRAKE,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HORN,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_START,
    TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_CAMERA_TOGGLE,

    //minigame final summary 
    TOUCH_HUD_CONTROL_FRONTEND_MINIGAME_SUMMARY_SELECT,


     // ScrapBookContents
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_UP,
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_DOWN,
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_LEFT,
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_RIGHT,
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_SELECT,
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_BACK,
    
     // ScrapBookContents invisible half-screen zones
    TOUCH_HUD_CONTROL_SCRAPBOOK_L1_ZONE,
    TOUCH_HUD_CONTROL_SCRAPBOOK_R1_ZONE,

     // Touch controls editor
    TOUCH_HUD_CONTROL_EDITOR_ENTER_IN_GAME, // Icono flechas en menu opciones in-game
    TOUCH_HUD_CONTROL_EDITOR_ENTER_MAIN_MENU, // mismo icono flechas pero en menu principañ 
    TOUCH_HUD_CONTROL_EDITOR_NEXT_LAYOUT, // 1/3, 2/3, 3/3 
    TOUCH_HUD_CONTROL_EDITOR_RESET, //icono RESET

    TOUCH_HUD_CONTROL_COUNT

};

struct TouchHudFingerState
{
    TouchHudFingerState();

    void Reset();

    bool active;
    TouchHudFingerId fingerId;
    TouchHudFingerRole role;
    TouchHudControlId controlId;

    TouchVector2 startPosition;
    TouchVector2 currentPosition;
    TouchVector2 lastPosition;
    TouchVector2 delta;

    float pressure;
};

struct TouchHudMovementState
{
    TouchHudMovementState();

    void Reset();

    bool active;
    TouchHudFingerId fingerId;

    // Dynamic joystick center. In the future, the joystick PNG will be drawn here.
    TouchVector2 origin;

    TouchVector2 current;
    TouchVector2 direction;

    float radius;

    // Current queued movement output states.
    bool moveUpActive;
    bool moveDownActive;
    bool moveLeftActive;
    bool moveRightActive;
};

struct TouchHudCameraDragState
{
    TouchHudCameraDragState();

    void Reset();

    bool active;
    TouchHudFingerId fingerId;

    TouchVector2 startPosition;
    TouchVector2 currentPosition;
    TouchVector2 lastPosition;
    TouchVector2 delta;
    TouchVector2 accumulatedDelta;
};

struct TouchHudControlDefinition
{
    TouchHudControlDefinition();

    void Reset();

    TouchHudControlId id;
    TouchProfile profile;
    TouchActionId action;
    TouchRect rect;

    bool enabled;
    bool visibleByDefault;

    const char* name;
};

class TouchHudSystem
{
public:
    static TouchHudSystem& GetInstance();

    void Reset();

    void SetEnabled( bool enabled );
    bool IsEnabled() const;

    void Update( unsigned int elapsedMs );

    bool HandleFingerDown( TouchHudFingerId fingerId, float x, float y, float pressure );
    bool HandleFingerMove( TouchHudFingerId fingerId, float x, float y, float pressure );
    bool HandleFingerUp( TouchHudFingerId fingerId, float x, float y, float pressure );
    bool HandleFingerCancel( TouchHudFingerId fingerId );

    void ClearActiveTouches();

    TouchProfile GetCurrentProfile() const;

    const TouchHudMovementState& GetMovementState() const;
    const TouchHudCameraDragState& GetCameraDragState() const;

    // se consume y limpia el delta acumulado cada frame
    TouchVector2 ConsumeCameraDragDelta();

    void SetCharacterTouchSplitX( float splitX );
    float GetCharacterTouchSplitX() const;
    
    bool IsMovementActive() const;
    bool IsCameraDragActive() const;
    unsigned int GetControlCount() const;
    const TouchHudControlDefinition* GetControlByIndex( unsigned int index ) const;
    const TouchHudControlDefinition* GetControlDefinition( TouchHudControlId controlId ) const;

    bool IsControlPressed( TouchHudControlId controlId ) const;
    TouchActionId GetControlAction( TouchHudControlId controlId ) const;

    TouchInteractionType GetCurrentInteractionType() const;
    TouchInteractionIcon GetCurrentInteractionIcon() const;
    bool HasCurrentInteraction() const;

    bool IsControlVisible( TouchHudControlId controlId ) const;

    TouchRect GetEffectiveControlRect( TouchHudControlId controlId ) const;

    void SetTouchControlsEditorEntryAllowed( bool allowed );
    bool IsTouchControlsEditorEntryAllowed() const;

    void SetTouchControlsEditorMainMenuEntryAllowed( bool allowed );
    bool IsTouchControlsEditorMainMenuEntryAllowed() const;

    bool IsTouchControlsEditModeActive() const;
    TouchEditableLayout GetCurrentEditableLayout() const;

    bool IsEditorControl( TouchHudControlId controlId ) const;

    bool ShouldControlBeUsedForProfile( TouchHudControlId controlId, TouchProfile profile ) const;
private:
    TouchHudSystem();
    ~TouchHudSystem();

    // Intentionally not implemented.
    TouchHudSystem( const TouchHudSystem& );
    TouchHudSystem& operator=( const TouchHudSystem& );

    enum
    {
        MAX_ACTIVE_FINGERS = 10,
        MAX_TOUCH_CONTROLS = 48
    };

    TouchHudFingerState* FindFinger( TouchHudFingerId fingerId );
    TouchHudFingerState* AllocateFinger( TouchHudFingerId fingerId );

    void ReleaseFinger( TouchHudFingerState* finger );

    void BeginMovement( TouchHudFingerState* finger, float x, float y );
    void UpdateMovement( TouchHudFingerState* finger, float x, float y );
    void EndMovement( TouchHudFingerState* finger );

    void BeginCameraDrag( TouchHudFingerState* finger, float x, float y );
    void UpdateCameraDrag( TouchHudFingerState* finger, float x, float y );
    void EndCameraDrag( TouchHudFingerState* finger );
    void UpdateCurrentInteraction();

    bool IsInsideCharacterMovementArea( float x, float y ) const;
    bool IsInsideCharacterCameraArea( float x, float y ) const;

    bool mTouchInputWasSuppressed;
    bool RejectTouchInputIfSuppressed();
    
    bool mTouchControlsEditorEntryAllowed;
    bool mTouchControlsEditModeActive;
    TouchEditableLayout mCurrentEditableLayout;

    bool mTouchControlsEditorMainMenuEntryAllowed;
    
    float Clamp01( float value ) const;

    void InitializeDefaultControls();

    bool AddControl
    (
        TouchHudControlId id,
        TouchProfile profile,
        TouchActionId action,
        const TouchRect& rect,
        bool visibleByDefault,
        const char* name
    );

    const TouchHudControlDefinition* FindControlAtPosition
    (
        TouchProfile profile,
        float x,
        float y
    ) const;

    bool IsPointInsideRect( float x, float y, const TouchRect& rect ) const;

    void BeginButton
    (
        TouchHudFingerState* finger,
        const TouchHudControlDefinition* control,
        float x,
        float y
    );

    void EndButton( TouchHudFingerState* finger );
    void QueueTouchAction( TouchActionId action, float value );
    void QueueControlAction( TouchHudControlId controlId, float value );

    void SetMovementActionState
    (
        TouchActionId action,
        bool pressed,
        bool& storedState
    );

    void ApplyMovementActions();
    void ClearMovementActions();

    void BeginTouchControlsEditMode();
    void EndTouchControlsEditMode();
    void AdvanceTouchControlsEditLayout();
    void ResetCurrentTouchControlsEditLayout();

    TouchProfile GetProfileForEditableLayout( TouchEditableLayout layout ) const;

    bool IsControlEditableInCurrentLayout( TouchHudControlId controlId ) const;

    void BeginEditControl
    (
        TouchHudFingerState* finger,
        const TouchHudControlDefinition* control,
        float x,
        float y
    );

    void UpdateEditControl
    (
        TouchHudFingerState* finger,
        float x,
        float y
    );

    void EndEditControl( TouchHudFingerState* finger );

    void ApplyCharacterEditMovementRestriction
    (
        TouchHudControlId controlId
    );

    bool IsFrontendArrowControl( TouchHudControlId controlId ) const;
    bool IsFrontendArrowGroupTouch( float x, float y ) const;

    void MoveFrontendArrowGroup( float deltaX, float deltaY );

private:
    bool mEnabled;
    TouchProfile mCurrentProfile;

    TouchHudFingerState mFingers[ MAX_ACTIVE_FINGERS ];

    TouchHudControlDefinition mControls[ MAX_TOUCH_CONTROLS ];
    unsigned int mControlCount;

    TouchHudMovementState mMovement;
    TouchHudCameraDragState mCameraDrag;

    //Variable que delimita a partir de que valor la pantalla se considera zona izquierda para joystick o zona derecha para cámara
    float mCharacterTouchSplitX;

    
    TouchInteractionType mCurrentInteractionType;
    TouchInteractionIcon mCurrentInteractionIcon;

    
    TouchHudControlId mEditingControlId;
    bool mEditingControlWasDragged;
    bool mEditingFrontendArrowGroup;

    TouchVector2 mEditingStartPosition;
    TouchVector2 mEditingLastPosition;
        
};

#endif // TOUCHHUDSYSTEM_H_