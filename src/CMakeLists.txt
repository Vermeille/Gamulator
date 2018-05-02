set(SRC
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

    addressbus.cpp
    addressbus.h
    cartridge.cpp
    cartridge.h
    main.cpp
    z80.cpp
    z80_no_instr.cpp
    z80.h
    utils.h
    sdl.h
    )

if ("${MAKE_JS}" STREQUAL "yes")
    add_executable(emujs.html ${SRC})
    target_include_directories(emujs.html PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(emujs.html PROPERTIES COMPILE_FLAGS "-std=c++14 -Wall -Wextra -Werror=return-type -O3 -DNDEBUG -s USE_WEBGL2=0 -s FORCE_FILESYSTEM=1 -s INVOKE_RUN=0")
    set_target_properties(emujs.html PROPERTIES LINK_FLAGS " -s USE_SDL=2 -s USE_WEBGL2=0 -s FORCE_FILESYSTEM=1 -s INVOKE_RUN=0")
else()
    add_executable(emu ${SRC})
    target_link_libraries(emu SDL2)
    target_include_directories(emu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(emu PROPERTIES COMPILE_FLAGS "-std=c++14 -Wall -Wextra -Werror=return-type -O3 -g3 -march=native")
endif()

