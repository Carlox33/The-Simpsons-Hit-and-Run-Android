#ifndef TOUCHCONTROLSCONFIGURATIONMANAGER_H_
#define TOUCHCONTROLSCONFIGURATIONMANAGER_H_

#include <input/touch/touchhudsystem.h>

//=============================================================================
// TouchControlsConfigurationManager
//
// Owns the persistent touch controls configuration file.
//
// Responsibilities:
// - create configuration file if missing
// - maintain a separate version file
// - load configuration once into memory
// - save user customizations
// - provide effective rects: base rect + offset + scale
//=============================================================================

class TouchControlsConfigurationManager
{
public:
    static TouchControlsConfigurationManager& GetInstance();

    bool Initialize();
    void Shutdown();

    bool IsInitialized() const;

    bool Save();

    float GetOpacity() const;
    float GetPressedOpacity() const;

    void SetOpacity( float opacity );
    void SetPressedOpacity( float opacity );

    const TouchControlCustomization& GetCustomization
    (
        TouchHudControlId controlId
    ) const;

    void SetCustomization
    (
        TouchHudControlId controlId,
        const TouchControlCustomization& customization
    );

    void SetControlOffset
    (
        TouchHudControlId controlId,
        float offsetX,
        float offsetY
    );

    void AddControlOffset
    (
        TouchHudControlId controlId,
        float deltaX,
        float deltaY
    );

    void AdvanceControlSizeStep( TouchHudControlId controlId );

    void ResetControl( TouchHudControlId controlId );
    void ResetLayout( TouchEditableLayout layout );
    void ResetAll();

    TouchRect GetEffectiveRect
    (
        TouchHudControlId controlId,
        const TouchRect& baseRect
    ) const;

private:
    TouchControlsConfigurationManager();
    ~TouchControlsConfigurationManager();

    TouchControlsConfigurationManager( const TouchControlsConfigurationManager& );
    TouchControlsConfigurationManager& operator=( const TouchControlsConfigurationManager& );

    bool BuildPaths();

    bool EnsureDirectory( const char* path ) const;
    bool FileExists( const char* path ) const;

    bool EnsureConfigurationFile();

    bool IsVersionFileValid() const;
    bool WriteVersionFile() const;

    bool WriteDefaultConfigurationFile() const;
    bool Load();

    void ResetDefaultsInMemory();

    bool IsValidControlId( TouchHudControlId controlId ) const;
    bool IsEditorControl( TouchHudControlId controlId ) const;
    bool IsControlInLayout( TouchHudControlId controlId, TouchEditableLayout layout ) const;

    TouchHudControlId FindControlIdByName( const char* name ) const;

    float ClampOpacity( float value ) const;
    int ClampSizeStep( int value ) const;

    
private:
    enum
    {
        MAX_TOUCH_CONFIG_PATH = 512,
        MAX_TOUCH_CONFIG_LINE = 256
    };

    bool mInitialized;
    bool mDirty;

    char mConfigRoot[ MAX_TOUCH_CONFIG_PATH ];
    char mConfigPath[ MAX_TOUCH_CONFIG_PATH ];
    char mVersionPath[ MAX_TOUCH_CONFIG_PATH ];

    TouchControlCustomization mCustomizations[ TOUCH_HUD_CONTROL_COUNT ];

    float mOpacity;
    float mPressedOpacity;
};

#endif // TOUCHCONTROLSCONFIGURATIONMANAGER_H_