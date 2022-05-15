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

# Create main executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME} PRIVATE "include/")

if (WIN32)
  # Need to get provided path to SDL2
  set(SDL2_PATH "D:/Documents/Windows/SDL2-2.0.22")
  set(SDL2_LIB_PATH "${SDL2_PATH}/lib/x64")
  set(SDL2_INC_PATH "${SDL2_PATH}/include")
  
  add_library(SDL2::SDL2 SHARED IMPORTED)
  set_property(TARGET SDL2::SDL2 PROPERTY IMPORTED_IMPLIB "${SDL2_LIB_PATH}/SDL2.lib")
  set_property(TARGET SDL2::SDL2 PROPERTY IMPORTED_LOCATION "${SDL2_LIB_PATH}/SDL2.dll")
  configure_file("${SDL2_LIB_PATH}/SDL2.dll" "SDL2.dll" COPYONLY)
  target_include_directories(SDL2::SDL2 SYSTEM INTERFACE ${SDL2_INC_PATH})
  
  add_library(SDL2::SDL2main SHARED IMPORTED)
  set_property(TARGET SDL2::SDL2main PROPERTY IMPORTED_IMPLIB "${SDL2_LIB_PATH}/SDL2main.lib")
  
  # Need to get provided path to SDL2_image
  set(SDL2_IMAGE_PATH "D:/Documents/Windows/SDL2_image-2.0.5")
  set(SDL2_IMAGE_LIB_PATH "${SDL2_IMAGE_PATH}/lib/x64")
  set(SDL2_IMAGE_INC_PATH "${SDL2_IMAGE_PATH}/include")
  
  add_library(SDL2_image SHARED IMPORTED)
  set_property(TARGET SDL2_image PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/SDL2_image.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/SDL2_image.dll" "SDL2_image.dll" COPYONLY)
  
  add_library(SDL2_image::jpeg SHARED IMPORTED)
  set_property(TARGET SDL2_image::jpeg PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image::jpeg PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/libjpeg-9.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/libjpeg-9.dll" "libjpeg-9.dll" COPYONLY)
  
  add_library(SDL2_image::png SHARED IMPORTED)
  set_property(TARGET SDL2_image::png PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image::png PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/libpng16-16.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/libpng16-16.dll" "libpng16-16.dll" COPYONLY)
  
  add_library(SDL2_image::tiff SHARED IMPORTED)
  set_property(TARGET SDL2_image::tiff PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image::tiff PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/libtiff-5.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/libtiff-5.dll" "libtiff-5.dll" COPYONLY)
  
  add_library(SDL2_image::webp SHARED IMPORTED)
  set_property(TARGET SDL2_image::webp PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image::webp PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/libwebp-7.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/libwebp-7.dll" "libwebp-7.dll" COPYONLY)

  add_library(SDL2_image::zlib SHARED IMPORTED)
  set_property(TARGET SDL2_image::zlib PROPERTY IMPORTED_IMPLIB "${SDL2_IMAGE_LIB_PATH}/SDL2_image.lib")
  set_property(TARGET SDL2_image::zlib PROPERTY IMPORTED_LOCATION "${SDL2_IMAGE_LIB_PATH}/zlib1.dll")
  configure_file("${SDL2_IMAGE_LIB_PATH}/zlib1.dll" "zlib1.dll" COPYONLY)  

  target_include_directories(SDL2_image SYSTEM INTERFACE ${SDL2_IMAGE_INC_PATH})
  target_link_libraries(SDL2_image INTERFACE SDL2_image::jpeg SDL2_image::png SDL2_image::tiff SDL2_image::webp SDL2_image::zlib)
  
  set(SDL2_FOUND true)
else ()
  find_package(SDL2 CONFIG REQUIRED)
endif ()

# Add ACGL library as subdirectory
add_subdirectory(ACGL)
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC "ACGL/include/")

# Link against math, ACGL, and SDL2
target_link_libraries(${PROJECT_NAME} PRIVATE acgl SDL2::SDL2main SDL2::SDL2 SDL2_image)
if (WIN32 AND (NOT MSVC))
    configure_file(ACGL/libacgl.dll libacgl.dll COPYONLY)
endif()
