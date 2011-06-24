# Machine I had access to had limits on continuous CPU usage
# by one process, so I had to split the work into small chunks
# so that workers wouldn't be killed during a computation.
#
# pa_kt
# gdtr.wordpress.com

import subprocess as sub
import sys

if len(sys.argv)<3:
    print sys.argv[0], "<start val> <end val> <slaves count>"
    sys.exit(1)

start = int(sys.argv[1], 16)
end =  int(sys.argv[2], 16)
count =  int(sys.argv[3], 10)

N_SLAVES = 32
BIG_CHUNK = 0X10000000/2
SMALL_CHUNK = BIG_CHUNK/N_SLAVES

f = open("out.txt", "w")

for k in range(end/BIG_CHUNK): 
    f.write("starting %d chunk\n"%k)
    f.flush()

    slaves = []
    for i in range(N_SLAVES):
        s = start+i*SMALL_CHUNK
        e = s+SMALL_CHUNK
        
        f.write("i=%d s=0x%08x e=0x%08x\n"%(i,s,e))
        f.flush()

        p = sub.Popen(["./a.out", hex(s), hex(e)], stdout=sub.PIPE)
        slaves.append(p)
    
    for p in slaves:
        stdout, stderr = p.communicate()
        f.write(stdout)
        f.write("###\n")
        f.flush()
    
    start += BIG_CHUNK
