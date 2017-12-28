import sys
from typing import Tuple, List
from addr import Addr
from cpu import CPU, CallTracker, RetTracker, RstTracker, Instr, FunCallTracker
from abc import ABCMeta, abstractmethod


class Command(metaclass=ABCMeta):
    @abstractmethod
    def must_stop(self, instr: Instr) -> bool:
        pass


class Continue(Command):
    def must_stop(self, instr: Instr) -> bool:
        return False


class Next(Command):
    def must_stop(self, instr: Instr) -> bool:
        return True


class ToCall(Command):
    def __init__(self) -> None:
        self.call_tracker = CallTracker()

    def must_stop(self, instr: Instr) -> bool:
        return self.call_tracker.happened(instr)


class ToRet(Command):
    def __init__(self) -> None:
        self.ret_tracker = Step()
        self.ret_tracker.depth = 1

    def must_stop(self, instr: Instr) -> bool:
        return self.ret_tracker.must_stop(instr)


class Step(Command):
    def __init__(self) -> None:
        self.ret_tracker = RetTracker()
        self.rst_tracker = RstTracker()
        self.call_tracker = CallTracker()
        self.depth = 0

    def must_stop(self, instr: Instr) -> bool:
        if self.call_tracker.happened(instr):
            self.depth += 1
        if self.rst_tracker.happened(instr):
            self.depth += 1
        if self.ret_tracker.happened(instr):
            self.depth -= 1
        return self.depth == 0


class Debugger:
    def __init__(self) -> None:
        self.breakpoints = []  # type: List[str]
        self.cmd = ''
        self.args = []  # type: List[str]
        self.state = Next()  # type: Command

    def readcmd(self) -> Tuple[str, List[str]]:
        cmd_toks = input('> ').split()

        if len(cmd_toks) == 0:
            return self.cmd, self.args
        elif len(cmd_toks) == 1:
            cmd = cmd_toks[0]
            args = []  # type: List[str]
        else:
            args = cmd_toks[1:]
            cmd = cmd_toks[0]

        if not cmd:
            return self.cmd, self.args

        self.cmd = cmd
        self.args = args
        return self.cmd, self.args

    def prompt(self, cpu: CPU) -> None:
        if cpu.instr.addr in self.breakpoints:
            self.state = Next()

        if not self.state.must_stop(cpu.instr):
            return

        while True:
            cmd, args = self.readcmd()

            if cmd == 'b':
                self.breakpoints += args
            if cmd == 'c':
                self.state = Continue()
                self.state.must_stop(cpu.instr)
                break
            if cmd == 'n':
                self.state = Next()
                self.state.must_stop(cpu.instr)
                break
            if cmd == 'tc':
                self.state = ToCall()
                self.state.must_stop(cpu.instr)
                break
            if cmd == 'tr':
                self.state = ToRet()
                self.state.must_stop(cpu.instr)
                break
            if cmd == 's':
                self.state = Step()
                self.state.must_stop(cpu.instr)
                break
            if cmd == '?':
                print('Commands:\n'
                      '  bt:          show backtrace\n'
                      '  b <addr>:    add a breakpoint at addr\n'
                      '  n:           skip to next instruction\n'
                      '  tc:          skip to next function call\n'
                      '  tr:          skip to ret from current function\n'
                      '  s:           step to the next instr in the current\n'
                      '               function\n'
                      '  ?:           display this help')

def run_dbg(trace_file: str) -> None:
    call = FunCallTracker()
    ret = RetTracker()

    cpu = CPU()
    dbg = Debugger()
    for instr in cpu.execute(trace_file):
        print(instr.addr + " " + instr.code)
        dbg.prompt(cpu)


if __name__ == '__main__':
    if len(sys.argv) == 3:
        Addr.init(sys.argv[2])

    run_dbg(sys.argv[1])
