//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        sdldrive.cpp
//
// Subsystem:   Radical Drive System
//
// Description:	This file contains the implementation of the radSdlDrive class.
//
// Revisions:
//
// Notes:       We keep a serial number when the first file is opened. Then if the
//              media is removed, we don't allow ops until the original serial number
//              is detected, or all files are closed.
//=============================================================================

//=============================================================================
// Include Files
//=============================================================================

#include "pch.hpp"
#include <algorithm>
#include <limits.h>
#include "sdldrive.hpp"
#include <string>
#include <SDL.h>
#if SDL_MAJOR_VERSION < 3
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#endif

#ifdef RAD_ANDROID
#include <android/log.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>     // strerror()
#include <sys/stat.h>   // stat()
#include <SDL_system.h>
#endif



#if defined(RAD_ANDROID)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SimpsonsHitAndRun", __VA_ARGS__)
#endif

#ifdef RAD_ANDROID
static void LogPathStatus(const char* label, const char* path)
{
    struct stat st;
    int s = stat(path, &st);
    if (s == 0) {
        LOGI("%s: stat('%s') OK isDir=%d size=%lld",
             label, path, S_ISDIR(st.st_mode) ? 1 : 0, (long long)st.st_size);
    } else {
        LOGI("%s: stat('%s') FAIL errno=%d (%s)",
             label, path, errno, strerror(errno));
    }
}
#endif


//=============================================================================
// Public Functions 
//=============================================================================

//=============================================================================
// Function:    radSdlDriveFactory
//=============================================================================
// Description: This member is responsible for constructing a radSdlDriveObject.
//
// Parameters:  pointer to receive drive object
//              pointer to the drive name
//              allocator
//              
// Returns:     
//------------------------------------------------------------------------------

void radSdlDriveFactory
( 
    radDrive**         ppDrive, 
    const char*        pDriveName,
    radMemoryAllocator alloc
)
{
    //
    // Simply constuct the drive object.
    //
    *ppDrive = new( alloc ) radSdlDrive( pDriveName, alloc );
    rAssert( *ppDrive != NULL );
}


//=============================================================================
// Public Member Functions
//=============================================================================

//=============================================================================
// Function:    radSdlDrive::radSdlDrive
//=============================================================================

/*
radSdlDrive::radSdlDrive( const char* pdrivespec, radMemoryAllocator alloc )
    : 
    radDrive( ),
    m_OpenFiles( 0 ),
    m_pMutex( NULL )
{
    //
    // Create a mutex for lock/unlock
    //
    radThreadCreateMutex( &m_pMutex, alloc );
    rAssert( m_pMutex != NULL );

    //
    // Create the drive thread.
    //
    m_pDriveThread = new( alloc ) radDriveThread( m_pMutex, alloc );
    rAssert( m_pDriveThread != NULL );

    //
    // Copy the drivename
    //
    radGetDefaultDrive( m_DriveName );
    if ( strcmp(m_DriveName, pdrivespec ) != 0 )
    {
        strncpy( m_DriveName, pdrivespec, radFileDrivenameMax );
        strncpy( m_DrivePath, pdrivespec, radFileFilenameMax );
        m_DriveName[radFileDrivenameMax] = '\0';
        m_DrivePath[radFileFilenameMax] = '\0';
        SDL_strupr( m_DriveName );
        SDL_strlwr( m_DrivePath );
    }

    if(!m_DrivePath[0])
    {
#if SDL_MAJOR_VERSION < 3
#ifdef WIN32
        _getcwd( m_DrivePath, radFileFilenameMax );
        strncat(m_DrivePath, "/", radFileFilenameMax);
#else
        getcwd( m_DrivePath, radFileFilenameMax );
        strncat(m_DrivePath, "/", radFileFilenameMax);
#endif
#else
        char* cwd = SDL_GetCurrentDirectory();
        strncpy(m_DrivePath, cwd, radFileFilenameMax);
        SDL_free(cwd);
#endif
        m_DrivePath[radFileFilenameMax] = '\0';
    }

#if SDL_MAJOR_VERSION < 3
    m_Capabilities = ( radDriveWriteable | radDriveFile );
#else
    m_Capabilities = ( radDriveEnumerable | radDriveWriteable | radDriveDirectory | radDriveFile );
#endif
}
*/

//android change function
  /*
radSdlDrive::radSdlDrive(const char* pdrivespec, radMemoryAllocator alloc)
    :
    radDrive(),
    m_OpenFiles(0),
    m_pMutex(NULL)
{
    radThreadCreateMutex(&m_pMutex, alloc);
    rAssert(m_pMutex != NULL);

    m_pDriveThread = new(alloc) radDriveThread(m_pMutex, alloc);
    rAssert(m_pDriveThread != NULL);

    radGetDefaultDrive(m_DriveName);

    // ============================================================
    // CAMBIO AQUÍ: En Android IGNORAMOS pdrivespec para que NO pise
    // el root (a ti te llega "/" y te rompe m_DrivePath).
    // ============================================================
#if defined(RAD_ANDROID)
    (void)pdrivespec; // CAMBIO AQUÍ: evitar warning y NO usar pdrivespec
#else
    // Si te pasan drive spec (ej: "D:"), lo respetas
    if (pdrivespec && strcmp(m_DriveName, pdrivespec) != 0)
    {
        strncpy(m_DriveName, pdrivespec, radFileDrivenameMax);
        strncpy(m_DrivePath, pdrivespec, radFileFilenameMax);
        m_DriveName[radFileDrivenameMax] = '\0';
        m_DrivePath[radFileFilenameMax] = '\0';
        SDL_strupr(m_DriveName);

        // IMPORTANTE: solo lower-case en WIN32, nunca en Android
    #if defined(WIN32)
        SDL_strlwr(m_DrivePath);
    #endif
    }
#endif

    // ============================================================
    // CAMBIO AQUÍ: En Android FORZAMOS siempre el root a tu carpeta.
    // No dependemos de cwd, ni de pdrivespec, ni de nada.
    // ============================================================
#if defined(RAD_ANDROID)
    strncpy(m_DrivePath, "/data/data/org.libsdl.app/files/", radFileFilenameMax); // CAMBIO AQUÍ
    m_DrivePath[radFileFilenameMax] = '\0';                                      // CAMBIO AQUÍ
#else
    // Si no te han puesto drive path, defines el root por plataforma
    if (!m_DrivePath[0])
    {
    #if SDL_MAJOR_VERSION < 3
    #ifdef WIN32
        _getcwd(m_DrivePath, radFileFilenameMax);
    #else
        getcwd(m_DrivePath, radFileFilenameMax);
    #endif
        strncat(m_DrivePath, "/", radFileFilenameMax);
    #else
        char* cwd = SDL_GetCurrentDirectory();
        strncpy(m_DrivePath, cwd, radFileFilenameMax);
        SDL_free(cwd);
    #endif
        m_DrivePath[radFileFilenameMax] = '\0';
    }
#endif

    // ============================================================
    // CAMBIO AQUÍ: En Android NO hacemos strncat para añadir "/".
    // Ya viene con "/" en el literal; así evitas corrupción rara.
    // ============================================================
#if !defined(RAD_ANDROID)
    // (Opcional pero recomendable) asegurar "/" final siempre
    size_t len = strlen(m_DrivePath);
    if (len > 0 && m_DrivePath[len - 1] != '/' && len < radFileFilenameMax) {
        strncat(m_DrivePath, "/", radFileFilenameMax - len - 1);
    }
#endif

#if SDL_MAJOR_VERSION < 3
    m_Capabilities = (radDriveWriteable | radDriveFile);
#else
    m_Capabilities = (radDriveEnumerable | radDriveWriteable | radDriveDirectory | radDriveFile);
#endif

#ifdef RAD_ANDROID
    // Logs útiles para confirmar que ya NO sale "???p/"
    LOGI("radSdlDrive ctor: pdrivespec='%s'", pdrivespec ? pdrivespec : "(null)");
    LOGI("radSdlDrive ctor: m_DriveName='%s'", m_DriveName);
    LOGI("radSdlDrive ctor: m_DrivePath='%s'", m_DrivePath);

    // Comprueba que el root existe y que sound/ existe
    LogPathStatus("DrivePath", m_DrivePath);
    LogPathStatus("SoundDir", "/data/data/org.libsdl.app/files/sound");

    char cwd[512];
    if (getcwd(cwd, sizeof(cwd))) {
        LOGI("radSdlDrive ctor: getcwd()='%s'", cwd);
    } else {
        LOGI("radSdlDrive ctor: getcwd() FAIL errno=%d (%s)", errno, strerror(errno));
    }
#endif
}
*/
  // android change function
radSdlDrive::radSdlDrive(const char* pdrivespec, radMemoryAllocator alloc)
    :
    radDrive(),
    m_OpenFiles(0),
    m_pMutex(NULL)
{
    radThreadCreateMutex(&m_pMutex, alloc);
    rAssert(m_pMutex != NULL);

    m_pDriveThread = new(alloc) radDriveThread(m_pMutex, alloc);
    rAssert(m_pDriveThread != NULL);

    radGetDefaultDrive(m_DriveName);

    // ============================================================
    // CAMBIO AQUÍ: En Android IGNORAMOS pdrivespec para que NO pise
    // el root (a ti te llega "/" y te rompe m_DrivePath).
    // ============================================================
#if defined(RAD_ANDROID)
    (void)pdrivespec; // CAMBIO AQUÍ
#else
    // Si te pasan drive spec (ej: "D:"), lo respetas
    if (pdrivespec && strcmp(m_DriveName, pdrivespec) != 0)
    {
        strncpy(m_DriveName, pdrivespec, radFileDrivenameMax);
        strncpy(m_DrivePath, pdrivespec, radFileFilenameMax);
        m_DriveName[radFileDrivenameMax] = '\0';
        m_DrivePath[radFileFilenameMax] = '\0';
        SDL_strupr(m_DriveName);

    #if defined(WIN32)
        SDL_strlwr(m_DrivePath);
    #endif
    }
#endif

    // ============================================================
    // CAMBIO AQUÍ: En Android FORZAMOS SIEMPRE el root al external
    // storage "Android/data/<package>/files/" usando SDL.
    // ============================================================
#if defined(RAD_ANDROID)

    // CAMBIO AQUÍ: usamos SDL_AndroidGetExternalStoragePath (SDL_system.h)
    const char* ext = SDL_AndroidGetExternalStoragePath();

    if (ext && ext[0])
    {
        
		strncpy(m_DrivePath, ext, radFileFilenameMax);
        m_DrivePath[radFileFilenameMax] = '\0'; // CAMBIO AQUÍ
    }
    else
    {
        // CAMBIO AQUÍ: fallback por si SDL devuelve null (raro)
        strncpy(m_DrivePath, "/storage/emulated/0/Android/data/com.c4rlox.simpsons/files/", radFileFilenameMax);
        m_DrivePath[radFileFilenameMax] = '\0';
    }

#else
    // Si no te han puesto drive path, defines el root por plataforma
    if (!m_DrivePath[0])
    {
    #if SDL_MAJOR_VERSION < 3
    #ifdef WIN32
        _getcwd(m_DrivePath, radFileFilenameMax);
    #else
        getcwd(m_DrivePath, radFileFilenameMax);
    #endif
        strncat(m_DrivePath, "/", radFileFilenameMax);
    #else
        char* cwd = SDL_GetCurrentDirectory();
        strncpy(m_DrivePath, cwd, radFileFilenameMax);
        SDL_free(cwd);
    #endif
        m_DrivePath[radFileFilenameMax] = '\0';
    }
#endif

    // ============================================================
    // CAMBIO AQUÍ: asegurar "/" final SOLO si no lo trae ya.
    // (En Android ya lo construimos con "/" al final, pero lo dejo
    //  defensivo por si cambias el snprintf en el futuro)
    // ============================================================
    size_t len = strlen(m_DrivePath);
    if (len > 0 && m_DrivePath[len - 1] != '/' && len < radFileFilenameMax) {
        strncat(m_DrivePath, "/", radFileFilenameMax - len - 1);
    }

#if SDL_MAJOR_VERSION < 3
    m_Capabilities = (radDriveWriteable | radDriveFile);
#else
    m_Capabilities = (radDriveEnumerable | radDriveWriteable | radDriveDirectory | radDriveFile);
#endif

#ifdef RAD_ANDROID
    // CAMBIO AQUÍ: logs para verificar que ya apunta donde toca
    //LOGI("radSdlDrive ctor: ext='%s'", ext ? ext : "(null)");
    //LOGI("radSdlDrive ctor: m_DriveName='%s'", m_DriveName);
    //LOGI("radSdlDrive ctor: m_DrivePath='%s'", m_DrivePath);

    LogPathStatus("DrivePath", m_DrivePath);

    // CAMBIO AQUÍ: comprobar subcarpeta sound relativa a root
    // (evita hardcodear /data/data/..., ahora cuelga de m_DrivePath)
    char soundPath[radFileFilenameMax + 1];
    snprintf(soundPath, sizeof(soundPath), "%ssound", m_DrivePath);
    LogPathStatus("SoundDir", soundPath);
#endif
}

  
  
//=============================================================================
// Function:    radSdlDrive::~radSdlDrive
//=============================================================================

radSdlDrive::~radSdlDrive( void )
{
    m_pMutex->Release( );
    m_pDriveThread->Release( );
}

//=============================================================================
// Function:    radSdlDrive::Lock
//=============================================================================
// Description: Start a critical section
//
// Parameters:  
//
// Returns:     
//------------------------------------------------------------------------------

void radSdlDrive::Lock( void )
{
    m_pMutex->Lock( );
}

//=============================================================================
// Function:    radSdlDrive::Unlock
//=============================================================================
// Description: End a critical section
//
// Parameters:  
//
// Returns:     
//------------------------------------------------------------------------------

void radSdlDrive::Unlock( void )
{
    m_pMutex->Unlock( );
}

//=============================================================================
// Function:    radSdlDrive::GetCapabilities
//=============================================================================

unsigned int radSdlDrive::GetCapabilities( void )
{
    return m_Capabilities;
}

//=============================================================================
// Function:    radGcnDVDDrive::GetDriveName
//=============================================================================

const char* radSdlDrive::GetDriveName( void )
{
    return m_DriveName;
}

//=============================================================================
// Function:    radSdlDrive::Initialize
//=============================================================================

radDrive::CompletionStatus radSdlDrive::Initialize( void )
{
    SetMediaInfo();

    //
    // Success
    //
    m_LastError = Success;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::OpenFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::OpenFile
( 
    const char*         fileName, 
    radFileOpenFlags    flags, 
    bool                writeAccess, 
    radFileHandle*      pHandle, 
    unsigned int*       pSize 
)
{
    //
    // Build the full filename
    //
    char fullName[ radFileFilenameMax + 1 ];
    BuildFileSpec( fileName, fullName, radFileFilenameMax + 1 );

    //
    // Translate flags to SDL
    //
    const char* createFlags;
    switch( flags )
    {
    case OpenExisting:
        createFlags = writeAccess ? "rb+" : "rb";
        break;
    case OpenAlways:
        createFlags = "ab+";
        break;
    case CreateAlways:
        createFlags = "wb+";
        break;
    default:
        rAssertMsg( false, "radFileSystem: sdldrive: attempting to open file with unknown flag" );
        return Error;
    }
	
	
    #if defined(RAD_ANDROID) && defined(RAD_DEBUG)
    LOGI("OpenFile: fileName='%s' -> fullName='%s' mode='%s'",
         fileName ? fileName : "(null)",
         fullName,
         createFlags ? createFlags : "(null)");

    // ¿Existe? ¿se puede leer?
    errno = 0;
    int a = access(fullName, R_OK);
    LOGI("OpenFile: access(R_OK)=%d errno=%d (%s)", a, errno, strerror(errno));
#endif


#if SDL_MAJOR_VERSION < 3
    *pHandle = SDL_RWFromFile(fullName, createFlags);
#else
#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
        LOGI("OpenFile FAILED: fullName='%s' errno=%d (%s) SDL_GetError='%s'",
             fullName, errno, strerror(errno), SDL_GetError());
#endif

    *pHandle = SDL_IOFromFile( fullName, createFlags );
#endif

    if ( *pHandle )
    {
        m_OpenFiles++;
#if SDL_MAJOR_VERSION < 3
        *pSize = SDL_RWsize( (SDL_RWops*)*pHandle );
#else
        *pSize = SDL_GetIOSize( (SDL_IOStream*)*pHandle );
#endif
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::CloseFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CloseFile( radFileHandle handle, const char* fileName )
{
#if SDL_MAJOR_VERSION < 3
    SDL_RWclose( (SDL_RWops*)handle );
#else
    SDL_CloseIO( (SDL_IOStream*)handle );
#endif
    m_OpenFiles--;
    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::ReadFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::ReadFile
( 
    radFileHandle   handle, 
    const char*     fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int    position, 
    void*           pData, 
    unsigned int    bytesToRead, 
    unsigned int*   bytesRead, 
    radMemorySpace  pDataSpace 
)
{
    rAssertMsg( pDataSpace == radMemorySpace_Local, 
                "radFileSystem: radSdlDrive: External memory not supported for reads." );

    //
    // set file pointer
    //
#if SDL_MAJOR_VERSION < 3
    if ( SDL_RWseek( (SDL_RWops*)handle, position, RW_SEEK_SET ) >= 0 )
    {
        if (SDL_RWread( (SDL_RWops*)handle, pData, 1, bytesToRead ) > 0 )
        {
#else
    if ( SDL_SeekIO( (SDL_IOStream*)handle, position, SDL_IO_SEEK_SET ) >= 0 )
    {
        if ( SDL_ReadIO( (SDL_IOStream*)handle, pData, bytesToRead ) > 0 )
        {
#endif
            //
            // Successful read!
            //
            
            //
            // Change this during buffered read!!
            //
            *bytesRead = bytesToRead;
            m_LastError = Success;
            return Complete;
        }
    }

    //
    // Failed!
    //
    m_LastError = FileNotFound;
    return Error;
}

//=============================================================================
// Function:    radSdlDrive::WriteFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::WriteFile
( 
    radFileHandle     handle,
    const char*       fileName,
    IRadFile::BufferedReadState buffState,
    unsigned int      position, 
    const void*       pData, 
    unsigned int      bytesToWrite, 
    unsigned int*     bytesWritten, 
    unsigned int*     pSize, 
    radMemorySpace    pDataSpace 
)
{
    if ( !( m_Capabilities & radDriveWriteable ) )
    {
        rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the WriteFile function." );
        return Error;
    }

    rAssertMsg( pDataSpace == radMemorySpace_Local, 
                "radFileSystem: radSdlDrive: External memory not supported for reads." );

    //
    // do the write
    //
#if SDL_MAJOR_VERSION < 3
    if ( SDL_RWseek( (SDL_RWops*)handle, position, RW_SEEK_SET ) >= 0 )
    {
        *bytesWritten = SDL_RWwrite( (SDL_RWops*)handle, pData, 1, bytesToWrite );
#else
    if ( SDL_SeekIO( (SDL_IOStream*)handle, position, SDL_IO_SEEK_SET ) >= 0 )
    {
        *bytesWritten = SDL_WriteIO( (SDL_IOStream*)handle, pData, bytesToWrite );
#endif
        if ( *bytesWritten == bytesToWrite )
        {
            //
            // Sucessful write
            //
#if SDL_MAJOR_VERSION < 3
            *pSize = SDL_RWsize( (SDL_RWops*)handle );
#else
            *pSize = SDL_GetIOSize( (SDL_IOStream*)handle );
#endif
            m_LastError = Success;
            return Complete;
        }
    }

    //
    // Failed!
    //
    m_LastError = FileNotFound;
    return Error;
}

#if SDL_MAJOR_VERSION > 2
//=============================================================================
// Function:    radSdlDrive::FindFirst
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindFirst
( 
    const char*                 searchSpec, 
    IRadDrive::DirectoryInfo*   pDirectoryInfo, 
    radFileDirHandle*           pHandle,
    bool                        firstSearch
)
{
    //
    // Find first
    //
    const char* pattern = strrchr(searchSpec, '\\');
    std::string path;
    if (!pattern)
        pattern = strrchr(searchSpec, '/');
    if (pattern)
        path = std::string(searchSpec, pattern - searchSpec);
    else
        pattern = searchSpec;
    path = m_DrivePath + path;
    std::replace(path.begin(), path.end(), '\\', '/');

    char** handle = SDL_GlobDirectory( path.c_str(), pattern, SDL_GLOB_CASEINSENSITIVE, NULL );
    if ( handle )
    {
        SDL_PathInfo info;
        if ( SDL_GetPathInfo( handle[0], &info))
            m_LastError = TranslateDirInfo( pDirectoryInfo, &info, pHandle );
        else
            m_LastError = TranslateDirInfo( pDirectoryInfo, NULL, pHandle );
        // HACK: We don't need the first element anymore, so use it to store the iterator
        ((char***)handle)[0] = handle;
    }
    else
    {
        m_LastError = FileNotFound;
    }

    //
    // Fill in our directory info structure
    //
    if ( m_LastError == Success )
    {
        return Complete;
    }
    else
    {
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::FindNext
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindNext( radFileDirHandle* pHandle, IRadDrive::DirectoryInfo* pDirectoryInfo )
{
    //
    // If we don't have a handle, return file not found.
    //
    if ( *pHandle == NULL )
    {
        m_LastError = FileNotFound;
        return Error;
    }

    //
    // Find the next entry
    //
    char*** handle = (char***)*pHandle;
    *handle++;
    SDL_PathInfo info;
    if ( SDL_GetPathInfo(  **handle, &info ))
        m_LastError = TranslateDirInfo( pDirectoryInfo, &info, pHandle );
    else
        m_LastError = TranslateDirInfo( pDirectoryInfo, NULL, pHandle );
    
    if ( m_LastError == Success )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::FindClose
//=============================================================================

radDrive::CompletionStatus radSdlDrive::FindClose( radFileDirHandle* pHandle )
{
    SDL_free( *pHandle );
    *pHandle = NULL;

    return Complete;
}

//=============================================================================
// Function:    radSdlDrive::CreateDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::CreateDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory, 
        "This drive does not support the CreateDir function." );

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( SDL_CreateDirectory( fullSpec ) )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::DestroyDir
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyDir( const char* pName )
{
    rWarningMsg( m_Capabilities & radDriveDirectory,
        "This drive does not support the DestroyDir function." );

    //
    // Someday check if pName is a dir!
    //

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( pName, fullSpec, radFileFilenameMax + 1 );

    if ( SDL_RemovePath( fullSpec ) )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}

//=============================================================================
// Function:    radSdlDrive::DestroyFile
//=============================================================================

radDrive::CompletionStatus radSdlDrive::DestroyFile( const char* filename )
{
    rWarningMsg( m_Capabilities & radDriveWriteable, "This drive does not support the DestroyFile function." );

    //
    // Someday check if the file is open!
    //

    //
    // Build the full filename
    //
    char fullSpec[ radFileFilenameMax + 1 ];
    BuildFileSpec( filename, fullSpec, radFileFilenameMax + 1 );

    if ( SDL_RemovePath( fullSpec ) )
    {
        m_LastError = Success;
        return Complete;
    }
    else
    {
        m_LastError = FileNotFound;
        return Error;
    }
}
#endif

//=============================================================================
// Private Member Functions
//=============================================================================

//=============================================================================
// Function:    radSdlDrive::SetMediaInfo
//=============================================================================

void radSdlDrive::SetMediaInfo( void )
{
    //
    // Get volume information.
    //
    const char* realDriveName = m_DriveName;

    //rAssert( strlen( realDriveName ) == 2 );
    strcpy(m_MediaInfo.m_VolumeName, realDriveName );
    //strcat(m_MediaInfo.m_VolumeName, "\\");

    m_MediaInfo.m_SectorSize = SDL_DEFAULT_SECTOR_SIZE;

    /*
    if(!error)
    {
        m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
        m_MediaInfo.m_FreeSpace = space.free;

        //
        // No file limit, so set it to the available space
        //
        m_MediaInfo.m_FreeFiles = space.available / m_MediaInfo.m_SectorSize;
        m_LastError = Success;
    }
    else
    */
    {
        //
        // Don't have media info, so fill structure in with dummy info
        //
        m_MediaInfo.m_MediaState = IRadDrive::MediaInfo::MediaPresent;
        m_MediaInfo.m_FreeSpace = UINT_MAX;
        m_MediaInfo.m_FreeFiles = m_MediaInfo.m_FreeSpace / m_MediaInfo.m_SectorSize;
        m_LastError = Success;
    }
}

//=============================================================================
// Function:    radSdlDrive::BuildFileSpec
//=============================================================================

void radSdlDrive::BuildFileSpec( const char* fileName, char* fullName, unsigned int size )
{
    std::string path(m_DrivePath);
    path += fileName;
    std::replace(path.begin(), path.end(), '\\', '/');

    strncpy( fullName, path.c_str(), size - 1 );
    fullName[ size - 1 ] = '\0';
}

#if SDL_MAJOR_VERSION > 2
//=============================================================================
// Function:    radSdlDrive::TranslateDirInfo
//=============================================================================
// Description: Translate the directory info and return an error status. A handle
//              with value directory_iterator() means the find_first/next call
//              failed and needs to be checked if something went wrong or if the
//              search just ended.
//
// Parameters:  
//              
// Returns:     
//------------------------------------------------------------------------------

radFileError radSdlDrive::TranslateDirInfo
( 
    IRadDrive::DirectoryInfo*   pDirectoryInfo, 
    const SDL_PathInfo*         pPathInfo,
    const radFileDirHandle*     pHandle
)
{
    char*** handle = (char***)*pHandle;
    if ( !pPathInfo || !*handle )
    {
        //
        // Either we failed or we're out of games.
        //
        if ( !pPathInfo )
        {
            return FileNotFound;
        }
        else
        {
            pDirectoryInfo->m_Name[0] = '\0';
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDone;
        }
    }
    else
    {
        strncpy( pDirectoryInfo->m_Name, **handle, radFileFilenameMax );
        pDirectoryInfo->m_Name[ radFileFilenameMax ] = '\0';

        if ( pPathInfo->type == SDL_PATHTYPE_DIRECTORY )
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsDirectory;
        }
        else
        {
            pDirectoryInfo->m_Type = IRadDrive::DirectoryInfo::IsFile;
        }
    }
    return Success;
}
#endif
