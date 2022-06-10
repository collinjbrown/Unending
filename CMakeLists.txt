# CMakeList.txt : CMake project for Unending, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (unending LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BASE_SRCS
    "src/external/stb_image.cpp"
    "src/external/stb_image.h"
    "src/game.cpp"
    "src/game.h"
    "src/main.cpp"
    "src/renderer.cpp"
    "src/renderer.h"
    "src/shader.cpp"
    "src/shader.h"
    "src/texture.cpp"
    "src/texture.h"
    "src/util.cpp"
    "src/util.h"
    )

# Add source to this project's executable.
add_executable (unending ${BASE_SRCS})

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy-assets.cmake
)

add_dependencies(unending copy_assets)

add_subdirectory(libs/glfw-3.3.7)
add_subdirectory(libs/glad)
add_subdirectory(libs/glm-0.9.9.8)
add_subdirectory(libs/freetype-2.12.1)

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
     set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

target_link_libraries(unending glfw glad glm freetype)
