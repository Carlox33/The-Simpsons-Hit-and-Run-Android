//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        radfile.hpp
//
// Subsystem:	Foundation Technologies - File Subsystem
//
// Description:	This file contains all definitions and interfaces required to
//              interact with the file subsystem. This subsystem provides
//              services to access hard drives, CDROMs, CDVD, and memory cards
//              in a platform independant manner.
//
// Revisions:  	
//
//=============================================================================

#ifndef	RADFILE_HPP
#define RADFILE_HPP

//=============================================================================
// Build Configuration Check
//=============================================================================
 
#if !defined(RAD_GAMECUBE) && !defined(RAD_PS2) && !defined(RAD_XBOX) && !defined(RAD_WIN32)
    #error 'FTech requires definition of RAD_GAMECUBE, RAD_PS2, RAD_XBOX, or RAD_WIN32'
#endif
 
//=============================================================================
// Include Files
//=============================================================================

#include <radobject.hpp>
#include <radmemory.hpp>
#include <radthread.hpp>

#if defined(RAD_ANDROID)
// Shared game storage root, resolved by the SDL drive for this run.
const char* radGetAndroidSharedStoragePath();
#endif

//=============================================================================
// Forward Class/Struct Declarations
//=============================================================================

struct IRadFileCompletionCallback;
struct IRadCementLibraryCompletionCallback;
struct IRadDriveCompletionCallback;
struct IRadDriveErrorCallback;
struct IRadFile;
struct IRadCementLibrary;
struct IRadDrive;

//=============================================================================
// Drive and file naming conventions
//=============================================================================
// The default path token seperator employed by the file system is the forward
// slash ie ("\\"). It will be used whenever the the file system returns a file 
// name or path. When the caller provides a path name or file name he can use
// either the forward or backslash. (ie "\\" or "/" ).
//
// PS2 Drive and filename considerations: 
// the host drive is used by default in debug mode and the CDVD is used in release mode.
// The caller can use the drive spec if he wants explicit control of where the file 
// is opened or set the default drive.
//
// Drive name       Details  
// CDROM:           PS2 CDVD drive, always available, default drive (tune/release)
// MEMCARD1A:       PS2 Memcard slot 1A, always available
// MEMCARD1B:       PS2 Memcard slot 1B, media is not present if there is no mutitap
// MEMCARD1C:       PS2 Memcard slot 1C, media is not present if there is no mutitap
// MEMCARD1D:       PS2 Memcard slot 1D, media is not present if there is no mutitap
// MEMCARD2A:       PS2 Memcard slot 2A, always available
// MEMCARD2B:       PS2 Memcard slot 2B, media is not present if there is no mutitap
// MEMCARD2C:       PS2 Memcard slot 2C, media is not present if there is no mutitap
// MEMCARD2D:       PS2 Memcard slot 2D, media is not present if there is no mutitap
// REMOTEDRIVE:     Radical File Server, only available if host communication is enabled
// HOSTDRIVE:       native debugger file server (ie SNs TargetManager), default drive (debug)
//
// Win32 Drive and filename considerations: 
// Just use drive letters A: B: C: etc...
// By default the drive containing the current directory is used.
//
// XBOX Drive and filename considerations:
// The dvd drive is used by default, but is mapped to the HDD by XBOX SDK during debugging.
//
// Drive name       Details  
// D:               DVD drive, always available (default)
// T:               Title data, always available
// U:               Saved games, always available
// W:               WMA soundtrack drive, always available
// Z:               Utility drive, available only if mounted (either using the SDK or as a
//                                 parameter when building the XBE file)
// MEMCARD1A:       XBox memory unit in controller 1 (top),    always available
// MEMCARD1B:       XBox memory unit in controller 1 (bottom), always available
// MEMCARD2A:       XBox memory unit in controller 2 (top),    always available
// MEMCARD2B:       XBox memory unit in controller 2 (bottom), always available
// MEMCARD3A:       XBox memory unit in controller 3 (top),    always available
// MEMCARD3B:       XBox memory unit in controller 3 (bottom), always available
// MEMCARD4A:       XBox memory unit in controller 4 (top),    always available
// MEMCARD4B:       XBox memory unit in controller 4 (bottom), always available
// REMOTEDRIVE:     Radical File Server, only available if host communication is enabled
//
// GameCube Drive and filename considerations:
// By default the GameCube uses the DVD drive. This is mapped to the host PC during debugging.
//
// Drive name       Details  
// DVD:             Gamecube DVD drive (default)
// MEMCARDCHANNELA: Gamecube Memcard channel A, always available
// MEMCARDCHANNELB: Gamecube Memcard channel B, always available
// REMOTEDRIVE:     Radical file server, only available if host communication is enabled 
//

//=============================================================================
// Macros and defines
//=============================================================================

#ifdef RAD_XBOX
//
// Drive file caching is enabled only on the XBOX
//
#define CACHING_ENABLED
#endif // RAD_XBOX

//=============================================================================
// Enumerations and constants
//=============================================================================

//
// This constant defines the optimal memory buffer alignment for use with
// file read and write operations on the current platform.
//
#if defined RAD_GAMECUBE
#define radFileOptimalMemoryAlignment 32
#elif defined RAD_PS2
#define radFileOptimalMemoryAlignment 64
#else
#define radFileOptimalMemoryAlignment 16
#endif

//
// This constant defines the best disk transfer size for the current platform.
//
#if defined RAD_GAMECUBE
#define radFileMaxSectorSize          32
#define radFileMaxMemcardSectorSize 8192
#elif defined RAD_PS2
#define radFileMaxSectorSize        2048
#define radFileMaxMemcardSectorSize 1024
#elif defined RAD_XBOX
#define radFileMaxSectorSize        2048
#else
#define radFileMaxSectorSize         512
#endif

//
// This enumeration is used to control the priority at which file operations
// are performed with respect to other file operations in the system. Normally
// first-in first-out manner unless the priority is altered.
//
enum radFilePriority
{   
    HighPriority,
    NormalPriority,
    LowPriority,
    NumPriorities
};

//
// This enumeration is used to govern the way a file is opened. Note: not all drive
// types support creation.
//
enum radFileOpenFlags
{
    OpenExisting = 0,               // Will open only if file exists,
    OpenAlways = 1,                 // If file does not exist, file will be created
    CreateAlways = 3                // Will create always even if file exists.
};

//
// This enumeration is used to define the various errors that are reported by the
// file system.
//
enum radFileError
{
    Success,                    // No Error
    FileNotFound,               // File not found on media
    ShellOpen,                  // Disk door, shell open
    WrongMedia,                 // Different media (ie files open and disk was changed)
    NoMedia,                    // No media present
    MediaNotFormatted,          // Media not formatted
    MediaCorrupt,               // Corrupt media
    NoFreeSpace,                // Not enough space to carry out operation
    HardwareFailure,            // General failure
    MediaEncodingErr,           // Media formatted for a different market
    MediaWrongType,             // Unsupported media
    MediaInvalid,               // Unrecognized media
    DataCorrupt                 // Data failed CRC check
};    

//
// Constants to govern the maximum length of drive names and filenames. Does not
// include null terminator.
//
const unsigned int radFileFilenameMax = 256;
const unsigned int radFileDrivenameMax = 16;

//
// Maximum number of drives, logical or physical
//
const unsigned int radFileDriveMax = 26;

//=============================================================================
// Save Game Structures
//=============================================================================

#ifdef RAD_PS2 
#include <radstring.hpp>

struct radPs2IconSys
{
    char m_Header[ 4 ];

    unsigned short m_Reserved1;         // Must be 0x0000
    unsigned short m_LineBreakPosition; // in bytes (2 bytes per character)
    unsigned m_Reserved2;               // Must be 0x00000000

    unsigned m_BgTransparency;

    int m_BgColor[ 4 ][ 4 ];
    float m_LightDir[ 3 ][ 4 ];
    float m_LightColor[ 3 ][ 4 ];
    float m_Ambient[ 4 ];

    char m_TitleName[ 68 ]; // 32 chars in SJIS

    char m_ListIconName[ 64 ];
    char m_CopyIconName[ 64 ];
    char m_DeleteIconName[ 64 ];

    unsigned char m_Reserved3[ 512 ];   // All zero
};

struct radMemcardInfo
{
    radPs2IconSys   m_IconSys;
    void*           m_ListIcon;
    unsigned int    m_ListIconSize;
    void*           m_CopyIcon;
    unsigned int    m_CopyIconSize;
    void*           m_DelIcon;
    unsigned int    m_DelIconSize;
};

//
// Function to fill out a radPs2IconSys structure. 
// title is in SJIS, lineBreak is in characters.
//
void radMakeIconSys( radPs2IconSys* pIconSys, radSJISChar* title, unsigned short lineBreak ); 
void radSetIconSysTitle( radPs2IconSys* pIconSys, radSJISChar* title, unsigned short lineBreak );  

#endif

#ifdef RAD_GAMECUBE

//
// One frame of the icon animation.
//
struct radMemcardIconData
{
    enum IconSpeed 
    {
        Slow   = 3,
        Middle = 2,
        Fast   = 1
    };

    enum IconFormat
    {
        RGB5A3  = 2,
        C8      = 1
    };

    void*       m_Data;
    void*       m_CLUT; // ignored in RGB mode
    IconFormat  m_Format;
    IconSpeed   m_Speed;
};

struct radMemcardInfo
{
    enum BannerFormat
    {
        RGB5A3  = 2,
        C8      = 1
    };

    enum IconAnimType 
    {
        Loop    = 0x00,
        Bounce  = 0x04
    };

    void*               m_Banner;
    void*               m_CLUT; // ignored in RGB mode
    BannerFormat        m_BannerFormat;

    unsigned int        m_NumFrames;
    radMemcardIconData  m_pIcon[ 8 ];

    IconAnimType        m_AnimType;

    char                m_CommentLine1[ 32 ];
    char                m_CommentLine2[ 32 ];
};

#endif

#ifdef RAD_XBOX

//
// We need a 64 x 64 DXT1 texture in an xbx file.
// If it's NULL, then we don't save that icon.
//
struct radMemcardInfo
{
    void*           m_Icon;
    unsigned int    m_IconSize;
};

#endif

#ifdef RAD_WIN32

//
// We don't have save games for win32.
//
struct radMemcardInfo
{
    char dummy;
};

#endif

//=============================================================================
// Functions
//=============================================================================

//
// Frame all filesystem operations with these two calls. During initialization, specify
// the system wide maximum number of file requests that will be oustanding at any
// one time. Used to preallocate internal data structures to prevent memory fragmentation.
// The maximum number of files objects that can be constructed at any one time is also
// specified here. There is no inhererent limitation of the file system as to the number
// of open files. However some operating systems, like the PS2 limit the number of 
// open files. If this limit is exceeded, the file system asserts and displays all open files.
//
void radFileInitialize( unsigned int maxOutstandingRequests = 50, 
                        unsigned int maxOpenFiles = 14, // Ps2 Limit
                        radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT );

void radFileTerminate( void );

//
// For the file system callbacks to operate correctly, this service must be invoked
// periodically by the game's main loop. This call is also needed to run the RemoteDrive.
//
void radFileService( void );

//
// This function can be used to specify what directory on a drive will be
// used as the root directory for file and drive operations.
// This is useful for placing an entire game in a subdirectory as on a demo disk. 
//
// If root directory relocation is desired, this call 
// should be made as soon as possible after calling radFileInitialize. 
// Any file or directory requests that are in-progress when a root directory
// is changed may produce incorrect results. Also, any relocations made
// after drive/file operations are started may not take effect for an
// arbitrarily long time.
//
// Note that root directory relocation is completely transparent. This means
// that the name of the root directory will never appear in any path strings
// visible to the user - even those produced by enumeration.
//
// This function can be called more than once - ie, for each physical drive.
// It can also be called again for the same drive, but generally this should
// not be done.
// For example:
//  radFileSetRootDirectory( "C:", "foo" ) - C: root is now C:\foo\
//      - A request for C:\mydir\myfile will actually get C:\foo\mydir\myfile
//  radFileSetRootDirectory( "D:", "bar" ) - D: root is now D:\bar\
//  radFileSetRootDirectory( "C:", "" ) - C: root is restored to C:\
//
// NOTE root directory relocation is not quite the same thing as setting
// the current working directory. If you changed the current working directory,
// you could still open files off the root directory by using an absolute
// path like "C:\myfile". With root directory relocation, even an absolute
// path will be made relative to the specified root.
//
// NOTE root relocations should be set up before setting cache file names,
// otherwise the names will not be hashed correctly.
//
// NOTE that on the XBOX, you must set the root of the cache drive to the same
// as the root of the drive being cached. That is, if you set the root of
// D: to "foo" you must also set the root of CACHED: to "foo", regardless of
// what you set the cache directory name to. If this is not done, cached
// files will be ignored.
//

void radFileSetRootDirectory( const char* pDrive, const char *pRootDir );
void radFileGetRootDirectory( const char* pDrive, char* pRootDir );

//
// These functions open a file. The object must be checked to see if it was 
// opened/created successfully before use. Caller can specify a memory cache to 
// associate with the file. This can increase read performance significantly if 
// caller plans on reading a few bytes at a time.
//
// radFileOpen( ) is asynchronous and returns an object immediately which has not
// necessarily been created. A completion callback or event can be used to inform
// the user when their operation is complete.
//
void radFileOpenAsync( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize = 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_TEMP,
                  radMemorySpace cacheSpace = radMemorySpace_Local );

void radFileOpenSync( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize = 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_TEMP,
                  radMemorySpace cacheSpace = radMemorySpace_Local );

//
// Special function to open save games
//
void radSaveGameOpenAsync( 
    IRadFile** pIRadFile, 
    const char* pFileName,
    bool writeAccess = false,
    radFileOpenFlags flags = OpenExisting,
    radMemcardInfo* memcardInfo = NULL,
    unsigned int maxSize = 0,
    radFilePriority priority = NormalPriority
);

void radSaveGameOpenSync( 
    IRadFile** pIRadFile, 
    const char* pFileName,
    bool writeAccess = false,
    radFileOpenFlags flags = OpenExisting,
    radMemcardInfo* memcardInfo = NULL,
    unsigned int maxSize = 0,
    radFilePriority priority = NormalPriority
);

//
// These functions open a drive object. When a drive is opened, it will perform 
// initialization. To prevent, this protentially costly operation from occuring in 
// the game, it is advised that initialization code open and hold the drive for the 
// life of the game.
//
// radDriveOpen( ) is asynchronous and returns an object immediately which has not
// necessarily been created. A completion callback or event can be used to inform
// the user when their operation is complete.
//
void radDriveOpenAsync( IRadDrive** pIRadDrive,
                   const char* pDriveName,
                   radFilePriority priority = NormalPriority, 
                   radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT );

void radDriveOpenSync( IRadDrive** pIRadDrive,
                   const char* pDriveName,
                   radFilePriority priority = NormalPriority, 
                   radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT );

//
// TEMPORARY! These are for temporary backward compatibility ONLY!
//
inline void radFileOpen( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize = 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_TEMP,
                  radMemorySpace cacheSpace = radMemorySpace_Local )
{
    radFileOpenAsync( pIRadFile, pFileName, writeAccess, flags, priority, cacheSize, alloc, cacheSpace );
}

inline void radDriveOpen( IRadDrive** pIRadDrive,
                   const char* pDriveName,
                   radFilePriority priority = NormalPriority, 
                   radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT )
{
    radDriveOpenAsync( pIRadDrive, pDriveName, priority, alloc );
}

//
// Get/Set the default drive. See Section "Drive and file naming conventions"
// for the default drive on each platform.
//
bool radSetDefaultDrive( const char* pDriveName );
void radGetDefaultDrive( char* pDriveName );

#ifdef CACHING_ENABLED
//
// On the XBOX, you can mirror files that are used regularly to the hard drive
// to help shorten loading time.
// All files in the given file name array (pCacheFileNameArray) will be
// automattically mirrored onto the XBOX hard drive (currently Z:\radcache0R1G)
// when they are opened. The last filename in the list
// must always be NULL.
// If you wish to cache files that are not in this list, explicitly specify
// that you are using CACHED:, CACHEF:, etc.  (as opposed to D:, F:, etc. )
//
void radSetCacheFileNames( const char* pCacheFileNameArray[ ] );

//
// You can change the actual directory where files are cached manually.
// Make sure that this is changed only when no files are currently open.
//
void radSetCacheDirectory( const char* pCacheDirectory );

#endif //CACHING_ENABLED

//
// Set the default size of the various chunks transfered during a file read/write.
//
void radSetDefaultGranularity( unsigned int defaultGranularity );

//
// Priorities to dictate if the library is searched before or after the drive.
//
enum radCementLibraryPriority
{
    radCementLibraryBeforeDrive,
    radCementLibraryAfterDrive,
    radCementLibraryNumPriorities
};

//
// Use a cement file to group files into a single library. After registering the
// cement library, all files in that library will act as if they are a part of
// the normal file system. To access the files, just issue a normal file open command.
// In particular, the drive on which these files are located will be the exact same
// drive that the cement library is found on. This drive can be either the default
// drive for the platform, or is determined by a cementLibraryName of the form
// "drivename:filename". 
//
// radFileRegisterCementLibrary( ) is asynchronous and returns an object immediately 
// which has not necessarily been created. A completion callback or event can be used to 
// inform the user when ther operation is complete.
//
void radFileRegisterCementLibraryAsync
(
    IRadCementLibrary** pIRadCementLibrary,
    const char* cementLibraryFileName,
    radCementLibraryPriority priority = radCementLibraryBeforeDrive,
    unsigned int cacheSize = 0,
    radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
    radMemorySpace cacheSpace = radMemorySpace_Local
);

void radFileRegisterCementLibrarySync
(
    IRadCementLibrary** pIRadCementLibrary,
    const char* cementLibraryFileName,
    radCementLibraryPriority priority = radCementLibraryBeforeDrive,
    unsigned int cacheSize = 0,
    radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
    radMemorySpace cacheSpace = radMemorySpace_Local
);

//
// TEMPORARY! This is for temporary backward compatibility only!
//
inline void radFileRegisterCementLibrary
(
    IRadCementLibrary** pIRadCementLibrary,
    const char* cementLibraryFileName,
    radCementLibraryPriority priority = radCementLibraryBeforeDrive,
    unsigned int cacheSize = 0,
    radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
    radMemorySpace cacheSpace = radMemorySpace_Local
)
{
    radFileRegisterCementLibraryAsync( pIRadCementLibrary, cementLibraryFileName, priority, cacheSize, alloc, cacheSpace );
}

//
// This function allows the client to adjust how long the remote drive will wait
// for the host file server to appear before failing requests. This service is needed
// for applications like the boot loader and should not be required by games.
//
void radFileSetConnectTimeOut( unsigned int milliseconds );

//
// Drive mount points, All drives must be mounted before they can be used. If no argument
// is given, then the default drive is used. Both of these functions are synchronous.
// Unmounting waits for all outstanding requests to finish, so it can take a long time.
//
bool radDriveMount( const char* pDriveSpec = NULL, radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT );
bool radDriveUnmount( const char* pDriveSpec = NULL);

//
// Mostly for the sake of windows tools, by default auto-mounting is turned on. This
// will mount a drive when it is used (by opening a file or drive). It will use the
// same allocator which is given to radFileInitialize( ). Games should turn auto-mounting
// off using radFileSetAutoMount( ), and mount all drives explicitly. This will allow the
// game to dictate when a drive is mounted, and which allocator is used. If auto-mount is
// turned on, radFileTerminate( ) will auto-unmount.
//
void radFileSetAutoMount( bool auto_mount );

//=============================================================================
// Interfaces
//=============================================================================

//
// This interface is used to interact with a file object. 
//
struct IRadFile : public IRefCount
{
    //
    // Basic asynchronous file operations. Objects passed by reference must survive
    // until the operation completes.
    //
    virtual void ReadAsync( void* pBuffer, unsigned int bytesToRead, radMemorySpace pBufferSpace = radMemorySpace_Local ) = 0;
    virtual void WriteAsync( const void* pBuffer, unsigned int bytesToWrite, radMemorySpace pBufferSpace = radMemorySpace_Local ) = 0;
    virtual void SetPositionAsync( unsigned int position ) = 0;
    virtual void GetPositionAsync( unsigned int* pPosition ) = 0;
    virtual void GetSizeAsync( unsigned int* pSize ) = 0;
    virtual void CommitAsync( void ) = 0;

    //
    // Basic synchronous file operations.
    //
    virtual void ReadSync( void* pBuffer, unsigned int bytesToRead, radMemorySpace pBufferSpace = radMemorySpace_Local ) = 0;
    virtual void WriteSync( const void* pBuffer, unsigned int bytesToWrite, radMemorySpace pBufferSpace = radMemorySpace_Local ) = 0;
    virtual void SetPositionSync( unsigned int position ) = 0;
    virtual void GetPositionSync( unsigned int* pPosition ) = 0;
    virtual void GetSizeSync( unsigned int* pSize ) = 0;
    virtual void CommitSync( void ) = 0;

    //
    // This member is provided for convience. It can be used to obtain the file size 
    // without waiting. It will assert if the file has not completed opening. Bad things
    // will happen if an asynchronous write operation is queued up.
    //
    virtual unsigned int GetSize( void ) = 0;

    //
    // Get the optimal read size/alignment (sector size)
    //
    virtual unsigned int GetOptimalSize( void ) = 0;

    //
    // Use this member to cancel all outstanding requests issued against the file object.
    // Callbacks will still be invoked.
    //
    virtual void CancelAsync( void ) = 0;

    //
    // This allows the caller to receive asychronous notification when all previously issued
    // file operations complete.
    //
    virtual void AddCompletionCallback( IRadFileCompletionCallback* pCallback, void* pUserData ) = 0;
    virtual void AddCompletionEvent( IRadThreadSemaphore* pSemaphore ) = 0;

    //
    // Use this member to check for completion of all operations issued against this file
    // object. Returns true if no outstanding operations exist.
    //
    virtual bool CheckForCompletion( void ) = 0;

    //
    // Use this member to wait for completion of an asynchronous operation.
    //
    virtual void WaitForCompletion( void ) = 0;            
    
    //
    // Use this member to retrieve the status of the last operation performed on the file.
    // 
    virtual radFileError GetLastError( void ) = 0;

    //
    // Use this member to determine if the file is open.
    //
    virtual bool IsOpen( void ) = 0;

    //
    // This member function is provided for convenience. It allows the caller to obtain 
    // the name of the file. Can be useful if caller does not want to maintain the name.
    // Pointer will be valid as long a file object is valid.
    //
    virtual const char* GetFilename( void ) = 0;

    //
    // This member function is provided for convenience. It allows the caller to obtain 
    // the name of the drive which own the file.
    //
    virtual const char* GetDrivename( void ) = 0;

	//
	// Sets the priority of the file after the file has been created, there
	// must be 0 pending requests against the file.
	//
    virtual void SetPriority( radFilePriority priority ) = 0;
	virtual radFilePriority GetPriority( void ) = 0;

    virtual bool IsSaveGame( void ) = 0;

    //
    // Turn buffered reads on/off. By default it is auto, which means it is used
    // as needed. Off will cause an assert on all non-optimal reads.
    // Setting the state can only occur if there are no outstanding requests.
    //
    enum BufferedReadState
    {
        BufferedReadAuto,
        BufferedReadOn,
        BufferedReadOff
    };

    virtual void SetBufferedRead( BufferedReadState state ) = 0;

    virtual void* GetHandle( void ) = 0;
};

//
// This interface is used to configure a cement library.  Each cement file
// is owned by the user and, to unregister the library, just release it.
//
struct IRadCementLibrary : public IRefCount
{
    //
    // Check if it's loaded.
    //
    enum CementLibraryStatus
    {
        CementLibraryPending,
        CementLibraryReady,
        CementLibraryError
    };

    virtual CementLibraryStatus GetStatus( void ) = 0;

    //
    // Use this member to wait for completion of an asynchronous operation.
    //
    virtual void WaitForCompletion( void ) = 0;            

    //
    // This allows the caller to receive asychronous notification when the
    // cement library is completely registered.
    //
    virtual void SetCompletionCallback( IRadCementLibraryCompletionCallback* pCallback,
                                        void* pUserData ) = 0;

    //
    // Use this member to determine if the file is open (same as GetStatus == CementLibraryReady).
    //
    virtual bool IsOpen( void ) = 0;
};

//
// Use this interface to interact with the drive object.
//
struct IRadDrive : public IRefCount
{
    //
    // This member can be used to determine capabilities about the drive. Returns
    // a combination of these flags. Note all drives are readable by default. This 
    // service is synchronous.
    // 
    #define radDriveWriteable   1<<0          // Supports writes and file creates/destroys
    #define radDriveEnumerable  1<<1          // Supports enumeration
    #define radDriveRemovable   1<<2          // Drive can be removed
    #define radDriveFormat      1<<3          // Drive can be formatted
    #define radDriveDirectory   1<<4          // Drive supports directory creation/deletion 
    #define radDriveFile        1<<5          // Drive supports OpenFile
    #define radDriveSaveGame    1<<6          // Drive supports OpenSaveGame

    virtual unsigned int GetCapabilities( void ) = 0;    

    struct MediaInfo
    {
        enum MediaState
        {
            MediaPresent,                       // Present and valid
            MediaNotPresent,                    // No media present
            MediaNotFormatted,                  // Media not formatted
            MediaInvalid,                       // Media not understood by system
            MediaWrongType,                     // Unsupported type
            MediaEncodingErr,                   // Formatted for different market
            MediaDamaged                        // Media file system damaged or media failing
        };

        MediaState    m_MediaState;             // State of media
        unsigned int  m_FreeSpace;              // Free space
        unsigned int  m_FreeFiles;              // Free files
        unsigned int  m_SectorSize;             // Sector size for read/write alignments
        char          m_VolumeName[ 64 + 1];    // Volume name of media
                                                // needs to be at least 64 + 1 for ps2 cd
    };

    struct DirectoryInfo
    {
        enum DirectoryEntryType
        {
            IsFile,                         // Returned if record is file
            IsDirectory,                    // Returned if record is directory
            IsDone,                         // Return when no more records match
            IsSaveGame                      // Returned if record is a save game
        }   m_Type;
               
        char    m_Name[ radFileFilenameMax + 1 ];   // Name matching search.
    };   
    
    //
    // Basic drive operations.
    // GetMediaInfo:     get information about the current media.
    // Format:           not supported on all drives.
    // CreateDirectory:  (see DestroyDirectory)
    // DestroyDirectory: not supported on all drives. Fully qualified path must be
    //                   specified. Only one level of directory can be destroyed/created
    //                   per operation.
    // FindFirst:        (see FindNext)
    // FindNext:         enumerate a drive. Caller first provides the searchspec containing
    //                   wild cards. When no more matches are found DirectoryInfo::IsDone 
    //                   is set and DirectoryInfo::m_Name is invalid.
    // DestroyFile:      Destroy a file. Not supported by all drives. Will fail if the file
    //                   is open.
    //
    // Basic asynchronous drive operations. Objects passed by reference must survive
    // until the operation completes.
    //
    virtual void GetMediaInfoAsync( MediaInfo* pMediaInfo ) = 0;
    virtual void FormatAsync( void ) = 0;
    virtual void CreateDirectoryAsync( const char* pDirectoryName ) = 0;
    virtual void DestroyDirectoryAsync( const char* pDirectoryName ) = 0;
    virtual void FindFirstAsync( const char* searchspec, DirectoryInfo* pDirectoryInfo ) = 0;
    virtual void FindNextAsync(  DirectoryInfo* pDirectoryInfo ) = 0;
    virtual void DestroyFileAsync( const char* pFileName, bool simpleName = false ) = 0;

    //
    // Basic synchronous drive operations. Operation occurs after all asynchronous
    // operations are finished.
    //
    virtual void GetMediaInfoSync( MediaInfo* pMediaInfo ) = 0;
    virtual void FormatSync( void ) = 0;
    virtual void CreateDirectorySync( const char* pDirectoryName ) = 0;
    virtual void DestroyDirectorySync( const char* pDirectoryName ) = 0;
    virtual void FindFirstSync( const char* searchspec, DirectoryInfo* pDirectoryInfo ) = 0;
    virtual void FindNextSync( DirectoryInfo* pDirectoryInfo ) = 0;
    virtual void DestroyFileSync( const char* pFileName, bool simpleName = false ) = 0;

    //
    // Use this member to cancel all outstanding requests issued against the drive object.
    // Callbacks will still be invoked.
    //
    virtual void CancelAsync( void ) = 0;

    //
    // This allows the caller to receive asychronous notification when all previously issued
    // drive operations complete.
    //
    virtual void AddCompletionCallback( IRadDriveCompletionCallback* pCallback, void* pUserData ) = 0;
    virtual void AddCompletionEvent( IRadThreadSemaphore* pSemaphore ) = 0;

    //
    // Use this member to check for completion of all operations issued against this drive
    // object. Returns true if no outstanding operations exist.
    //
    virtual bool CheckForCompletion( void ) = 0;

    //
    // Use this member to wait for completion of an asynchronous operation.
    //
    virtual void WaitForCompletion( void ) = 0;            
    
    //
    // Use this member to retrieve the status of the last operation performed on the drive.
    // 
    virtual radFileError GetLastError( void ) = 0;

    //
    // Global error handling. When there is an error, the file system will do one of three
    // things: fail, retry the operation or suspend itself. In the third case, an error
    // handler must be used. This can happen in the following ways:
    // 
    // The first is to install an error handler callback which will be invoked with the 
    // reason for failure. Returning true from the callback will retry the operation,
    // false will fail the operation.
    //
    // The second is to install an error event. This event will be signalled when there
    // is an error. ResumeRequest( ) must be called after the error is cleared.
    //
    // The third is to poll the error status using CheckForErrorState( ). This must
    // be preceeded by a call to SetDefaultErrorBehaviour( Suspend ). When the error
    // is cleared, ResumeRequest( ) must be called.
    //
    // NOTE: the error handler will be invoked again with Success when the error condition
    // is cleared.
    //
    // NOTE: error callbacks will only be invoked at the next call to radFileService( ).
    //
    // NOTE: registering callbacks and events automatically set the default error
    // behaviour to Suspend. This must be done explicitly for CheckForErrorState( ).
    //
    // NOTE: by default, the error behaviour is Fail.
    //

    enum ErrorBehaviour
    {
        Fail,       // Fail operation on error
        Retry,      // Retry the operation
        Suspend     // Suspend file system until ResumeRequest( ) is called
    };

    virtual void RegisterErrorHandler( IRadDriveErrorCallback* callback, void* pUserData ) = 0;
    virtual void UnregisterErrorHandler( IRadDriveErrorCallback* callback ) = 0;

    virtual void RegisterErrorEvent( IRadThreadSemaphore* pSemaphore ) = 0;
    virtual void UnregisterErrorEvent( IRadThreadSemaphore* pSemaphore ) = 0;

    virtual bool CheckForErrorState( void ) = 0;
    virtual void ResumeRequest( bool retry ) = 0;

    virtual void SetDefaultErrorBehaviour( ErrorBehaviour behaviour ) = 0;

    //
    // This member controls whether the global error handler will be invoked again when
    // an error condition clears. The default is yes. If this happens, the error condition
    // passed to the handler will be Success.
    //
    virtual void SetErrorClearReporting( bool notifyOnErrorClear ) = 0;

    //
    // These members are used to set/get the size in bytes that file read and write requests
    // will be partitioned into. By using a smaller size, priority file requests can
    // more quickly be scheduled. By default, the size is infinity.
    //
    virtual void SetReadWriteGranularity( unsigned int Size ) = 0;
    virtual unsigned int GetReadWriteGranularity( void ) = 0;

    //
    // This function is purely provided as a helper. It allows the caller to explicity
    // open or create a file on this drive. File name should not contain the drive.
    // FileOpen( ) is asynchronous and FileOpenSync( ) is synchronous.
    // NOTE: to wait for the operation to complete, wait on the FILE not the drive.
    //
    virtual void  FileOpenAsync( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize= 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
                  radMemorySpace cacheSpace = radMemorySpace_Local ) = 0;

    // TEMPORARY!!!
    virtual void  FileOpen( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize= 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
                  radMemorySpace cacheSpace = radMemorySpace_Local ) = 0;

    virtual void  FileOpenSync( IRadFile** pIRadFile, 
                  const char* pFileName,
                  bool writeAccess = false,
                  radFileOpenFlags flags = OpenExisting,
                  radFilePriority priority = NormalPriority, 
                  unsigned int cacheSize= 0,
                  radMemoryAllocator alloc = RADMEMORY_ALLOC_DEFAULT,
                  radMemorySpace cacheSpace = radMemorySpace_Local ) = 0;

    virtual void SaveGameOpenAsync( IRadFile** pIRadFile, 
                    const char* pFileName,
                    bool writeAccess = false,
                    radFileOpenFlags flags = OpenExisting,
                    radMemcardInfo* memcardInfo = NULL,
                    unsigned int maxSize = 0,
					bool simpleName = false,
                    radFilePriority priority = NormalPriority ) = 0;

    virtual void SaveGameOpenSync( IRadFile** pIRadFile, 
                    const char* pFileName,
                    bool writeAccess = false,
                    radFileOpenFlags flags = OpenExisting,
                    radMemcardInfo* memcardInfo = NULL,
                    unsigned int maxSize = 0,
					bool simpleName = false,
                    radFilePriority priority = NormalPriority ) = 0;

    virtual void SetPriority( radFilePriority priority ) = 0;
	virtual radFilePriority GetPriority( void ) = 0;

    virtual const char* GetDriveName( void ) = 0;

    //
    // Return the amount of space needed on the drive to create a file of size `size'
    // using the given memcardinfo. For regular files, this will just return the size,
    // for save games, it will return the smallest size that will allow creation.
    //
    virtual unsigned int GetCreationSize( radMemcardInfo* memcardInfo, unsigned int size ) = 0;
};

//
// Interface for callbacks used to notify the user when asynchronous cement file registration
// is complete.
//
struct IRadCementLibraryCompletionCallback : public IRefCount
{
    virtual void OnCementLibraryRegistered( void* pUserData ) = 0;
};

//
// Interface for callbacks used to notify the user when asynchronous file operations are
// complete.
//
struct IRadFileCompletionCallback : public IRefCount
{
    virtual void OnFileOperationsComplete( void* pUserData ) = 0;
};

//
// Interface for callbacks used to notify the user when asynchronous drive operations are
// complete.
//

struct IRadDriveCompletionCallback : public IRefCount
{
    virtual void OnDriveOperationsComplete( void* pUserData ) = 0;    
};

//
// Interface for callbacks used to notify the user when an error has occurred on a drive.
// Returning true will retry the request, and false will fail the request.
//
struct IRadDriveErrorCallback 
{
    virtual bool OnDriveError( radFileError error, const char* pDriveName, void* pUserData ) = 0;
};


#endif //RADFILE_HPP
