/*
** main.cpp for gameboy
**
** Made by Guillaume "Vermeille" Sanchez
** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
**
** Started on  ven. 27 avril 2012 12:15:42 CEST Guillaume "Vermeille" Sanchez
** Last update lun. 30 avril 2012 03:24:54 CEST Guillaume "Vermeille" Sanchez
*/

#include <iostream>
#include <cstdlib>

#include "addressbus.h"
#include "cartridge.h"
#include "z80.h"

int main(int argc, char** argv)
{
    if (argc <= 1)
        return EXIT_FAILURE;

    Video v;
    Cartridge card(argv[1]);
    AddressBus addrbus(card, v);
    Z80 processor(addrbus, v);
    processor.Process();
    return 0;
}
