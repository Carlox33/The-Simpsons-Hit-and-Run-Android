#ifndef TOUCHTYPES_H_
#define TOUCHTYPES_H_

//=============================================================================
// Touch input common types
//
// This file intentionally contains only small enums/structs.
// It should be safe to include from input, HUD, renderer and gameplay helpers.
//=============================================================================

enum TouchInputMode
{
    TOUCH_INPUT_MODE_TOUCH = 0,
    TOUCH_INPUT_MODE_GAMEPAD
};

enum TouchProfile
{
    TOUCH_PROFILE_HIDDEN = 0,
    TOUCH_PROFILE_FRONTEND,
    TOUCH_PROFILE_CHARACTER,
    TOUCH_PROFILE_VEHICLE,
    TOUCH_PROFILE_MINIGAME_VEHICLE,
    TOUCH_PROFILE_MINIGAME_SUMMARY,
    TOUCH_PROFILE_CINEMATIC,
    TOUCH_PROFILE_SCRAPBOOK_CONTENTS,
    TOUCH_PROFILE_SPECIAL
};

enum TouchActionCategory
{
    TOUCH_ACTION_CATEGORY_CHARACTER = 0,
    TOUCH_ACTION_CATEGORY_VEHICLE,
    TOUCH_ACTION_CATEGORY_FRONTEND,
    TOUCH_ACTION_CATEGORY_CAMERA,
    TOUCH_ACTION_CATEGORY_SPECIAL
};

enum TouchControlKind
{
    TOUCH_CONTROL_NONE = 0,
    TOUCH_CONTROL_BUTTON,
    TOUCH_CONTROL_STICK,
    TOUCH_CONTROL_DPAD,
    TOUCH_CONTROL_DRAG_AREA,
    TOUCH_CONTROL_TOGGLE_BUTTON,
    TOUCH_CONTROL_SWIPE_GESTURE
};

enum TouchActionId
{
    TOUCH_ACTION_INVALID = -1,

    // Character / on-foot actions
    TOUCH_ACTION_MOVE_UP = 0,
    TOUCH_ACTION_MOVE_DOWN,
    TOUCH_ACTION_MOVE_LEFT,
    TOUCH_ACTION_MOVE_RIGHT,
    TOUCH_ACTION_ATTACK,
    TOUCH_ACTION_JUMP,
    TOUCH_ACTION_SPRINT,
    TOUCH_ACTION_DO_ACTION,

    // Character camera actions
    TOUCH_ACTION_CAMERA_LEFT,
    TOUCH_ACTION_CAMERA_RIGHT,
    TOUCH_ACTION_CAMERA_MOVE_IN,
    TOUCH_ACTION_CAMERA_MOVE_OUT,
    TOUCH_ACTION_CAMERA_ZOOM,
    TOUCH_ACTION_CAMERA_LOOK_UP,

    // Vehicle actions
    TOUCH_ACTION_ACCELERATE,
    TOUCH_ACTION_REVERSE,
    TOUCH_ACTION_STEER_LEFT,
    TOUCH_ACTION_STEER_RIGHT,
    TOUCH_ACTION_GET_OUT_CAR,
    TOUCH_ACTION_HAND_BRAKE,
    TOUCH_ACTION_HORN,
    TOUCH_ACTION_RESET_CAR,

    // Vehicle camera actions
    TOUCH_ACTION_CAMERA_CAR_LEFT,
    TOUCH_ACTION_CAMERA_CAR_RIGHT,
    TOUCH_ACTION_CAMERA_CAR_LOOK_UP,
    TOUCH_ACTION_CAMERA_CAR_LOOK_BACK,
    TOUCH_ACTION_CAMERA_TOGGLE,
    // Special toggle minigame
    TOUCH_ACTION_SUPERSPRINT_CAMERA_TOGGLE,
    // Frontend actions
    TOUCH_ACTION_FE_BACK,
    TOUCH_ACTION_FE_MOVE_UP,
    TOUCH_ACTION_FE_MOVE_DOWN,
    TOUCH_ACTION_FE_MOVE_LEFT,
    TOUCH_ACTION_FE_MOVE_RIGHT,
    TOUCH_ACTION_FE_SELECT, // Esto es el boton start
    TOUCH_ACTION_FE_FUNCTION_1,
    TOUCH_ACTION_FE_FUNCTION_2,
    TOUCH_ACTION_FE_MOUSE_LEFT,
    TOUCH_ACTION_FE_MOUSE_RIGHT,
    TOUCH_ACTION_FE_MOUSE_UP,
    TOUCH_ACTION_FE_MOUSE_DOWN,
    TOUCH_ACTION_FE_L1,
    TOUCH_ACTION_FE_R1,

    // PC / SuperSprint special actions
    TOUCH_ACTION_P1_KBD_START,
    TOUCH_ACTION_P1_KBD_GAS,
    TOUCH_ACTION_P1_KBD_BRAKE,
    TOUCH_ACTION_P1_KBD_EBRAKE,
    TOUCH_ACTION_P1_KBD_NITRO,
    TOUCH_ACTION_P1_KBD_LEFT,
    TOUCH_ACTION_P1_KBD_RIGHT,

    TOUCH_ACTION_COUNT
};

enum TouchInteractionType
{
    TOUCH_INTERACTION_NONE = 0,

    TOUCH_INTERACTION_MISSION_MAIN,
    TOUCH_INTERACTION_MISSION_BONUS,
    TOUCH_INTERACTION_RACE,

    TOUCH_INTERACTION_INTERIOR_DOOR,
    TOUCH_INTERACTION_PHONE_BOOTH,
    TOUCH_INTERACTION_PURCHASE,
    TOUCH_INTERACTION_CLOTHING,
    TOUCH_INTERACTION_ENTER_CAR
};

enum TouchInteractionIcon
{
    TOUCH_INTERACTION_ICON_NONE = 0,
    TOUCH_INTERACTION_ICON_EXCLAMATION,
    TOUCH_INTERACTION_ICON_DOOR,
    TOUCH_INTERACTION_ICON_PHONE,
    TOUCH_INTERACTION_ICON_DOLLAR,
    TOUCH_INTERACTION_ICON_ENTER_CAR
};



struct TouchVector2
{
    TouchVector2() : x( 0.0f ), y( 0.0f ) {}
    TouchVector2( float inX, float inY ) : x( inX ), y( inY ) {}

    float x;
    float y;
};

struct TouchRect
{
    TouchRect()
        :
        x( 0.0f ),
        y( 0.0f ),
        width( 0.0f ),
        height( 0.0f )
    {
    }

    TouchRect( float inX, float inY, float inWidth, float inHeight )
        :
        x( inX ),
        y( inY ),
        width( inWidth ),
        height( inHeight )
    {
    }

    float x;
    float y;
    float width;
    float height;
};

inline const char* TouchInputModeToString( TouchInputMode mode )
{
    switch ( mode )
    {
        case TOUCH_INPUT_MODE_TOUCH:
            return "Touch";
        case TOUCH_INPUT_MODE_GAMEPAD:
            return "Gamepad";
        default:
            return "Unknown";
    }
}

inline const char* TouchProfileToString( TouchProfile profile )
{
    switch ( profile )
    {
        case TOUCH_PROFILE_HIDDEN:
            return "Hidden";
        case TOUCH_PROFILE_FRONTEND:
            return "Frontend";
        case TOUCH_PROFILE_CHARACTER:
            return "Character";
        case TOUCH_PROFILE_VEHICLE:
            return "Vehicle";
        case TOUCH_PROFILE_MINIGAME_VEHICLE:
            return "MinigameVehicle";
        case TOUCH_PROFILE_MINIGAME_SUMMARY:
            return "MinigameSummary";
        case TOUCH_PROFILE_CINEMATIC:
            return "Cinematic";
        case TOUCH_PROFILE_SPECIAL:
            return "Special";
        default:
            return "Unknown";
    }
}

inline const char* TouchControlKindToString( TouchControlKind kind )
{
    switch ( kind )
    {
        case TOUCH_CONTROL_NONE:
            return "None";
        case TOUCH_CONTROL_BUTTON:
            return "Button";
        case TOUCH_CONTROL_STICK:
            return "Stick";
        case TOUCH_CONTROL_DPAD:
            return "DPad";
        case TOUCH_CONTROL_DRAG_AREA:
            return "DragArea";
        case TOUCH_CONTROL_TOGGLE_BUTTON:
            return "ToggleButton";
        case TOUCH_CONTROL_SWIPE_GESTURE:
            return "SwipeGesture";
        default:
            return "Unknown";
    }
}

inline const char* TouchActionCategoryToString( TouchActionCategory category )
{
    switch ( category )
    {
        case TOUCH_ACTION_CATEGORY_CHARACTER:
            return "Character";
        case TOUCH_ACTION_CATEGORY_VEHICLE:
            return "Vehicle";
        case TOUCH_ACTION_CATEGORY_FRONTEND:
            return "Frontend";
        case TOUCH_ACTION_CATEGORY_CAMERA:
            return "Camera";
        case TOUCH_ACTION_CATEGORY_SPECIAL:
            return "Special";
        default:
            return "Unknown";
    }
}

inline TouchInteractionIcon TouchInteractionTypeToIcon( TouchInteractionType type )
{
    switch ( type )
    {
        case TOUCH_INTERACTION_MISSION_MAIN:
        case TOUCH_INTERACTION_MISSION_BONUS:
        case TOUCH_INTERACTION_RACE:
        {
            return TOUCH_INTERACTION_ICON_EXCLAMATION;
        }

        case TOUCH_INTERACTION_INTERIOR_DOOR:
        {
            return TOUCH_INTERACTION_ICON_DOOR;
        }

        case TOUCH_INTERACTION_PHONE_BOOTH:
        {
            return TOUCH_INTERACTION_ICON_PHONE;
        }

        case TOUCH_INTERACTION_PURCHASE:
        case TOUCH_INTERACTION_CLOTHING:
        {
            return TOUCH_INTERACTION_ICON_DOLLAR;
        }

        case TOUCH_INTERACTION_ENTER_CAR:
        {
            return TOUCH_INTERACTION_ICON_ENTER_CAR;
        }

        case TOUCH_INTERACTION_NONE:
        default:
        {
            return TOUCH_INTERACTION_ICON_NONE;
        }
    }
}

#endif // TOUCHTYPES_H_