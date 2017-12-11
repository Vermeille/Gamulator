import bisect
import sys
from collections import defaultdict
from cpu import CPU
from dbg import Debugger, JumpTracker
from addr import Addr


class Function:
    def __init__(self):
        self.instrs = []

    def log(self, instr):
        pos = bisect.bisect_left(self.instrs, instr)
        if len(self.instrs) == pos or self.instrs[pos] != instr:
            self.instrs.insert(pos, instr)

    def show(self):
        for i in self.instrs:
            print(i.addr + '\t' + i.code)

class Program:
    def __init__(self):
        self.stack = []
        self.funs = defaultdict(Function)

    def push(self, addr):
        self.stack.append(addr)

    def pop(self):
        self.stack.pop()

    def log(self, instr):
        self.funs[self.stack[-1]].log(instr)

    def show(self):
        for f in self.funs.values():
            print('-------')
            f.show()
        print('-------')

if len(sys.argv) == 3:
    Addr.init(sys.argv[2])

call = JumpTracker('call')
ret = JumpTracker('ret')
rst = JumpTracker('rst')

f = Program()
f.push('0x100')
cpu = CPU()
#dbg = Debugger()
try:
    for instr in cpu.execute(sys.argv[1]):
        #print(Addr.get(instr.addr) + ':\t' + instr.code)
        #dbg.prompt(cpu)
        if call.happened(instr):
            f.push(instr.addr)
        if rst.happened(instr):
            f.push(instr.addr)

        f.log(instr)

        if ret.happened(instr):
            f.pop()
except:
    pass
finally:
    f.show()
