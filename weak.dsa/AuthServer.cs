using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Numerics;
using System.Security.Cryptography;
using System.Text.RegularExpressions;

namespace weak_dsa_kg
{
    class AuthServer
    {
        public static BigInteger p = BigInteger.Parse("12343166190249099963844614971858188890660252314196981857116019178266498099196944436576774068596061785922642532486910253402920733695804947624905546363393363");
        public static BigInteger q = BigInteger.Parse("6171583095124549981922307485929094445330126157098490928558009589133249049598472218288387034298030892961321266243455126701460366847902473812452773181696681");
        public static BigInteger g = BigInteger.Parse("6786183533171661594262558814260314083384466978959974116431945959977589683740679922916498942642093589315722172714694250494775306223775833010063525305963887");
        public static BigInteger y = BigInteger.Parse("10064244825758094072963970119807064710181088090418826682084640363233124440395354645467056460109079832312427767666487651151696929816590482407358254249751172");
        public static int fieldSize = 0x40;
        public static int idSize = 6 ;
        public static int fingerPrintSize = fieldSize - idSize;
        public static MD5CryptoServiceProvider my_md5 = new MD5CryptoServiceProvider();
        /*
        public AuthServer()
        {
            fieldSize = 0x40;
            idSize = 6;
            fingerPrintSize = fieldSize - idSize;
            p = BigInteger.Parse("12343166190249099963844614971858188890660252314196981857116019178266498099196944436576774068596061785922642532486910253402920733695804947624905546363393363");
            q = BigInteger.Parse("6171583095124549981922307485929094445330126157098490928558009589133249049598472218288387034298030892961321266243455126701460366847902473812452773181696681");
            g = BigInteger.Parse("6786183533171661594262558814260314083384466978959974116431945959977589683740679922916498942642093589315722172714694250494775306223775833010063525305963887");
            y = BigInteger.Parse("10064244825758094072963970119807064710181088090418826682084640363233124440395354645467056460109079832312427767666487651151696929816590482407358254249751172");
        }
        */
        public static byte[] hash(MD5CryptoServiceProvider md5, string id, int h_size)
        {
            byte[] h = md5.ComputeHash(Encoding.ASCII.GetBytes(id));
            h[h_size - 1] = (byte)(h[h_size - 1] & 0x7f);
            return h;
        }

        public static byte[] deriveData(string id, string fingerPrint)
        {
            if (!new Regex(@"^\d+$").IsMatch(id))
            {
                throw new ArgumentException("Incorrect id format.");
            }            
            byte[] buffer2 = Convert.FromBase64String(fingerPrint);
            if (buffer2.Length != fingerPrintSize)
            {
                throw new ArgumentException("Incorrect fingerprint size.");
            }
            byte[] destinationArray = new byte[fieldSize];
            byte[] h = hash(my_md5, id, idSize);
            Array.Copy(buffer2, 0, destinationArray, 0, fingerPrintSize);
            Array.Copy(h, 0, destinationArray, fingerPrintSize, idSize);
            //destinationArray[fieldSize - 1] = (byte)(destinationArray[fieldSize - 1] & 0x7f);
            return destinationArray;
        }
        public static bool dsa_vrf(BigInteger h, BigInteger r, BigInteger s)
        {
            BigInteger w = BigInteger.ModPow(s, q - 2, q);
            BigInteger u1 = (h * w) % q;
            BigInteger u2 = (r * w) % q;
            BigInteger v = ((BigInteger.ModPow(g, u1, p) * BigInteger.ModPow(y, u2, p)) % p) % q;
            return (r == v);
        }
        public static bool Verify(string id, string fingerPrint, string signature)
        {
            BigInteger h = new BigInteger(deriveData(id, fingerPrint));
            byte[] sourceArray = Convert.FromBase64String(signature);
            if (sourceArray.Length != (fieldSize * 2))
            {
                throw new ArgumentException("Incorrect signature size.");
            }
            byte[] destinationArray = new byte[fieldSize];
            byte[] buffer3 = new byte[fieldSize];
            Array.Copy(sourceArray, 0, destinationArray, 0, fieldSize);
            Array.Copy(sourceArray, fieldSize, buffer3, 0, fieldSize);
            BigInteger r = new BigInteger(destinationArray);
            BigInteger s = new BigInteger(buffer3);
            if (((r <= 0L) || (r >= q)) || ((s <= 0L) || (s >= q)))
            {
                return false;
            }
            return dsa_vrf(h, r, s);
        }

 

    }
}
