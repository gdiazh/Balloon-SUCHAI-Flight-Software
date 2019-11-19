/*!
 * @file Adafruit_BMP3XX.h
 *
 * Adafruit BMP3XX temperature & barometric pressure sensor driver
 *
 * This is the documentation for Adafruit's BMP3XX driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit BMP388 breakout: https://www.adafruit.com/products/3966
 *
 * These sensors use I2C or SPI to communicate
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#ifndef __BMP3XX_H__
#define __BMP3XX_H__

#include "bmp3.h"
#include "i2c.h"
#include "math.h"
#include "osDelay.h"



/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define BMP3XX_DEFAULT_ADDRESS       (0x76)     ///< The default I2C address

/** Adafruit_BMP3XX Class for both I2C and SPI usage.
 *  Wraps the Bosch library for Arduino usage
 */

int bmp3_begin(uint8_t addr);
float bmp3_readTemperature(void);
float bmp3_readPressure(void);
float bmp3_readAltitude(float seaLevel);

int bmp3_setTemperatureOversampling(uint8_t os);
int bmp3_setPressureOversampling(uint8_t os);
int bmp3_setIIRFilterCoeff(uint8_t fs);
int bmp3_setOutputDataRate(uint8_t odr);

/// Perform a reading in blocking mode
int bmp3_performReading(void);

/// Temperature (Celsius) assigned after calling performReading()
double bmp3_temperature;
/// Pressure (Pascals) assigned after calling performReading()
double bmp3_pressure;

int bmp3_filterEnabled, bmp3_tempOSEnabled, bmp3_presOSEnabled, bmp3_ODREnabled;
uint8_t bmp3_i2caddr;
int32_t bmp3_sensorID;
unsigned long bmp3_meas_end;

struct bmp3_dev the_sensor;

#endif
