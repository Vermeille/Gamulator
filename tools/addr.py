import bisect


class MemSym:
    def __init__(self, beg, end, name):
        self.begin = int(beg, 16)
        self.end = int(end, 16) + 1
        self.name = name

    def __repr__(self):
        return hex(self.begin) + '-' + hex(self.end) + ': ' + self.name

    def __eq__(self, m):
        if type(m) is str:
            return self.begin == int(m, 16)
        return self.begin == m.begin

    def __lt__(self, m):
        if type(m) is str:
            return self.begin < int(m, 16)
        return self.begin < m.begin

    def __le__(self, m):
        if type(m) is str:
            return self.begin <= int(m, 16)
        return self.begin <= m.begin

    def __gt__(self, m):
        if type(m) is str:
            return self.begin > int(m, 16)
        return self.begin > m.begin

    def __ge__(self, m):
        if type(m) is str:
            return self.begin >= int(m, 16)
        return self.begin >= m.begin


class Addr:
    symbols = []

    @classmethod
    def init(cls, path):
        with open(path) as f:
            for l in f.readlines():
                addr, name = l.split()
                addr = addr.split('-')
                addr_start = addr[0]
                addr_end = addr[1] if len(addr) == 2 else addr[0]
                m = MemSym(addr_start, addr_end, name)
                bisect.insort(cls.symbols, m)

    @classmethod
    def get(cls, addr):
        i = bisect.bisect_right(cls.symbols, addr)
        if i:
            sym = cls.symbols[i - 1]
            iaddr = int(addr, 16)
            if sym.begin == iaddr:
                return sym.name + '(' + addr + ')'
            if sym.begin <= iaddr < sym.end:
                return sym.name + "+" + str(iaddr - sym.begin)
        return addr
