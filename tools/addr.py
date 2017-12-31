import bisect
from typing import List


class MemSym:
    def __init__(self, beg: str, end: str, name: str) -> None:
        self.begin = int(beg, 16)
        self.end = int(end, 16) + 1
        self.name = name

    def __repr__(self):
        return hex(self.begin) + '-' + hex(self.end) + ': ' + self.name

    def __eq__(self, m) -> bool:
        if type(m) is str:
            return self.begin == int(m, 16)
        return self.begin == m.begin

    def __lt__(self, m) -> bool:
        if type(m) is str:
            return self.begin < int(m, 16)
        return self.begin < m.begin

    def __le__(self, m) -> bool:
        if type(m) is str:
            return self.begin <= int(m, 16)
        return self.begin <= m.begin

    def __gt__(self, m) -> bool:
        if type(m) is str:
            return self.begin > int(m, 16)
        return self.begin > m.begin

    def __ge__(self, m) -> bool:
        if type(m) is str:
            return self.begin >= int(m, 16)
        return self.begin >= m.begin


class Addr:
    symbols = [] # type: List[MemSym]
    as_ret = [] # type: List[int]

    @classmethod
    def init(cls, path: str):
        io_ports = [
            ('0xff00', 'joyp'),
            ('0xff01', 'serial_transfer'),
            ('0xff02', 'serial_ctrl'),
            ('0xff04', 'timer_divider'),
            ('0xff05', 'timer_counter'),
            ('0xff06', 'timer_modulo'),
            ('0xff07', 'timer_control'),
            ('0xff03', 'io_ports'),
            ('0xff0f', 'int_flag'),
            ('0xff10', 'io_ports'),
            ('0xff25', 'snd_out_term_select'),
            ('0xff40', 'lcdc'),
            ('0xff41', 'lcd_status'),
            ('0xff42', 'scroll_y'),
            ('0xff43', 'scroll_x'),
            ('0xff44', 'y_coord'),
            ('0xff45', 'ly_compare'),
            ('0xff46', 'dma'),
            ('0xff47', 'bg_palette_data'),
            ('0xff48', 'obj_palette0_data'),
            ('0xff49', 'obj_palette1_data'),
            ('0xff4a', 'win_y_pos'),
            ('0xff4b', 'win_x_pos'),
            ('0xffff', 'interrupt_master'),
        ]
        for a, s in io_ports:
            m = MemSym(a, a, s)
            bisect.insort(cls.symbols, m)

        with open(path) as f:
            for l in f.readlines():
                if not l:
                    continue
                if l.startswith('0'):
                    addr, name = l.split()
                    addrs = addr.split('-')
                    addr_start = addrs[0]
                    addr_end = addrs[1] if len(addrs) == 2 else addrs[0]
                    m = MemSym(addr_start, addr_end, name)
                    bisect.insort(cls.symbols, m)
                elif l.startswith('force-ret'):
                    cls.as_ret.append(int(l.split()[1], 16))

    @classmethod
    def get(cls, addr: str) -> str:
        i = bisect.bisect_right(cls.symbols, addr)
        if i:
            sym = cls.symbols[i - 1]
            iaddr = int(addr, 16)
            if sym.begin == iaddr:
                return sym.name + '(' + addr + ')'
            if sym.begin <= iaddr < sym.end:
                return sym.name + "+" + str(iaddr - sym.begin)
        return addr

    @classmethod
    def get_aligned(cls, addr: str) -> str:
        i = bisect.bisect_right(cls.symbols, addr)
        if i:
            sym = cls.symbols[i - 1]
            iaddr = int(addr, 16)
            if sym.begin == iaddr:
                return "{}(0x{:0>4x})".format(sym.name, iaddr)
            if sym.begin <= iaddr < sym.end:
                return sym.name + "+" + "{:<7}".format(iaddr - sym.begin)
        return addr
