# Gamulator

This is a gameboy emulator. Well, this is *intended* to be a gameboy emulator.
For now, it's just not working.

- The screen is kinda half emulated but display isn't working except on test
  ROMs
- There's no sound
- No joypad

I test it with Tetris.

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
