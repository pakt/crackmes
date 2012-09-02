import itertools as it
from z3 import *

SEED = 0x48AEEFD486289CFB
TAPS = 0x9B1ADEDF847D3481
MASK=(1<<64)-1

def possible_buttons():
    possible_taps = []
    for i in range(16):
        tap = (1+29*i)%64
        possible_taps.append(tap)
    return possible_taps

def idx_set(x, idx):
    return (x&(1<<idx))>>idx

def to_bits(x):
    idx = 63
    o = []
    while idx>=0:
        o.append(idx_set(x,idx))
        idx = idx - 1
    return o

def idx2button(idx):
    for i in range(16):
        if (1+29*i)%64==idx:
            return i
    assert(False)

def solve2(seed, final, rounds):
    buttons = possible_buttons()
    buttons_bvs = map(lambda i: (i,BitVecVal(1<<i, 64)), buttons)

    taps = to_bits(TAPS)
    taps = [(63-i,t) for (i,t) in enumerate(taps)]
    taps = filter(lambda (i,t): t!=0, taps)
    taps_bvs = map(lambda (i,t): (i,BitVecVal(1<<i, 64)), taps)

    state_bv = BitVecVal(seed, 64)
    mask_bv = BitVecVal(MASK, 64)
    #poly_bv = BitVecVal(TAPS, 64)

    s = Solver()
    unks = []
    for j in range(rounds):
        t = BitVec("t%d"%j, 64)
        x = BitVec("x%d"%j, 64)
        ors = map(lambda (i,btn_bv): x==btn_bv, buttons_bvs)
        alts = reduce(lambda acc,x: Or(acc,x), ors)

        #print alts
        #print t==state_bv^x

        s.add(alts)
        t = state_bv ^ x
        #s.add(t==state_bv^x)
        #t = state_bv ^ x

        masked = map(lambda (i,tap_bv): LShR((t & tap_bv),i), taps_bvs)
        parity = reduce(lambda acc,x: acc^x, masked)
        state_bv = (t<<1) | (parity)
        #state_bv = state_bv & mask_bv
        unks.append(x)
    
    final_bv = BitVecVal(final, 64)
    #s.add(state_bv != BitVecVal(0, 64))
    s.add(state_bv == final_bv)
    #print s.sexpr()
    if s.check() == sat:
        m = s.model()
    else:
        return []

    key = []
    for bv in unks:
        b = m[bv].as_long()
        l = set_bits(b)
        assert(len(l)==1)
        idx = l[0]
        #print bv, b, idx
        key.append(idx)
    
    key = map(lambda i: idx2button(i), key)
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

if __name__=="__main__":
    final1 = 0xe7fd097289cbb591
    final2 = 0xe7fd097289cbef79 
    #solve_with_z3(seed1)
    o = final1
    rounds = 22
    key = solve2(SEED, o, rounds)

    print key
    s = map(lambda x: hex(x).replace("0x", ""), key)
    print "".join(s)

