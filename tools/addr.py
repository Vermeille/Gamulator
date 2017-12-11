class Addr:
    symbols = {}

    @classmethod
    def init(cls, path):
        with open(path) as f:
            for l in f.readlines():
                addr, name = l.split()
                cls.symbols[int(addr, 16)] = name

    @classmethod
    def get(cls, addr):
        try:
            return cls.symbols[int(addr, 16)]
        except:
            return addr
