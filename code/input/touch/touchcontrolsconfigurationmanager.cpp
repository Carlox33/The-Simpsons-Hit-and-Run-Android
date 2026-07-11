#include <input/touch/touchcontrolsconfigurationmanager.h>

#include <stdio.h>
#include <string.h>

#if defined(RAD_ANDROID)
#include <SDL.h>
#include <SDL_system.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <android/log.h>

#define TOUCH_CONTROLS_CONFIG_LOG_TAG "SimpsonsHitAndRun"
#define TOUCH_CONTROLS_CONFIG_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TOUCH_CONTROLS_CONFIG_LOG_TAG, __VA_ARGS__)
#define TOUCH_CONTROLS_CONFIG_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TOUCH_CONTROLS_CONFIG_LOG_TAG, __VA_ARGS__)
#else
#define TOUCH_CONTROLS_CONFIG_LOGI(...)
#define TOUCH_CONTROLS_CONFIG_LOGE(...)
#endif

// Increase this when the TXT format changes and must be migrated or regenered if is corrupted or not exist
static const char* TOUCH_CONTROLS_CONFIGURATION_VERSION = "1";

static const char* TOUCH_CONTROLS_CONFIGURATION_FILENAME =
    "Simpsons_touch_controls_configuration.txt";

static const char* TOUCH_CONTROLS_CONFIGURATION_VERSION_FILENAME =
    "Simpsons_touch_controls_configuration.version";

static const float TOUCH_CONTROLS_DEFAULT_OPACITY = 0.25f;
static const float TOUCH_CONTROLS_DEFAULT_PRESSED_OPACITY = 0.85f;

//=============================================================================
// Local helpers
//=============================================================================

static char* TrimTouchConfigLine( char* text )
{
    if ( text == 0 )
    {
        return text;
    }

    while
    (
        *text == ' '  ||
        *text == '\t' ||
        *text == '\r' ||
        *text == '\n'
    )
    {
        ++text;
    }

    if ( *text == '\0' )
    {
        return text;
    }

    char* end = text + strlen( text ) - 1;

    while
    (
        end > text &&
        (
            *end == ' '  ||
            *end == '\t' ||
            *end == '\r' ||
            *end == '\n'
        )
    )
    {
        *end = '\0';
        --end;
    }

    return text;
}

//=============================================================================
// Singleton
//=============================================================================

TouchControlsConfigurationManager& TouchControlsConfigurationManager::GetInstance()
{
    static TouchControlsConfigurationManager sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchControlsConfigurationManager::TouchControlsConfigurationManager()
:
mInitialized( false ),
mDirty( false ),
mOpacity( TOUCH_CONTROLS_DEFAULT_OPACITY ),
mPressedOpacity( TOUCH_CONTROLS_DEFAULT_PRESSED_OPACITY )
{
    mConfigRoot[ 0 ] = '\0';
    mConfigPath[ 0 ] = '\0';
    mVersionPath[ 0 ] = '\0';

    ResetDefaultsInMemory();
}

TouchControlsConfigurationManager::~TouchControlsConfigurationManager()
{
    Shutdown();
}

//=============================================================================
// Public
//=============================================================================

bool TouchControlsConfigurationManager::Initialize()
{
    if ( mInitialized )
    {
        return true;
    }

    ResetDefaultsInMemory();

    if ( !BuildPaths() )
    {
        TOUCH_CONTROLS_CONFIG_LOGE(
            "[TouchControlsConfig] BuildPaths failed."
        );
        return false;
    }

    if ( !EnsureConfigurationFile() )
    {
        TOUCH_CONTROLS_CONFIG_LOGE(
            "[TouchControlsConfig] EnsureConfigurationFile failed."
        );
        return false;
    }

    if ( !Load() )
    {
        TOUCH_CONTROLS_CONFIG_LOGE(
            "[TouchControlsConfig] Load failed. Recreating default configuration."
        );

        ResetDefaultsInMemory();

        if ( !WriteDefaultConfigurationFile() )
        {
            return false;
        }

        if ( !WriteVersionFile() )
        {
            return false;
        }

        if ( !Load() )
        {
            return false;
        }
    }

    mInitialized = true;
    mDirty = false;

   

    return true;
}

void TouchControlsConfigurationManager::Shutdown()
{
    mInitialized = false;
    mDirty = false;
}

bool TouchControlsConfigurationManager::IsInitialized() const
{
    return mInitialized;
}

bool TouchControlsConfigurationManager::Save()
{
#if !defined(RAD_ANDROID)
    return true;
#else
    if ( !BuildPaths() )
    {
        return false;
    }

    if ( !EnsureDirectory( mConfigRoot ) )
    {
        return false;
    }

    FILE* file = fopen( mConfigPath, "wb" );

    if ( file == 0 )
    {
        TOUCH_CONTROLS_CONFIG_LOGE(
            "[TouchControlsConfig] Failed to open config for write: %s",
            mConfigPath
        );
        return false;
    }

    fprintf( file, "# Simpsons Hit & Run Android touch controls configuration\n" );
    fprintf( file, "# version=%s\n\n", TOUCH_CONTROLS_CONFIGURATION_VERSION );

    fprintf( file, "[GENERAL]\n" );
    fprintf( file, "# Default values: opactity-> 0.25 pressed_opacity-> 0.85\n\n" );
    fprintf( file, "opacity=%.3f\n", mOpacity );
    fprintf( file, "pressed_opacity=%.3f\n\n", mPressedOpacity );

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const TouchEditableLayout layouts[ TOUCH_EDITABLE_LAYOUT_COUNT ] =
    {
        TOUCH_EDITABLE_LAYOUT_CHARACTER,
        TOUCH_EDITABLE_LAYOUT_VEHICLE,
        TOUCH_EDITABLE_LAYOUT_FRONTEND
    };

    const char* sectionNames[ TOUCH_EDITABLE_LAYOUT_COUNT ] =
    {
        "CHARACTER",
        "VEHICLE",
        "FRONTEND"
    };

    for ( int layoutIndex = 0; layoutIndex < TOUCH_EDITABLE_LAYOUT_COUNT; ++layoutIndex )
    {
        fprintf( file, "[%s]\n", sectionNames[ layoutIndex ] );

        const unsigned int controlCount = hudSystem.GetControlCount();

        for ( unsigned int i = 0; i < controlCount; ++i )
        {
            const TouchHudControlDefinition* control =
                hudSystem.GetControlByIndex( i );

            if ( control == 0 )
            {
                continue;
            }

            if ( !control->enabled || !control->visibleByDefault )
            {
                continue;
            }

            if ( IsEditorControl( control->id ) )
            {
                continue;
            }

            if ( !IsControlInLayout(
                    control->id,
                    layouts[ layoutIndex ] ) )
            {
                continue;
            }

            if ( control->name == 0 || control->name[ 0 ] == '\0' )
            {
                continue;
            }

            const TouchControlCustomization& customization =
                mCustomizations[ control->id ];

            fprintf(
                file,
                "%s=%.3f,%.3f,%d\n",
                control->name,
                customization.offsetX,
                customization.offsetY,
                ClampSizeStep( customization.sizeStep )
            );
        }

        fprintf( file, "\n" );
    }

    fclose( file );

    if ( !WriteVersionFile() )
    {
        return false;
    }

    mDirty = false;

   

    return true;
#endif
}

float TouchControlsConfigurationManager::GetOpacity() const
{
    return mOpacity;
}

float TouchControlsConfigurationManager::GetPressedOpacity() const
{
    return mPressedOpacity;
}

void TouchControlsConfigurationManager::SetOpacity( float opacity )
{
    mOpacity = ClampOpacity( opacity );
    mDirty = true;
}

void TouchControlsConfigurationManager::SetPressedOpacity( float opacity )
{
    mPressedOpacity = ClampOpacity( opacity );
    mDirty = true;
}

const TouchControlCustomization& TouchControlsConfigurationManager::GetCustomization
(
    TouchHudControlId controlId
) const
{
    if ( !IsValidControlId( controlId ) )
    {
        return mCustomizations[ TOUCH_HUD_CONTROL_NONE ];
    }

    return mCustomizations[ controlId ];
}

void TouchControlsConfigurationManager::SetCustomization
(
    TouchHudControlId controlId,
    const TouchControlCustomization& customization
)
{
    if ( !IsValidControlId( controlId ) )
    {
        return;
    }

    TouchControlCustomization safeCustomization = customization;
    safeCustomization.sizeStep = ClampSizeStep( safeCustomization.sizeStep );

    mCustomizations[ controlId ] = safeCustomization;
    mDirty = true;
}

void TouchControlsConfigurationManager::SetControlOffset
(
    TouchHudControlId controlId,
    float offsetX,
    float offsetY
)
{
    if ( !IsValidControlId( controlId ) )
    {
        return;
    }

    mCustomizations[ controlId ].offsetX = offsetX;
    mCustomizations[ controlId ].offsetY = offsetY;
    mDirty = true;
}

void TouchControlsConfigurationManager::AddControlOffset
(
    TouchHudControlId controlId,
    float deltaX,
    float deltaY
)
{
    if ( !IsValidControlId( controlId ) )
    {
        return;
    }

    mCustomizations[ controlId ].offsetX += deltaX;
    mCustomizations[ controlId ].offsetY += deltaY;
    mDirty = true;
}

void TouchControlsConfigurationManager::AdvanceControlSizeStep
(
    TouchHudControlId controlId
)
{
    if ( !IsValidControlId( controlId ) )
    {
        return;
    }

    int nextStep = mCustomizations[ controlId ].sizeStep + 1;

    if ( nextStep > TOUCH_CONTROL_SIZE_MAX_STEP )
    {
        nextStep = TOUCH_CONTROL_SIZE_MIN_STEP;
    }

    mCustomizations[ controlId ].sizeStep = nextStep;
    mDirty = true;
}

void TouchControlsConfigurationManager::ResetControl( TouchHudControlId controlId )
{
    if ( !IsValidControlId( controlId ) )
    {
        return;
    }

    mCustomizations[ controlId ].Reset();
    mDirty = true;
}

void TouchControlsConfigurationManager::ResetLayout( TouchEditableLayout layout )
{
    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const unsigned int controlCount = hudSystem.GetControlCount();

    for ( unsigned int i = 0; i < controlCount; ++i )
    {
        const TouchHudControlDefinition* control =
            hudSystem.GetControlByIndex( i );

        if ( control == 0 )
        {
            continue;
        }

        if ( IsEditorControl( control->id ) )
        {
            continue;
        }

        if ( IsControlInLayout( control->id, layout ) )
        {
            ResetControl( control->id );
        }
    }

    mDirty = true;
}

void TouchControlsConfigurationManager::ResetAll()
{
    ResetDefaultsInMemory();
    mDirty = true;
}

TouchRect TouchControlsConfigurationManager::GetEffectiveRect
(
    TouchHudControlId controlId,
    const TouchRect& baseRect
) const
{
    if ( !IsValidControlId( controlId ) )
    {
        return baseRect;
    }

    const TouchControlCustomization& customization =
        mCustomizations[ controlId ];

    const float scale =
        TouchControlSizeStepToScale( customization.sizeStep );

    TouchRect result;

    result.width = baseRect.width * scale;
    result.height = baseRect.height * scale;

    const float baseCenterX =
        baseRect.x + ( baseRect.width * 0.5f );

    const float baseCenterY =
        baseRect.y + ( baseRect.height * 0.5f );

    const float finalCenterX =
        baseCenterX + customization.offsetX;

    const float finalCenterY =
        baseCenterY + customization.offsetY;

    result.x = finalCenterX - ( result.width * 0.5f );
    result.y = finalCenterY - ( result.height * 0.5f );

    if ( result.width > 1.0f )
    {
        result.width = 1.0f;
    }

    if ( result.height > 1.0f )
    {
        result.height = 1.0f;
    }

    if ( result.x < 0.0f )
    {
        result.x = 0.0f;
    }

    if ( result.y < 0.0f )
    {
        result.y = 0.0f;
    }

    if ( result.x + result.width > 1.0f )
    {
        result.x = 1.0f - result.width;
    }

    if ( result.y + result.height > 1.0f )
    {
        result.y = 1.0f - result.height;
    }

    return result;
}

//=============================================================================
// Internal
//=============================================================================

bool TouchControlsConfigurationManager::BuildPaths()
{
#if !defined(RAD_ANDROID)
    strncpy( mConfigRoot, ".", sizeof( mConfigRoot ) );
    mConfigRoot[ sizeof( mConfigRoot ) - 1 ] = '\0';

    strncpy(
        mConfigPath,
        TOUCH_CONTROLS_CONFIGURATION_FILENAME,
        sizeof( mConfigPath )
    );
    mConfigPath[ sizeof( mConfigPath ) - 1 ] = '\0';

    strncpy(
        mVersionPath,
        TOUCH_CONTROLS_CONFIGURATION_VERSION_FILENAME,
        sizeof( mVersionPath )
    );
    mVersionPath[ sizeof( mVersionPath ) - 1 ] = '\0';

    return true;
#else
    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == 0 || storagePath[ 0 ] == '\0' )
    {
        return false;
    }

    snprintf(
        mConfigRoot,
        sizeof( mConfigRoot ),
        "%s/touch_controls",
        storagePath
    );

    snprintf(
        mConfigPath,
        sizeof( mConfigPath ),
        "%s/%s",
        mConfigRoot,
        TOUCH_CONTROLS_CONFIGURATION_FILENAME
    );

    snprintf(
        mVersionPath,
        sizeof( mVersionPath ),
        "%s/%s",
        mConfigRoot,
        TOUCH_CONTROLS_CONFIGURATION_VERSION_FILENAME
    );

    return true;
#endif
}

bool TouchControlsConfigurationManager::EnsureDirectory( const char* path ) const
{
#if !defined(RAD_ANDROID)
    (void)path;
    return true;
#else
    if ( path == 0 || path[ 0 ] == '\0' )
    {
        return false;
    }

    char temp[ MAX_TOUCH_CONFIG_PATH ];
    strncpy( temp, path, sizeof( temp ) );
    temp[ sizeof( temp ) - 1 ] = '\0';

    for ( char* p = temp + 1; *p != '\0'; ++p )
    {
        if ( *p == '/' )
        {
            *p = '\0';

            if ( mkdir( temp, 0777 ) != 0 && errno != EEXIST )
            {
                *p = '/';
                return false;
            }

            *p = '/';
        }
    }

    if ( mkdir( temp, 0777 ) != 0 && errno != EEXIST )
    {
        return false;
    }

    return true;
#endif
}

bool TouchControlsConfigurationManager::FileExists( const char* path ) const
{
#if !defined(RAD_ANDROID)
    FILE* file = fopen( path, "rb" );

    if ( file == 0 )
    {
        return false;
    }

    fclose( file );
    return true;
#else
    if ( path == 0 || path[ 0 ] == '\0' )
    {
        return false;
    }

    struct stat info;
    return stat( path, &info ) == 0 && S_ISREG( info.st_mode );
#endif
}

bool TouchControlsConfigurationManager::EnsureConfigurationFile()
{
    if ( !EnsureDirectory( mConfigRoot ) )
    {
        return false;
    }

    const bool configExists = FileExists( mConfigPath );
    const bool versionValid = IsVersionFileValid();

    // Primera instalación: no existe TXT.
    // Creamos configuración completa por defecto.
    if ( !configExists )
    {
        ResetDefaultsInMemory();

        if ( !WriteDefaultConfigurationFile() )
        {
            return false;
        }

        if ( !WriteVersionFile() )
        {
            return false;
        }

        return true;
    }

    // Config existe y versión correcta.
    // No tocamos nada para no sobrescribir al usuario.
    if ( versionValid )
    {
        return true;
    }

    // Config existe pero la versión no coincide o falta el .version.
    // Migración conservadora:
    // 1. Cargamos lo que exista.
    // 2. Los controles que ya estaban se conservan.
    // 3. Los controles nuevos quedan con defaults porque Load() empieza con ResetDefaultsInMemory().
    // 4. Guardamos de nuevo con el formato actual y nueva versión.
    if ( Load() )
    {
        if ( !Save() )
        {
            return false;
        }

        if ( !WriteVersionFile() )
        {
            return false;
        }

        return true;
    }
   
    // Si el archivo existe pero está corrupto o no se puede leer,
    // entonces sí regeneramos defaults.
    ResetDefaultsInMemory();

    if ( !WriteDefaultConfigurationFile() )
    {
        return false;
    }

    if ( !WriteVersionFile() )
    {
        return false;
    }

    return true;
}

bool TouchControlsConfigurationManager::IsVersionFileValid() const
{
    FILE* file = fopen( mVersionPath, "rb" );

    if ( file == 0 )
    {
        return false;
    }

    char versionBuffer[ 32 ];
    memset( versionBuffer, 0, sizeof( versionBuffer ) );

    fread( versionBuffer, 1, sizeof( versionBuffer ) - 1, file );
    fclose( file );

    return strcmp(
        versionBuffer,
        TOUCH_CONTROLS_CONFIGURATION_VERSION
    ) == 0;
}

bool TouchControlsConfigurationManager::WriteVersionFile() const
{
    FILE* file = fopen( mVersionPath, "wb" );

    if ( file == 0 )
    {
        return false;
    }

    fwrite(
        TOUCH_CONTROLS_CONFIGURATION_VERSION,
        1,
        strlen( TOUCH_CONTROLS_CONFIGURATION_VERSION ),
        file
    );

    fclose( file );
    return true;
}

bool TouchControlsConfigurationManager::WriteDefaultConfigurationFile() const
{
#if !defined(RAD_ANDROID)
    return true;
#else
    FILE* file = fopen( mConfigPath, "wb" );

    if ( file == 0 )
    {
        return false;
    }

    fprintf( file, "# Simpsons Hit & Run Android touch controls configuration\n" );
    fprintf( file, "# This file is automatically created if missing.\n" );
    fprintf( file, "# version=%s\n\n", TOUCH_CONTROLS_CONFIGURATION_VERSION );

    fprintf( file, "[GENERAL]\n" );
    fprintf( file, "version=%s\n", TOUCH_CONTROLS_CONFIGURATION_VERSION );
    fprintf( file, "opacity=%.3f\n", TOUCH_CONTROLS_DEFAULT_OPACITY );
    fprintf( file, "pressed_opacity=%.3f\n\n", TOUCH_CONTROLS_DEFAULT_PRESSED_OPACITY );

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const TouchEditableLayout layouts[ TOUCH_EDITABLE_LAYOUT_COUNT ] =
    {
        TOUCH_EDITABLE_LAYOUT_CHARACTER,
        TOUCH_EDITABLE_LAYOUT_VEHICLE,
        TOUCH_EDITABLE_LAYOUT_FRONTEND
    };

    const char* sectionNames[ TOUCH_EDITABLE_LAYOUT_COUNT ] =
    {
        "CHARACTER",
        "VEHICLE",
        "FRONTEND"
    };

    for ( int layoutIndex = 0; layoutIndex < TOUCH_EDITABLE_LAYOUT_COUNT; ++layoutIndex )
    {
        fprintf( file, "[%s]\n", sectionNames[ layoutIndex ] );

        const unsigned int controlCount = hudSystem.GetControlCount();

        for ( unsigned int i = 0; i < controlCount; ++i )
        {
            const TouchHudControlDefinition* control =
                hudSystem.GetControlByIndex( i );

            if ( control == 0 )
            {
                continue;
            }

            if ( !control->enabled || !control->visibleByDefault )
            {
                continue;
            }

            if ( IsEditorControl( control->id ) )
            {
                continue;
            }

            if ( !IsControlInLayout(
                    control->id,
                    layouts[ layoutIndex ] ) )
            {
                continue;
            }

            if ( control->name == 0 || control->name[ 0 ] == '\0' )
            {
                continue;
            }

            fprintf(
                file,
                "%s=0.000,0.000,%d\n",
                control->name,
                TOUCH_CONTROL_SIZE_BASE_STEP
            );
        }

        fprintf( file, "\n" );
    }

    fclose( file );

    return true;
#endif
}

bool TouchControlsConfigurationManager::Load()
{
    ResetDefaultsInMemory();

    FILE* file = fopen( mConfigPath, "rb" );

    if ( file == 0 )
    {
        return false;
    }

    char line[ MAX_TOUCH_CONFIG_LINE ];

    while ( fgets( line, sizeof( line ), file ) != 0 )
    {
        char* trimmedLine = TrimTouchConfigLine( line );

        if ( trimmedLine == 0 || trimmedLine[ 0 ] == '\0' )
        {
            continue;
        }

        if ( trimmedLine[ 0 ] == '#' )
        {
            continue;
        }

        if ( trimmedLine[ 0 ] == '[' )
        {
            continue;
        }

        char* equals = strchr( trimmedLine, '=' );

        if ( equals == 0 )
        {
            continue;
        }

        *equals = '\0';

        char* key = TrimTouchConfigLine( trimmedLine );
        char* value = TrimTouchConfigLine( equals + 1 );

        if ( key == 0 || value == 0 )
        {
            continue;
        }

        if ( strcmp( key, "opacity" ) == 0 )
        {
            float opacity = TOUCH_CONTROLS_DEFAULT_OPACITY;

            if ( sscanf( value, "%f", &opacity ) == 1 )
            {
                mOpacity = ClampOpacity( opacity );
            }

            continue;
        }

        if ( strcmp( key, "pressed_opacity" ) == 0 )
        {
            float opacity = TOUCH_CONTROLS_DEFAULT_PRESSED_OPACITY;

            if ( sscanf( value, "%f", &opacity ) == 1 )
            {
                mPressedOpacity = ClampOpacity( opacity );
            }

            continue;
        }

        if ( strcmp( key, "version" ) == 0 )
        {
            continue;
        }

        TouchHudControlId controlId = FindControlIdByName( key );

        if ( !IsValidControlId( controlId ) )
        {
            continue;
        }

        float offsetX = 0.0f;
        float offsetY = 0.0f;
        int sizeStep = TOUCH_CONTROL_SIZE_BASE_STEP;

        if ( sscanf( value, "%f,%f,%d", &offsetX, &offsetY, &sizeStep ) == 3 )
        {
            TouchControlCustomization customization;
            customization.offsetX = offsetX;
            customization.offsetY = offsetY;
            customization.sizeStep = ClampSizeStep( sizeStep );

            mCustomizations[ controlId ] = customization;
        }
    }

    fclose( file );

    mDirty = false;

    

    return true;
}

void TouchControlsConfigurationManager::ResetDefaultsInMemory()
{
    for ( int i = 0; i < TOUCH_HUD_CONTROL_COUNT; ++i )
    {
        mCustomizations[ i ].Reset();
    }

    mOpacity = TOUCH_CONTROLS_DEFAULT_OPACITY;
    mPressedOpacity = TOUCH_CONTROLS_DEFAULT_PRESSED_OPACITY;
    mDirty = false;
}

bool TouchControlsConfigurationManager::IsValidControlId
(
    TouchHudControlId controlId
) const
{
    return controlId >= TOUCH_HUD_CONTROL_NONE &&
           controlId < TOUCH_HUD_CONTROL_COUNT;
}

bool TouchControlsConfigurationManager::IsEditorControl
(
    TouchHudControlId controlId
) const
{
    return controlId == TOUCH_HUD_CONTROL_EDITOR_ENTER_IN_GAME ||
           controlId == TOUCH_HUD_CONTROL_EDITOR_NEXT_LAYOUT ||
           controlId == TOUCH_HUD_CONTROL_EDITOR_RESET;
}

bool TouchControlsConfigurationManager::IsControlInLayout
(
    TouchHudControlId controlId,
    TouchEditableLayout layout
) const
{
    if ( !IsValidControlId( controlId ) )
    {
        return false;
    }

    const TouchHudControlDefinition* control =
        TouchHudSystem::GetInstance().GetControlDefinition( controlId );

    if ( control == 0 )
    {
        return false;
    }

    switch ( layout )
    {
        case TOUCH_EDITABLE_LAYOUT_CHARACTER:
        {
            return control->profile == TOUCH_PROFILE_CHARACTER;
        }

        case TOUCH_EDITABLE_LAYOUT_VEHICLE:
        {
            return control->profile == TOUCH_PROFILE_VEHICLE;
        }

        case TOUCH_EDITABLE_LAYOUT_FRONTEND:
        {
            return control->profile == TOUCH_PROFILE_FRONTEND;
        }

        default:
        {
            break;
        }
    }

    return false;
}

TouchHudControlId TouchControlsConfigurationManager::FindControlIdByName
(
    const char* name
) const
{
    if ( name == 0 || name[ 0 ] == '\0' )
    {
        return TOUCH_HUD_CONTROL_NONE;
    }

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const unsigned int controlCount = hudSystem.GetControlCount();

    for ( unsigned int i = 0; i < controlCount; ++i )
    {
        const TouchHudControlDefinition* control =
            hudSystem.GetControlByIndex( i );

        if ( control == 0 )
        {
            continue;
        }

        if ( control->name == 0 )
        {
            continue;
        }

        if ( strcmp( control->name, name ) == 0 )
        {
            return control->id;
        }
    }

    return TOUCH_HUD_CONTROL_NONE;
}

float TouchControlsConfigurationManager::ClampOpacity( float value ) const
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

int TouchControlsConfigurationManager::ClampSizeStep( int value ) const
{
    if ( value < TOUCH_CONTROL_SIZE_MIN_STEP )
    {
        return TOUCH_CONTROL_SIZE_MIN_STEP;
    }

    if ( value > TOUCH_CONTROL_SIZE_MAX_STEP )
    {
        return TOUCH_CONTROL_SIZE_MAX_STEP;
    }

    return value;
}
