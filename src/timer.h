#pragma once

#include "utils.h"

class Timer {
   public:
    Timer()
        : cnt_(uint8_t(0)),
          cycle_(kCpuFreq / 16384),
          div_cycle_cnt_(0),
          tima_cycle_cnt_(0),
          tima_(uint8_t(0)),
          tma_(uint8_t(0)) {}

    void Clock() {
        int_ = false;
        ++div_cycle_cnt_;
        if (div_cycle_cnt_ == cycle_) {
            ++cnt_.u;
            div_cycle_cnt_ = 0;
        }

        if ((tac_.u & 0b100) == 0) {
            return;
        }

        ++tima_cycle_cnt_;
        if (tima_cycle_cnt_ >= TimerFreq()) {
            tima_cycle_cnt_ = 0;
            ++tima_.u;
            if (tima_.u == 0) {
                tima_.u = tma_.u;
                int_ = true;
            }
        }
    }

    void Reset() {
        cnt_.u = 0;
        tima_cycle_cnt_ = 1;
    }
    Data8 div() const { return cnt_; }

    Data8 tima() const { return tima_; }
    void set_tima(Data8 tima) { tima_ = tima; }

    Data8 tma() const { return tma_; }
    void set_tma(Data8 tma) { tma_ = tma; }

    Data8 tac() const { return tac_; }
    void set_tac(Data8 tac) { tac_ = tac; }

    bool tima_int() const { return int_; }

   private:
    int TimerFreq() const {
        switch (tac_.u & 0b11) {
            case 0:
                return 1024;
            case 1:
                return 16;
            case 2:
                return 64;
            case 3:
                return 256;
        }
        assert(false);
        return 0;
    }

    Data8 cnt_;
    const int cycle_;
    int div_cycle_cnt_;
    int tima_cycle_cnt_;
    Data8 tima_;
    Data8 tma_;
    Data8 tac_;
    bool int_;
};
