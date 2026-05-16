#pragma once

#include <raddebug.hpp>   // para rDebugPrintf()

#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
    #define LOGI(...) rDebugPrintf(__VA_ARGS__)
#else
    // Importante: esto evita evaluación de argumentos
    #define LOGI(...) ((void)0)
#endif

// ERROR: recomendado que exista también en Release
// - En Debug: rDebugPrintf
// - En Release: rReleasePrintf (Radical “siempre disponible”)
#if defined(RAD_ANDROID) && defined(RAD_DEBUG)
    #define LOGE(...) rDebugPrintf("[ERROR] " __VA_ARGS__)
#else
    #define LOGE(...) rReleasePrintf("[ERROR] " __VA_ARGS__)
#endif