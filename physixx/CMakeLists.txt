add_library (physixx STATIC "include/PhysixxSystem.h" "src/PhysixxSystem.cpp" "include/PhysixxBody.h" "src/PhysixxBody.cpp")

target_include_directories (physixx PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")


target_link_libraries (physixx deps)