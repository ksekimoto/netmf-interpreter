﻿// Important
// Should chagne board definition in Max72197221.cs Library!!!
//
//#define CQ_FRK_FM3
//#define CQFM3DUINO
//#define CQ_FRK_RX62N
//#define CQ_FRK_NXP_ARM
//#define KS_MB9BF506
//#define KS_MB9BF568
//#define GR_SAKURA
#define GR_ROSE

using System;
using System.Collections;
using System.Threading;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
//using SecretLabs.NETMF.Hardware;
//using SecretLabs.NETMF.Hardware.Netduino;
using netduino.helpers.Hardware;

namespace SampleMax7219
{
    /*
    Copyright (C) 2011 by Fabien Royer
    
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
    */
    public class Program
    {
        private static Max72197221 _max;
        private static ArrayList _displayList;
        private static byte _intensity = 0;

        public static void Main()
        {
            InitializeSpinnerDisplay();

            _max = new Max72197221();

            DisplayTestMode();

            ShutdownTestMode();

            //DigitScanLimitTest();

            //DigitDecodeTest();

            StringToDigitTest();

            var spinnerThread = new Thread(new ThreadStart(Spinner));
            spinnerThread.Start();

            var IntensityThread = new Thread(new ThreadStart(IntensityTest));
            IntensityThread.Start();

            WaitForEver();
        }

        private static void WaitForEver()
        {
            while (true)
            {
                Thread.Sleep(10000);
            }
        }

        private static void StringToDigitTest()
        {
            _max.DecodeMode = Max72197221.DecodeModeRegister.DecodeDigitAll;
            _max.Display("        ");
            Thread.Sleep(1000);
            _max.Display("12345678");
            Thread.Sleep(3000);
            _max.Display("-6.5.4.3.2.1.0.");
            Thread.Sleep(3000);
            _max.Display("z-z-z-z-");
            Thread.Sleep(3000);
            _max.Display("-.-.-. . . . .");
            Thread.Sleep(3000);
            _max.DecodeMode = Max72197221.DecodeModeRegister.NoDecodeMode;
        }

        private static void DigitDecodeTest()
        {
            _max.DecodeMode = Max72197221.DecodeModeRegister.DecodeDigitAll;
            _max.Display(Max72197221.RegisterAddressMap.Digit0, Max72197221.CodeBFont.Zero, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit1, Max72197221.CodeBFont.One, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit2, Max72197221.CodeBFont.Two, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit3, Max72197221.CodeBFont.Three, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit4, Max72197221.CodeBFont.Four, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit5, Max72197221.CodeBFont.Five, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit6, Max72197221.CodeBFont.Six, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit7, Max72197221.CodeBFont.Seven, Max72197221.CodeBDecimalPoint.OFF);
            Thread.Sleep(4000);
            _max.Display(Max72197221.RegisterAddressMap.Digit0, Max72197221.CodeBFont.Eight, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit1, Max72197221.CodeBFont.Nine, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit2, Max72197221.CodeBFont.Dash, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit3, Max72197221.CodeBFont.E, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit4, Max72197221.CodeBFont.H, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit5, Max72197221.CodeBFont.L, Max72197221.CodeBDecimalPoint.OFF);
            _max.Display(Max72197221.RegisterAddressMap.Digit6, Max72197221.CodeBFont.P, Max72197221.CodeBDecimalPoint.ON);
            _max.Display(Max72197221.RegisterAddressMap.Digit7, Max72197221.CodeBFont.Blank, Max72197221.CodeBDecimalPoint.OFF);
            Thread.Sleep(4000);
            _max.DecodeMode = Max72197221.DecodeModeRegister.NoDecodeMode;
        }

        private static void ShutdownTestMode()
        {
            _max.DecodeMode = Max72197221.DecodeModeRegister.NoDecodeMode;
            _max.DigitScanLimit = 7;
            _max.Intensity = 3;

            _max.Display(new byte[] { 255, 129, 189, 165, 165, 189, 129, 255 });

            for (int I = 0; I < 3; I++)
            {
                Thread.Sleep(300);
                _max.Shutdown = Max72197221.ShutdownRegister.ShutdownMode;
                Thread.Sleep(300);
                _max.Shutdown = Max72197221.ShutdownRegister.NormalOperation;
            }
        }

        private static void DigitScanLimitTest()
        {
            _max.DigitScanLimitSafety = false;
            _max.Intensity = 1;

            for (int I = 0; I < 3; I++)
            {
                byte limit = 7;
                for (; limit > 1; limit--)
                {
                    _max.DigitScanLimit = limit;
                    Thread.Sleep(80);
                }
                for (; limit < 8; limit++)
                {
                    _max.DigitScanLimit = limit;
                    Thread.Sleep(80);
                }
            }
            _max.DigitScanLimitSafety = true;
            _max.Intensity = 3;
        }

        private static void DisplayTestMode()
        {
            _max.DisplayTest = Max72197221.DisplayTestRegister.DisplayTestMode;
            Thread.Sleep(4000);
            _max.DisplayTest = Max72197221.DisplayTestRegister.NormalOperation;
        }

        private static void InitializeSpinnerDisplay()
        {
            _displayList = new ArrayList();
            _displayList.Add(new byte[] { 1, 2, 4, 8, 16, 32, 64, 128 });
            _displayList.Add(new byte[] { 0, 0, 0, 255, 0, 0, 0, 0 });
            _displayList.Add(new byte[] { 128, 64, 32, 16, 8, 4, 2, 1 });
            _displayList.Add(new byte[] { 16, 16, 16, 16, 16, 16, 16, 16 });
        }

        private static void IntensityTest()
        {
            while (true)
            {
                for (_intensity = 0; _intensity <= 15; _intensity++)
                {
                    Thread.Sleep(80);
                }
                for (_intensity = 15; _intensity != 255; _intensity--)
                {
                    Thread.Sleep(80);
                }
            }
        }

        private static void Spinner()
        {
            while (true)
            {
                foreach (byte[] matrix in _displayList)
                {
                    _max.Display(matrix);
                    _max.Intensity = _intensity;
                    Thread.Sleep(80);
                }
            }
        }
    }

}