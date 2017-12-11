from addr import Addr


def instrs(filepath):
    with open(filepath) as f:
        for l in f.readlines():
            if not l.startswith('0x'):
                continue

            tab = l.index('\t')
            instr_tab = tab + 1 + l[tab + 1:].index('\t') + 1

            addr = l[:tab]
            instr = l[instr_tab:-1]

            yield Instr(addr, instr)

class Instr:
    def __init__(self, addr, code):
        self.addr = addr
        self.code = code

    def __eq__(self, i):
        return self.addr == i.addr

    def __ne__(self, i):
        return self.addr != i.addr

    def __lt__(self, i):
        return self.addr < i.addr

    def __le__(self, i):
        return self.addr <= i.addr

    def __gt__(self, i):
        return self.addr > i.addr

    def __ge__(self, i):
        return self.addr >= i.addr


class CPU:
    def __init__(self):
        self.is_call = False
        self.is_ret = False
        self.stack = [Addr.get('0x100')]

    def feed(self, instr):
        self.instr = instr

        if self.is_call:
            self.stack.append(instr.addr)
        if self.is_ret:
            self.stack.pop()

        self.is_call = instr.code.startswith('call')
        self.is_ret = instr.code.startswith('ret')

    def execute(self, filepath):
        for instr in instrs(filepath):
            self.feed(instr)
            yield instr
