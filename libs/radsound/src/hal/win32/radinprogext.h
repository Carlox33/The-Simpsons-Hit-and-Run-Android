#ifndef RAD_ANDROID
  #include <al.h>
#endif
#ifdef RAD_ANDROID
  #include <AL/al.h> 
#endif


#ifndef RAD_ANDROID
#define AL_MAP_READ_BIT_SOFT                     0x00000001
#define AL_MAP_WRITE_BIT_SOFT                    0x00000002
#define AL_MAP_PERSISTENT_BIT_SOFT               0x00000004
typedef unsigned int ALbitfieldSOFT;
typedef void (AL_APIENTRY*LPALBUFFERSTORAGESOFT)(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq, ALbitfieldSOFT flags);
typedef void* (AL_APIENTRY*LPALMAPBUFFERSOFT)(ALuint buffer, ALsizei offset, ALsizei length, ALbitfieldSOFT access);
typedef void (AL_APIENTRY*LPALUNMAPBUFFERSOFT)(ALuint buffer);
extern LPALBUFFERSTORAGESOFT radBufferStorageSOFT;
extern LPALMAPBUFFERSOFT radMapBufferSOFT;
extern LPALUNMAPBUFFERSOFT radUnmapBufferSOFT;
#else
// Android stubs - not used
#define AL_MAP_READ_BIT_SOFT                     0x00000001
#define AL_MAP_WRITE_BIT_SOFT                    0x00000002
#define AL_MAP_PERSISTENT_BIT_SOFT               0x00000004
typedef unsigned int ALbitfieldSOFT;
typedef void (*LPALBUFFERSTORAGESOFT)(unsigned int buffer, unsigned int format, const void *data, int size, int freq, ALbitfieldSOFT flags);
typedef void* (*LPALMAPBUFFERSOFT)(unsigned int buffer, int offset, int length, ALbitfieldSOFT access);
typedef void (*LPALUNMAPBUFFERSOFT)(unsigned int buffer);
extern LPALBUFFERSTORAGESOFT radBufferStorageSOFT;
extern LPALMAPBUFFERSOFT radMapBufferSOFT;
extern LPALUNMAPBUFFERSOFT radUnmapBufferSOFT;
#endif