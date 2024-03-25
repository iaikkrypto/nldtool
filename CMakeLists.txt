cmake_minimum_required(VERSION 3.5)
project(nldproject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

# set the default build type for single-config generators if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS None Debug Release RelWithDebInfo MinSizeRel)
endif()

# add external interface library cxxopts
set(CXXOPTS_FILES ${CMAKE_SOURCE_DIR}/cxxopts/include/cxxopts.hpp)
add_library(cxxopts INTERFACE)
target_sources(cxxopts INTERFACE ${CXXOPTS_FILES})
target_include_directories(cxxopts SYSTEM INTERFACE cxxopts/include)

# add external library tinyxml2
set(TINYXML_FILES tinyxml2/tinyxml2.h tinyxml2/tinyxml2.cpp)
add_library(tinyxml2 ${TINYXML_FILES})
target_include_directories(tinyxml2 PUBLIC tinyxml2)

# add verbose warnings for all subsequent targets (nldcore and nldtool)
if(MSVC)
  add_compile_options(/W4 /wd4018 /wd4389 /wd4100)
else()
  add_compile_options(-Wall -Wextra -pedantic -fmessage-length=0
    -Wno-sign-compare -Wno-unused-parameter -Wno-missing-braces)
endif()

# add library nldcore
include(src/sources.cmake)
add_library(nldcore ${NLDCORE_FILES})
target_include_directories(nldcore PUBLIC src examples)
target_link_libraries(nldcore PUBLIC tinyxml2 cxxopts)

# add executable nldtool
include(examples/sources.cmake)
add_executable(nldtool ${NLDTOOL_FILES})
target_include_directories(nldtool PRIVATE examples)
target_link_libraries(nldtool PRIVATE nldcore)
