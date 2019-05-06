using System;
using System.Threading;
using System.IO.Ports;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace SampleSerial
{
    public class Program
    {
        static Cpu.Pin pinLED1 = (Cpu.Pin)80;
        static Cpu.Pin pinLED2 = (Cpu.Pin)81;
        static OutputPort portLED2 = new OutputPort(pinLED2, false);
        static private string PortName = Serial.COM1;
        static private OutputPort LEDPort;

        public static void Main()
        {
            Debug.Print("SampleSerial");
            Debug.Print("Please connect device COM1 to Host and press 1 and 0 keys?");
            Debug.Print("1: LED ON");
            Debug.Print("0: LED OFF");
            LEDPort = new OutputPort(pinLED1, false);    // LED on
            SerialPort comx = new SerialPort(
                PortName,
                115200,
                Parity.None,
                8,
                StopBits.One
            );
            comx.DataReceived += new SerialDataReceivedEventHandler(Serial_DataReceived);
            comx.Open();
            int i = 0;
            while (true)
            {
                Debug.Print("i=" + i.ToString());
                //byte[] data = System.Text.Encoding.UTF8.GetBytes(i.ToString());
                //comx.Write(data, 0, data.Length);
                comx.WriteByte(0x2d);
                portLED2.Write(false);
                Thread.Sleep(500);
                portLED2.Write(true);
                Thread.Sleep(500);
                i = i + 1;
            }
        }

        static void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort comx = (SerialPort)sender;
            byte[] key = new byte[1];
            int len = comx.Read(key, 0, key.Length);
            if (key[0] == '1')
            {
                LEDPort.Write(false);
            }
            else if (key[0] == '0')
            {
                LEDPort.Write(true);
            }
            comx.Write(key, 0, key.Length);
        }
    }
}
