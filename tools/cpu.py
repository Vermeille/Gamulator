from typing import Iterator
from addr import Addr

class Instr:
    def __init__(self, addr: str, code, is_interrupt) -> None:
        self.addr = addr
        self.iaddr = int(addr, 16)
        self.code = code
        self._is_interrupt = is_interrupt
        self.format_code = self.pretty_code(self.code)

    def __eq__(self, i):
        return self.iaddr == i.iaddr

    def __ne__(self, i):
        return self.iaddr != i.iaddr

    def __lt__(self, i):
        return self.iaddr < i.iaddr

    def __le__(self, i):
        return self.iaddr <= i.iaddr

    def __gt__(self, i):
        return self.iaddr > i.iaddr

    def __ge__(self, i):
        return self.iaddr >= i.iaddr

    @property
    def is_interrupt(self) -> bool:
        return self._is_interrupt

    def pretty_code(self, code: str) -> str:
        if code.startswith('call'):
            instr = code.split()[0]
            addr = code.split()[1].split('/')[0]
            code = instr + " " + Addr.get(addr)
        elif code.startswith('jp'):
            instr = code.split()[0]
            addr = code.split()[1].split('/')[0]
            if addr.startswith('0x'):
                code = instr + " " + Addr.get(addr)
        elif code.startswith('jr'):
            instr = code.split()[0]
            iaddr = int(code.split()[1].split('/')[2])
            code = instr + " " + Addr.get(hex(self.iaddr + iaddr + 2))

        if '[' in code:
            b = code.index('[') + 1
            e = code.index(']')
            if ' ' not in code[b:e] and code[b] == '0':
                addr = code[b:e].split('/')[0]
                code = code[:b] + Addr.get(addr) + code[e:]
            elif '+' in code[b:e] and code[b:e].startswith('0xFF00 + 0x'):
                iaddr = int(code[b + len('0xFF00 + '):e].split('/')[0], 16)
                code = code[:b] + Addr.get(hex(0xFF00 + iaddr)) + code[e:]

        return code


def instrs(filepath: str) -> Iterator[Instr]:
    with open(filepath) as f:
        for l in f.readlines():
            if l.startswith('0x'):
                tab = l.index('\t')
                instr_tab = tab + 1 + l[tab + 1:].index('\t') + 1

                addr = l[:tab]
                instr = l[instr_tab:-1]

                yield Instr(addr, instr, is_interrupt=False)
            elif l.startswith('INT'):
                yield Instr(l.split()[1], 'INT', is_interrupt=True)

class CallTracker:
    def __init__(self) -> None:
        self.prev = Instr('0x0', '', False)

    def happened(self, instr: Instr) -> bool:
        prev = self.prev
        self.prev = instr

        return self.is_call(prev) and self.get_addr(prev) == instr.iaddr

    def is_call(self, i: Instr) -> bool:
        return i.code.startswith('call')

    def get_addr(self, i: Instr) -> int:
        addr = i.code.split()[1]
        addr = addr[:addr.index('/')]
        return int(addr, 16)

class RstTracker:
    def __init__(self) -> None:
        self.prev = Instr('0x0', '', False)

    def happened(self, instr: Instr) -> bool:
        prev = self.prev
        self.prev = instr

        return self.is_call(prev) and self.get_addr(prev) == instr.iaddr

    def is_call(self, i: Instr) -> bool:
        return i.code.startswith('rst')

    def get_addr(self, i: Instr) -> int:
        addr = i.code.split()[1]
        return int(addr, 16)


class FunCallTracker:
    def __init__(self) -> None:
        self.call_tracker = CallTracker()
        self.rst_tracker = RstTracker()

    def happened(self, instr: Instr) -> bool:
        hap = instr.is_interrupt
        hap = self.call_tracker.happened(instr) or hap
        hap = self.rst_tracker.happened(instr) or hap
        return hap

class RetTracker:
    def __init__(self) -> None:
        self.prev = Instr('0x0', '', False)

    def happened(self, instr: Instr) -> bool:
        prev = self.prev
        self.prev = instr

        if prev.iaddr in Addr.as_ret:
            return True
        return self.is_ret(prev) and prev.iaddr + 1 != instr.iaddr

    def is_ret(self, i: Instr) -> bool:
        return i.code.startswith('ret')

class Stack:
    def __init__(self, init_addr: str) -> None:
        self.stack = [init_addr]
        self.call_tracker = FunCallTracker()
        self.ret_tracker = RetTracker()

    def feed(self, instr: Instr):
        if self.call_tracker.happened(instr):
            self.stack.append(instr.addr)
        if self.ret_tracker.happened(instr):
            self.stack.pop()

    def current_frame(self):
        return self.stack[-1]

class CPU:
    def __init__(self):
        self.instr = None
        self.stack = Stack('0x100')

    def feed(self, instr: Instr):
        self.instr = instr
        self.stack.feed(instr)

    def execute(self, filepath: str) -> Iterator[Instr]:
        for instr in instrs(filepath):
            self.feed(instr)
            yield instr
