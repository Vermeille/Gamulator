# Gamulator

This emulator, except for some quircks, _kinda_ works.

- Kirby Dream Land works fine (the game can be finished)
- Castlevania II seems to be okay except for some mysterious sprite flickering
- The Legend of Zelda: Link's Awakening seems fine
- Pokémon blue freezes when you go to a Pokémon Center for some unknown reason

Tests:

- Blargg's `cpu_instr`, `instr_timing`, `int_time` are okay
- Mooneye-gb's MBC1 ram/rom, are okay

# Debug it

When launched with `--show-instr`  the emulator generates a trace. This trace
is particularly hard to read as a text file, so I wrote reverse engineering
tools.

## Code Rebuilder

This takes the trace as an input, a memory symbols table file, and follows
`call`s and `ret`s to find code and bundle it as functions blocks.

## Code debugger

This intends to mimick a gdb interface to read the trace. You don't have memory
/ registers inspection for obvious reasons, but can put breakpoint and all
those navigation stuff that help you to make sense of the code in a dynamic way
