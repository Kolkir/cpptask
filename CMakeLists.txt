cmake_minimum_required(VERSION 2.8)
project(cpptask)

include_directories("${PROJECT_SOURCE_DIR}/3rdparty/gtest/include/")
include_directories("${PROJECT_SOURCE_DIR}/3rdparty/gtest/")
include_directories("${PROJECT_SOURCE_DIR}/include/")

if(MSVC)
    set(CMAKE_CXX_FLAGS "/W4 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1 /MP2")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG /MP")
    set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG /MP")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG /MP")
else()
    SET (CMAKE_C_FLAGS                "-Wall -std=c99")
    SET (CMAKE_C_FLAGS_DEBUG          "-g")
    SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
    SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

    SET (CMAKE_CXX_FLAGS                "-Wall -std=c++11 -pthread -fPIE -fpie")
#-fsanitize=thread
    SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
    SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELEASE        "-O2 -DNDEBUG")
    SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif()

file(GLOB SRC_FILES include/cpptask/*
    "*.h"
)

if(MSVC)
    file(GLOB SRC_PLATFORM_FILES include/cpptask/Win/*
        "*.h"
    )
else()
    file(GLOB SRC_PLATFORM_FILES include/cpptask/Unix/*
        "*.h"
    )
endif()


file(GLOB SRC_TESTS_FILES tests/*
    "*.h"
    "*.cpp"
)

set(GTEST_SOURCES
    ${PROJECT_SOURCE_DIR}/3rdparty/gtest/src/gtest-all.cc
    )
    
add_library(gtest STATIC ${GTEST_SOURCES})

add_executable(tests ${SRC_FILES} ${SRC_PLATFORM_FILES} ${SRC_TESTS_FILES})
target_link_libraries (tests gtest)
