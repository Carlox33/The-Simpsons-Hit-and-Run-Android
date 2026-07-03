#include <input/touch/touchhudsystem.h>
#include <input/touch/touchcontextresolver.h>
#include <input/touch/touchinputadapter.h>
#include <input/touch/touchinputmodemanager.h>
#include <radmath/radmath.hpp>
#include <input/inputmanager.h>
#include <math.h>
#if defined(RAD_ANDROID)
#include <input/touch/touchcameracontroller.h>
#include <input/touch/touchinteractionresolver.h>
#endif


//=============================================================================
// TouchHudFingerState
//=============================================================================

TouchHudFingerState::TouchHudFingerState()
{
    Reset();
}

void TouchHudFingerState::Reset()
{
    active = false;
    fingerId = 0;
    role = TOUCH_HUD_FINGER_ROLE_NONE;
    controlId = TOUCH_HUD_CONTROL_NONE;

    startPosition = TouchVector2( 0.0f, 0.0f );
    currentPosition = TouchVector2( 0.0f, 0.0f );
    lastPosition = TouchVector2( 0.0f, 0.0f );
    delta = TouchVector2( 0.0f, 0.0f );

    pressure = 0.0f;
    
    
}

//=============================================================================
// TouchHudMovementState
//=============================================================================

TouchHudMovementState::TouchHudMovementState()
{
    Reset();
}

void TouchHudMovementState::Reset()
{
    active = false;
    fingerId = 0;

    origin = TouchVector2( 0.0f, 0.0f );
    current = TouchVector2( 0.0f, 0.0f );
    direction = TouchVector2( 0.0f, 0.0f );

    
    radius = 0.12f;

    moveUpActive = false;
    moveDownActive = false;
    moveLeftActive = false;
    moveRightActive = false;
}

//=============================================================================
// TouchHudCameraDragState
//=============================================================================

TouchHudCameraDragState::TouchHudCameraDragState()
{
    Reset();
}

void TouchHudCameraDragState::Reset()
{
    active = false;
    fingerId = 0;

    startPosition = TouchVector2( 0.0f, 0.0f );
    currentPosition = TouchVector2( 0.0f, 0.0f );
    lastPosition = TouchVector2( 0.0f, 0.0f );
    delta = TouchVector2( 0.0f, 0.0f );
    accumulatedDelta = TouchVector2( 0.0f, 0.0f );
}

//=============================================================================
// TouchHudControlDefinition
//=============================================================================

TouchHudControlDefinition::TouchHudControlDefinition()
{
    Reset();
}

void TouchHudControlDefinition::Reset()
{
    id = TOUCH_HUD_CONTROL_NONE;
    profile = TOUCH_PROFILE_HIDDEN;
    action = TOUCH_ACTION_INVALID;
    rect = TouchRect( 0.0f, 0.0f, 0.0f, 0.0f );
    enabled = false;
    visibleByDefault = false;
    name = "";
}

//=============================================================================
// Singleton
//=============================================================================

TouchHudSystem& TouchHudSystem::GetInstance()
{
    static TouchHudSystem sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchHudSystem::TouchHudSystem():mEnabled( true ),mCurrentProfile( TOUCH_PROFILE_HIDDEN ),mControlCount( 0 ),mCharacterTouchSplitX( 0.32f ),
mCurrentInteractionType( TOUCH_INTERACTION_NONE ),mCurrentInteractionIcon( TOUCH_INTERACTION_ICON_NONE ),mTouchInputWasSuppressed (false)
{
    InitializeDefaultControls();
    ClearActiveTouches();
}

TouchHudSystem::~TouchHudSystem()
{
}

//=============================================================================
// Public API
//=============================================================================

void TouchHudSystem::Reset()
{
    mEnabled = true;
    mCurrentProfile = TOUCH_PROFILE_HIDDEN;

    mCurrentInteractionType = TOUCH_INTERACTION_NONE;
    mCurrentInteractionIcon = TOUCH_INTERACTION_ICON_NONE;

    ClearActiveTouches();

    if ( mControlCount == 0 )
    {
        InitializeDefaultControls();
    }
    mTouchInputWasSuppressed = false;
}

void TouchHudSystem::SetEnabled( bool enabled )
{
    if ( mEnabled != enabled )
    {
        mEnabled = enabled;

        if ( !mEnabled )
        {
            ClearActiveTouches();
        }
    }
}

TouchInteractionType TouchHudSystem::GetCurrentInteractionType() const
{
    return mCurrentInteractionType;
}

TouchInteractionIcon TouchHudSystem::GetCurrentInteractionIcon() const
{
    return mCurrentInteractionIcon;
}

bool TouchHudSystem::HasCurrentInteraction() const
{
    return mCurrentInteractionIcon != TOUCH_INTERACTION_ICON_NONE;
}

bool TouchHudSystem::IsControlVisible( TouchHudControlId controlId ) const
{
    const TouchHudControlDefinition* control = GetControlDefinition( controlId );

    if ( control == 0 )
    {
        return false;
    }

    if ( !control->enabled || !control->visibleByDefault )
    {
        return false;
    }

    switch ( controlId )
    {
        case TOUCH_HUD_CONTROL_CHARACTER_CONTEXT_ACTION:
        {
            return mCurrentProfile == TOUCH_PROFILE_CHARACTER &&
                   mCurrentInteractionIcon != TOUCH_INTERACTION_ICON_NONE;
        }

        case TOUCH_HUD_CONTROL_CHARACTER_SECONDARY_CONTEXT_ACTION:
        {
            /*Lo desactive para un inicio pero quizas lo vuelva a habilitar para tener 
            * la puerta del coche entrar  como un caso aparte aunque aun no lo se seguro
            * Tambien su touchRect fue puesto a false, ojo ahi si luego se quiere habilitar
            */
            return false;
        }

        default:
        {
            return true;
        }
    }
}

bool TouchHudSystem::IsEnabled() const
{
    return mEnabled;
}

void TouchHudSystem::Update( unsigned int elapsedMs )
{
    (void)elapsedMs;

    /*
     * If a physical gamepad is connected, touch HUD input is suppressed.
     * This also clears active touch state even if no new finger event arrives.
     */
    if ( RejectTouchInputIfSuppressed() )
    {
        mCurrentInteractionType = TOUCH_INTERACTION_NONE;
        mCurrentInteractionIcon = TOUCH_INTERACTION_ICON_NONE;
        return;
    }

    TouchProfile resolvedProfile = TouchContextResolver::GetInstance().Resolve();

    if ( resolvedProfile != mCurrentProfile )
    {
        ClearActiveTouches();
        mCurrentProfile = resolvedProfile;
    }
    UpdateCurrentInteraction();
    #if defined(RAD_ANDROID)
        /*
        *Se actualiza aunque este el dedo pulsado en pantalla
        */
        if ( mCurrentProfile == TOUCH_PROFILE_CHARACTER && mMovement.active )
        {
            ApplyMovementActions();
        }
    #endif
}

//Bloquea input tactil en caso de existir mando conectado
// primer frame limpia fuerte 
// los siguientes una vez esta suprimido el touch simplemente retorna true 

bool TouchHudSystem::RejectTouchInputIfSuppressed()
{
#if defined(RAD_ANDROID)
    const bool touchSuppressed =
        !TouchInputModeManager::GetInstance().ShouldShowTouchHud();

    if ( touchSuppressed )
    {
        /*
         * First frame entering suppressed state.
         *
         * This is the only moment where we need the expensive/full cleanup:
         * - remove active touches
         * - clear queued touch inputs
         * - release virtual buttons/axes currently held by touch
         */
        if ( !mTouchInputWasSuppressed )
        {
            ClearActiveTouches();

            TouchInputAdapter::GetInstance().ClearQueuedInputs();
            TouchInputAdapter::GetInstance().ClearActiveInputs();

            mTouchInputWasSuppressed = true;
        }

        return true;
    }

    /*
     * Touch input is no longer suppressed.
     *
     * Reset the transition flag and make sure the touch system resumes from
     * a clean state. We do not need to clear active virtual inputs here because
     * they were already released when suppression started.
     */
    if ( mTouchInputWasSuppressed )
    {
        ClearActiveTouches();
        TouchInputAdapter::GetInstance().ClearQueuedInputs();

        mTouchInputWasSuppressed = false;
    }
#endif

    return false;
}

bool TouchHudSystem::HandleFingerDown
(
    TouchHudFingerId fingerId,
    float x,
    float y,
    float pressure
)
{
        if ( RejectTouchInputIfSuppressed() )
    {
        return false;
    }

    if ( !mEnabled )
    {
        return false;
    }

    Update( 0 );

    if ( mCurrentProfile == TOUCH_PROFILE_HIDDEN )
    {
        return false;
    }

    x = Clamp01( x );
    y = Clamp01( y );

    TouchHudFingerState* finger = AllocateFinger( fingerId );
    if ( finger == 0 )
    {
        return false;
    }

    finger->active = true;
    finger->fingerId = fingerId;
    finger->startPosition = TouchVector2( x, y );
    finger->currentPosition = TouchVector2( x, y );
    finger->lastPosition = TouchVector2( x, y );
    finger->delta = TouchVector2( 0.0f, 0.0f );
    finger->pressure = pressure;

     const TouchHudControlDefinition* control =
        FindControlAtPosition( mCurrentProfile, x, y );

    if ( control != 0 )
    {
        BeginButton( finger, control, x, y );
        return true;
    }

    switch ( mCurrentProfile )
    {
        case TOUCH_PROFILE_CHARACTER:
        {
            // Character movement:
            // Any press in the left side creates a dynamic joystick centered
            // on that initial touch point.
            if ( IsInsideCharacterMovementArea( x, y ) )
            {
                BeginMovement( finger, x, y );
                return true;
            }

            // Character camera:
            // Any press in the right side that is not a button will be treated
            // as a free camera drag area. Buttons will get priority in a later phase.
            if ( IsInsideCharacterCameraArea( x, y ) )
            {
                BeginCameraDrag( finger, x, y );
                return true;
            }

            break;
        }

         case TOUCH_PROFILE_SCRAPBOOK_CONTENTS:
        {
            /*
             * ScrapBookContents has its own visible frontend-style buttons.
             *
             * If the touch did not hit any rendered button, use the remaining
             * screen area as invisible L1/R1 zones:
             *
             * Left half  -> L1
             * Right half -> R1
             */
            const TouchHudControlId zoneControlId =
                ( x < 0.5f ) ?
                TOUCH_HUD_CONTROL_SCRAPBOOK_L1_ZONE :
                TOUCH_HUD_CONTROL_SCRAPBOOK_R1_ZONE;

            const TouchHudControlDefinition* zoneControl =
                GetControlDefinition( zoneControlId );

            if ( zoneControl != 0 )
            {
                BeginButton( finger, zoneControl, x, y );
                return true;
            }

            break;
        }

        case TOUCH_PROFILE_VEHICLE:
        case TOUCH_PROFILE_FRONTEND:
        case TOUCH_PROFILE_CINEMATIC:
        case TOUCH_PROFILE_SPECIAL:
        default:
        {
            // Future phases will add button/control hit testing here.
            break;
        }
    }

    ReleaseFinger( finger );
    return false;
}

bool TouchHudSystem::HandleFingerMove
(
    TouchHudFingerId fingerId,
    float x,
    float y,
    float pressure
)
{
    if ( RejectTouchInputIfSuppressed() )
    {
        return false;
    }
    

    if ( !mEnabled )
    {
        return false;
    }

    x = Clamp01( x );
    y = Clamp01( y );

    TouchHudFingerState* finger = FindFinger( fingerId );
    if ( finger == 0 )
    {
        return false;
    }

    finger->lastPosition = finger->currentPosition;
    finger->currentPosition = TouchVector2( x, y );
    finger->delta = TouchVector2(
        finger->currentPosition.x - finger->lastPosition.x,
        finger->currentPosition.y - finger->lastPosition.y
    );
    finger->pressure = pressure;

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_MOVEMENT )
    {
        UpdateMovement( finger, x, y );
        return true;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_CAMERA )
    {
        UpdateCameraDrag( finger, x, y );
        return true;
    }

    return false;
}

bool TouchHudSystem::HandleFingerUp
(
    TouchHudFingerId fingerId,
    float x,
    float y,
    float pressure
)
{
    if ( RejectTouchInputIfSuppressed() )
    {
        return false;
    }

    (void)x;
    (void)y;
    (void)pressure;

    TouchHudFingerState* finger = FindFinger( fingerId );
    if ( finger == 0 )
    {
        return false;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_MOVEMENT )
    {
        EndMovement( finger );
        return true;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_CAMERA )
    {
        EndCameraDrag( finger );
        return true;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_BUTTON )
    {
        EndButton( finger );
        return true;
    }

    ReleaseFinger( finger );
    return true;
}

bool TouchHudSystem::HandleFingerCancel( TouchHudFingerId fingerId )
{
    if ( RejectTouchInputIfSuppressed() )
    {
        return false;
    }

    TouchHudFingerState* finger = FindFinger( fingerId );
    if ( finger == 0 )
    {
        return false;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_MOVEMENT )
    {
        EndMovement( finger );
        return true;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_CAMERA )
    {
        EndCameraDrag( finger );
        return true;
    }

    if ( finger->role == TOUCH_HUD_FINGER_ROLE_BUTTON )
    {
        EndButton( finger );
        return true;
    }

    ReleaseFinger( finger );
    return true;
}

void TouchHudSystem::ClearActiveTouches()
{
    // Release queued movement actions before clearing the state.
    if ( mMovement.active )
    {
        ClearMovementActions();
    }

    // Release any active button actions before resetting fingers.
    unsigned int i = 0;
    for ( i = 0; i < MAX_ACTIVE_FINGERS; ++i )
    {
        if ( mFingers[ i ].active &&
             mFingers[ i ].role == TOUCH_HUD_FINGER_ROLE_BUTTON )
        {
            QueueControlAction( mFingers[ i ].controlId, 0.0f );
        }

        mFingers[ i ].Reset();
    }

    mMovement.Reset();
    mCameraDrag.Reset();
}

TouchProfile TouchHudSystem::GetCurrentProfile() const
{
    return mCurrentProfile;
}

const TouchHudMovementState& TouchHudSystem::GetMovementState() const
{
    return mMovement;
}

const TouchHudCameraDragState& TouchHudSystem::GetCameraDragState() const
{
    return mCameraDrag;
}

TouchVector2 TouchHudSystem::ConsumeCameraDragDelta()
{
    TouchVector2 result = mCameraDrag.accumulatedDelta;

    mCameraDrag.accumulatedDelta = TouchVector2( 0.0f, 0.0f );

    return result;
}

void TouchHudSystem::SetCharacterTouchSplitX( float splitX )
{
    mCharacterTouchSplitX = Clamp01( splitX );
}

float TouchHudSystem::GetCharacterTouchSplitX() const
{
    return mCharacterTouchSplitX;
}

bool TouchHudSystem::IsMovementActive() const
{
    return mMovement.active;
}

bool TouchHudSystem::IsCameraDragActive() const
{
    return mCameraDrag.active;
}

unsigned int TouchHudSystem::GetControlCount() const
{
    return mControlCount;
}

const TouchHudControlDefinition* TouchHudSystem::GetControlByIndex( unsigned int index ) const
{
    if ( index >= mControlCount )
    {
        return 0;
    }

    return &mControls[ index ];
}

const TouchHudControlDefinition* TouchHudSystem::GetControlDefinition( TouchHudControlId controlId ) const
{
    unsigned int i = 0;
    for ( i = 0; i < mControlCount; ++i )
    {
        if ( mControls[ i ].id == controlId )
        {
            return &mControls[ i ];
        }
    }

    return 0;
}

bool TouchHudSystem::IsControlPressed( TouchHudControlId controlId ) const
{
    unsigned int i = 0;
    for ( i = 0; i < MAX_ACTIVE_FINGERS; ++i )
    {
        if ( mFingers[ i ].active &&
             mFingers[ i ].role == TOUCH_HUD_FINGER_ROLE_BUTTON &&
             mFingers[ i ].controlId == controlId )
        {
            return true;
        }
    }

    return false;
}

TouchActionId TouchHudSystem::GetControlAction( TouchHudControlId controlId ) const
{
    const TouchHudControlDefinition* control = GetControlDefinition( controlId );
    return control ? control->action : TOUCH_ACTION_INVALID;
}


//=============================================================================
// Finger management
//=============================================================================

TouchHudFingerState* TouchHudSystem::FindFinger( TouchHudFingerId fingerId )
{
    unsigned int i = 0;
    for ( i = 0; i < MAX_ACTIVE_FINGERS; ++i )
    {
        if ( mFingers[ i ].active && mFingers[ i ].fingerId == fingerId )
        {
            return &mFingers[ i ];
        }
    }

    return 0;
}

TouchHudFingerState* TouchHudSystem::AllocateFinger( TouchHudFingerId fingerId )
{
    TouchHudFingerState* existingFinger = FindFinger( fingerId );
    if ( existingFinger != 0 )
    {
        return existingFinger;
    }

    unsigned int i = 0;
    for ( i = 0; i < MAX_ACTIVE_FINGERS; ++i )
    {
        if ( !mFingers[ i ].active )
        {
            mFingers[ i ].Reset();
            mFingers[ i ].active = true;
            mFingers[ i ].fingerId = fingerId;
            return &mFingers[ i ];
        }
    }

    return 0;
}

void TouchHudSystem::ReleaseFinger( TouchHudFingerState* finger )
{
    if ( finger != 0 )
    {
        finger->Reset();
    }
}

//=============================================================================
// Movement
//=============================================================================

void TouchHudSystem::BeginMovement( TouchHudFingerState* finger, float x, float y )
{
    if ( finger == 0 )
    {
        return;
    }

    // Only one movement finger at a time.
    if ( mMovement.active )
    {
        return;
    }

    finger->role = TOUCH_HUD_FINGER_ROLE_MOVEMENT;
    finger->controlId = TOUCH_HUD_CONTROL_CHARACTER_MOVEMENT_ZONE;

    mMovement.active = true;
    mMovement.fingerId = finger->fingerId;
    mMovement.origin = TouchVector2( x, y );
    mMovement.current = TouchVector2( x, y );
    mMovement.direction = TouchVector2( 0.0f, 0.0f );
}

void TouchHudSystem::UpdateMovement( TouchHudFingerState* finger, float x, float y )
{
    if ( finger == 0 )
    {
        return;
    }

    if ( !mMovement.active || mMovement.fingerId != finger->fingerId )
    {
        return;
    }

    mMovement.current = TouchVector2( x, y );

    float dx = mMovement.current.x - mMovement.origin.x;
    float dy = mMovement.current.y - mMovement.origin.y;

    if ( mMovement.radius > 0.0f )
    {
        dx /= mMovement.radius;
        dy /= mMovement.radius;
    }

    dx = rmt::Clamp( dx, -1.0f, 1.0f );
    dy = rmt::Clamp( dy, -1.0f, 1.0f );

    
    const float lengthSq = ( dx * dx ) + ( dy * dy );

    if ( lengthSq > 1.0f )
    {
        const float length = sqrtf( lengthSq );

        if ( length > 0.0f )
        {
            dx /= length;
            dy /= length;
        }
    }

    mMovement.direction = TouchVector2( dx, dy );

    ApplyMovementActions();
}

void TouchHudSystem::EndMovement( TouchHudFingerState* finger )
{
    if ( finger == 0 )
    {
        return;
    }

    if ( mMovement.active && mMovement.fingerId == finger->fingerId )
    {
        ClearMovementActions();
        mMovement.Reset();
    }

    ReleaseFinger( finger );
}

//=============================================================================
// Camera drag
//=============================================================================

void TouchHudSystem::BeginCameraDrag( TouchHudFingerState* finger, float x, float y )
{
    if ( finger == 0 )
    {
        return;
    }

    // Only one camera drag finger at a time.
    if ( mCameraDrag.active )
    {
        return;
    }

    finger->role = TOUCH_HUD_FINGER_ROLE_CAMERA;
    finger->controlId = TOUCH_HUD_CONTROL_CHARACTER_CAMERA_ZONE;

    mCameraDrag.active = true;
    mCameraDrag.fingerId = finger->fingerId;
    mCameraDrag.startPosition = TouchVector2( x, y );
    mCameraDrag.currentPosition = TouchVector2( x, y );
    mCameraDrag.lastPosition = TouchVector2( x, y );
    mCameraDrag.delta = TouchVector2( 0.0f, 0.0f );
}

void TouchHudSystem::UpdateCameraDrag( TouchHudFingerState* finger, float x, float y )
{
    if ( finger == 0 )
    {
        return;
    }

    if ( !mCameraDrag.active || mCameraDrag.fingerId != finger->fingerId )
    {
        return;
    }

    mCameraDrag.lastPosition = mCameraDrag.currentPosition;
    mCameraDrag.currentPosition = TouchVector2( x, y );
    mCameraDrag.delta = TouchVector2(
        mCameraDrag.currentPosition.x - mCameraDrag.lastPosition.x,
        mCameraDrag.currentPosition.y - mCameraDrag.lastPosition.y
    );

    mCameraDrag.accumulatedDelta.x += mCameraDrag.delta.x;
    mCameraDrag.accumulatedDelta.y += mCameraDrag.delta.y;

}

void TouchHudSystem::EndCameraDrag( TouchHudFingerState* finger )
{
    if ( finger == 0 )
    {
        return;
    }

    if ( mCameraDrag.active && mCameraDrag.fingerId == finger->fingerId )
    {
        TouchVector2 pendingDelta = mCameraDrag.accumulatedDelta;

        mCameraDrag.Reset();

        
        mCameraDrag.accumulatedDelta = pendingDelta;
    }

    ReleaseFinger( finger );
}

//=============================================================================
// Areas
//=============================================================================

bool TouchHudSystem::IsInsideCharacterMovementArea( float x, float y ) const
{
    (void)y;

    // lado izquierdo
    return x < mCharacterTouchSplitX; 
}

bool TouchHudSystem::IsInsideCharacterCameraArea( float x, float y ) const
{
    (void)y;

    // lado derecho 
    return x >=mCharacterTouchSplitX;
}

//=============================================================================
// Controls / default layouts
//=============================================================================

void TouchHudSystem::InitializeDefaultControls()
{
    mControlCount = 0;

    /*
     * Normalized screen coordinates.
     *
     * x/y are top-left corner.
     * width/height are normalized sizes.
     *
     * Character layout is based on the reference image:
     * - left red area is dynamic movement joystick zone.
     * - right side contains action buttons.
     */

    // -------------------------------------------------------------------------
    // Character buttons
    // -------------------------------------------------------------------------
    // Start / Pause button.
    // Same top-right visual position as Cinematic Skip, but active during gameplay.
    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_START,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_FE_SELECT,
        TouchRect( 0.015f, 0.02f, 0.09f, 0.18f ), // originalmente arriba derecha TouchRect( 0.90f, 0.02f, 0.09f, 0.18f )
        true,
        "CharacterStart"
    );
    // Exclamation / generic contextual action.
    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_CONTEXT_ACTION,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_DO_ACTION,
        TouchRect( 0.63f, 0.38f, 0.12f, 0.24f ),
        true,
        "CharacterContextAction"
    );

    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_SECONDARY_CONTEXT_ACTION,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_DO_ACTION,
        TouchRect( 0.63f, 0.34f, 0.10f, 0.24f ),
        false,
        "CharacterSecondaryContextAction"
    );

    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_JUMP,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_JUMP,
        TouchRect( 0.76f, 0.38f, 0.12f, 0.24f ),
        true,
        "CharacterJump"
    );

    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_ATTACK,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_ATTACK,
        TouchRect( 0.63f, 0.67f, 0.12f, 0.24f ),
        true,
        "CharacterAttack"
    );

    AddControl(
        TOUCH_HUD_CONTROL_CHARACTER_SPRINT,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_ACTION_SPRINT,
        TouchRect( 0.76f, 0.68f, 0.12f, 0.24f ),
        true,
        "CharacterSprint"
    );

    // -------------------------------------------------------------------------
    // Vehicle buttons
    // -------------------------------------------------------------------------

    // Start / Pause button while driving.
    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_START,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_FE_SELECT,
        TouchRect( 0.015f, 0.02f, 0.09f, 0.18f ),
        true,
        "VehicleStart"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_CAMERA_TOGGLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_CAMERA_TOGGLE,
        TouchRect( 0.06f, 0.42f, 0.08f, 0.16f ), // Originalmente TouchRect( 0.01f, 0.30f, 0.065f, 0.13f )
        true,
        "VehicleCameraToggle"
    );

    // Vehicle steering uses two buttons, not a joystick.
    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_STEER_LEFT,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_STEER_LEFT,
        TouchRect( 0.06f, 0.68f, 0.12f, 0.22f ),
        true,
        "VehicleSteerLeft"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_STEER_RIGHT,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_STEER_RIGHT,
        TouchRect( 0.20f, 0.68f, 0.12f, 0.22f ),
        true,
        "VehicleSteerRight"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_ACCELERATE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_ACCELERATE,
        TouchRect( 0.80f, 0.58f, 0.13f, 0.27f ),
        true,
        "VehicleAccelerate"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_REVERSE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_REVERSE,
        TouchRect( 0.69f, 0.71f, 0.11f, 0.22f ), // Originalmente TouchRect( 0.69f, 0.72f, 0.11f, 0.20f ),
        true,
        "VehicleReverse"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_HAND_BRAKE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_HAND_BRAKE,
        TouchRect( 0.69f, 0.49f, 0.11f, 0.20f ), 
        true,
        "VehicleHandBrake"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_GET_OUT,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_GET_OUT_CAR,
        TouchRect( 0.797f, 0.325f, 0.127f, 0.230f ), // anterior  TouchRect( 0.805f, 0.34f, 0.11f, 0.20f )
        true,
        "VehicleGetOut"
        );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_HORN,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_HORN,
        TouchRect( 0.565f, 0.72f, 0.11f, 0.20f ),
        true,
        "VehicleHorn"
    );

   AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_RESET,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_ACTION_RESET_CAR,
        TouchRect( 0.01f, 0.30f, 0.065f, 0.13f ), // originalmente TouchRect( 0.06f, 0.42f, 0.08f, 0.16f )
        true,
        "VehicleReset"
    );

    // -------------------------------------------------------------------------
    // Frontend buttons
    // -------------------------------------------------------------------------

     AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_MOVE_UP,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_FE_MOVE_UP,
        TouchRect( 0.135f, 0.530f, 0.144f, 0.120f ),
        true,
        "FrontendMoveUp"
    );

    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_MOVE_DOWN,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_FE_MOVE_DOWN,
        TouchRect( 0.135f, 0.770f, 0.144f, 0.120f ),
        true,
        "FrontendMoveDown"
    );

    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_MOVE_LEFT,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_FE_MOVE_LEFT,
        TouchRect( 0.065f, 0.650f, 0.144f, 0.120f ),
        true,
        "FrontendMoveLeft"
    );

    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_MOVE_RIGHT,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_FE_MOVE_RIGHT,
        TouchRect( 0.205f, 0.650f, 0.144f, 0.120f ),
        true,
        "FrontendMoveRight"
    );

    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_SELECT,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_JUMP,// en este caso es el boton A para aceptar, quizas deba luego cambiar esto para ser más claro
        TouchRect( 0.78f, 0.68f, 0.12f, 0.22f ),
        true,
        "FrontendSelect"
    );

    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_BACK,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_ACTION_FE_BACK,
        TouchRect( 0.64f, 0.68f, 0.12f, 0.22f ),
        true,
        "FrontendBack"
    );

    AddControl(
    TOUCH_HUD_CONTROL_FRONTEND_START,
    TOUCH_PROFILE_FRONTEND,
    TOUCH_ACTION_FE_SELECT,
    TouchRect( 0.015f, 0.02f, 0.09f, 0.18f ),
    true,
    "FrontendStart"
    );

    // -------------------------------------------------------------------------
    // ScrapBookContents frontend menu 
    // -------------------------------------------------------------------------

    AddControl(
    TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_UP,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_ACTION_FE_MOVE_UP,
    TouchRect(0.135f, 0.530f, 0.144f, 0.120f ),
    true,
    "ScrapBookFeMoveUp"
    );

    AddControl(
        TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_DOWN,
        TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
        TOUCH_ACTION_FE_MOVE_DOWN,
        TouchRect( 0.135f, 0.770f, 0.144f, 0.120f ),
        true,
        "ScrapBookFeMoveDown"
    );

    AddControl(
        TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_LEFT,
        TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
        TOUCH_ACTION_FE_MOVE_LEFT,
        TouchRect( 0.065f, 0.650f, 0.144f, 0.120f ),
        true,
        "ScrapBookFeMoveLeft"
    );

    AddControl(
        TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_RIGHT,
        TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
        TOUCH_ACTION_FE_MOVE_RIGHT,
        TouchRect( 0.205f, 0.650f, 0.144f, 0.120f ),
        true,
        "ScrapBookFeMoveRight"
    );

    AddControl(
        TOUCH_HUD_CONTROL_SCRAPBOOK_FE_SELECT,
        TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
        TOUCH_ACTION_JUMP,
        TouchRect( 0.78f, 0.68f, 0.12f, 0.22f ),
        true,
        "ScrapBookFeSelect"
    );

    AddControl(
        TOUCH_HUD_CONTROL_SCRAPBOOK_FE_BACK,
        TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
        TOUCH_ACTION_FE_BACK,
        TouchRect( 0.64f, 0.68f, 0.12f, 0.22f ),
        true,
        "ScrapBookFeBack"
    );


    // ScrapBookContents invisible half-screen zones

    AddControl(
    TOUCH_HUD_CONTROL_SCRAPBOOK_L1_ZONE,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_ACTION_FE_L1,
    TouchRect( 0.0f, 0.0f, 0.0f, 0.0f ),
    false,
    "ScrapBookL1Zone"
    );

    AddControl(
    TOUCH_HUD_CONTROL_SCRAPBOOK_R1_ZONE,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_ACTION_FE_R1,
    TouchRect( 0.0f, 0.0f, 0.0f, 0.0f ),
    false,
    "ScrapBookR1Zone"
    );
    
    // -------------------------------------------------------------------------
    // Cinematic
    // -------------------------------------------------------------------------

    /*
     * Physical skip is A.
     *
     * For now we associate this with TOUCH_ACTION_JUMP because on the
     * Android/console-style mapping Jump resolves to the A button.
     *
     * If we later find that cinematic skip listens to a different virtual
     * input, only this action mapping needs to change.
     */
    AddControl(
        TOUCH_HUD_CONTROL_CINEMATIC_SKIP,
        TOUCH_PROFILE_CINEMATIC,
        TOUCH_ACTION_JUMP,
        TouchRect( 0.90f, 0.02f, 0.09f, 0.18f ),
        true,
        "CinematicSkip"
    );

    // -------------------------------------------------------------------------
    // Minigame vehicle buttons - SuperSprint
    // 

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_START,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_FE_SELECT,
        TouchRect( 0.015f, 0.02f, 0.09f, 0.18f ),
        true,
        "MinigameVehicleStart"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_CAMERA_TOGGLE,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_SUPERSPRINT_CAMERA_TOGGLE,
        TouchRect( 0.06f, 0.42f, 0.08f, 0.16f ), // Originalmente TouchRect( 0.01f, 0.30f, 0.065f, 0.13f )
        true,
        "MinigameVehicleCameraToggle"
    );

    // Vehicle steering uses two buttons, not a joystick.
    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_LEFT,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_STEER_LEFT,
        TouchRect( 0.06f, 0.68f, 0.12f, 0.22f ),
        true,
        "MinigameVehicleSteerLeft"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_RIGHT,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_STEER_RIGHT,
        TouchRect( 0.20f, 0.68f, 0.12f, 0.22f ),
        true,
        "MinigameVehicleSteerRight"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_ACCELERATE,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_ACCELERATE,
        TouchRect( 0.80f, 0.58f, 0.13f, 0.27f ),
        true,
        "MinigameVehicleAccelerate"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_REVERSE,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_REVERSE,
        TouchRect( 0.69f, 0.71f, 0.11f, 0.22f ), // Originalmente TouchRect( 0.69f, 0.72f, 0.11f, 0.20f ),
        true,
        "MinigameVehicleReverse"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HAND_BRAKE,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_HAND_BRAKE,
        TouchRect( 0.69f, 0.49f, 0.11f, 0.20f ), 
        true,
        "MinigameVehicleHandBrake"
    );

    AddControl(
        TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HORN,
        TOUCH_PROFILE_MINIGAME_VEHICLE,
        TOUCH_ACTION_HORN,
        TouchRect( 0.797f, 0.325f, 0.127f, 0.230f ), // anterior  TouchRect( 0.805f, 0.34f, 0.11f, 0.20f )
        true,
        "MinigameVehicleHorn"
        );

    // -------------------------------------------------------------------------
    // Minigame summary button - SuperSprint
    //
    
    AddControl(
        TOUCH_HUD_CONTROL_FRONTEND_MINIGAME_SUMMARY_SELECT,
        TOUCH_PROFILE_MINIGAME_SUMMARY,
        TOUCH_ACTION_JUMP,
        TouchRect( 0.78f, 0.68f, 0.12f, 0.22f ),
        true,
        "MinigameFrontendSelect"
    );
}

bool TouchHudSystem::AddControl
(
    TouchHudControlId id,
    TouchProfile profile,
    TouchActionId action,
    const TouchRect& rect,
    bool visibleByDefault,
    const char* name
)
{
    if ( mControlCount >= MAX_TOUCH_CONTROLS )
    {
        return false;
    }

    TouchHudControlDefinition& control = mControls[ mControlCount ];
    control.id = id;
    control.profile = profile;
    control.action = action;
    control.rect = rect;
    control.enabled = true;
    control.visibleByDefault = visibleByDefault;
    control.name = name ? name : "";

    ++mControlCount;

    return true;
}

const TouchHudControlDefinition* TouchHudSystem::FindControlAtPosition
(
    TouchProfile profile,
    float x,
    float y
) const
{
    unsigned int i = 0;

    
    for ( i = mControlCount; i > 0; --i )
    {
        const TouchHudControlDefinition& control = mControls[ i - 1 ];

        if ( !control.enabled )
        {
            continue;
        }
        

        if ( control.profile != profile )
        {
            continue;
        }

        if ( !IsControlVisible( control.id ) )
        {
            continue;
        }

        if ( IsPointInsideRect( x, y, control.rect ) )
        {
            return &control;
        }
    }

    return 0;
}

bool TouchHudSystem::IsPointInsideRect( float x, float y, const TouchRect& rect ) const
{
    return x >= rect.x &&
           x <= ( rect.x + rect.width ) &&
           y >= rect.y &&
           y <= ( rect.y + rect.height );
}

void TouchHudSystem::BeginButton
(
    TouchHudFingerState* finger,
    const TouchHudControlDefinition* control,
    float x,
    float y
)
{
    if ( finger == 0 || control == 0 )
    {
        return;
    }

    finger->role = TOUCH_HUD_FINGER_ROLE_BUTTON;
    finger->controlId = control->id;
    finger->startPosition = TouchVector2( x, y );
    finger->currentPosition = TouchVector2( x, y );
    finger->lastPosition = TouchVector2( x, y );
    finger->delta = TouchVector2( 0.0f, 0.0f );
    QueueTouchAction( control->action, 1.0f );
}

void TouchHudSystem::EndButton( TouchHudFingerState* finger )
{
    if ( finger == 0 )
    {
        return;
    }

   QueueControlAction( finger->controlId, 0.0f );

    ReleaseFinger( finger );
}


//=============================================================================
// Queued action output
//=============================================================================

void TouchHudSystem::QueueTouchAction( TouchActionId action, float value )
{
    if ( action == TOUCH_ACTION_INVALID )
    {
        return;
    }

    #if defined(RAD_ANDROID)
        if ( action == TOUCH_ACTION_CAMERA_TOGGLE )
        {
            if ( value > 0.5f )
            {
                TouchCameraController::GetInstance().RequestCameraToggle();
            }

            return;
        }
    #endif

    TouchInputAdapter::GetInstance().QueueAction( action, value );
}

void TouchHudSystem::QueueControlAction( TouchHudControlId controlId, float value )
{
    const TouchHudControlDefinition* control = GetControlDefinition( controlId );

    if ( control == 0 )
    {
        return;
    }

    QueueTouchAction( control->action, value );
}

void TouchHudSystem::SetMovementActionState
(
    TouchActionId action,
    bool pressed,
    bool& storedState
)
{
    if ( storedState == pressed )
    {
        return;
    }

    storedState = pressed;

    QueueTouchAction( action, pressed ? 1.0f : 0.0f );
}

void TouchHudSystem::ApplyMovementActions()
{
#if defined(RAD_ANDROID)
    if ( !mMovement.active )
    {
        return;
    }

    float stickX = mMovement.direction.x;
    // Si hemos tenido que invertir y para que el jostick virtual funcione como corresponde
    float stickY = -mMovement.direction.y;

  
    const float deadZone = 0.04f;
    const float antiDeadZone = 0.30f;

    float lengthSq = ( stickX * stickX ) + ( stickY * stickY );

    if ( lengthSq < deadZone * deadZone )
    {
        stickX = 0.0f;
        stickY = 0.0f;
    }
    else
    {
        float length = sqrtf( lengthSq );

        if ( length > 0.0f )
        {
            float dirX = stickX / length;
            float dirY = stickY / length;

            /*
            * Reescala la magnitud:
            *
            * Antes:
            *   salir del centro podía mandar 0.05, 0.10, 0.15...
            *
            * Ahora:
            *   al salir de la deadzone, empieza ya en antiDeadZone.
            *
            * Esto compensa la deadzone/sensibilidad interna del juego original.
            */
            float normalizedLength = ( length - deadZone ) / ( 1.0f - deadZone );
            normalizedLength = rmt::Clamp( normalizedLength, 0.0f, 1.0f );

            float outputLength = antiDeadZone +
                ( normalizedLength * ( 1.0f - antiDeadZone ) );

            stickX = dirX * outputLength;
            stickY = dirY * outputLength;
        }
    }

    TouchInputAdapter::GetInstance().QueueInputManagerAxis(
        InputManager::LeftStickX,
        stickX
    );

    TouchInputAdapter::GetInstance().QueueInputManagerAxis(
        InputManager::LeftStickY,
        stickY
    );
#endif
}

void TouchHudSystem::ClearMovementActions()
{
#if defined(RAD_ANDROID)
    /*
     * Soltar el joystick táctil equivale a devolver el stick izquierdo
     * al centro.
     *
     * Como nuestra vía virtual escribe directamente en mButtonArray,
     * el centro correcto es 0.0f.
     */
    TouchInputAdapter::GetInstance().QueueInputManagerAxis(
        InputManager::LeftStickX,
        0.0f
    );

    TouchInputAdapter::GetInstance().QueueInputManagerAxis(
        InputManager::LeftStickY,
        0.0f
    );

    /*
     * Limpieza de seguridad:
     * Se podría quitar en el futuro, pero no hay problema en cuanto a rendimiento por dejarlo
     */
    SetMovementActionState(
        TOUCH_ACTION_MOVE_LEFT,
        false,
        mMovement.moveLeftActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_RIGHT,
        false,
        mMovement.moveRightActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_UP,
        false,
        mMovement.moveUpActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_DOWN,
        false,
        mMovement.moveDownActive
    );
#else
    SetMovementActionState(
        TOUCH_ACTION_MOVE_LEFT,
        false,
        mMovement.moveLeftActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_RIGHT,
        false,
        mMovement.moveRightActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_UP,
        false,
        mMovement.moveUpActive
    );

    SetMovementActionState(
        TOUCH_ACTION_MOVE_DOWN,
        false,
        mMovement.moveDownActive
    );
#endif
}

float TouchHudSystem::Clamp01( float value ) const
{
    if ( value < 0.0f )
    {
        return 0.0f;
    }

    if ( value > 1.0f )
    {
        return 1.0f;
    }

    return value;
}

void TouchHudSystem::UpdateCurrentInteraction()
{
#if defined(RAD_ANDROID)
    if ( mCurrentProfile != TOUCH_PROFILE_CHARACTER )
    {
        mCurrentInteractionType = TOUCH_INTERACTION_NONE;
        mCurrentInteractionIcon = TOUCH_INTERACTION_ICON_NONE;
        return;
    }

    mCurrentInteractionType =
        TouchInteractionResolver::GetInstance().ResolveCharacterInteraction();

    mCurrentInteractionIcon =
        TouchInteractionTypeToIcon( mCurrentInteractionType );
#else
    mCurrentInteractionType = TOUCH_INTERACTION_NONE;
    mCurrentInteractionIcon = TOUCH_INTERACTION_ICON_NONE;
#endif
}