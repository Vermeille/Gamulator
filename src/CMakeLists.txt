set(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -O3 -D_GLIBCXX_DEBUG")

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
    utils.h
    lcdc.h
    lcdstatus.h
    palette.cpp
    palette.h
    spritestable.h
    spritestable.cpp
    sound.h
    osc.h
    osc.cpp
    envelope.h
    envelope.cpp
    wavereader.h
    wavereader.cpp
    waveoutput.h
    sweep.h
    )

target_link_libraries(emu sfml-window sfml-system sfml-graphics sfml-audio)
