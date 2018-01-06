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
    ~Cartridge() {
        if (_has_battery) {
            _ctrl->SaveRam(_game_name + ".save");
        }
    }

    struct Addr {
        std::string _name;
        int _begin;
        int _end;
        std::function<byte(uint16_t)> _get;
        std::function<void(uint16_t, byte)> _set;
    };

    class Controller {
       public:
        Controller(std::vector<byte>&& data, int ram_size)
            : _data(std::move(data)), _ram(ram_size) {}

        virtual ~Controller() = default;

        const Addr& Find(uint16_t idx) const { return FindAddr(idx); }
        byte Read(uint16_t idx) const { return FindAddr(idx)._get(idx); }

        void Write(uint16_t idx, byte v) { FindAddr(idx)._set(idx, v); }
        void LoadRam(const std::string& filename);
        void SaveRam(const std::string& filename);
        int rom_size() const { return _data.size(); }
        int rom_banks() const { return rom_size() / 0x4000; }

       protected:
        byte& Rom(uint32_t idx) { return _data[idx]; }
        byte Rom(uint32_t idx) const { return _data[idx]; }

        byte& Ram(uint32_t idx) { return _ram[idx]; }
        byte Ram(uint32_t idx) const { return _ram[idx]; }

        std::vector<Addr> _mem_map;

       private:
        const Addr& FindAddr(uint16_t) const;

        std::vector<byte> _data;
        std::vector<byte> _ram;
    };

    const Addr& Find(uint16_t index) const { return _ctrl->Find(index); }
    byte Read(uint16_t index) const { return _ctrl->Read(index); }
    void Write(uint16_t index, byte val) { _ctrl->Write(index, val); }

   private:
    std::vector<byte> LoadGame(const std::string& filename);
    std::unique_ptr<Controller> _ctrl;
    std::string _game_name;
    bool _has_battery;
};
