set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -Werror=return-type -O3 -g3 -march=native -DNDEBUG")

add_executable(emu
    gpu/video.cpp
    gpu/video.h
    gpu/lcdc.h
    gpu/lcdstatus.h
    gpu/palette.cpp
    gpu/palette.h
    gpu/spritestable.h
    gpu/spritestable.cpp
    gpu/renderzone.h
    gpu/renderzone.cpp

    apu/sound.h
    apu/osc.h
    apu/osc.cpp
    apu/envelope.h
    apu/envelope.cpp
    apu/wavereader.h
    apu/wavereader.cpp
    apu/waveoutput.h
    apu/sweep.h

    addressable.h
    addressbus.cpp
    addressbus.h
    cartridge.cpp
    cartridge.h
    main.cpp
    z80.cpp
    z80_no_instr.cpp
    z80.h
    utils.h
    )

target_link_libraries(emu SDL2)

target_include_directories(emu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
