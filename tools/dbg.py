from addr import Addr


class JumpTracker:
    def __init__(self, ty):
        self.ty = ty
        self.addr = None
        self.is_jump = False

    def happened(self, instr):
        if instr.code.startswith(self.ty):
            addr = instr.code.split()
            if len(addr) == 1:
                return True
            addr = addr[1]
            try:
                self.addr = addr[:addr.index('/')]
            except:
                self.addr = addr
            self.is_jump = True
            return False
        if self.is_jump:
            self.is_jump = False
            return instr.addr == self.addr


class Continue:
    def must_stop(self, instr):
        return False


class Next:
    def must_stop(self, instr):
        return True


class ToCall:
    def __init__(self):
        self.call_tracker = JumpTracker('call')

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
        self.ret_tracker = JumpTracker('ret')
        self.rst_tracker = JumpTracker('rst')
        self.call_tracker = JumpTracker('call')
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

            if cmd == 'bt':
                for f in cpu.stack:
                    print(f)
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
