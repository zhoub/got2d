cmake_minimum_required(VERSION 3.8)

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

source_group(include FILES ${GOT2D_INCLUDE_FILES})
source_group(source FILES ${GOT2D_SOURCE_FILES})
source_group(source\\input FILES ${GOT2D_SOURCE_INPUT_FILES})
source_group(source\\render FILES ${GOT2D_SOURCE_RENDER_FILES})
source_group(source\\scene FILES ${GOT2D_SOURCE_SCENE_FILES})

list(APPEND GOT2D_SOURCE_FILES 
	${GOT2D_SOURCE_INPUT_FILES}
	${GOT2D_SOURCE_RENDER_FILES}
	${GOT2D_SOURCE_SCENE_FILES}
)

include_directories(got2d ../extern/res)
add_library(got2d SHARED ${GOT2D_INCLUDE_FILES} ${GOT2D_SOURCE_FILES})
set_target_properties(got2d PROPERTIES COMPILE_DEFINITIONS GOT2D_EXPORTS)
target_link_libraries(got2d res gml)

source_group(gml REGULAR_EXPRESSION "gml*")