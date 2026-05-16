# Si estamos usando Bink, habilitamos la compilación de Bink, de lo contrario usamos FFmpeg
if(SRR2_USE_BINK)
    # Configuración de los archivos de Bink (lo dejaremos igual ya que es el código original)
    set(BINK_SOURCES
        "../sdks/bink/binkacd.cpp"
        "../sdks/bink/binkasyncRR.c"
        "../sdks/bink/binkasyncthread.c"
        "../sdks/bink/binkcomm.c"
        "../sdks/bink/binkfile.c"
        "../sdks/bink/binkmarkers.c"
        "../sdks/bink/binkread.c"
        "../sdks/bink/binkoal.c"
        "../sdks/bink/binkssmix.c"
        "../sdks/bink/binkutil.c"
        "../sdks/bink/blitter.c" # custom
        "../sdks/bink/expand2.c"
        "../sdks/bink/popmal.c"
        "../sdks/bink/radfft.cpp"
    )

    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        list(APPEND BINK_SOURCES "../sdks/bink/x86_cpu.c")
    endif()

    if (WIN32)
        list(APPEND BINK_SOURCES
            "../sdks/bink/binkplugin.c"
            "../sdks/bink/binkpluginw32.c"
            "../sdks/bink/binkxa2.cpp"
            "../sdks/bink/win32_radmem.c"
            "../sdks/bink/win32_rrAtomics.cpp"
            "../sdks/bink/win32_rrCpu.c"
            "../sdks/bink/win32_rrThreads2.c"
            "../sdks/bink/wintimer.c"
        )
    else()
        list(APPEND BINK_SOURCES
            "../sdks/bink/binklnx.c"
            "../sdks/bink/binkssmix.c"
            "../sdks/bink/posix_defread.c"
            "../sdks/bink/posix_radmem.c"
            "../sdks/bink/posix_rrCpu.c"
            "../sdks/bink/posix_rrThreads2.c"
            "../sdks/bink/cpp_rratomics.cpp"
        )
    endif()

    # Creamos la biblioteca estática de Bink
    add_library(bink STATIC ${BINK_SOURCES})

    target_compile_definitions(bink
        PRIVATE
            USING_EGT
            INC_BINK2
            NO_BINK20
            NO_BINK10_SUPPORT
        PUBLIC
            __RADFINAL__
            __RADINSTATICLIB__
    )
    target_link_libraries(bink ${OPENAL_LIBRARY})

    if (WIN32)
        target_link_libraries(bink Winmm)
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(bink
            PRIVATE -Wno-multichar
        )
    endif()

    target_include_directories(bink
        PUBLIC "../sdks/bink/"
    )

    set(RADMOVIE_SOURCES
        "src/common/binkmovieplayer.cpp"
        "src/common/binkradfile.cpp"
        "src/common/binkrenderstrategy.cpp"
        "src/pch/pch.cpp"
    )

else()  # Si no estamos usando Bink, usamos FFmpeg para el manejo de video

    # Aquí configuramos los archivos para usar FFmpeg en lugar de Bink
    set(RADMOVIE_SOURCES
        "src/common/ffmpegmovieplayer.cpp"
        "src/common/binkrenderstrategy.cpp"
        "src/pch/pch.cpp"
    )
if(ANDROID)
		find_package(ffmpeg CONFIG REQUIRED)
		# ✅ CAMBIO: lista de libs/targets de FFmpeg a linkear desde Prefab
		
endif()  
endif()  # ✅ ESTE es el fin del if(SRR2_USE_BINK)

# Crear la biblioteca estática para el proyecto radmovie
add_library(radmovie STATIC ${RADMOVIE_SOURCES})

# Directorios de inclusión
target_include_directories(radmovie
    PUBLIC "inc/"
    PRIVATE "src/pch/"
    PRIVATE ${OPENAL_INCLUDE_DIR}  # OpenAL para el proyecto
)

# Librerías a vincular para radmovie
target_link_libraries(radmovie
    PRIVATE
        p3d
        radcore
        radsound
)

# Agregar OpenAL para Android si se necesita
if(ANDROID)
    target_link_libraries(radmovie PRIVATE OpenAL)
	
endif()
# ✅ Linkear FFmpeg SOLO si NO usas Bink y estás en Android
# ✅ Android + FFmpeg: asegurar includes y libs
# ✅ Android + FFmpeg (Prefab): localizar headers y libs
if(ANDROID AND NOT SRR2_USE_BINK)
    find_package(ffmpeg CONFIG REQUIRED)

    # headers
    target_link_libraries(radmovie PRIVATE ffmpeg::headers)

    # libs (elige .so o .a según prefieras)
    target_link_libraries(radmovie PRIVATE
        ffmpeg::libavcodec.so
        ffmpeg::libavformat.so
        ffmpeg::libavutil.so
        ffmpeg::libswresample.so
        ffmpeg::libswscale.so
    )
endif()




# Agregar un ejecutable de prueba si es necesario
if(SRR2_BUILD_TESTS)
    add_executable(simplemovie "sample/src/main.cpp" "sample/src/util.cpp"
        "${CMAKE_SOURCE_DIR}/code/main/tuidunaligned.cpp")

if(ANDROID)
        find_library(log-lib log)# Este if tambien lo tenido que poner para usar LOGI en Memorymanager.cpp
    endif()
    target_link_libraries(simplemovie
        radmovie
        ${SDL_LIBRARY}
        ${OPENAL_LIBRARY}  # Vincular OpenAL para pruebas
        p3d
        pddi
        radcore
        radsound
		android  #He puesto esto ahora para solucionar "un error"
		${log-lib}# Esto he puesto nuevo para el LOGI en Memorymanager.cpp
	)
endif()
