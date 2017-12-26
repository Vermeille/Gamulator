from addr import Addr
from cpu import CallTracker, RetTracker, RstTracker


class Continue:
    def must_stop(self, instr):
        return False


class Next:
    def must_stop(self, instr):
        return True


class ToCall:
    def __init__(self):
        self.call_tracker = CallTracker('call')

    def must_stop(self, instr):
        return self.call_tracker.happened(instr)


class ToRet:
    def __init__(self):
        self.ret_tracker = Step()
        self.ret_tracker.depth = 1

    def must_stop(self, instr):
        return self.ret_tracker.must_stop(instr)


class Step:
    def __init__(self):
        self.ret_tracker = RetTracker()
        self.rst_tracker = RstTracker()
        self.call_tracker = CallTracker()
        self.depth = 0

    def must_stop(self, instr):
        if self.call_tracker.happened(instr):
            self.depth += 1
        if self.rst_tracker.happened(instr):
            self.depth += 1
        if self.ret_tracker.happened(instr):
            self.depth -= 1
        return self.depth == 0


class Debugger:
    def __init__(self):
        self.breakpoints = []
        self.cmd = None
        self.args = []
        self.state = Next()

    def readcmd(self):
        cmd = raw_input('> ').split()

        if len(cmd) == 0:
            return self.cmd, self.args
        elif len(cmd) == 1:
            cmd = cmd[0]
            args = []
        else:
            args = cmd[1:]
            cmd = cmd[0]

        if not cmd:
            return self.cmd, self.args

        self.cmd = cmd
        self.args = args
        return self.cmd, self.args

    def prompt(self, cpu):
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
