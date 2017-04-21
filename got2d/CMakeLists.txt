set(GOT2D_INCLUDE_FILES
	include/g2dconfig.h
	include/g2dengine.h
	include/g2dinput.h
	include/g2dmessage.h
	include/g2drender.h
	include/g2dscene.h
)

set(GOT2D_SOURCE_FILES
	source/engine.h
	source/engine.cpp
	source/inner_utility.h
	source/scope_utility.h
)

set(GOT2D_SOURCE_INPUT_FILES
	source/input.h
	source/input.cpp
	source/message.cpp
)

if(MSVC)
list(APPEND GOT2D_SOURCE_INPUT_FILES 
source/message_win32.cpp
)
endif(MSVC)

set(GOT2D_SOURCE_RENDER_FILES
	source/render_system.h
	source/render_system.cpp
	source/mesh.cpp
	source/shader.cpp
	source/texture.cpp
)

set(GOT2D_SOURCE_SCENE_FILES
	source/scene.h
	source/scene.cpp
	source/scene_node.cpp
	source/container.cpp
	source/transform.cpp
	source/spatial_graph.h
	source/spatial_graph.cpp	
	source/component.h
	source/component.cpp
)

set(GOT2D_RHI_INCLUDE_FILES
	RHI/RHI.h
)

if(MSVC)
list(APPEND GOT2D_RHI_INCLUDE_FILES 
	RHI/dx11/inner_RHI.h
	RHI/dx11/dx11_enum.h
	RHI/dx11/RHI.cpp
	RHI/dx11/rhi_device.cpp
	RHI/dx11/rhi_context.cpp
	RHI/dx11/rhi_swapchain.cpp
	RHI/dx11/rhi_shader.cpp
)
source_group(RHI_DX11 FILES ${GOT2D_RHI_INCLUDE_FILES})
else()
source_group(RHI FILES ${GOT2D_RHI_INCLUDE_FILES})
endif(MSVC)

source_group(include FILES ${GOT2D_INCLUDE_FILES})
source_group(source FILES ${GOT2D_SOURCE_FILES})
source_group(source\\input FILES ${GOT2D_SOURCE_INPUT_FILES})
source_group(source\\render FILES ${GOT2D_SOURCE_RENDER_FILES})
source_group(source\\scene FILES ${GOT2D_SOURCE_SCENE_FILES})

list(APPEND GOT2D_SOURCE_FILES 
	${GOT2D_SOURCE_INPUT_FILES}
	${GOT2D_SOURCE_RENDER_FILES}
	${GOT2D_SOURCE_SCENE_FILES}
	${GOT2D_RHI_INCLUDE_FILES}
)

include_directories(got2d ../extern/res)
add_library(got2d SHARED ${GOT2D_INCLUDE_FILES} ${GOT2D_SOURCE_FILES})
add_dependencies(got2d gml)
set_target_properties(got2d PROPERTIES COMPILE_DEFINITIONS GOT2D_EXPORTS)
target_include_directories(got2d PRIVATE ${GML_STAGE_DIR}/include)
target_link_libraries(got2d res)
if(MSVC)
target_link_libraries(got2d d3d11.lib dxgi.lib d3dcompiler.lib)
endif(MSVC)
source_group(gml REGULAR_EXPRESSION "gml*")