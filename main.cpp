/*
** main.cpp for gameboy
**
** Made by Guillaume "Vermeille" Sanchez
** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
**
** Started on  ven. 27 avril 2012 12:15:42 CEST Guillaume "Vermeille" Sanchez
** Last update 2014-02-19 18:35 vermeille
*/

#include <iostream>
#include <cstdlib>
#include <signal.h>

#include "addressbus.h"
#include "cartridge.h"
#include "z80.h"
#include "video.h"

static Z80* z80_ptr;

void segv_handler(int)
{
    z80_ptr->Dump();
    exit(1);
}

int main(int argc, char** argv)
{
    if (argc <= 1)
        return EXIT_FAILURE;

    Video v;
    Cartridge card(argv[1]);
    AddressBus addrbus(card, v);
    Z80 processor(addrbus, v);
    z80_ptr = &processor;
    struct sigaction action;
    action.sa_handler = segv_handler;
    sigaction(SIGSEGV, &action, nullptr);
    processor.Process();
    return 0;
}
