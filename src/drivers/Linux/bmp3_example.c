/***************************************************************************
  This is a library for the BMP3XX temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP388 Breakout
  ----> http://www.adafruit.com/products/3966

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include "bmp3_interface.h"

#define SEALEVELPRESSURE_HPA (1013.25)

int main()
{
    printf("BMP388 test\n");
    int rslt = bmp3_begin(0x76);

    if (!rslt) {
        printf("Could not find a valid BMP3 sensor, check wiring!\n");
        while (1);
    }

    // Set up oversampling and filter initialization
    bmp3_setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp3_setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp3_setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    //bmp3_setOutputDataRate(BMP3_ODR_50_HZ);

    while (1) {
        if (!bmp3_performReading()) {
            printf("Failed to perform reading :(\n");
            return;
        }
        printf("Temperature = %f *C\n", bmp3_temperature);

        printf("Pressure = %f hPa\n", bmp3_pressure / 100.0);

        printf("Approx. Altitude = %f m\n", bmp3_readAltitude(SEALEVELPRESSURE_HPA));

        usleep(2000e3);
    }
}