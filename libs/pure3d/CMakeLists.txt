set(P3D_SOURCES
	"p3d/ambientlight.cpp"
	"p3d/anim/animate.cpp"
	"p3d/anim/animatedobject.cpp"
	"p3d/anim/billboardobjectanimation.cpp"
	"p3d/anim/cameraanimation.cpp"
	"p3d/anim/channel.cpp"
	"p3d/anim/compositedrawable.cpp"
	"p3d/anim/drawablepose.cpp"
	"p3d/anim/eventanimation.cpp"
	"p3d/anim/event.cpp"
	"p3d/anim/expressionanimation.cpp"
	"p3d/anim/expression.cpp"
	"p3d/anim/expressionoffsets.cpp"
	"p3d/anim/instancedgeometry.cpp"
	"p3d/anim/lightanimation.cpp"
	"p3d/anim/multicontroller.cpp"
	"p3d/anim/polyskin.cpp"
	"p3d/anim/poseanimation.cpp"
	"p3d/anim/pose.cpp"
	"p3d/anim/sequencer.cpp"
	"p3d/anim/shaderanimation.cpp"
	"p3d/anim/skeleton.cpp"
	"p3d/anim/textureanimation.cpp"
	"p3d/anim/vertexanimcontroller.cpp"
	"p3d/anim/vertexanimkey.cpp"
	"p3d/anim/vertexanimobject.cpp"
	"p3d/anim/vertexoffsetexpression.cpp"
	"p3d/anim/visibilityanimation.cpp"
	"p3d/billboardobject.cpp"
	"p3d/bmp.cpp"
	"p3d/camera.cpp"
	"p3d/cameraloader.cpp"
	"p3d/chunkfile.cpp"
	"p3d/context.cpp"
	"p3d/debugdraw.cpp"
	"p3d/directionallight.cpp"
	"p3d/displaylist.cpp"
	"p3d/drawable.cpp"
	"p3d/dxtn.cpp"
	"p3d/effects/effect.cpp"
	"p3d/effects/optic.cpp"
	"p3d/effects/opticlensflare.cpp"
	"p3d/effects/opticloader.cpp"
	"p3d/effects/particlearray.cpp"
	"p3d/effects/particleemitter.cpp"
	"p3d/effects/particlegenerator.cpp"
	"p3d/effects/particleloader.cpp"
	"p3d/effects/particlepool.cpp"
	"p3d/effects/particlesystem.cpp"
	"p3d/effects/particletype.cpp"
	"p3d/effects/particleutility.cpp"
	"p3d/effects/transitionmanager.cpp"
	"p3d/entity.cpp"
	"p3d/error.cpp"
	"p3d/file.cpp"
	"p3d/fileftt.cpp"
	"p3d/font.cpp"
	"p3d/gameattr.cpp"
	"p3d/geometry.cpp"
	"p3d/imageconverter.cpp"
	"p3d/image.cpp"
	"p3d/imagefactory.cpp"
	"p3d/imagefont.cpp"
	"p3d/inventory.cpp"
	"p3d/light.cpp"
	"p3d/lightloader.cpp"
	"p3d/lightschooser.cpp"
	"p3d/loaders.cpp"
	"p3d/loadmanager.cpp"
	"p3d/locator.cpp"
	"p3d/lzr.cpp"
	"p3d/matrixstack.cpp"
	"p3d/memheap.cpp"
	"p3d/memory.cpp"
	"p3d/memorysection.cpp"
	#"p3d/platform/linux/plat_filemap.cpp"
	"p3d/platform/win32/platform.cpp"
	"p3d/png.cpp"
	"p3d/pointcamera.cpp"
	"p3d/pointlight.cpp"
	"p3d/primgroup.cpp"
	"p3d/rawimage.cpp"
	"p3d/refcounted.cpp"
	"p3d/scenegraph/nodeanimation.cpp"
	"p3d/scenegraph/scenegraph.cpp"
	"p3d/shader.cpp"
	"p3d/shadow/shadow_common.cpp"
	"p3d/shadow/shadow_gc.cpp"
	"p3d/shadow/shadow_generic.cpp"
	"p3d/shadow/shadow_ps2.cpp"
	"p3d/spotlight.cpp"
	"p3d/sprite.cpp"
	"p3d/table.cpp"
	"p3d/targa.cpp"
	"p3d/textdataparser.cpp"
	"p3d/textstring.cpp"
	"p3d/texture.cpp"
	"p3d/texturefont.cpp"
	"p3d/unicode.cpp"
	"p3d/utility.cpp"
	"p3d/vectorcamera.cpp"
	"p3d/vertexlist.cpp"
	"p3d/view.cpp"
)
if (WIN32)
	list(APPEND P3D_SOURCES "p3d/platform/win32/plat_filemap.cpp")
endif()

set(PDDI_SOURCES
	"pddi/base/basecontext.cpp"
	"pddi/base/baseshader.cpp"
	"pddi/base/font.cpp"
	"pddi/base/pddiobj.cpp"
)
if(SRR2_P3D_PDDI STREQUAL "GXM")
	list(APPEND PDDI_SOURCES
		"pddi/gxm/context.cpp"
		"pddi/gxm/device.cpp"
		"pddi/gxm/material.cpp"
		"pddi/gxm/texture.cpp"
		"pddi/gxm/program.cpp"
		"pddi/gxm/display.cpp"
		"pddi/gxm/texture_swizzler.cpp"
	)
elseif(SRR2_P3D_PDDI STREQUAL "GLES2")
	list(APPEND PDDI_SOURCES
		"pddi/gles/glcon.cpp"
		"pddi/gles/gldev.cpp"
		"pddi/gles/glmat.cpp"
		"pddi/gles/gltex.cpp"
		"pddi/gles/glprog.cpp"
		"pddi/gles/glad/src/glad.c"
	)
	if(SRR2_USE_VITAGL)
		list(APPEND PDDI_SOURCES "pddi/gles/display_vita/gldisplay.cpp")
	else()
		list(APPEND PDDI_SOURCES
			"pddi/gles/decompress.cpp"
			"pddi/gles/display_win32/gldisplay.cpp"
		)
	endif()
else()
	list(APPEND PDDI_SOURCES
		"pddi/gl/glcon.cpp"
		"pddi/gl/gldev.cpp"
		"pddi/gl/glmat.cpp"
		"pddi/gl/gltex.cpp"
		"pddi/gl/glad/src/glad.c"
	)
	if(SRR2_P3D_PDDI STREQUAL "GLES1")
		list(APPEND PDDI_SOURCES "pddi/gl/decompress.cpp")
	endif()
	if(VITA)
		list(APPEND PDDI_SOURCES "pddi/gl/display_vita/gldisplay.cpp")
	else()
		list(APPEND PDDI_SOURCES "pddi/gl/display_win32/gldisplay.cpp")
	endif()
endif()

set(SRR2_P3D_CG_PATH "C:/Program Files (x86)/NVIDIA Corporation/Cg")
add_library(pddi STATIC ${PDDI_SOURCES})
target_include_directories(pddi PUBLIC "")
if(SRR2_P3D_PDDI STREQUAL GXM)
	target_link_libraries(pddi radcontent)
elseif(SRR2_P3D_PDDI STREQUAL GLES2)
	target_include_directories(pddi PRIVATE "pddi/gles/glad/include/")
else()
	target_include_directories(pddi PRIVATE "pddi/gl/glad/include/")
endif()
if(SRR2_P3D_CG)
	target_include_directories(pddi PRIVATE "${SRR2_P3D_CG_PATH}/include/")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		target_link_directories(pddi PUBLIC "${SRR2_P3D_CG_PATH}/lib.x64/")
	else()
		target_link_directories(pddi PUBLIC "${SRR2_P3D_CG_PATH}/lib/")
	endif()
	target_link_libraries(pddi cg cgGL)
endif()
target_link_libraries(pddi
	${SDL_LIBRARY}
	radmath
	radcore
)

add_library(p3d STATIC ${P3D_SOURCES})
target_include_directories(p3d
	PUBLIC "."
	PRIVATE ".." # wtf
)



if(ANDROID)
    # headers para que encuentre <png.h>
    target_include_directories(p3d PRIVATE ${PNG_INCLUDE_DIR})

    # link para resolver símbolos PNG + zlib
    target_link_libraries(p3d
        ${PNG_LIBRARY}
        ${ZLIB_LIB}
        pddi
        radcore
        radcontent
        radmath
    )

    message(STATUS "Android: PNG linked for p3d (libpng + zlib)")
else()
    target_link_libraries(p3d
        PNG::PNG
        pddi
        radcore
        radcontent
        radmath
    )
    message(STATUS "Desktop: PNG linked for p3d")
endif()

