import itertools as it
from z3 import *
from tab import * #TAB

SEED = 0x48AEEFD486289CFB
FINAL = 0xA3FE45ADF1EC2AB4
TAPS = 0x9B1ADEDF847D3481
#FINAL = 0xF1EC2AB4A3FE45AD

MASK=(1<<64)-1

def xorall64(n):
    ret=0
    for i in range(64):
        if n&(1<<i):
            ret^=1
    return ret

def tick_noxor(n64):
    n64=((n64<<1) | xorall64(n64 & TAPS)) & MASK
    return n64

def tick(n64, idx):
    n64^=(1<<((1+29*idx)%64))
    n64=((n64<<1) | xorall64(n64 & TAPS)) & MASK
    return n64

def stage1(n64,buttonsPressed):
    print "#"
    for buttonPressed in buttonsPressed:
        #print buttonPressed
        n64 = tick(n64, buttonPressed)
        print hex(buttonPressed), hex(n64)
        #print hex(n64)
    print "#"
    return n64

def idx2button(idx):
    for i in range(16):
        if (1+29*i)%64==idx:
            return i
    assert(False)

def idx_set(x, idx):
    return (x&(1<<idx))>>idx

def find_lowest(x, possible_taps, bit):
    idx = 0
    for idx in possible_taps:
        if idx_set(x, idx) == bit:
            return idx
    assert(False)

def gen_zero(seed, possible_taps):
    key = []
    x = seed
    for i in range(8):
        y = x & TAPS
        b = xorall64(y)
        idx = find_lowest(y, possible_taps, 1^b)
        print y, b, idx
        k = idx2button(idx)
        x = tick(x, k)
        print hex(x)
        key.append(k)
    return key

def set_bits(x):
    i = 0
    bits = []
    while i<64:
        if x&1:
            bits.append(i)
        x = x>>1
        i += 1
    return bits

def test():
    key = []
    for i in range(10):
        key.append(i)
    o = stage1(SEED, key)
    assert(o == 0xb3b75a9822f3653fL)

def comp(seed, possible_taps):
    s_noxor = seed
    s_xor = seed
    for i in range(64):
        tap = possible_taps[i%16]
        s_noxor = tick_noxor(s_noxor)
        s_xor = tick(s_xor, tap)
        print hex(s_noxor ^ s_xor)

def conv(seed):
    row = [0]*64
    tab = []
    for i in range(63):
        row[i+1] = 1
        r = map(lambda x: str(x), row)
        tab.append("[%s]"%(",".join(r)))
        row[i+1] = 0
    print ",\n".join(tab),
    bits = []
    for i in range(64):
        b = idx_set(seed,i)
        bits.append(str(b))
    bits = bits[::-1]
    print ","
    print "[%s]"%(",".join(bits))

def post_(state, tab):
    assert(len(tab)==512)

    n = 0
    idx = 0
    res = 0
    matches = 0
    cur_c = 0
    while idx<64:
        c = tab[cur_c]
        if n<8:
            all_bits_match = int(state & c == c)
            matches += all_bits_match
            n += 1
            cur_c += 1
            if n==8:
                b = matches % 2
                if b:
                    res = res | (1<<idx)
                assert(idx<64)
                idx += 1
                n = 0
                matches = 0
    return res

def post(state): 
    return post_(state, TAB)

def is_power2(x):
    return (x!=0) and (x & (x-1) == 0)

def to_bits(x):
    idx = 63
    o = []
    while idx>=0:
        o.append(idx_set(x,idx))
        idx = idx - 1
    return o

def solve_with_z3():
    s = Solver()
    state = [BitVec("b%d"%(i), 1) for i in range(64)]
    final = to_bits(FINAL)
    one = BitVecVal(1, 1)
    zero = BitVecVal(0, 1)
    l = [zero, one]
    final = map(lambda b: l[b], final)
    print state
    print final
    and_gates = []
    xor_gates = []
    idx = 63
    for c in TAB:
        assert(idx<64)
        bits = to_bits(c)
        #print "bits:",bits
        bvars = it.compress(state, bits)
        bvars = [bv for bv in bvars]
        gate = reduce(lambda acc,x: acc&x, bvars)
        and_gates.append(gate)
        if len(and_gates) == 8:
            xor_gate = reduce(lambda acc,x: acc^x, and_gates)
            final_bit = final[idx]
            s.add(xor_gate == final_bit)
            #print s
            idx -= 1
            and_gates = []

    #ban = 0xe7fd097289cbef79
    banned = []
    s.push()
    while True:
        for ban in banned:
            bits = to_bits(ban)
            bits = map(lambda b: l[b], bits)
            pairs = [(bv,b) for (bv,b) in zip(state, bits)]
            ne = map(lambda (bv,b): bv!=b, pairs)
            neq = reduce(lambda acc,x: Or(acc,x), ne)
            s.add(neq)

        if s.check().r:
            m = s.model()
            fin = 0
            for idx,bv in enumerate(state):
                idx = 63-idx
                b = m[bv].as_long()
                fin = fin | (b<<idx)
            o = post(fin)
            print hex(fin), hex(o)
            banned = [fin]
        else:
            break

            #y = post(fin)
            #print hex(FINAL), hex(y)

def go_back(seed, state):
    bits = set_bits(TAPS)
    s1 = set(possible_taps)
    s2 = set(bits)
    common = s1.intersection(s2)
    uncommon = set([i for i in range(64)])-s1

    print common
    print uncommon
    
    common = list(common)
    uncommon = list(uncommon)
    common.sort()
    uncommon.sort()

    pattern = seed
    i = 63
    while i>=0:
        xorres = state & 1
        state = state >> 1
        wanted = pattern & 1
        prev_xorres = xorall64(state & TAPS)
        
        co = filter(lambda x: x<=i, common)
        un = filter(lambda x: x<=i, uncommon)

        if wanted == 0:
            if xorres == prev_xorres:
                v = un[0]
            else:
                v = co[0]
        else:
            state = state | (1<<63)
            if xorres == prev_xorres:
                v = co[0]
            else:
                v = un[0]
        state = state ^ (1<<v)
        assert(xorall64(state & TAPS) == xorres)

        print "i:", i, "v:", v, "diff:", hex((state>>i)^seed)
        i -= 1
        pattern = pattern >> 1

if __name__=='__main__':
    test()
    
    possible_taps = []
    for i in range(16):
        tap = (1+29*i)%64
        print "%d->%d"%(i,tap)
        possible_taps.append(tap)

    s = tick(SEED, 0)
    o = post(s)
    assert(o == 0x889fdedae042bf1dL)
    
    #go_back(SEED, 11111)

    #conv(TAPS)
    print hex(tick(SEED, 0))

    k = [1, 1, 10, 9, 11, 1, 8, 10, 7, 14, 15, 1, 3, 14, 15, 6, 14, 10, 3, 10, 12, 14]
    s = map(lambda x: hex(x).replace("0x", ""), k)
    print "".join(s)
    o = stage1(SEED, k)
    print hex(o)
