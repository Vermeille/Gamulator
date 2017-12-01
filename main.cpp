/*
** main.cpp for gameboy
**
** Made by Guillaume "Vermeille" Sanchez
** Login   sanche_g <Guillaume.V.Sanchez@gmail.com>
**
** Started on  ven. 27 avril 2012 12:15:42 CEST Guillaume "Vermeille" Sanchez
** Last update 2014-02-19 18:35 vermeille
*/

#include <signal.h>
#include <cstdlib>
#include <iostream>

#include "addressbus.h"
#include "cartridge.h"
#include "keypad.h"
#include "video.h"
#include "z80.h"

static Z80* z80_ptr;

Logger cinstr;
Logger cdebug;
Logger cevent;
Logger cerror;
Logger serial;

void segv_handler(int) {
    z80_ptr->Dump();
    exit(1);
}

int main(int argc, char** argv) {
    if (argc <= 1) return EXIT_FAILURE;

    Video v;

    std::string gamefile;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            gamefile = argv[i];
        } else if (argv[i] == std::string("--show-instr")) {
            cinstr.enabled = true;
        } else if (argv[i] == std::string("--show-debug")) {
            cdebug.enabled = true;
        } else if (argv[i] == std::string("--show-event")) {
            cevent.enabled = true;
        } else if (argv[i] == std::string("--serial")) {
            serial.enabled = true;
        } else if (argv[i] == std::string("--errors")) {
            cerror.enabled = true;
        } else if (!gamefile.empty()) {
            std::cerr << "unrecognized option " << argv[i] << "\n";
            return 1;
        }
    }
    Cartridge card(gamefile);
    LinkCable lk;
    Keypad kp;
    Timer timer;
    AddressBus addrbus(card, v, lk, kp, timer);
    Z80 processor(addrbus, v, lk, timer);
    z80_ptr = &processor;
    struct sigaction action;
    action.sa_handler = segv_handler;
    sigaction(SIGSEGV, &action, nullptr);
    processor.Process();
    return 0;
}
