#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "addressable.h"
#include "utils.h"

class Cartridge {
   public:
    Cartridge(std::string filename);

    struct Addr {
        std::string _name;
        int _begin;
        int _end;
        std::function<byte(uint16_t)> _get;
        std::function<void(uint16_t, byte)> _set;
    };

    class Controller {
       public:
        Controller(std::vector<byte>&& data) : _data(std::move(data)) {}

        byte Read(uint16_t idx) const { return FindAddr(idx)._get(idx); }

        void Write(uint16_t idx, byte v) { FindAddr(idx)._set(idx, v); }

       protected:
        byte ReadRom(uint16_t idx) const { return _data[idx]; }
        void WriteRom(uint16_t idx, byte x) { _data[idx] = x; }

        std::vector<Addr> _mem_map;

       private:
        const Addr& FindAddr(uint16_t) const;

        std::vector<byte> _data;
    };

    byte Read(uint16_t index) const { return _ctrl->Read(index); }
    void Write(uint16_t index, byte val) { _ctrl->Write(index, val); }

   private:
    std::unique_ptr<Controller> _ctrl;
};
