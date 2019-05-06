using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace SampleI2CTMP102
{
    class TMP102
    {
        private I2CDevice i2c;
        private int timeout = 1000;
        private byte[] data = new byte[2];
        private I2CDevice.I2CTransaction[] transaction;

        public int Read()
        {
            int temperature = 0;
            int result = i2c.Execute(transaction, timeout);
            if (result == 2)
            {
                temperature = ((int)data[0] << 4) + ((int)data[1] >> 4);
                return temperature;
            }
            else
                return 0;
        }

        public TMP102(int clockRate, int timeout)
        {
            this.timeout = timeout;
            i2c = new I2CDevice(new I2CDevice.Configuration(0x48, clockRate));
            transaction = new I2CDevice.I2CTransaction[] { I2CDevice.CreateReadTransaction(data) };
        }

    }
}