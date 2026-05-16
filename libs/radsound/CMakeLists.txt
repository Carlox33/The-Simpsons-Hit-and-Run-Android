# Fuentes comunes (válidas para todas las plataformas)
set(RADSOUND_SOURCES
	"src/common/radsoundobject.cpp"
	"src/common/radsoundupdatableobject.cpp"
	"src/hal/common/audioformat.cpp"
	"src/hal/common/banner.cpp"
	"src/hal/common/memoryregion.cpp"
	"src/hal/common/radsoundfile.cpp"
	"src/hal/common/rolloff.cpp"
	"src/math/radsoundmath.cpp"
	"src/pch/pch.cpp"
	"src/radsound/buffereddatasource.cpp"
	"src/radsound/clip.cpp"
	"src/radsound/clipplayer.cpp"
	"src/radsound/datacache.cpp"
	"src/radsound/memorydatasource.cpp"
	"src/radsound/memoryspaceobject.cpp"
	"src/radsound/radicaladpcm.cpp"
	"src/radsound/rsdfiledatasource.cpp"
	"src/radsound/stitcheddatasource.cpp"
	"src/radsound/streamplayer.cpp"
)

# Backend Win32 SOLO en Windows desktop
if(WIN32 OR ANDROID)
	list(APPEND RADSOUND_SOURCES
		"src/hal/win32/buffer.cpp"
		"src/hal/win32/bufferloader.cpp"
		"src/hal/win32/effect.cpp"
		"src/hal/win32/listener.cpp"
		"src/hal/win32/positionalgroup.cpp"
		"src/hal/win32/radsoundwin.cpp"
		"src/hal/win32/system.cpp"
		"src/hal/win32/voice.cpp"
	)
endif()

add_library(radsound STATIC ${RADSOUND_SOURCES})

target_include_directories(radsound
	PUBLIC "inc/"
	PRIVATE "src/pch/"
	PRIVATE "src/common/"
)

# radcore siempre
target_link_libraries(radsound PUBLIC radcore)

# OpenAL: en Android linkea contra openal-soft (target OpenAL) o contra find_package(OpenAL)
if(ANDROID)
	# si añadiste openal-soft con add_subdirectory, normalmente el target es "OpenAL"
	target_link_libraries(radsound PUBLIC OpenAL)
else()
	target_include_directories(radsound PRIVATE ${OPENAL_INCLUDE_DIR})
	target_link_libraries(radsound PUBLIC ${OPENAL_LIBRARY})
endif()


if(SRR2_BUILD_TESTS AND NOT ANDROID)

	add_executable(simplesound "sample/simplesound/src/main.cpp")
	target_link_libraries(simplesound
		radsound
		Threads::Threads
	)
	file(COPY "sample/simplesound/bin/win/sample.rsd"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
	file(COPY "sample/simplesound/bin/win/stream.rsd"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
	if(VITA)
		vita_create_self(simplesound.self simplesound)
	endif()
endif()






#  OpenAL fuera de Android (en Android no se usa por ahora)
if(NOT ANDROID)
	target_include_directories(radsound PRIVATE ${OPENAL_INCLUDE_DIR})
	target_link_libraries(radsound
	radcore
)
endif()

target_link_libraries(radsound PUBLIC radcore)

#  OpenAL fuera de Android (en Android no se linkea por ahora)
if(NOT ANDROID)
	target_link_libraries(radsound ${OPENAL_LIBRARY})
endif()

