#define GR_ROSE

using System;
using System.Threading;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace SampleAD
{
    public class Program
    {
#if GR_ROSE
        // LED1 = PA0,
        // LED2 = PA1,
        static Cpu.Pin LEDpin = (Cpu.Pin)80;                    // PA0 - LED
        static Cpu.AnalogChannel ch0 = (Cpu.AnalogChannel)0;    // Analog 0
#endif

        public static void Main()
        {
            OutputPort LED = new OutputPort(LEDpin, false);
            AnalogInput aiVR2 = new AnalogInput(ch0);
            Int32 i = 0;
            while (true)
            {
                Debug.Print(i.ToString() + " times " +
                            "Slide Resistor = " + aiVR2.ReadRaw().ToString()
                            );
                LED.Write(false);
                Thread.Sleep(500);
                LED.Write(true);
                Thread.Sleep(500);
                i++;
            }
        }

    }
}
