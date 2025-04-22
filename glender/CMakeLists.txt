cmake_minimum_required (VERSION 3.8)

add_library (glender STATIC  
							"include/Window.h" "src/Window.cpp" 
							"include/App.h" "src/App.cpp" 
							"include/Mesh.h" "src/Mesh.cpp" 
							"include/Material.h" "src/Material.cpp" 
							"include/Shader.h"  "src/Shader.cpp"
							"include/Renderer.h" "src/Renderer.cpp" 
							"include/Scene.h" "src/Scene.cpp" 
							"include/Texture.h" "src/Texture.cpp" 
							"include/Camera.h" "src/Camera.cpp" 
							"include/Entity.h" "src/Entity.cpp" 
							"include/Framebuffer.h" "src/Framebuffer.cpp" 
							"include/Lighting.h" "src/Lighting.cpp" 
							"include/Logging.h" "src/Logging.cpp"
							"include/Text.h" "src/Text.cpp" 
							"include/Skeleton.h" "src/Skeleton.cpp")

target_compile_features(glender PUBLIC cxx_std_23)

target_include_directories (glender SYSTEM BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/include)
target_include_directories (glender SYSTEM BEFORE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
target_link_libraries (glender glfw)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/assimp ${CMAKE_CURRENT_BINARY_DIR}/assimp)
target_link_libraries (glender assimp)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/freetype ${CMAKE_CURRENT_BINARY_DIR}/freetype)
target_link_libraries (glender freetype)

add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/imgui ${CMAKE_CURRENT_BINARY_DIR}/imgui)
target_link_libraries (glender imgui)

target_link_libraries (glender deps)

target_sources (glender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/src/glad/gl.c)


target_sources (glender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/src/stb/stb_image.cpp)

add_custom_target(copy_assets ALL 
					COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_CURRENT_SOURCE_DIR}/shaders
					${GLENDER_APP_DIRECTORY}/shaders
					COMMENT "Copying assets into binary directory.")

add_custom_target(copy_resources ALL 
					COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/Resources
					${GLENDER_APP_DIRECTORY}/Resources
					COMMENT "Copying resources into binary directory.")

add_dependencies(glender copy_assets)
add_dependencies(glender copy_resources)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET glender PROPERTY CXX_STANDARD 23)
endif()