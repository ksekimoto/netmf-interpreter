using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.IO.Ports;
using System.Threading;

namespace SampleSerialServo
{
    public class Program
    {
        static int SetPos(string PortName, int pin_no, int id, int pos)
        {
            SerialPort comx = new SerialPort(PortName, 115200, Parity.Even, 8, StopBits.One);
            Cpu.Pin pin = (Cpu.Pin)pin_no;
            OutputPort port = new OutputPort(pin, true);

            byte[] tx = new byte[3]; // unsigned char tx[3]
            byte[] rx = new byte[6]; // unsigned char Rx[6]
            int dat;

            tx[0] = (byte)(0x80 | id);       // CMD
            tx[1] = (byte)(pos >> 7 & 0x7F); // POS_H
            tx[2] = (byte)(pos & 0x7f);      // POS_L

            comx.Open();
            comx.Flush();
            comx.Write(tx, 0, 3);

            int i = 0;
            int read_count = 0;
            while (read_count < rx.Length)
            {
                i = comx.Read(rx, i, rx.Length - read_count);
                read_count += i;
                Thread.Sleep(5);
            }
            dat = (int)rx[4];
            dat = (dat << 7) + (int)rx[5];

            return dat;
        }

        public static void Main()
        {
            int i = 0;
            int dat;
            Debug.Print(Resources.GetString(Resources.StringResources.String1));
            while (i <= 5)
            {
                dat = SetPos(Serial.COM1, 18, 0, 7500);
                Debug.Print("Returned Position: " + dat.ToString());
                Thread.Sleep(1000);
                dat = SetPos(Serial.COM1, 18, 0, 8000);
                Thread.Sleep(1000);
                Debug.Print("Returned Position: " + dat.ToString());
                i++;
            }
        }
    }
}
