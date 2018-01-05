import bisect
import sys
from typing import List, Dict
from collections import defaultdict
from cpu import Instr, CPU, CallTracker, RetTracker, FunCallTracker
from dbg import Debugger
from addr import Addr


class Function:
    def __init__(self) -> None:
        self.instrs = []  # type: List[Instr]

    def log(self, instr: Instr) -> None:
        pos = bisect.bisect_left(self.instrs, instr)
        if len(self.instrs) == pos or self.instrs[pos] != instr:
            self.instrs.insert(pos, instr)

    def show(self) -> None:
        for i in self.instrs:
            print(Addr.get_aligned(i.addr) + '  ' + i.format_code)


class Program:
    def __init__(self, debug=False) -> None:
        self.stack = []  # type: List[str]
        self.funs = defaultdict(Function)  # type: Dict[str, Function]
        self.debug = debug

    def push(self, addr: str) -> None:
        self.stack.append(addr)
        if self.debug:
            print("." * (len(self.stack) - 1) + '}')
            self.inline_stack()

    def pop(self) -> None:
        self.stack.pop()
        if self.debug:
            print("." * (len(self.stack) - 1) + '}')
            self.inline_stack()

    def log(self, instr: Instr) -> None:
        if not instr.is_interrupt:
            self.funs[self.stack[-1]].log(instr)

        if self.debug:
            print("." * (len(self.stack) - 1) + Addr.get(instr.addr) + ':\t' +
                  instr.code)

        if instr.iaddr in Addr.jp_to_fun:
            self.stack[-1] = instr.extract_addr()

    def show(self) -> None:
        for k, v in self.funs.items():
            print('-------' + Addr.get(k))
            v.show()
        print('-------')

    def inline_stack(self) -> None:
        print("." * (len(self.stack) - 1) + "[" + " -> ".join(self.stack))


def reconstruct_functions(trace_file: str) -> Program:
    call = FunCallTracker()
    ret = RetTracker()

    f = Program(debug=False)
    f.push('0x100')
    cpu = CPU()
    for instr in cpu.execute(trace_file):
        if ret.happened(instr):
            f.pop()

        if call.happened(instr):
            f.push(instr.addr)

        f.log(instr)

    return f


if __name__ == '__main__':
    if len(sys.argv) == 3:
        Addr.init(sys.argv[2])

    reconstruct_functions(sys.argv[1]).show()
