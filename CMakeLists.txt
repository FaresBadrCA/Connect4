# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("C4AlphaBeta")

add_executable(C4AlphaBeta 
    main.cpp
    Solver.cpp
)

set_property(TARGET C4AlphaBeta PROPERTY CXX_STANDARD 20)

add_custom_target(copy-tests ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Tests
    ${PROJECT_BINARY_DIR}/Tests
   )

add_dependencies(C4AlphaBeta copy-tests)