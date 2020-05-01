cmake_minimum_required(VERSION 3.11)
set(PROJECT_NAME "puyo")
#if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG)
#endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/ACGL/cmake/sdl2")
set(PUYO_BUILD_STATIC YES)
if (DEFINED PUYO_BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    if (UNIX)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(CMAKE_EXE_LINKER_FLAGS "-static")
    endif (UNIX)

    #[[ if (MSVC)
        target_compile_options(puyo PUBLIC/PRIVATE /MT)
        target_compile_options(puyo PUBLIC/PRIVATE  /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
    endif (MSVC) ]]
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

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
# If using an external version of ACGL
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(ACGL REQUIRED acgl)
add_subdirectory(ACGL)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} acgl SDL2::Main SDL2::Image)
if ((NOT DEFINED PUYO_BUILD_STATIC) AND WIN32 AND (NOT MSVC))
    configure_file(ACGL/libacgl.dll libacgl.dll COPYONLY)
endif()
target_include_directories(${PROJECT_NAME} PRIVATE "include/")
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ACGL/include/")

# adding the spritesheet inline
# No longer needed, keeping around just because it's cool
#[[
set( RC_DEPENDS "" )

function( add_resource input )
    string( MAKE_C_IDENTIFIER ${input} input_identifier )
    set( output "${CMAKE_BINARY_DIR}/${input_identifier}.o" )
    message(${output})
    target_link_libraries( ${PROJECT_NAME} ${output})

    add_custom_command(
        OUTPUT ${output}
        COMMAND ${CMAKE_LINKER} --relocatable --format binary --output ${output} ${input}
        DEPENDS ${input}
    )

    set( RC_DEPENDS ${RC_DEPENDS} ${output} PARENT_SCOPE )
endfunction()

add_resource("spritesheet.png")
add_custom_target(rc ALL DEPENDS ${RC_DEPENDS})
add_dependencies(${PROJECT_NAME} rc )]]
