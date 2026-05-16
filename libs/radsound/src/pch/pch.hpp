//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef RADSOUND_PCH_HPP
#define RADSOUND_PCH_HPP

//
// only win32 and xbox take advantage of the pch at this moment.
//
#if defined RAD_WIN32 || defined RAD_XBOX || defined RAD_ANDROID
//
// Microsoft header files
//
#ifdef RAD_WIN32
#define WINVER 0x0501
#define _WIN32_WINNT 0x400
#endif

#ifdef RAD_XBOX
#include <xtl.h>
#include <dsound.h>
#elif defined RAD_ANDROID

#else
#include <al.h>
#include <alc.h>
#include <efx.h>
#endif

//
// Standard C header files
//

#include <stdio.h>

#endif // defined RAD_WIN32 || defined RAD_XBOX|| defined RAD_ANDROID

#endif // RADSOUND_PCH_HPP
