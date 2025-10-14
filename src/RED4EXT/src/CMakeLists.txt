add_library(${CMAKE_PROJECT_NAME} SHARED ""
        globals.h
        InfiniteRandomizerFrameworkNative.cpp
        InfiniteRandomizerFrameworkNative.h
        StreamingSectorNodeBuffer.h)

set(PROJECT_HEADER_FILES
)

set(PROJECT_SRC_FILES
        main.cpp
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${PROJECT_HEADER_FILES} ${PROJECT_SRC_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE RED4ext::SDK)