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
