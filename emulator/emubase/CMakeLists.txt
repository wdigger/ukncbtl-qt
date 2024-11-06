cmake_minimum_required(VERSION 3.16)

project(emubase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EMUBASE_SOURCES
    Board.cpp
    Disasm.cpp
    Floppy.cpp
    Hard.cpp
    Memory.cpp
    Processor.cpp
    SoundAY.cpp
)

set(EMUBASE_HEADERS
    Board.h
    Defines.h
    Floppy.h
    Emubase.h
    Memory.h
    Processor.h
    Hard.h
)

add_library(emubase STATIC
    ${EMUBASE_SOURCES}
    ${EMUBASE_HEADERS}
)

include_directories(${util_SOURCE_DIR})
