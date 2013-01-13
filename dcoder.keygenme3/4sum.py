import random as rnd

N = 32
M = 1<<N
L = 11
PER_LIST = 1<<L

def low(x):
    return x & (PER_LIST - 1)

def neg(x):
    return -x % M

def inv(l):
    return map(lambda x: neg(x), l)

def make_hash(l):
    h = {}
    for x in l:
        lowx = low(x)
        try:
            h[lowx].append(x)
        except:
            h[lowx] = [x]
    return h

def merge(l1, l2):
    h1 = make_hash(l1)
    h2 = make_hash(l2)
    h = {}
    ol = []
    for lowx, li1 in h1.iteritems():
        if lowx not in h2:
            continue
        li2 = h2[lowx]
        for x1 in li1:
            for x2 in li2:
                x2 = neg(x2)
                x = (x1+x2)%M
                print hex(x)
                ol.append(x)
    return ol

def rnd_list(total):
    l = []
    for i in range(total):
        x = rnd.randint(0, M)
        l.append(x)
    return l

def common(l1, l2):
    return set(l1) & set(l2)

if __name__=="__main__":

    [l1, l2, l3, l4] = map(lambda i: rnd_list(PER_LIST), range(4))

    l2 = inv(l2)
    l4 = inv(l4)
    l12 = merge(l1, l2)
    l34 = merge(l3, l4)
    print len(l12)
    print len(l34)
    inv(l34)
    l = common(l12, l34)
    print l
