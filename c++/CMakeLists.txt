cmake_minimum_required(VERSION 4.2.3)

project(cpp_experiments)

set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CXX_SRC_EXT .cpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
add_compile_options(
    -Werror -Wall -Wextra -pedantic
    -Wconversion -Wsign-conversion
    -Wswitch-default -Wswitch-enum
    -Wno-mismatched-new-delete
    -Wimplicit-fallthrough=5
    -Wno-sized-deallocation
)

file(GLOB SOURCES RELATIVE ${CMAKE_SOURCE_DIR} *${CXX_SRC_EXT})
message("SOURCES=${SOURCES}")
foreach(SRC ${SOURCES})
    string(REGEX REPLACE "${CXX_SRC_EXT}$" "" EXE ${SRC})
    if(EXE STREQUAL new_delete)
    else()
        add_executable(${EXE} ${SRC})
        if(EXE STREQUAL std_any)
            target_sources(${EXE} PRIVATE new_delete.cpp)
        endif()
        message("Added executable: ${EXE}")
    endif()
endforeach()
