add_library(deps STATIC "include/lm/lm.h" "src/lm/lm.cpp" "include/Logger.hpp" "include/Events.hpp" "include/Types.h" "src/Types.cpp")

target_include_directories(deps PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")