# pa_kt
# 7e476857-pcp1aa1agslatl3tptgs

def import_tab(fn):
    f = open(fn, "r")
    l = f.readlines()
    l = map(lambda x: x.strip(), l)
    
    o = []
    for x in l:
        d1, d2, c = x.split(" ")
        #print d1, d2, c
        
        d1, d2, c = int(d1, 16), int(d2, 16), int(c, 16)
        
        o.append((d1,d2))
    
    return o

def dump_tab(tab):
    
    h = map(lambda (x,y): "0x%08x"%x, tab)
    l = map(lambda (x,y): "0x%08x"%y, tab)
    
    o = ", ".join(h)
    print "tab_hi[] = {%s}" % o

    o = ", ".join(l)
    print "tab_lo[] = {%s}" % o
    
def crc64(hi, lo, tab, dw):
    HI = 0
    LO = 1
    
    old_lo = lo
        
    lo = 0
    hi = 0
    for pos in range(32):
        b = dw>>pos
        b = b & 1
        if b:
            hi ^= tab[pos][HI]  
            lo ^= tab[pos][LO]
    
    return (hi^old_lo,lo)

def tests(tab):

    hi,lo = crc64(0, 0, tab, 3)    
    print hex(hi), hex(lo)
    assert(hi == 0x318e06d4 and lo == 0xf344b26c)
    
    hi,lo = crc64(0xffffffff, 0xffffffff, tab, 3)    
    print hex(hi), hex(lo)
    assert(hi == 0xce71f92b and lo == 0xf344b26c)

    hi,lo = crc64(0, 0, tab, 0xaabbccdd)    
    print hex(hi), hex(lo)      
    assert (hi == 0x1907d4f7 and lo == 0x5ac9db3f)

    hi,lo = crc64(0x11223344, 0x55667788, tab, 0xaabbccdd)    
    print hex(hi), hex(lo)      
    assert (hi == 0x4c61a37f and lo == 0x5ac9db3f)

    hi,lo = crc64(0, 0x12345678, tab, 0xaabbccdd)    
    print hex(hi), hex(lo)      
    assert (hi == 0xb33828f and lo == 0x5ac9db3f)

def lol(s, tab):

    o = []
    for c in s:
        x = ord(c)
        i = tab.index(x)
        o.append(i)
    return o

def encode(words):
    tab = [0x00, 0x35, 0x0D, 0x39, 0x20, 0x23, 0x2C, 0x2E, 0x0A, 0x29, 0x34, 0x26, 0x38, 0x30, 0x3A, 0x3B, 
            0x33, 0x22, 0x24, 0x3F, 0x27, 0x36, 0x21, 0x2F, 0x2D, 0x32, 0x27, 0x20, 0x37, 0x31, 0x28, 0x20]
    
    serial = ""
    for w in words: 
        d = str(w)
        n = len(d)
        d = "0"*max(0, 5-n)+d
        l = lol(d, tab)
        l2 = []
        for x in l:
            if x<10:
                y = x+0x30
            else:
                y = x+ord('W')
            l2.append(chr(y))
            
        print hex(w),d,l2
        serial += "".join(l2)
    
    return serial
        
    
if __name__=="__main__":
    
    tab = import_tab("tab.txt")

#
# 0x01814a43 -> 0x70cd1b75 0x386ddc48 (h3, l3)
# 0xffffc8ca -> 0xcb49b5bc 0x2381b46f
#
    
    #pa_kt
    
    magic1 = 0xffffc8ca
    magic2 = 0x01814a43
        
    hi = 0x4e82a694
    lo = 0xe5a10556
    
    d1 = hi ^ magic1
    
    hi,lo = crc64(hi, lo, tab, magic1)
    print hex(hi), hex(lo)     
    
    d2 = hi ^ magic2
    
    hi,lo = crc64(hi, lo, tab, 0x01814a43)
    print hex(hi), hex(lo) 

    print hex(d1), hex(d2)
    words = [d1 & 0xFFFF, d1>>16, d2 & 0xFFFF, d2 >> 16]
    print words
    o = encode(words)
    print o
    

    
