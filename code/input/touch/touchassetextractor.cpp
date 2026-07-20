#include <input/touch/touchassetextractor.h>

#include <string.h>
#include <stdio.h>

#if defined(RAD_ANDROID)
#include <SDL.h>
#include <SDL_system.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <android/log.h>

#define TOUCH_ASSET_LOG_TAG "SimpsonsHitAndRun"
#define TOUCH_ASSET_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TOUCH_ASSET_LOG_TAG, __VA_ARGS__)
#define TOUCH_ASSET_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TOUCH_ASSET_LOG_TAG, __VA_ARGS__)
#endif

// Increase this when you change bundled touch PNGs and want to force recopy.
//Version 1 fue los controles táctiles al completo 
//Version 2 (actual) es controles tactiles + iconos para la edición

static const char* TOUCH_ASSET_VERSION = "2";


static const char* TOUCH_ASSET_FILES[] =
{
    // Character
    "touch_controls/character/circulo.png",
    "touch_controls/character/circuloInteriorJoystick.png",
    "touch_controls/character/correr.png",
    "touch_controls/character/exclamacion.png",
    "touch_controls/character/golpear.png",
    "touch_controls/character/Puerta.png",
    "touch_controls/character/PuertaEntrar.png",
    "touch_controls/character/saltar.png",
    "touch_controls/character/Telefono.png",
    "touch_controls/character/dollar.png",
    // Frontend
    "touch_controls/frontend/atras.png",
    "touch_controls/frontend/check.png",
    "touch_controls/frontend/fabajo.png",
    "touch_controls/frontend/farriba.png",
    "touch_controls/frontend/fderecha.png",
    "touch_controls/frontend/fizquierda.png",
    "touch_controls/frontend/start.png",

    // Vehicle
    "touch_controls/vehicle/acelerador.png",
    "touch_controls/vehicle/bocina.png",
    "touch_controls/vehicle/derrape.png",
    "touch_controls/vehicle/freno.png",
    "touch_controls/vehicle/puertaSalir.png",
    "touch_controls/vehicle/reset.png",
    "touch_controls/vehicle/camara.png",

    // editor touch_controls 
    "touch_controls/editor/1De3.png",
    "touch_controls/editor/2De3.png",
    "touch_controls/editor/3De3.png",
    "touch_controls/editor/moverControlesTactiles.png",
    "touch_controls/editor/RESET.png"

};

static const unsigned int TOUCH_ASSET_FILE_COUNT =
    sizeof( TOUCH_ASSET_FILES ) / sizeof( TOUCH_ASSET_FILES[ 0 ] );

//=============================================================================
// Singleton
//=============================================================================

TouchAssetExtractor& TouchAssetExtractor::GetInstance()
{
    static TouchAssetExtractor sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchAssetExtractor::TouchAssetExtractor()
:
mHasRun( false )
{
    mOutputRoot[ 0 ] = '\0';
    mLastError[ 0 ] = '\0';
}

TouchAssetExtractor::~TouchAssetExtractor()
{
}

//=============================================================================
// Public
//=============================================================================

bool TouchAssetExtractor::EnsureAssetsExtracted()
{
#if !defined(RAD_ANDROID)
    return true;
#else
    if ( mHasRun && !NeedsExtraction() )
    {
        return true;
    }

    mHasRun = true;

    if ( !BuildOutputRoot() )
    {
        return false;
    }

    if ( !NeedsExtraction() )
    {
        /** 
        TOUCH_ASSET_LOGI(
            "[TouchAssets] Assets already extracted at: %s",
            mOutputRoot
        );
        */
        return true;
    }
 /** 
    TOUCH_ASSET_LOGI(
        "[TouchAssets] Extracting touch assets to: %s",
        mOutputRoot
    );
    */

    if ( !EnsureDirectory( mOutputRoot ) )
    {
        SetLastError( "Failed to create touch asset output directory." );
        TOUCH_ASSET_LOGE( "[TouchAssets] %s", mLastError );
        return false;
    }

    if ( !CopyKnownAssets() )
    {
        TOUCH_ASSET_LOGE( "[TouchAssets] Extraction failed: %s", mLastError );
        return false;
    }

    if ( !WriteVersionFile() )
    {
        SetLastError( "Failed to write touch asset version file." );
        TOUCH_ASSET_LOGE( "[TouchAssets] %s", mLastError );
        return false;
    }

    //TOUCH_ASSET_LOGI( "[TouchAssets] Extraction completed." );

    return true;
#endif
}

const char* TouchAssetExtractor::GetOutputRoot() const
{
    return mOutputRoot;
}

const char* TouchAssetExtractor::GetLastError() const
{
    return mLastError;
}

//=============================================================================
// Internal
//=============================================================================

bool TouchAssetExtractor::BuildOutputRoot()
{
#if !defined(RAD_ANDROID)
    return true;
#else
    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == NULL || storagePath[ 0 ] == '\0' )
    {
        SetLastError( "SDL_AndroidGetExternalStoragePath returned null/empty." );
        return false;
    }

    snprintf(
        mOutputRoot,
        sizeof( mOutputRoot ),
        "%s/touch_controls",
        storagePath
    );

    return true;
#endif
}

bool TouchAssetExtractor::NeedsExtraction() const
{
#if !defined(RAD_ANDROID)
    return false;
#else
    if ( !IsVersionFileValid() )
    {
        return true;
    }

    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == NULL || storagePath[ 0 ] == '\0' )
    {
        return true;
    }

    for ( unsigned int i = 0; i < TOUCH_ASSET_FILE_COUNT; ++i )
    {
        char outputPath[ MAX_TOUCH_PATH ];

        snprintf(
            outputPath,
            sizeof( outputPath ),
            "%s/%s",
            storagePath,
            TOUCH_ASSET_FILES[ i ]
        );

        if ( !FileExists( outputPath ) )
        {
            return true;
        }
    }

    return false;
#endif
}

bool TouchAssetExtractor::CopyKnownAssets()
{
#if !defined(RAD_ANDROID)
    return true;
#else
    for ( unsigned int i = 0; i < TOUCH_ASSET_FILE_COUNT; ++i )
    {
        if ( !CopyAssetFile( TOUCH_ASSET_FILES[ i ] ) )
        {
            return false;
        }
    }

    return true;
#endif
}

bool TouchAssetExtractor::CopyAssetFile( const char* relativePath )
{
#if !defined(RAD_ANDROID)
    (void)relativePath;
    return true;
#else
    if ( relativePath == NULL || relativePath[ 0 ] == '\0' )
    {
        SetLastError( "Invalid asset relative path." );
        return false;
    }

    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == NULL || storagePath[ 0 ] == '\0' )
    {
        SetLastError( "Cannot copy asset: external storage path is invalid." );
        return false;
    }

    char outputPath[ MAX_TOUCH_PATH ];

    snprintf(
        outputPath,
        sizeof( outputPath ),
        "%s/%s",
        storagePath,
        relativePath
    );

    if ( !EnsureParentDirectoryForFile( outputPath ) )
    {
        SetLastError( "Failed to create parent directory for asset output." );
        return false;
    }

#if SDL_MAJOR_VERSION < 3
    SDL_RWops* input = SDL_RWFromFile( relativePath, "rb" );
#else
    SDL_IOStream* input = SDL_IOFromFile( relativePath, "rb" );
#endif

    if ( input == NULL )
    {
        char error[ MAX_TOUCH_ERROR ];
        snprintf(
            error,
            sizeof( error ),
            "Failed to open APK asset: %s",
            relativePath
        );
        SetLastError( error );
        return false;
    }

    FILE* output = fopen( outputPath, "wb" );

    if ( output == NULL )
    {
#if SDL_MAJOR_VERSION < 3
        SDL_RWclose( input );
#else
        SDL_CloseIO( input );
#endif
        char error[ MAX_TOUCH_ERROR ];
        snprintf(
            error,
            sizeof( error ),
            "Failed to open output file: %s",
            outputPath
        );
        SetLastError( error );
        return false;
    }

    char buffer[ 16 * 1024 ];
    size_t totalBytes = 0;

    while ( true )
    {
#if SDL_MAJOR_VERSION < 3
        size_t bytesRead = SDL_RWread( input, buffer, 1, sizeof( buffer ) );
#else
        size_t bytesRead = SDL_ReadIO( input, buffer, sizeof( buffer ) );
#endif

        if ( bytesRead == 0 )
        {
            break;
        }

        size_t bytesWritten = fwrite( buffer, 1, bytesRead, output );

        if ( bytesWritten != bytesRead )
        {
            fclose( output );
#if SDL_MAJOR_VERSION < 3
            SDL_RWclose( input );
#else
            SDL_CloseIO( input );
#endif
            SetLastError( "Failed while writing copied asset." );
            return false;
        }

        totalBytes += bytesWritten;
    }

    fclose( output );

#if SDL_MAJOR_VERSION < 3
    SDL_RWclose( input );
#else
    SDL_CloseIO( input );
#endif

    if ( totalBytes == 0 )
    {
        SetLastError( "Copied asset has zero bytes." );
        return false;
    }

    /** 
    TOUCH_ASSET_LOGI(
        "[TouchAssets] Copied %s -> %s (%u bytes)",
        relativePath,
        outputPath,
        (unsigned int)totalBytes
    );
    */

    return true;
#endif
}

bool TouchAssetExtractor::EnsureDirectory( const char* path ) const
{
#if !defined(RAD_ANDROID)
    (void)path;
    return true;
#else
    if ( path == NULL || path[ 0 ] == '\0' )
    {
        return false;
    }

    char temp[ MAX_TOUCH_PATH ];
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

bool TouchAssetExtractor::EnsureParentDirectoryForFile( const char* path ) const
{
#if !defined(RAD_ANDROID)
    (void)path;
    return true;
#else
    if ( path == NULL || path[ 0 ] == '\0' )
    {
        return false;
    }

    char parent[ MAX_TOUCH_PATH ];
    strncpy( parent, path, sizeof( parent ) );
    parent[ sizeof( parent ) - 1 ] = '\0';

    char* lastSlash = strrchr( parent, '/' );

    if ( lastSlash == NULL )
    {
        return false;
    }

    *lastSlash = '\0';

    return EnsureDirectory( parent );
#endif
}

bool TouchAssetExtractor::FileExists( const char* path ) const
{
#if !defined(RAD_ANDROID)
    (void)path;
    return false;
#else
    if ( path == NULL || path[ 0 ] == '\0' )
    {
        return false;
    }

    struct stat info;
    return stat( path, &info ) == 0 && S_ISREG( info.st_mode );
#endif
}

bool TouchAssetExtractor::WriteVersionFile() const
{
#if !defined(RAD_ANDROID)
    return true;
#else
    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == NULL || storagePath[ 0 ] == '\0' )
    {
        return false;
    }

    char versionPath[ MAX_TOUCH_PATH ];

    snprintf(
        versionPath,
        sizeof( versionPath ),
        "%s/touch_controls/.version",
        storagePath
    );

    if ( !EnsureParentDirectoryForFile( versionPath ) )
    {
        return false;
    }

    FILE* file = fopen( versionPath, "wb" );

    if ( file == NULL )
    {
        return false;
    }

    fwrite( TOUCH_ASSET_VERSION, 1, strlen( TOUCH_ASSET_VERSION ), file );
    fclose( file );

    return true;
#endif
}

bool TouchAssetExtractor::IsVersionFileValid() const
{
#if !defined(RAD_ANDROID)
    return true;
#else
    const char* storagePath = SDL_AndroidGetExternalStoragePath();

    if ( storagePath == NULL || storagePath[ 0 ] == '\0' )
    {
        return false;
    }

    char versionPath[ MAX_TOUCH_PATH ];

    snprintf(
        versionPath,
        sizeof( versionPath ),
        "%s/touch_controls/.version",
        storagePath
    );

    FILE* file = fopen( versionPath, "rb" );

    if ( file == NULL )
    {
        return false;
    }

    char versionBuffer[ 32 ];
    memset( versionBuffer, 0, sizeof( versionBuffer ) );

    fread( versionBuffer, 1, sizeof( versionBuffer ) - 1, file );
    fclose( file );

    return strcmp( versionBuffer, TOUCH_ASSET_VERSION ) == 0;
#endif
}

void TouchAssetExtractor::SetLastError( const char* error )
{
    if ( error == NULL )
    {
        mLastError[ 0 ] = '\0';
        return;
    }

    strncpy( mLastError, error, sizeof( mLastError ) );
    mLastError[ sizeof( mLastError ) - 1 ] = '\0';
}