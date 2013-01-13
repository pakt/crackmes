Dcoder keygenme3
----------------

Here's a high-level pseudocode of the problem:

sum = 0
name_hash = hash(name)
for i in 0..15:
    x = (i<<8) | serial[i]
    sn_hash = hash(x)
    sum += sn_hash

if(sum == name_hash)
    good
else
    bad

Hash function used is SipHash [0], which is a lightweight PRF (pseudorandom
function). 

You might recognize the problem stated by Dcoder as the subset sum problem (SUM) 
[1]. SUM is NP-complete and it's unknown if there is a polynomial time algorithm
that solves it exactly. The best exact general-case algorithm has complexity 
O(2^n/2), where n is the size of the set we can pick numbers from. In our case
n=16*256=2^12, which makes 2^n/2 prohibitively large.

There's a probabilistic algorithm that solves SUM in O(2^b/3) time, where b is
equal to the bit size of numbers used. In our case b=64 bits, so 2^b/3 ~ 2^21.
The algorithm takes 4 lists of numbers: l1, l2, l3, l4 and returns a set of 
tuples {(x1,x2,x3,x4): x1+x2+x3+x4=0}, where x[i] belongs to l[i]. The only 
requirement is that we can 'extend' all lists freely. All details are clearly
explained in [2], so I won't repeat them here.

At first sight, there are few problems:
1. we have 16 lists of numbers, instead of 4 (see pseudocode above),
2. each of our 16 lists contains exactly 256 numbers,
3. the target sum is (almost always) nonzero.

We can deal with 1 & 2 by observing that instead of working with 16 lists, we
can work with 4 lists containing random 4-element sums, so L[0] will contain
4-sums of elements from l[0],l[1],l[2],l[3], L[1] 4-sums of elements from 
l[4],..,l[7], L[2] - l[8],..,l[11], and L[3] - l[11],..,l[15]. Solving SUM for
L[0],..,L[3] solves it for l[0],..,l[15], because if y0+y1+y2+y3=0 and y[i] 
belongs to L[i], then y[i]=x0+..+x3, where x0 belongs to l[0], x1 belongs to 
l[1], etc.

Problem 3 is easily dealt with by observing that subtracting a constant from 
all elements of a single list is equivalent to solving SUM for nonzero sum. 
Indeed: x1+x2+x3+(x4-c)=0 implies x1+x2+x3+x4=c. Here, c is the target sum 
subtracted from list number 4.

4sum.py is an easy to read implementation of the original algorithm from [2].
sum4.c is the actual keygen source. Implementation had to be more complicated,
because otherwise keygen is slow :(. On my machine (2GHz), solving takes <4s.
Should compile without any warnings with 'make'.

% time ./keygen pk                                           
c75699f4c79c038da8e83d6172bef93a
d59a129cc62b0db2be79bf1482687a84
b922ea4aef7ed1fbe73e4ebe38f8146b
f1b728f382e886b280f8177372e3e202
41fd2b233c7807f373a36867b6fcf0e5
32f83a5f4903c7d890f2d1fc42eb752f
7a9a20358b5840cc728b3aba74a44614
./keygen pk  3.72s user 0.79s system 99% cpu 4.531 total

0 - https://131002.net/siphash/
1 - http://en.wikipedia.org/wiki/Subset_sum_problem
3 - http://www.cs.berkeley.edu/~daw/papers/genbday-long.ps

pk
gdtr.wordpress.com
29.12.2012
