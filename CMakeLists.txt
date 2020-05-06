cmake_minimum_required(VERSION 3.7)

set(PROJECT_NAME "puyo")
if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
endif()
project(${PROJECT_NAME})

set(SOURCE_FILES
  "src/main.c"
  "src/spritesheet_png.c"
  "src/spritesheet.c"
  "src/puyo.c"
  "src/gameboard.c"
  "src/runner.c"
)
set(HEADER_FILES
  "include/main.h"
  "include/spritesheet_png.h"
  "include/spritesheet.h"
  "include/puyo.h"
  "include/gameboard.h"
  "include/runner.h"
)

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)
# If using an external version of ACGL
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(ACGL REQUIRED acgl)
add_subdirectory(ACGL)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} PRIVATE acgl SDL2::SDL2main SDL2::SDL2 SDL2::SDL2_image)
if (WIN32 AND (NOT MSVC))
    configure_file(ACGL/libacgl.dll libacgl.dll COPYONLY)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ACGL/include/")
