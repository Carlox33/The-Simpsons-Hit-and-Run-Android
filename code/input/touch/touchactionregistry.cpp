#include <input/touch/touchactionregistry.h>
#include <input/inputmanager.h>

#include <string.h>

//=============================================================================
// TouchActionRegistry
//
// The touch system is designed around semantic virtual actions:
//
//     Jump
//     Attack
//     Accelerate
//     GetOutCar
//     feSelect
//     CameraCarLookBack
//
// However, depending on build flags, InputManager::eButtonMap may expose either:
//
//     RAD_PC style:
//         MoveUp, Jump, Accelerate, feSelect, etc.
//
// or console-style buttons:
//
//     RAD_CONSOLE && RAD_WIN32 style:
//         DPadUp, A, B, Square, Y, Start, Back, etc.
//
// For that reason, this registry keeps the semantic TouchActionId stable, while
// inputManagerButtonId stores the best platform-specific button id available.
//
// If a semantic action has no direct platform id in the current build, we keep it
// registered but assign Input::INVALID_CONTROLLERID. Later systems can still show
// it in layouts, bind it manually, or implement special handling.
//=============================================================================

//-----------------------------------------------------------------------------
// Platform mapping helpers
//-----------------------------------------------------------------------------

#if defined(RAD_PC) && !(defined(RAD_CONSOLE) && defined(RAD_WIN32))

    // Pure RAD_PC mapping: InputManager already exposes virtual action names.

    #define TOUCH_IM_MOVE_UP                 InputManager::MoveUp
    #define TOUCH_IM_MOVE_DOWN               InputManager::MoveDown
    #define TOUCH_IM_MOVE_LEFT               InputManager::MoveLeft
    #define TOUCH_IM_MOVE_RIGHT              InputManager::MoveRight
    #define TOUCH_IM_ATTACK                  InputManager::Attack
    #define TOUCH_IM_JUMP                    InputManager::Jump
    #define TOUCH_IM_SPRINT                  InputManager::Sprint
    #define TOUCH_IM_DO_ACTION               InputManager::DoAction

    #define TOUCH_IM_CAMERA_LEFT             InputManager::CameraLeft
    #define TOUCH_IM_CAMERA_RIGHT            InputManager::CameraRight
    #define TOUCH_IM_CAMERA_MOVE_IN          InputManager::CameraMoveIn
    #define TOUCH_IM_CAMERA_MOVE_OUT         InputManager::CameraMoveOut
    #define TOUCH_IM_CAMERA_ZOOM             InputManager::CameraZoom
    #define TOUCH_IM_CAMERA_LOOK_UP          InputManager::CameraLookUp

    #define TOUCH_IM_ACCELERATE              InputManager::Accelerate
    #define TOUCH_IM_REVERSE                 InputManager::Reverse
    #define TOUCH_IM_STEER_LEFT              InputManager::SteerLeft
    #define TOUCH_IM_STEER_RIGHT             InputManager::SteerRight
    #define TOUCH_IM_GET_OUT_CAR             InputManager::GetOutCar
    #define TOUCH_IM_HAND_BRAKE              InputManager::HandBrake
    #define TOUCH_IM_HORN                    InputManager::Horn
    #define TOUCH_IM_RESET_CAR               InputManager::ResetCar

    #define TOUCH_IM_CAMERA_CAR_LEFT         InputManager::CameraCarLeft
    #define TOUCH_IM_CAMERA_CAR_RIGHT        InputManager::CameraCarRight
    #define TOUCH_IM_CAMERA_CAR_LOOK_UP      InputManager::CameraCarLookUp
    #define TOUCH_IM_CAMERA_CAR_LOOK_BACK    InputManager::CameraCarLookBack
    #define TOUCH_IM_CAMERA_TOGGLE           InputManager::CameraToggle

    //Minigame
    #define TOUCH_IM_SUPERSPRINT_CAMERA_TOGGLE  Input::INVALID_CONTROLLERID


    #define TOUCH_IM_FE_BACK                 InputManager::feBack
    #define TOUCH_IM_FE_MOVE_UP              InputManager::feMoveUp
    #define TOUCH_IM_FE_MOVE_DOWN            InputManager::feMoveDown
    #define TOUCH_IM_FE_MOVE_LEFT            InputManager::feMoveLeft
    #define TOUCH_IM_FE_MOVE_RIGHT           InputManager::feMoveRight
    #define TOUCH_IM_FE_SELECT               InputManager::feSelect
    #define TOUCH_IM_FE_FUNCTION_1           InputManager::feFunction1
    #define TOUCH_IM_FE_FUNCTION_2           InputManager::feFunction2
    #define TOUCH_IM_FE_MOUSE_LEFT           InputManager::feMouseLeft
    #define TOUCH_IM_FE_MOUSE_RIGHT          InputManager::feMouseRight
    #define TOUCH_IM_FE_MOUSE_UP             InputManager::feMouseUp
    #define TOUCH_IM_FE_MOUSE_DOWN           InputManager::feMouseDown
    #define TOUCH_IM_FE_L1                   Input::INVALID_CONTROLLERID
    #define TOUCH_IM_FE_R1                   Input::INVALID_CONTROLLERID

    #define TOUCH_IM_P1_KBD_START            InputManager::P1_KBD_Start
    #define TOUCH_IM_P1_KBD_GAS              InputManager::P1_KBD_Gas
    #define TOUCH_IM_P1_KBD_BRAKE            InputManager::P1_KBD_Brake
    #define TOUCH_IM_P1_KBD_EBRAKE           InputManager::P1_KBD_EBrake
    #define TOUCH_IM_P1_KBD_NITRO            InputManager::P1_KBD_Nitro
    #define TOUCH_IM_P1_KBD_LEFT             InputManager::P1_KBD_Left
    #define TOUCH_IM_P1_KBD_RIGHT            InputManager::P1_KBD_Right

#else

    // Console-style mapping used by your Android build:
    //
    // #if defined(RAD_XBOX) || defined(RAD_CONSOLE) && defined(RAD_WIN32)
    //
    // CharacterMappable maps:
    //     DPadUp/Down/Left/Right
    //     Y -> DoAction
    //     A -> Jump
    //     B -> Dash/Sprint
    //     X -> Attack
    //
    // In this enum, Square is commented as "This is X", so we use Square for
    // the physical X-style attack/handbrake slot.

    #define TOUCH_IM_MOVE_UP                 InputManager::DPadUp
    #define TOUCH_IM_MOVE_DOWN               InputManager::DPadDown
    #define TOUCH_IM_MOVE_LEFT               InputManager::DPadLeft
    #define TOUCH_IM_MOVE_RIGHT              InputManager::DPadRight
    #define TOUCH_IM_ATTACK                  InputManager::Square
    #define TOUCH_IM_JUMP                    InputManager::A
    #define TOUCH_IM_SPRINT                  InputManager::B
    #define TOUCH_IM_DO_ACTION               InputManager::Y

    // Camera actions are registered semantically, but in this build there is
    // no one-to-one virtual eButtonMap entry for most of them yet.
    #define TOUCH_IM_CAMERA_LEFT             Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_RIGHT            Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_MOVE_IN          Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_MOVE_OUT         Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_ZOOM             Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_LOOK_UP          Input::INVALID_CONTROLLERID

    // VehicleMappable console mapping:
    //     DPadLeft/DPadRight -> SteerLeft/SteerRight
    //     A / RightTrigger   -> Gas
    //     B / LeftTrigger    -> Brake
    //     X                  -> HandBrake
    //     White / LeftThumb  -> Horn
    //     Back               -> Reset
    #define TOUCH_IM_ACCELERATE              InputManager::A
    #define TOUCH_IM_REVERSE                 InputManager::B
    #define TOUCH_IM_STEER_LEFT              InputManager::DPadLeft
    #define TOUCH_IM_STEER_RIGHT             InputManager::DPadRight

    // In RAD_CONSOLE && RAD_WIN32, CharacterMappable does not map GetOutCar
    // directly. InCarCharacterMappable processes DoAction for in-car behavior,
    // so we map semantic GetOutCar to Y/DoAction as a practical default.
    #define TOUCH_IM_GET_OUT_CAR             InputManager::Y

    #define TOUCH_IM_HAND_BRAKE              InputManager::Square
    #define TOUCH_IM_HORN                    InputManager::LeftThumb
    #define TOUCH_IM_RESET_CAR               InputManager::Back

    #define TOUCH_IM_CAMERA_CAR_LEFT         Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_CAR_RIGHT        Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_CAR_LOOK_UP      Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_CAR_LOOK_BACK    Input::INVALID_CONTROLLERID
    #define TOUCH_IM_CAMERA_TOGGLE           Input::INVALID_CONTROLLERID
    //MINIGAME CAMERA TOGGLE 
    #define TOUCH_IM_SUPERSPRINT_CAMERA_TOGGLE  InputManager::Black
 
    #define TOUCH_IM_FE_BACK                 InputManager::Back
    #define TOUCH_IM_FE_MOVE_UP              InputManager::DPadUp
    #define TOUCH_IM_FE_MOVE_DOWN            InputManager::DPadDown
    #define TOUCH_IM_FE_MOVE_LEFT            InputManager::DPadLeft
    #define TOUCH_IM_FE_MOVE_RIGHT           InputManager::DPadRight
    #define TOUCH_IM_FE_SELECT               InputManager::Start
    #define TOUCH_IM_FE_L1                   InputManager::Black
    #define TOUCH_IM_FE_R1                   InputManager::White

    #define TOUCH_IM_FE_FUNCTION_1           InputManager::A
    #define TOUCH_IM_FE_FUNCTION_2           InputManager::B

    #define TOUCH_IM_FE_MOUSE_LEFT           Input::INVALID_CONTROLLERID
    #define TOUCH_IM_FE_MOUSE_RIGHT          Input::INVALID_CONTROLLERID
    #define TOUCH_IM_FE_MOUSE_UP             Input::INVALID_CONTROLLERID
    #define TOUCH_IM_FE_MOUSE_DOWN           Input::INVALID_CONTROLLERID

    #define TOUCH_IM_P1_KBD_START            Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_GAS              Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_BRAKE            Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_EBRAKE           Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_NITRO            Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_LEFT             Input::INVALID_CONTROLLERID
    #define TOUCH_IM_P1_KBD_RIGHT            Input::INVALID_CONTROLLERID

#endif

//=============================================================================
// Complete touch action catalogue.
//=============================================================================

static const TouchActionDefinition sTouchActions[] =
{
    // ---------------------------------------------------------------------
    // Character / on-foot
    // ---------------------------------------------------------------------
    {
        TOUCH_ACTION_MOVE_UP,
        "MoveUp",
        TOUCH_IM_MOVE_UP,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_MOVE_DOWN,
        "MoveDown",
        TOUCH_IM_MOVE_DOWN,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_MOVE_LEFT,
        "MoveLeft",
        TOUCH_IM_MOVE_LEFT,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_MOVE_RIGHT,
        "MoveRight",
        TOUCH_IM_MOVE_RIGHT,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_ATTACK,
        "Attack",
        TOUCH_IM_ATTACK,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_JUMP,
        "Jump",
        TOUCH_IM_JUMP,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_SPRINT,
        "Sprint",
        TOUCH_IM_SPRINT,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_DO_ACTION,
        "DoAction",
        TOUCH_IM_DO_ACTION,
        TOUCH_ACTION_CATEGORY_CHARACTER,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        true
    },

    // ---------------------------------------------------------------------
    // Character camera
    // ---------------------------------------------------------------------
    {
        TOUCH_ACTION_CAMERA_LEFT,
        "CameraLeft",
        TOUCH_IM_CAMERA_LEFT,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_CAMERA_RIGHT,
        "CameraRight",
        TOUCH_IM_CAMERA_RIGHT,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_CAMERA_MOVE_IN,
        "CameraMoveIn",
        TOUCH_IM_CAMERA_MOVE_IN,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_CAMERA_MOVE_OUT,
        "CameraMoveOut",
        TOUCH_IM_CAMERA_MOVE_OUT,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_CAMERA_ZOOM,
        "CameraZoom",
        TOUCH_IM_CAMERA_ZOOM,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_CAMERA_LOOK_UP,
        "CameraLookUp",
        TOUCH_IM_CAMERA_LOOK_UP,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_CHARACTER,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },

    // ---------------------------------------------------------------------
    // Vehicle
    // ---------------------------------------------------------------------
    {
        TOUCH_ACTION_ACCELERATE,
        "Accelerate",
        TOUCH_IM_ACCELERATE,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_REVERSE,
        "Reverse",
        TOUCH_IM_REVERSE,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_STEER_LEFT,
        "SteerLeft",
        TOUCH_IM_STEER_LEFT,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_STEER_RIGHT,
        "SteerRight",
        TOUCH_IM_STEER_RIGHT,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_STICK,
        true
    },
    {
        TOUCH_ACTION_GET_OUT_CAR,
        "GetOutCar",
        TOUCH_IM_GET_OUT_CAR,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_HAND_BRAKE,
        "HandBrake",
        TOUCH_IM_HAND_BRAKE,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_HORN,
        "Horn",
        TOUCH_IM_HORN,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_RESET_CAR,
        "ResetCar",
        TOUCH_IM_RESET_CAR,
        TOUCH_ACTION_CATEGORY_VEHICLE,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        false
    },


    // ---------------------------------------------------------------------
    // Vehicle camera
    // ---------------------------------------------------------------------
    {
        TOUCH_ACTION_CAMERA_CAR_LEFT,
        "CameraCarLeft",
        TOUCH_IM_CAMERA_CAR_LEFT,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_CAMERA_CAR_RIGHT,
        "CameraCarRight",
        TOUCH_IM_CAMERA_CAR_RIGHT,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_CAMERA_CAR_LOOK_UP,
        "CameraCarLookUp",
        TOUCH_IM_CAMERA_CAR_LOOK_UP,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_CAMERA_CAR_LOOK_BACK,
        "CameraCarLookBack",
        TOUCH_IM_CAMERA_CAR_LOOK_BACK,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_CAMERA_TOGGLE,
        "CameraToggle",
        TOUCH_IM_CAMERA_TOGGLE,
        TOUCH_ACTION_CATEGORY_CAMERA,
        TOUCH_PROFILE_VEHICLE,
        TOUCH_CONTROL_TOGGLE_BUTTON,
        true
},
   

    // ---------------------------------------------------------------------
    // Frontend
    // ---------------------------------------------------------------------
    {
        TOUCH_ACTION_FE_BACK,
        "feBack",
        TOUCH_IM_FE_BACK,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_FE_MOVE_UP,
        "feMoveUp",
        TOUCH_IM_FE_MOVE_UP,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DPAD,
        true
    },
    {
        TOUCH_ACTION_FE_MOVE_DOWN,
        "feMoveDown",
        TOUCH_IM_FE_MOVE_DOWN,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DPAD,
        true
    },
    {
        TOUCH_ACTION_FE_MOVE_LEFT,
        "feMoveLeft",
        TOUCH_IM_FE_MOVE_LEFT,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DPAD,
        true
    },
    {
        TOUCH_ACTION_FE_MOVE_RIGHT,
        "feMoveRight",
        TOUCH_IM_FE_MOVE_RIGHT,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DPAD,
        true
    },
    {
        TOUCH_ACTION_FE_SELECT,
        "feSelect",
        TOUCH_IM_FE_SELECT,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_BUTTON,
        true
    },
    {
        TOUCH_ACTION_FE_FUNCTION_1,
        "feFunction1",
        TOUCH_IM_FE_FUNCTION_1,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_FE_FUNCTION_2,
        "feFunction2",
        TOUCH_IM_FE_FUNCTION_2,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_FE_MOUSE_LEFT,
        "feMouseLeft",
        TOUCH_IM_FE_MOUSE_LEFT,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_FE_MOUSE_RIGHT,
        "feMouseRight",
        TOUCH_IM_FE_MOUSE_RIGHT,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_FE_MOUSE_UP,
        "feMouseUp",
        TOUCH_IM_FE_MOUSE_UP,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },
    {
        TOUCH_ACTION_FE_MOUSE_DOWN,
        "feMouseDown",
        TOUCH_IM_FE_MOUSE_DOWN,
        TOUCH_ACTION_CATEGORY_FRONTEND,
        TOUCH_PROFILE_FRONTEND,
        TOUCH_CONTROL_DRAG_AREA,
        false
    },

    {
    TOUCH_ACTION_FE_L1,
    "feL1",
    TOUCH_IM_FE_L1,
    TOUCH_ACTION_CATEGORY_FRONTEND,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_CONTROL_BUTTON,
    false
    },
    {
    TOUCH_ACTION_FE_R1,
    "feR1",
    TOUCH_IM_FE_R1,
    TOUCH_ACTION_CATEGORY_FRONTEND,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_CONTROL_BUTTON,
    false
    },

    // ---------------------------------------------------------------------
    // PC / SuperSprint special actions
    // ---------------------------------------------------------------------
    {
    TOUCH_ACTION_SUPERSPRINT_CAMERA_TOGGLE,
    "SuperSprintCameraToggle",
    TOUCH_IM_SUPERSPRINT_CAMERA_TOGGLE,
    TOUCH_ACTION_CATEGORY_CAMERA,
    TOUCH_PROFILE_MINIGAME_VEHICLE,
    TOUCH_CONTROL_BUTTON,
    true
    },
    {
        TOUCH_ACTION_P1_KBD_START,
        "P1_KBD_Start",
        TOUCH_IM_P1_KBD_START,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_GAS,
        "P1_KBD_Gas",
        TOUCH_IM_P1_KBD_GAS,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_BRAKE,
        "P1_KBD_Brake",
        TOUCH_IM_P1_KBD_BRAKE,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_EBRAKE,
        "P1_KBD_EBrake",
        TOUCH_IM_P1_KBD_EBRAKE,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_NITRO,
        "P1_KBD_Nitro",
        TOUCH_IM_P1_KBD_NITRO,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_BUTTON,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_LEFT,
        "P1_KBD_Left",
        TOUCH_IM_P1_KBD_LEFT,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_STICK,
        false
    },
    {
        TOUCH_ACTION_P1_KBD_RIGHT,
        "P1_KBD_Right",
        TOUCH_IM_P1_KBD_RIGHT,
        TOUCH_ACTION_CATEGORY_SPECIAL,
        TOUCH_PROFILE_SPECIAL,
        TOUCH_CONTROL_STICK,
        false
    }
};

static const unsigned int sTouchActionCount =
    sizeof( sTouchActions ) / sizeof( sTouchActions[ 0 ] );

unsigned int TouchActionRegistry::GetActionCount()
{
    return sTouchActionCount;
}

const TouchActionDefinition* TouchActionRegistry::GetActionByIndex( unsigned int index )
{
    if ( index >= sTouchActionCount )
    {
        return 0;
    }

    return &sTouchActions[ index ];
}

const TouchActionDefinition* TouchActionRegistry::GetActionById( TouchActionId id )
{
    for ( unsigned int i = 0; i < sTouchActionCount; ++i )
    {
        if ( sTouchActions[ i ].id == id )
        {
            return &sTouchActions[ i ];
        }
    }

    return 0;
}

const TouchActionDefinition* TouchActionRegistry::GetActionByName( const char* name )
{
    if ( name == 0 )
    {
        return 0;
    }

    for ( unsigned int i = 0; i < sTouchActionCount; ++i )
    {
        if ( strcmp( sTouchActions[ i ].name, name ) == 0 )
        {
            return &sTouchActions[ i ];
        }
    }

    return 0;
}

const TouchActionDefinition* TouchActionRegistry::GetActionByInputManagerButtonId( int inputManagerButtonId )
{
    for ( unsigned int i = 0; i < sTouchActionCount; ++i )
    {
        if ( sTouchActions[ i ].inputManagerButtonId == inputManagerButtonId )
        {
            return &sTouchActions[ i ];
        }
    }

    return 0;
}

bool TouchActionRegistry::IsValidAction( TouchActionId id )
{
    return GetActionById( id ) != 0;
}

const char* TouchActionRegistry::GetActionName( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->name : "";
}

int TouchActionRegistry::GetInputManagerButtonId( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->inputManagerButtonId : Input::INVALID_CONTROLLERID;
}

TouchActionCategory TouchActionRegistry::GetActionCategory( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->category : TOUCH_ACTION_CATEGORY_SPECIAL;
}

TouchProfile TouchActionRegistry::GetDefaultProfile( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->defaultProfile : TOUCH_PROFILE_HIDDEN;
}

TouchControlKind TouchActionRegistry::GetDefaultControlKind( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->defaultControl : TOUCH_CONTROL_NONE;
}

bool TouchActionRegistry::IsVisibleByDefault( TouchActionId id )
{
    const TouchActionDefinition* action = GetActionById( id );
    return action ? action->visibleByDefault : false;
}