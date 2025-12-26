add_library(${CMAKE_PROJECT_NAME} SHARED ""
        DataStructs/Globals.h
        InfiniteRandomizerFrameworkNativeStateManager.cpp
        InfiniteRandomizerFrameworkNative.h
        DataStructs/StreamingSectorNodeBuffer.h
        DataStructs/Replacements.h
        RedLogger.cpp
        RedLogger.h
        InfiniteRandomizerFrameworkNativeSectorMod.cpp
        DataStructs/Category.h
        DataStructs/VariantPool.h
        FastRNG.cpp
        FastRNG.h
        main.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RED4ext::SDK)

set(DEST_DIR "E:/Games/Cyberpunk 2077/red4ext/plugins/InfiniteRandomizerFramework")

add_custom_command(
        TARGET ${CMAKE_PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
        "${DEST_DIR}"
)