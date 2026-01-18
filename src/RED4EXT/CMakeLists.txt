cmake_minimum_required(VERSION 3.14)

project(InfiniteRandomizerFrameworkNative LANGUAGES CXX)

add_compile_definitions(NOMINMAX)
add_subdirectory(vendor/RedLib)
cmake_policy(SET CMP0079 NEW)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_subdirectory(deps/red4ext.sdk)
set_target_properties(RED4ext.SDK PROPERTIES FOLDER "Dependencies")

mark_as_advanced(
        RED4EXT_BUILD_EXAMPLES
        RED4EXT_HEADER_ONLY
)

set(CMAKE_GENERATOR_PLATFORM x64)

add_subdirectory(src)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RedLib)

add_library(RapidJson INTERFACE IMPORTED)
target_include_directories(RapidJson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/RapidJson)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RapidJson)

add_library(XXHash INTERFACE IMPORTED)
target_include_directories(XXHash INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/XXHash)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE XXHash)