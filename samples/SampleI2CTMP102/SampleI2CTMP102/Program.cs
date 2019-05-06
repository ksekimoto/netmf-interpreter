using System;
using System.Threading;
using Microsoft.SPOT;

namespace SampleI2CTMP102
{
    public class Program
    {
        public static void Main()
        {
            TMP102 tmp102 = new TMP102(400, 1000);
            while (true)
            {
                int temp = tmp102.Read() / 16;
                Debug.Print("Temp = " + temp.ToString());
                Thread.Sleep(2000);
            }
        }

    }
}
