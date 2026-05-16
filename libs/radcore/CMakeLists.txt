set(RADCORE_SOURCES
	#"src/pch/pch.cpp"
	#"src/rad1394/win32lib/rad1394master.cpp"
	#"src/radcontroller/directinputcontroller.cpp"
	"src/radcontroller/sdlcontroller.cpp"
	#"src/raddebugcommunication/host1394socket.cpp"
	#"src/raddebugcommunication/hostchannel.cpp"
	#"src/raddebugcommunication/host.cpp"
	#"src/raddebugcommunication/hosthiosocket.cpp"
	#"src/raddebugcommunication/targetconnection.cpp"
	#"src/raddebugcommunication/targetsocketchannel.cpp"
	"src/raddebugcommunication/targetx.cpp"
	"src/raddebugconsole/consoleclient.cpp"
	"src/raddebug/debug.cpp"
	#"src/raddebugfileserver/fileserver.cpp"
	#"src/raddebugwatch/watchclient.cpp"
	"src/raddispatch/dispatcher.cpp"
	"src/radfile/common/cementer.cpp"
	"src/radfile/common/drive.cpp"
	"src/radfile/common/drivethread.cpp"
	"src/radfile/common/filecache.cpp"
	"src/radfile/common/file.cpp"
	"src/radfile/common/filesystem.cpp"
	"src/radfile/common/instancedrive.cpp"
	"src/radfile/common/platformdrives.cpp"
	"src/radfile/common/radfile.cpp"
	"src/radfile/common/remotedrive.cpp"
	"src/radfile/common/requests.cpp"
	"src/radkey/radkey.cpp"
	"src/radmemory/align.cpp"
	"src/radmemory/binallocator.cpp"
	"src/radmemory/dlheap.cpp"
	"src/radmemory/externalmemoryheap.cpp"
	"src/radmemory/externalmemoryobject.cpp"
	"src/radmemory/memorymanager.cpp"
	"src/radmemory/memoryobject.cpp"
	"src/radmemory/memorypool.cpp"
	"src/radmemory/memoryspacewin32.cpp"
	"src/radmemorymonitor/memmonitorclient.cpp"
	"src/radmemorymonitor/memmonitorclienthelp.cpp"
	"src/radmemory/platalloc.cpp"
	"src/radmemory/staticheap.cpp"
	"src/radmemory/trackingheap.cpp"
	"src/radmemory/twowayallocator.cpp"
	"src/radobjectbtree/objectavltree.cpp"
	"src/radobjectbtree/objectbtree.cpp"
	"src/radobjectlist/dynamicarray.cpp"
	"src/radobjectlist/objectlist.cpp"
	"src/radobjectlist/weakcallbackwrapper.cpp"
	"src/radobjectlist/weakinterfacewrapper.cpp"
	"src/radobject/object.cpp"
	"src/radplatform/platform.cpp"
	"src/radprofiler/profiler2.cpp"
	"src/radprofiler/profiler.cpp"
	"src/radprofiler/profilesample.cpp"
	"src/radremotecommand/functionlist.cpp"
	"src/radremotecommand/remotecommandserver.cpp"
	"src/radremotecommand/remotecommandtarget.cpp"
	"src/radstacktrace/win32/stacktrace.cpp"
	"src/radstats/simplestat.cpp"
	"src/radstats/statmanager.cpp"
	"src/radstring/string.cpp"
	"src/radtextdisplay/textdisplay.cpp"
	"src/radthread/mutex.cpp"
	"src/radthread/semaphore.cpp"
	"src/radthread/system.cpp"
	"src/radthread/thread.cpp"
	"src/radtime/stopwatch.cpp"
	"src/radtime/time.cpp"
)

# esto es la carga de los assets de la version de pc, aqui tendre luego que añadir un archivo
# o modificar el sdldrive.cpp para android 
if(VITA)
	list(APPEND RADCORE_SOURCES	"src/radfile/vita/vitadrive.cpp")
else()
	list(APPEND RADCORE_SOURCES	"src/radfile/sdl/sdldrive.cpp")
endif()
if(SRR2_MICROPROFILE)
	list(APPEND RADCORE_SOURCES	"src/radprofiler/microprofile.cpp")
endif()

#if(ANDROID)
	#list(REMOVE_ITEM RADCORE_SOURCES
	#	"src/radmemory/dlheap.cpp"
	#	"src/radmemory/platalloc.cpp"
	#)
	#message(STATUS "Android: Excluded Windows-specific memory allocators")
#endif()

add_library(radcore STATIC ${RADCORE_SOURCES})
target_include_directories(radcore
	PUBLIC "inc/"
	PRIVATE "src/pch/" # TODO
	PUBLIC "${CMAKE_SOURCE_DIR}/code/" # wtf (radobject.hpp includes memory/classsizetracker.h)
)
target_link_libraries(radcore ${SDL_LIBRARY})
