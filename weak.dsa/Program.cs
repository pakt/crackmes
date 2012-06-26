//keygen for weakdsa by mr haandi
//
//uses birthday paradox. expected number of iterations is ~2^24,
//not including the DB precalc.
//
//pk
//gdtr.wordpress.com
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Numerics;
using System.Diagnostics;
using System.Threading;
using System.Security.Cryptography;
using System.Collections.Concurrent;

namespace weak_dsa_kg
{
    public static class RandomHelper
    {
        private static int seedCounter = new Random().Next();

        [ThreadStatic]
        private static Random rng;

        public static Random Instance
        {
            get
            {
                if (rng == null)
                {
                    int seed = Interlocked.Increment(ref seedCounter);
                    rng = new Random(seed);
                }
                return rng;
            }
        }
    }

    class Program
    {
        static BigInteger p = AuthServer.p;
        static BigInteger q = AuthServer.q;
        static BigInteger g = AuthServer.g;
        static BigInteger y = AuthServer.y;
        static BigInteger C = 1337; //wtv
        static BigInteger ygc = y * BigInteger.ModPow(g, C, p) % p;
        static int MAX_THREADS = 4;
        static int MAX_ELTS = 1<<24;
        static int COL_BYTES = 6;

        static ConcurrentDictionary<BigInteger, int> cd = new ConcurrentDictionary<BigInteger, int>(MAX_THREADS, MAX_ELTS);

        static bool collision_found = false;
        static int oexp = 0;
        static String oid = "";

        // Modular multiplicative inverse using extended Euclidean algorithm.
        static BigInteger inv(BigInteger a, BigInteger b)
        {
            BigInteger dividend = a % b;
            BigInteger divisor = b;

            BigInteger last_x = BigInteger.One;
            BigInteger curr_x = BigInteger.Zero;

            while (divisor.Sign > 0)
            {
                BigInteger quotient = dividend / divisor;
                BigInteger remainder = dividend % divisor;
                if (remainder.Sign <= 0)
                {
                    break;
                }
                BigInteger next_x = last_x - curr_x * quotient;
                last_x = curr_x;
                curr_x = next_x;

                dividend = divisor;
                divisor = remainder;
            }

            if (divisor != BigInteger.One)
            {
                throw new Exception("Numbers a and b are not relatively primes");
            }
            return (curr_x.Sign < 0 ? curr_x + b : curr_x);
        }

        static void make_rs(int exp, ref BigInteger r, ref BigInteger s)
        {
            BigInteger w;

            r = BigInteger.ModPow(ygc, exp, p) % q;
            w = (inv(r, q) * exp) % q;
            s = inv(w, q);
        }

        static void make_sig(String id, BigInteger r, BigInteger s, ref String sig)
        {
            byte[] t = new Byte[AuthServer.fieldSize * 2];

            byte[] rt = r.ToByteArray();
            byte[] st = s.ToByteArray();

            Array.Copy(rt, 0, t, 0, AuthServer.fieldSize);
            Array.Copy(st, 0, t, AuthServer.fieldSize, AuthServer.fieldSize);
            sig = Convert.ToBase64String(t);
        }

        static void make_fp(int exp, BigInteger s, ref String fp)
        {
            byte[] t = new Byte[AuthServer.fingerPrintSize];
            BigInteger MASK = 1;
            MASK <<= AuthServer.fingerPrintSize * 8;
            MASK -= 1;
            s = ((C * exp * s) % q) & MASK;
            byte[] st = s.ToByteArray();
            Array.Copy(st, t, t.Length);
            fp = Convert.ToBase64String(t);
        }

        static void make_all(int exp, String id, BigInteger r, BigInteger s, ref String fp, ref String sig)
        {
            make_sig(id, r, s, ref sig);
            make_fp(exp, s, ref fp);
        }

        static void filler()
        {
            int i = 0;
            String id;
            byte[] h_short = new Byte[COL_BYTES];
            MD5CryptoServiceProvider md5 = new MD5CryptoServiceProvider();
            Random rnd = RandomHelper.Instance;

            for (i = 0; cd.Count < MAX_ELTS; i = rnd.Next())
            {
                id = i.ToString();
                byte[] harr = AuthServer.hash(md5, id, COL_BYTES);
                Array.Resize(ref harr, COL_BYTES);
                BigInteger bh = new BigInteger(harr);
                cd[bh] = i;
            }
        }

        //birthday paradox
        static void solver()
        {
            int e;
            int N = 1 << (COL_BYTES * 8 / 2);
            int SHIFT = (AuthServer.fieldSize - COL_BYTES) * 8;
            BigInteger r;
            BigInteger cur = 1;
            Random rnd = RandomHelper.Instance;

            e = rnd.Next() % (1 << 28); //don't wrap too soon
            cur = BigInteger.ModPow(ygc, e, p);
            for (int exp = e+1; !collision_found; exp++)
            {
                cur = cur * ygc % p;
                r = cur % q;
                //w = exp * inv(r, q) % q;
                //r = (C * exp * inv(w, q)) % q;
                r = (C * r) % q;
                r = r >> SHIFT;
                if (cd.ContainsKey(r))
                {
                    lock (oid)
                    {
                        oexp = exp;
                        oid = cd[r].ToString();
                        collision_found = true;
                    }
                    return;
                }
            }
            return;
        }

        static Thread[] mass_spawn(ThreadStart f, int n)
        {
            Thread[] threads = new Thread[n];
            for (int i = 0; i < n; i++)
            {
                Thread t = new Thread(f);
                threads[i] = t;
                t.Start();
            }
            return threads;
        }

        static void mass_join(Thread[] threads)
        {
            foreach(Thread t in threads){
                t.Join();
            }
        }

        static void Main(string[] args)
        {
            String fp = "", sig = "";

            BigInteger r = 0, s = 0;
            Thread[] threads;

            threads = mass_spawn(filler, MAX_THREADS);

            while (true)
            {
                int c = cd.Count;
                if (c >= MAX_ELTS) break;
                Console.Clear();
                Console.WriteLine("db fill: {0:0.}%", 100*cd.Count/(float)MAX_ELTS);
                Thread.Sleep(1000);
            }

            mass_join(threads);

            Stopwatch sw = new Stopwatch();
            while (true)
            {
                Console.WriteLine("wait..");
                sw.Reset();
                sw.Start();
                collision_found = false;
                threads = mass_spawn(solver, MAX_THREADS);
                mass_join(threads);
                sw.Stop();

                string delta = string.Format("{0}m:{1}s", sw.Elapsed.Minutes, sw.Elapsed.Seconds);
                Console.WriteLine("time: {1}", oid, delta);

                make_rs(oexp, ref r, ref s);
                make_all(oexp, oid, r, s, ref fp, ref sig);
                Debug.Assert(AuthServer.Verify(oid, fp, sig));
                Console.WriteLine("id: {0}\nfp: {1}\nsig: {2}", oid, fp, sig);
            }
        }
    }
}
