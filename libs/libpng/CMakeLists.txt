cmake_minimum_required(VERSION 3.6)

project(libpng C)

# Fuentes base de libpng 1.0.x
set(PNG_SOURCES
    png.c
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtrans.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c
)

add_library(png_static STATIC ${PNG_SOURCES})

# Los headers están en esta misma carpeta (png.h, pngconf.h, etc.)
target_include_directories(png_static PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# zlib (NDK) en Android: link con -lz
if(ANDROID)
    find_library(ZLIB_LIB z)
    target_link_libraries(png_static PUBLIC ${ZLIB_LIB})

    # Algunas versiones viejas agradecen esto en Android/Clang
    target_compile_definitions(png_static PRIVATE PNG_NO_MMX_CODE PNG_NO_ASSEMBLER_CODE)
endif()
