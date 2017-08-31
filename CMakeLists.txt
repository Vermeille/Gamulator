cmake_minimum_required(VERSION 2.6)

project(Gamulator CXX)

set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -ggdb3 -D_GLIBCXX_DEBUG")

add_executable(emu
    addressable.h
    addressbus.cpp
    addressbus.h
    cartridge.cpp
    cartridge.h
    main.cpp
    video.cpp
    video.h
    z80.cpp
    z80.h
    utils.h)

target_link_libraries(emu sfml-window sfml-system sfml-graphics)
