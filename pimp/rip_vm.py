import idc

VM_MIX = 1
VM_DIE = 2
VM_JMP = 3
VM_JZ = 4
VM_JNZ = 5

seed = 0
def prng():
    global seed

    v0 = ((10009 * seed + 31337) % 2**32) % 100000007
    seed = 5 * seed + 1337;
    seed = seed % 2**32
    return v0

def find_op(op, l):
    for i,u in enumerate(l):
        if op==u[0]:
            return i,u[0],u[1]
    return -1,None,None

def disasm(vm, handlers):
    i = 0 
    for op, arg, _ in vm:    
        oph, name, _, _ = handlers[op]
        #assert (op == oph)
        print "%d:\t[%02x]\t%s"%(i, op, name),
        if name in ["vm_JMP", "vm_JZ", "vm_JNZ"]:
            if arg & 0x80000000:
                print -((arg ^ 0xFFFFFFFF)+1)
            else:
                print arg
        else:
            print hex(arg)
        i += 1

def is_nice(vm, handlers):
    trans = []

    for op, arg, _ in vm:    
        oph, name, _, _ = handlers[op]
        trans.append((oph, arg, name))
    
    #premature VM_DIE
    f1 = lambda l: filter(lambda (op,arg,_): op==VM_DIE and arg!=0, l)
    # op reg1, reg2 with params
    f2 = lambda l: filter(lambda (op, arg, name): name.find("reg1_reg2")>=0 and arg!=0, l)
    # bad jumps
    f3 = lambda l: filter(lambda (op,arg,_): op in [VM_JMP,VM_JZ,VM_JNZ] and (arg==0 or arg>128 and arg<0x80000000), l)

    heurs = [f2, f3]
    for f in heurs:
        o = f(trans)
        if o:
            return False
    return True

def mutate(handlers):
    
    for j in range(2, len(handlers)):
        k = prng()
        k = (k % 0x1B)+2
        t = handlers[j]
        handlers[j] = handlers[k]
        handlers[k] = t
    return handlers

debug = False

vm_code_addr = 0x0091F940
vm_size = 1245

vm = []

for off in range(vm_code_addr, vm_code_addr+vm_size, 5):
    
    vm_op = idc.Byte(off)
    vm_arg = idc.Dword(off+1)

    vm.append((vm_op, vm_arg, off))    
    
    if debug:
        print hex(off)
        print hex(vm_op), hex(vm_arg)

print "total instructions:", len(vm)

handlers_addr = 0x0091FE24
handlers_count = 0x1c
handlers = [(None,None,None,None)]

for i in range(handlers_count):
    off = handlers_addr + i*20 
    op = idc.Dword(off)
    if i>7:
        off += 4
    addr = idc.Dword(off+4)
    name = idc.GetFunctionName(addr)

    print hex(off), hex(op), hex(addr), name
    
    handlers.append((op, name, addr, off))


i = 0
p,_,_ = find_op(VM_MIX, vm)
disasm(vm[:p+1], handlers)


print "possible disasms"
sn1 = 0

for pos in range(8):
    p,_,_ = find_op(VM_MIX, vm)
    assert(p>=0)
    vm = vm[p+1:]
    orig = handlers[:]
    solutions = []
    for i in range(16):
        seed = sn1 | (i << pos*4)
        #print "seed: 0x%08x"%seed
        handlers = orig[:]
        handlers = mutate(handlers)
        p,_,_ = find_op(VM_MIX, vm)
        if p<0 and pos==7:
            p = len(vm)
        if is_nice(vm[:p+1], handlers):
            if (pos==7 and handlers[0xc][0]!=VM_DIE):
                continue
            print i, "#"*10
            solutions.append((i,handlers))
            disasm(vm[:p+1], handlers)
    
    if pos!=7:
        assert(len(solutions)==1)
        i,handlers = solutions[0]
        sn1 = sn1 | (i << pos*4)
        print "0x%08x"%sn1
    else:
        for i,_ in solutions:
            sn = sn1 | (i << pos*4)
            print "solution: 0x%08x"%sn




