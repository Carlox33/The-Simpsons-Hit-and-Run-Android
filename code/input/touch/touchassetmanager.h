#ifndef TOUCHASSETMANAGER_H_
#define TOUCHASSETMANAGER_H_

//=============================================================================
// TouchAssetManager
//
// Loads touch control images extracted from APK assets into Pure3D sprites.
// The images are loaded once and cached.
// Rendering will be handled later by TouchHudRenderer.
//=============================================================================

#include <input/touch/touchhudsystem.h>

class tSprite;

enum TouchAssetId
{
    TOUCH_ASSET_INVALID = -1,

    // Character
    TOUCH_ASSET_CHARACTER_JOYSTICK_BASE = 0,
    TOUCH_ASSET_CHARACTER_JOYSTICK_KNOB,
    TOUCH_ASSET_CHARACTER_SPRINT,
    TOUCH_ASSET_CHARACTER_CONTEXT_EXCLAMATION,
    TOUCH_ASSET_CHARACTER_ATTACK,
    TOUCH_ASSET_CHARACTER_DOOR,
    TOUCH_ASSET_CHARACTER_ENTER_CAR,
    TOUCH_ASSET_CHARACTER_JUMP,
    TOUCH_ASSET_CHARACTER_PHONE,
    TOUCH_ASSET_CHARACTER_DOLLAR,

    // Frontend
    TOUCH_ASSET_FRONTEND_BACK,
    TOUCH_ASSET_FRONTEND_SELECT,
    TOUCH_ASSET_FRONTEND_DOWN,
    TOUCH_ASSET_FRONTEND_UP,
    TOUCH_ASSET_FRONTEND_RIGHT,
    TOUCH_ASSET_FRONTEND_LEFT,
    TOUCH_ASSET_FRONTEND_START,

    // Vehicle
    TOUCH_ASSET_VEHICLE_ACCELERATE,
    TOUCH_ASSET_VEHICLE_HORN,
    TOUCH_ASSET_VEHICLE_HANDBRAKE,
    TOUCH_ASSET_VEHICLE_BRAKE,
    TOUCH_ASSET_VEHICLE_EXIT,
    TOUCH_ASSET_VEHICLE_RESET,
    TOUCH_ASSET_VEHICLE_CAMERA_TOGGLE,

    // Editor
    TOUCH_ASSET_EDITOR_MOVE_CONTROLS,
    TOUCH_ASSET_EDITOR_LAYOUT_1_3,
    TOUCH_ASSET_EDITOR_LAYOUT_2_3,
    TOUCH_ASSET_EDITOR_LAYOUT_3_3,
    TOUCH_ASSET_EDITOR_RESET,

    TOUCH_ASSET_COUNT
};

class TouchAssetManager
{
public:
    static TouchAssetManager& GetInstance();

    bool Initialize();
    void Shutdown();

    bool IsInitialized() const;

    const char* GetAssetRoot() const;
    const char* GetLastError() const;

    tSprite* GetSprite( TouchAssetId assetId ) const;

    // Compatibility/helper for the first render tests.
    tSprite* GetCharacterJumpSprite() const;


    tSprite* GetSpriteForControl( TouchHudControlId controlId ) const;
    tSprite* GetSpriteForInteractionIcon( TouchInteractionIcon icon ) const;
    tSprite* GetSpriteForEditorNextLayout( TouchEditableLayout layout ) const;

private:
    TouchAssetManager();
    ~TouchAssetManager();

    TouchAssetManager( const TouchAssetManager& );
    TouchAssetManager& operator=( const TouchAssetManager& );

    bool BuildAssetRoot();

    bool LoadAllSprites();
    bool LoadSpriteForAsset
    (
        TouchAssetId assetId,
        const char* relativePath,
        const char* spriteName
    );

    tSprite* LoadSpriteFromFile
    (
        const char* relativePath,
        const char* spriteName
    );

    void ReleaseSprite( tSprite*& sprite );
    void ReleaseAllSprites();

    void SetLastError( const char* error );

private:
    enum
    {
        MAX_TOUCH_PATH = 512,
        MAX_TOUCH_ERROR = 256
    };

    bool mInitialized;

    char mAssetRoot[ MAX_TOUCH_PATH ];
    char mLastError[ MAX_TOUCH_ERROR ];

    tSprite* mSprites[ TOUCH_ASSET_COUNT ];
};

#endif // TOUCHASSETMANAGER_H_