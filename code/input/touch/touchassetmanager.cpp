#include <input/touch/touchassetmanager.h>

#include <string.h>
#include <stdio.h>

#include <p3d/imagefactory.hpp>
#include <p3d/image.hpp>
#include <p3d/sprite.hpp>
#include <p3d/file.hpp>
#if defined(RAD_ANDROID)
#include <SDL.h>
#include <SDL_system.h>
#include <android/log.h>

#define TOUCH_ASSET_MANAGER_LOG_TAG "SimpsonsHitAndRun"
#define TOUCH_ASSET_MANAGER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TOUCH_ASSET_MANAGER_LOG_TAG, __VA_ARGS__)
#define TOUCH_ASSET_MANAGER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TOUCH_ASSET_MANAGER_LOG_TAG, __VA_ARGS__)
#else
#define TOUCH_ASSET_MANAGER_LOGI(...)
#define TOUCH_ASSET_MANAGER_LOGE(...)
#endif

//=============================================================================
// Singleton
//=============================================================================

TouchAssetManager& TouchAssetManager::GetInstance()
{
    static TouchAssetManager sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchAssetManager::TouchAssetManager()
:
mInitialized( false )
{
    mAssetRoot[ 0 ] = '\0';
    mLastError[ 0 ] = '\0';

    for ( int i = 0; i < TOUCH_ASSET_COUNT; ++i )
    {
        mSprites[ i ] = 0;
    }
}

TouchAssetManager::~TouchAssetManager()
{
    Shutdown();
}

//=============================================================================
// Public
//=============================================================================

bool TouchAssetManager::LoadAllSprites()
{
#if !defined(RAD_ANDROID)
    return true;
#else
   bool allLoaded = true;

// Character
allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_JOYSTICK_BASE,
    "character/circulo.png",
    "tc_joy_base"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_JOYSTICK_KNOB,
    "character/circuloInteriorJoystick.png",
    "tc_joy_knob"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_SPRINT,
    "character/correr.png",
    "tc_sprint"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_CONTEXT_EXCLAMATION,
    "character/exclamacion.png",
    "tc_exclaim"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_ATTACK,
    "character/golpear.png",
    "tc_attack"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_DOOR,
    "character/Puerta.png",
    "tc_door"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_ENTER_CAR,
    "character/PuertaEntrar.png",
    "tc_enter"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_JUMP,
    "character/saltar.png",
    "tc_jump"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_PHONE,
    "character/Telefono.png",
    "tc_phone"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_CHARACTER_DOLLAR,
    "character/dollar.png",
    "tc_dollar"
) && allLoaded;


// Frontend
allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_BACK,
    "frontend/atras.png",
    "tf_back"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_SELECT,
    "frontend/check.png",
    "tf_select"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_DOWN,
    "frontend/fabajo.png",
    "tf_down"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_UP,
    "frontend/farriba.png",
    "tf_up"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_RIGHT,
    "frontend/fderecha.png",
    "tf_right"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_LEFT,
    "frontend/fizquierda.png",
    "tf_left"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_FRONTEND_START,
    "frontend/start.png",
    "tf_start"
) && allLoaded;


// Vehicle
allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_ACCELERATE,
    "vehicle/acelerador.png",
    "tv_accel"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_HORN,
    "vehicle/bocina.png",
    "tv_horn"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_HANDBRAKE,
    "vehicle/derrape.png",
    "tv_handbrake"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_BRAKE,
    "vehicle/freno.png",
    "tv_brake"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_EXIT,
    "vehicle/puertaSalir.png",
    "tv_exit"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_RESET,
    "vehicle/reset.png",
    "tv_reset"
) && allLoaded;

allLoaded = LoadSpriteForAsset(
    TOUCH_ASSET_VEHICLE_CAMERA_TOGGLE,
    "vehicle/camara.png",
    "tv_camera"
) && allLoaded;
    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] LoadAllSprites finished. allLoaded=%d",
        allLoaded ? 1 : 0
    );

    return allLoaded;
#endif
}

bool TouchAssetManager::LoadSpriteForAsset(TouchAssetId assetId,const char* relativePath,const char* spriteName)
{
#if !defined(RAD_ANDROID)
    (void)assetId;
    (void)relativePath;
    (void)spriteName;
    return true;
#else
    if ( assetId < 0 || assetId >= TOUCH_ASSET_COUNT )
    {
        SetLastError( "Invalid TouchAssetId." );
        return false;
    }

    ReleaseSprite( mSprites[ assetId ] );

    mSprites[ assetId ] = LoadSpriteFromFile(
        relativePath,
        spriteName
    );

    if ( mSprites[ assetId ] == 0 )
    {
        TOUCH_ASSET_MANAGER_LOGE(
            "[TouchAssetManager] Failed to load asset id=%d path=%s",
            static_cast<int>( assetId ),
            relativePath ? relativePath : "(null)"
        );

        return false;
    }

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Loaded asset id=%d path=%s",
        static_cast<int>( assetId ),
        relativePath ? relativePath : "(null)"
    );

    return true;
#endif
}

bool TouchAssetManager::Initialize()
{
#if !defined(RAD_ANDROID)
    mInitialized = true;
    return true;
#else
    if ( mInitialized )
    {
        return true;
    }

    if ( !BuildAssetRoot() )
    {
        TOUCH_ASSET_MANAGER_LOGE(
            "[TouchAssetManager] BuildAssetRoot failed: %s",
            mLastError
        );
        return false;
    }

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Initializing from root: %s",
        mAssetRoot
    );

    if ( !LoadAllSprites() )
    {
        TOUCH_ASSET_MANAGER_LOGE(
            "[TouchAssetManager] One or more touch sprites failed to load: %s",
            mLastError
        );

        // Important:
        // We do not crash the game if a visual icon fails.
        // Touch input can continue working without the visual HUD.
    }

    mInitialized = true;

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Initialized successfully."
    );

    return true;
#endif
}

void TouchAssetManager::ReleaseAllSprites()
{
    for ( int i = 0; i < TOUCH_ASSET_COUNT; ++i )
    {
        ReleaseSprite( mSprites[ i ] );
    }
}

void TouchAssetManager::Shutdown()
{
    ReleaseAllSprites();

    mInitialized = false;
}

bool TouchAssetManager::IsInitialized() const
{
    return mInitialized;
}

const char* TouchAssetManager::GetAssetRoot() const
{
    return mAssetRoot;
}

const char* TouchAssetManager::GetLastError() const
{
    return mLastError;
}

tSprite* TouchAssetManager::GetSprite( TouchAssetId assetId ) const
{
    if ( assetId < 0 || assetId >= TOUCH_ASSET_COUNT )
    {
        return 0;
    }

    return mSprites[ assetId ];
}

tSprite* TouchAssetManager::GetSpriteForControl( TouchHudControlId controlId ) const
{
    switch ( controlId )
    {
        // -------------------------------------------------------------
        // Character
        // -------------------------------------------------------------

        case TOUCH_HUD_CONTROL_CHARACTER_MOVEMENT_ZONE:
        {
            // Dynamic joystick is drawn separately:
            // base  -> TOUCH_ASSET_CHARACTER_JOYSTICK_BASE
            // knob  -> TOUCH_ASSET_CHARACTER_JOYSTICK_KNOB
            return 0;
        }

        case TOUCH_HUD_CONTROL_CHARACTER_CAMERA_ZONE:
        {
            // Free camera drag area has no visible joystick.
            return 0;
        }

        case TOUCH_HUD_CONTROL_CHARACTER_JUMP:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_JUMP );
        }

        case TOUCH_HUD_CONTROL_CHARACTER_SPRINT:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_SPRINT );
        }

        case TOUCH_HUD_CONTROL_CHARACTER_ATTACK:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_ATTACK );
        }

        case TOUCH_HUD_CONTROL_CHARACTER_START:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_START );
        }

        case TOUCH_HUD_CONTROL_CHARACTER_CONTEXT_ACTION:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_CONTEXT_EXCLAMATION );
        }

        case TOUCH_HUD_CONTROL_CHARACTER_SECONDARY_CONTEXT_ACTION:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_ENTER_CAR );
        }

        // -------------------------------------------------------------
        // Vehicle
        // -------------------------------------------------------------

        case TOUCH_HUD_CONTROL_VEHICLE_STEER_LEFT:
        {
            // Temporary fallback until you add dedicated vehicle steering icons.
            return GetSprite( TOUCH_ASSET_FRONTEND_LEFT );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_STEER_RIGHT:
        {
            // Temporary fallback until you add dedicated vehicle steering icons.
            return GetSprite( TOUCH_ASSET_FRONTEND_RIGHT );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_ACCELERATE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_ACCELERATE );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_REVERSE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_BRAKE );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_HAND_BRAKE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_HANDBRAKE );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_GET_OUT:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_EXIT );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_HORN:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_HORN );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_RESET:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_RESET );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_CAMERA_TOGGLE:
        {
            return GetSprite(TOUCH_ASSET_VEHICLE_CAMERA_TOGGLE);
        }

        case TOUCH_HUD_CONTROL_VEHICLE_START:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_START );
        }

        // -------------------------------------------------------------
        // Frontend
        // -------------------------------------------------------------

        case TOUCH_HUD_CONTROL_FRONTEND_MOVE_UP:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_UP );
        }

        case TOUCH_HUD_CONTROL_FRONTEND_MOVE_DOWN:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_DOWN );
        }

        case TOUCH_HUD_CONTROL_FRONTEND_MOVE_LEFT:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_LEFT );
        }

        case TOUCH_HUD_CONTROL_FRONTEND_MOVE_RIGHT:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_RIGHT );
        }

        case TOUCH_HUD_CONTROL_FRONTEND_SELECT:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_SELECT );
        }

        case TOUCH_HUD_CONTROL_FRONTEND_BACK:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_BACK );
        }
        case TOUCH_HUD_CONTROL_FRONTEND_START:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_START );
        }

        // -------------------------------------------------------------
        // Frontend CGuiScreenScrapBookContents
        // -------------------------------------------------------------

        case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_UP:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_MOVE_UP );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_DOWN:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_MOVE_DOWN );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_LEFT:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_MOVE_LEFT );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_MOVE_RIGHT:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_MOVE_RIGHT );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_SELECT:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_SELECT );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_FE_BACK:
    {
        return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_BACK );
    }

    case TOUCH_HUD_CONTROL_SCRAPBOOK_L1_ZONE:
    case TOUCH_HUD_CONTROL_SCRAPBOOK_R1_ZONE:
    {
    return 0;
    }

        // -------------------------------------------------------------
        // Cinematic
        // -------------------------------------------------------------

        case TOUCH_HUD_CONTROL_CINEMATIC_SKIP:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_RIGHT  );
        }

        // -------------------------------------------------------------
        // MINIGAME GAMEPLAY
        // -------------------------------------------------------------
        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_LEFT:
        {
            // Temporary fallback until you add dedicated vehicle steering icons.
            return GetSprite( TOUCH_ASSET_FRONTEND_LEFT );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_STEER_RIGHT:
        {
            // Temporary fallback until you add dedicated vehicle steering icons.
            return GetSprite( TOUCH_ASSET_FRONTEND_RIGHT );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_ACCELERATE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_ACCELERATE );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_REVERSE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_BRAKE );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HAND_BRAKE:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_HANDBRAKE );
        }


        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_HORN:
        {
            return GetSprite( TOUCH_ASSET_VEHICLE_HORN );
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_CAMERA_TOGGLE:
        {
            return GetSprite(TOUCH_ASSET_VEHICLE_CAMERA_TOGGLE);
        }

        case TOUCH_HUD_CONTROL_VEHICLE_MINIGAME_START:
        {
            return GetSprite( TOUCH_ASSET_FRONTEND_START );
        }




        // -------------------------------------------------------------
        // MINIGAME SUMMARY
        // -------------------------------------------------------------
        case TOUCH_HUD_CONTROL_FRONTEND_MINIGAME_SUMMARY_SELECT:
        {
            return GetSpriteForControl( TOUCH_HUD_CONTROL_FRONTEND_SELECT );
        }

        case TOUCH_HUD_CONTROL_NONE:
        default:
        {
            break;
        }
    }

    return 0;
}

//=============================================================================
// Internal
//=============================================================================

bool TouchAssetManager::BuildAssetRoot()
{
#if !defined(RAD_ANDROID)
    return true;
#else
    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == 0 || storagePath[ 0 ] == '\0' )
    {
        SetLastError( "SDL_AndroidGetExternalStoragePath returned null/empty." );
        return false;
    }

    snprintf(
        mAssetRoot,
        sizeof( mAssetRoot ),
        "%s/touch_controls",
        storagePath
    );

    return true;
#endif
}

tSprite* TouchAssetManager::LoadSpriteFromFile
(
    const char* relativePath,
    const char* spriteName
)
{
#if !defined(RAD_ANDROID)
    (void)relativePath;
    (void)spriteName;
    return 0;
#else
    if ( relativePath == 0 || relativePath[ 0 ] == '\0' )
    {
        SetLastError( "Invalid relative path." );
        return 0;
    }

    char fullPath[ MAX_TOUCH_PATH ];

    snprintf(
        fullPath,
        sizeof( fullPath ),
        "%s/%s",
        mAssetRoot,
        relativePath
    );

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Loading sprite from: %s",
        fullPath
    );

    /*
     * Pure3D old API expects mutable char*, not const char*.
     */
    char imageName[ 128 ];

    if ( spriteName != 0 && spriteName[ 0 ] != '\0' )
    {
        strncpy( imageName, spriteName, sizeof( imageName ) );
    }
    else
    {
        strncpy( imageName, relativePath, sizeof( imageName ) );
    }

    imageName[ sizeof( imageName ) - 1 ] = '\0';

   
    FILE* inputFile = fopen( fullPath, "rb" );

    if ( inputFile == 0 )
    {
        char error[ MAX_TOUCH_ERROR ];
        snprintf(
            error,
            sizeof( error ),
            "fopen failed for: %s",
            fullPath
        );

        SetLastError( error );

        TOUCH_ASSET_MANAGER_LOGE(
            "[TouchAssetManager] %s",
            mLastError
        );

        return 0;
    }

    fseek( inputFile, 0, SEEK_END );
    long inputSize = ftell( inputFile );
    fseek( inputFile, 0, SEEK_SET );

    if ( inputSize <= 0 )
    {
        fclose( inputFile );
        SetLastError( "File size is invalid for memory load." );
        return 0;
    }

    unsigned char* fileData = new unsigned char[ inputSize ];

    size_t readBytes = fread(
        fileData,
        1,
        static_cast<size_t>( inputSize ),
        inputFile
    );

    fclose( inputFile );

    if ( readBytes != static_cast<size_t>( inputSize ) )
    {
        delete[] fileData;
        SetLastError( "Failed to read full PNG into memory." );
        return 0;
    }

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] PNG file read into memory OK: %s size=%ld",
        fullPath,
        inputSize
    );

   /*
 * Give Pure3D a memory-backed tFile instead of asking Pure3D
 * to open the Android absolute path by itself.
 *
 * IMPORTANT:
 * Do NOT create tFileMem on the stack.
 * tImageFactory::LoadAsImage(tFile*) calls AddRef()/Release() internally,
 * so the tFile object must be heap-allocated and ref-counted safely.
 *
 * del=false because we manually delete[] fileData after LoadAsImage returns.
 */
tFileMem* memoryFile = new tFileMem(
    fileData,
    static_cast<unsigned>( inputSize ),
    false
);

if ( memoryFile == 0 )
{
    delete[] fileData;
    SetLastError( "Failed to allocate tFileMem." );
    return 0;
}

memoryFile->AddRef();
memoryFile->SetFilename( fullPath );

tImageFactory imageFactory;
imageFactory.SetDesiredDepth( 32 );

TOUCH_ASSET_MANAGER_LOGI(
    "[TouchAssetManager] Calling Pure3D LoadAsImage(tFile*)..."
);

tImage* image = imageFactory.LoadAsImage(
    memoryFile,
    imageName
);

TOUCH_ASSET_MANAGER_LOGI(
    "[TouchAssetManager] Returned from Pure3D LoadAsImage(tFile*). image=%p",
    image
);

/*
 * Release our external reference.
 * LoadAsImage() has already done its own internal AddRef()/Release().
 */
memoryFile->Release();
memoryFile = 0;

/*
 * tFileMem was created with del=false, so it should not own fileData.
 * We can safely release the buffer after LoadAsImage has returned.
 */
delete[] fileData;
fileData = 0;

    if ( image == 0 )
    {
        char error[ MAX_TOUCH_ERROR ];
        snprintf(
            error,
            sizeof( error ),
            "Pure3D failed to load image from memory: %s",
            fullPath
        );

        SetLastError( error );

        TOUCH_ASSET_MANAGER_LOGE(
            "[TouchAssetManager] %s",
            mLastError
        );

        return 0;
    }

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Image loaded: %s width=%d height=%d depth=%d alpha=%d alphaDepth=%d",
        fullPath,
        image->GetWidth(),
        image->GetHeight(),
        image->GetDepth(),
        image->HasAlpha() ? 1 : 0,
        image->GetAlphaDepth()
    );

    /*
     * Build a Pure3D sprite from the decoded image.
     * No rendering happens here yet.
     */
    tSprite* sprite = new tSprite(
        image,
        0,
        1,
        640,
        480
    );

    if ( sprite == 0 )
    {
        image->Release();
        SetLastError( "Failed to create tSprite from image." );
        return 0;
    }

    sprite->AddRef();

    image->Release();

    TOUCH_ASSET_MANAGER_LOGI(
        "[TouchAssetManager] Sprite created successfully: %s",
        imageName
    );

    return sprite;
#endif
}

void TouchAssetManager::ReleaseSprite( tSprite*& sprite )
{
    if ( sprite != 0 )
    {
        sprite->Release();
        sprite = 0;
    }
}

void TouchAssetManager::SetLastError( const char* error )
{
    if ( error == 0 )
    {
        mLastError[ 0 ] = '\0';
        return;
    }

    strncpy( mLastError, error, sizeof( mLastError ) );
    mLastError[ sizeof( mLastError ) - 1 ] = '\0';
}

tSprite* TouchAssetManager::GetSpriteForInteractionIcon
(
    TouchInteractionIcon icon
) const
{
    switch ( icon )
    {
        case TOUCH_INTERACTION_ICON_EXCLAMATION:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_CONTEXT_EXCLAMATION );
        }

        case TOUCH_INTERACTION_ICON_DOOR:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_DOOR );
        }

        case TOUCH_INTERACTION_ICON_PHONE:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_PHONE );
        }

        case TOUCH_INTERACTION_ICON_DOLLAR:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_DOLLAR );
        }

        case TOUCH_INTERACTION_ICON_ENTER_CAR:
        {
            return GetSprite( TOUCH_ASSET_CHARACTER_ENTER_CAR );
        }

        case TOUCH_INTERACTION_ICON_NONE:
        default:
        {
            return 0;
        }
    }
}