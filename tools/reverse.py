import bisect
import sys
from typing import List, Dict
from collections import defaultdict
from cpu import Instr, CPU, CallTracker, RetTracker, FunCallTracker
from dbg import Debugger
from addr import Addr


class Function:
    def __init__(self) -> None:
        self.instrs = [] # type: List[Instr]

    def log(self, instr: Instr) -> None:
        pos = bisect.bisect_left(self.instrs, instr)
        if len(self.instrs) == pos or self.instrs[pos] != instr:
            self.instrs.insert(pos, instr)

    def show(self) -> None:
        for i in self.instrs:
            print(Addr.get_aligned(i.addr) + '  ' + self.format_code(i))

    def format_code(self, i: Instr) -> str:
        code = i.code
        if i.code.startswith('call'):
            instr = i.code.split()[0]
            addr = i.code.split()[1].split('/')[0]
            code = instr + " " + Addr.get(addr)
        elif i.code.startswith('jp'):
            instr = i.code.split()[0]
            addr = i.code.split()[1].split('/')[0]
            if addr.startswith('0x'):
                code =  instr + " " + Addr.get(addr)
        elif i.code.startswith('jr'):
            instr = i.code.split()[0]
            addr = int(i.code.split()[1].split('/')[2])
            code =  instr + " " + Addr.get(hex(i.iaddr + addr + 2))

        if '[' in code:
            b = code.index('[') + 1
            e = code.index(']')
            if ' ' not in code[b:e] and code[b] == '0':
                addr = code[b:e].split('/')[0]
                code = code[:b] + Addr.get(addr) + code[e:]
            elif '+' in code[b:e] and code[b:e].startswith('0xFF00 + 0x'):
                addr = int(code[b + len('0xFF00 + '):e].split('/')[0], 16)
                code = code[:b] + Addr.get(hex(0xFF00 + addr)) + code[e:]

        return code


class Program:
    def __init__(self, debug=False) -> None:
        self.stack = [] # type: List[str]
        self.funs = defaultdict(Function) # type: Dict[str, Function]
        self.debug = debug

    def push(self, addr: str) -> None:
        self.stack.append(addr)
        if self.debug:
            print("." * (len(f.stack) - 1) + '}')
            self.inline_stack()

    def pop(self) -> None:
        self.stack.pop()
        if self.debug:
            print("." * (len(f.stack) - 1) + '}')
            self.inline_stack()

    def log(self, instr: Instr) -> None:
        if not instr.is_interrupt:
            self.funs[self.stack[-1]].log(instr)

        if self.debug:
            print("." * (len(f.stack) - 1) + Addr.get(instr.addr) + ':\t' + instr.code)

    def show(self) -> None:
        for k, v in self.funs.items():
            print('-------' + Addr.get(k))
            v.show()
        print('-------')

    def inline_stack(self) -> None:
        print("." * (len(self.stack) - 1) + "[" + " -> ".join(self.stack))


if len(sys.argv) == 3:
    Addr.init(sys.argv[2])

call = FunCallTracker()
ret = RetTracker()

f = Program(debug=False)
f.push('0x100')
cpu = CPU()
#dbg = Debugger()
try:
    for instr in cpu.execute(sys.argv[1]):
        #dbg.prompt(cpu)
        if ret.happened(instr):
            f.pop()

        if call.happened(instr):
            f.push(instr.addr)

        f.log(instr)

except:
    pass
finally:
    f.show()
