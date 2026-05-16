//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef RADSOUNDWIN_HPP
#define RADSOUNDWIN_HPP

#ifndef RAD_ANDROID
  #include <al.h>
#endif
#ifdef RAD_ANDROID
  #include <AL/al.h> 
#endif


//============================================================================
// Helper Functions
//============================================================================

unsigned int radSoundFloatAngleToULongWin( float angle );
float        radSoundULongAngleToFloatWin( unsigned int angle );
float        radSoundVolumeDbToHardwareWin( float volume );
float        radSoundVolumeHardwareToDbWin( float hardwareVolume );

#endif // RADSOUNDWIN32_HPP
