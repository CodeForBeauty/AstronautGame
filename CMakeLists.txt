cmake_minimum_required (VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Astronaut")

add_executable (app "app/main.cpp" "app/MyApp.h")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app)

if (WIN32)
    set_target_properties(app
        PROPERTIES
            LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
            LINK_FLAGS_RELEASE "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
            LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
            LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
        )
else()
    target_link_options(app PRIVATE "$<IF:$<NOT:$<CONFIG:Debug>>,-mwindows,>")
endif()

set(GLENDER_APP_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/")
set(AUDIO_AL_DIR "${CMAKE_CURRENT_BINARY_DIR}/")

add_subdirectory (${CMAKE_SOURCE_DIR}/Dependencies ${CMAKE_BINARY_DIR}/deps)
target_link_libraries (app deps)

add_subdirectory (${CMAKE_SOURCE_DIR}/glender ${CMAKE_BINARY_DIR}/glender)
target_link_libraries (app glender)

add_subdirectory (${CMAKE_SOURCE_DIR}/physixx ${CMAKE_BINARY_DIR}/physixx)
target_link_libraries (app physixx)

add_subdirectory (${CMAKE_SOURCE_DIR}/audio ${CMAKE_BINARY_DIR}/audio)
target_link_libraries (app audio)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET app PROPERTY CXX_STANDARD 20)
endif()
