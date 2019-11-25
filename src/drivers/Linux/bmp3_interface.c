/*!
 * @info Adapted from
 *
 * @file Adafruit_BMP3XX.cpp
 *
 * @mainpage Adafruit BMP3XX temperature & barometric pressure sensor driver
 *
 * @section intro_sec Introduction
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
 * @section author Author
 *
 * Written by Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "bmp3_interface.h"

//#define BMP3XX_DEBUG

char *I2CDeviceName = (char*)"/dev/i2c-1";

// Our hardware interface functions
//static int8_t bmp3_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
//static int8_t bmp3_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
static void delay_msec(uint32_t ms);

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Instantiates sensor with Hardware SPI or I2C.
    @param  cspin SPI chip select. If not passed in, I2C will be used
*/
/**************************************************************************/


/**************************************************************************/
/*!
    @brief Initializes the sensor

    Hardware ss initialized, verifies it is in the I2C or SPI bus, then reads
    calibration data in preparation for sensor reads.

    @param  addr Optional parameter for the I2C address of BMP3. Default is 0x77
    @param  theWire Optional parameter for the I2C device we will use. Default is "Wire"
    @return True on sensor initialization success. False on failure.
*/
/**************************************************************************/
int bmp3_begin(uint8_t addr) {
    bmp3_i2caddr = addr;

    the_sensor.dev_id = addr;
    the_sensor.intf = BMP3_I2C_INTF;
    the_sensor.read = &bmp3_i2c_read;
    the_sensor.write = &bmp3_i2c_write;

    the_sensor.delay_ms = delay_msec;

    int8_t rslt = BMP3_OK;
    rslt = bmp3_init(&the_sensor);
#ifdef BMP3XX_DEBUG
    printf("bmp3_interface Begin BMP3 Result: %d\n", rslt);
#endif

    if (rslt != BMP3_OK)
        return 0;

#ifdef BMP3XX_DEBUG
  printf("bmp3_interface T1 = %d\n", the_sensor.calib_data.reg_calib_data.par_t1);
  printf("bmp3_interface T2 = %d\n", the_sensor.calib_data.reg_calib_data.par_t2);
  printf("bmp3_interface T3 = %d\n", the_sensor.calib_data.reg_calib_data.par_t3);
  printf("bmp3_interface P1 = %d\n", the_sensor.calib_data.reg_calib_data.par_p1);
  printf("bmp3_interface P2 = %d\n", the_sensor.calib_data.reg_calib_data.par_p2);
  printf("bmp3_interface P3 = %d\n", the_sensor.calib_data.reg_calib_data.par_p3);
  printf("bmp3_interface P4 = %d\n", the_sensor.calib_data.reg_calib_data.par_p4);
  printf("bmp3_interface P5 = %d\n", the_sensor.calib_data.reg_calib_data.par_p5);
  printf("bmp3_interface P6 = %d\n", the_sensor.calib_data.reg_calib_data.par_p6);
  printf("bmp3_interface P7 = %d\n", the_sensor.calib_data.reg_calib_data.par_p7);
  printf("bmp3_interface P8 = %d\n", the_sensor.calib_data.reg_calib_data.par_p8);
  printf("bmp3_interface P9 = %d\n", the_sensor.calib_data.reg_calib_data.par_p9);
  printf("bmp3_interface P10 = %d\n", the_sensor.calib_data.reg_calib_data.par_p10);
  printf("bmp3_interface P11 = %d\n", the_sensor.calib_data.reg_calib_data.par_p11);
  //printf("bmp3_interface T  = %d\n", the_sensor.calib_data.reg_calib_data.t_lin);
#endif

    bmp3_setTemperatureOversampling(BMP3_NO_OVERSAMPLING);
    bmp3_setPressureOversampling(BMP3_NO_OVERSAMPLING);
    bmp3_setIIRFilterCoeff(BMP3_IIR_FILTER_DISABLE);

    // don't do anything till we request a reading
    the_sensor.settings.op_mode = BMP3_FORCED_MODE;

    return 1;
}


/**************************************************************************/
/*!
    @brief Performs a reading and returns the ambient temperature.
    @return Temperature in degrees Centigrade
*/
/**************************************************************************/
float bmp3_readTemperature(void) {
    bmp3_performReading();
    return bmp3_temperature;
}


/**************************************************************************/
/*!
    @brief Performs a reading and returns the barometric pressure.
    @return Barometic pressure in Pascals
*/
/**************************************************************************/
float bmp3_readPressure(void) {
    bmp3_performReading();
    return bmp3_pressure;
}

/**************************************************************************/
/*!
    @brief Calculates the altitude (in meters).

    Reads the current atmostpheric pressure (in hPa) from the sensor and calculates
    via the provided sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @return Altitude in meters
*/
/**************************************************************************/
float bmp3_readAltitude(float seaLevel)
{
    // Equation taken from BMP180 datasheet (page 16):
    //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

    // Note that using the equation from wikipedia can give bad results
    // at high altitude. See this thread for more information:
    //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

    float atmospheric = bmp3_readPressure() / 100.0F;
    return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    @brief Performs a full reading of all sensors in the BMP3XX.

    Assigns the internal Adafruit_BMP3XX#temperature & Adafruit_BMP3XX#pressure member variables

    @return True on success, False on failure
*/
/**************************************************************************/
int bmp3_performReading(void) {
    int8_t rslt;
    /* Used to select the settings user needs to change */
    uint16_t settings_sel = 0;
    /* Variable used to select the sensor component */
    uint8_t sensor_comp = 0;

    /* Select the pressure and temperature sensor to be enabled */
    the_sensor.settings.temp_en = BMP3_ENABLE;
    settings_sel |= BMP3_TEMP_EN_SEL;
    sensor_comp |= BMP3_TEMP;
    if (bmp3_tempOSEnabled) {
        settings_sel |= BMP3_TEMP_OS_SEL;
    }

    the_sensor.settings.press_en = BMP3_ENABLE;
    settings_sel |= BMP3_PRESS_EN_SEL;
    sensor_comp |= BMP3_PRESS;
    if (bmp3_presOSEnabled) {
        settings_sel |= BMP3_PRESS_OS_SEL ;
    }

    if (bmp3_filterEnabled) {
        settings_sel |= BMP3_IIR_FILTER_SEL;
    }

    if (bmp3_ODREnabled) {
        settings_sel |= BMP3_ODR_SEL;
    }

    // set interrupt to data ready
    //settings_sel |= BMP3_DRDY_EN_SEL | BMP3_LEVEL_SEL | BMP3_LATCH_SEL;

    /* Set the desired sensor configuration */
#ifdef BMP3XX_DEBUG
    printf("bmp3_interface Setting sensor settings\n");
#endif
    rslt = bmp3_set_sensor_settings(settings_sel, &the_sensor);
    if (rslt != BMP3_OK)
        return 0;

    /* Set the power mode */
    the_sensor.settings.op_mode = BMP3_FORCED_MODE;
#ifdef BMP3XX_DEBUG
    printf("bmp3_interface Setting power mode\n");
#endif
    rslt = bmp3_set_op_mode(&the_sensor);
    if (rslt != BMP3_OK)
        return 0;

    /* Variable used to store the compensated data */
    struct bmp3_data data;

    /* Temperature and Pressure data are read and stored in the bmp3_data instance */
    rslt = bmp3_get_sensor_data(sensor_comp, &data, &the_sensor);

    /* Save the temperature and pressure data */
    bmp3_temperature = data.temperature;
    bmp3_pressure = data.pressure;
    if (rslt != BMP3_OK)
        return 0;

    return 1;
}


/**************************************************************************/
/*!
    @brief  Setter for Temperature oversampling
    @param  oversample Oversampling setting, can be BMP3_NO_OVERSAMPLING, BMP3_OVERSAMPLING_2X, BMP3_OVERSAMPLING_4X, BMP3_OVERSAMPLING_8X, BMP3_OVERSAMPLING_16X, BMP3_OVERSAMPLING_32X
    @return True on success, False on failure
*/
/**************************************************************************/

int bmp3_setTemperatureOversampling(uint8_t oversample) {
    if (oversample > BMP3_OVERSAMPLING_32X) return 0;

    the_sensor.settings.odr_filter.temp_os = oversample;

    if (oversample == BMP3_NO_OVERSAMPLING)
        bmp3_tempOSEnabled = 0;
    else
        bmp3_tempOSEnabled = 1;

    return 1;
}


/**************************************************************************/
/*!
    @brief  Setter for Pressure oversampling
    @param  oversample Oversampling setting, can be BMP3_NO_OVERSAMPLING, BMP3_OVERSAMPLING_2X, BMP3_OVERSAMPLING_4X, BMP3_OVERSAMPLING_8X, BMP3_OVERSAMPLING_16X, BMP3_OVERSAMPLING_32X
    @return True on success, False on failure
*/
/**************************************************************************/
int bmp3_setPressureOversampling(uint8_t oversample) {
    if (oversample > BMP3_OVERSAMPLING_32X) return 0;

    the_sensor.settings.odr_filter.press_os = oversample;

    if (oversample == BMP3_NO_OVERSAMPLING)
        bmp3_presOSEnabled = 0;
    else
        bmp3_presOSEnabled = 1;

    return 1;
}

/**************************************************************************/
/*!
    @brief  Setter for IIR filter coefficient
    @param filtercoeff Coefficient of the filter (in samples). Can be BMP3_IIR_FILTER_DISABLE (no filtering), BMP3_IIR_FILTER_COEFF_1, BMP3_IIR_FILTER_COEFF_3, BMP3_IIR_FILTER_COEFF_7, BMP3_IIR_FILTER_COEFF_15, BMP3_IIR_FILTER_COEFF_31, BMP3_IIR_FILTER_COEFF_63, BMP3_IIR_FILTER_COEFF_127
    @return True on success, False on failure

*/
/**************************************************************************/
int bmp3_setIIRFilterCoeff(uint8_t filtercoeff) {
    if (filtercoeff > BMP3_IIR_FILTER_COEFF_127) return 0;

    the_sensor.settings.odr_filter.iir_filter = filtercoeff;

    if (filtercoeff == BMP3_IIR_FILTER_DISABLE)
        bmp3_filterEnabled = 0;
    else
        bmp3_filterEnabled = 1;

    return 1;
}

/**************************************************************************/
/*!
    @brief  Setter for output data rate (ODR)
    @param odr Sample rate in Hz. Can be BMP3_ODR_200_HZ, BMP3_ODR_100_HZ, BMP3_ODR_50_HZ, BMP3_ODR_25_HZ, BMP3_ODR_12_5_HZ, BMP3_ODR_6_25_HZ, BMP3_ODR_3_1_HZ, BMP3_ODR_1_5_HZ, BMP3_ODR_0_78_HZ, BMP3_ODR_0_39_HZ, BMP3_ODR_0_2_HZ, BMP3_ODR_0_1_HZ, BMP3_ODR_0_05_HZ, BMP3_ODR_0_02_HZ, BMP3_ODR_0_01_HZ, BMP3_ODR_0_006_HZ, BMP3_ODR_0_003_HZ, or BMP3_ODR_0_001_HZ
    @return True on success, False on failure

*/
/**************************************************************************/
int bmp3_setOutputDataRate(uint8_t odr) {
    if (odr > BMP3_ODR_0_001_HZ) return 0;

    the_sensor.settings.odr_filter.odr = odr;

    bmp3_ODREnabled = 1;

    return 1;
}


static void delay_msec(uint32_t ms){
    usleep(ms*1000);
}

/**************************************************************************/
/*!
    @brief  Reads 8 bit values over I2C
*/
/**************************************************************************/
int8_t bmp3_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int i2cHandle;
    uint8_t buf_len;
    if ((i2cHandle = open(I2CDeviceName, O_RDWR)) < 0)
    {
        printf("error opening I2C\n");
        return 1;
    }
    else
    {
        if (ioctl(i2cHandle, I2C_SLAVE, dev_id) < 0) {
            printf("Error at ioctl\n");
            // Close the i2c device bus
            close(*I2CDeviceName);
            return 1;
        }
        else
        {
            buf_len = sizeof(reg_data);
            memset(reg_data, 0, len);
            char wbuf[1];
            wbuf[0] = reg_addr;
            int res_tmp = write(i2cHandle, wbuf, 1);
            uint8_t bytes_r = read(i2cHandle, reg_data, len);
            #ifdef BMP3XX_DEBUG
                printf("[bmp3 i2c read] res_tmp: %d\n", res_tmp);
                printf("bmp3_i2c_read: [");
                for (int i = 0; i < len; i++) {
                    printf("%d,", reg_data[i]);
                }
                printf("]\n");
            #endif
            if (bytes_r != len)
            {
                perror("Failed to read from the i2c bus");
                return 1;
            }
        }
        // Close the i2c device bus
        close(*I2CDeviceName);
        return 0;
    }
}

/**************************************************************************/
/*!
    @brief  Writes 8 bit values over I2C
*/
/**************************************************************************/
int8_t bmp3_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int i2cHandle;
    int res_tmp;
    int res_tmp2;
    if ((i2cHandle = open(I2CDeviceName, O_RDWR)) < 0)
    {
        printf("error opening I2C\n");
        return 1;
    }
    else
    {
        if (ioctl(i2cHandle, I2C_SLAVE, dev_id) < 0)
        {
            printf("Error at ioctl\n");
            // Close the i2c device bus
            close(*I2CDeviceName);
            return 1;
        }
        else
        {
            uint8_t wbuf[len+1];
            #ifdef BMP3XX_DEBUG
                printf("data to write: [");
                for (int i = 0; i < len; i++) {
                    printf("%d,", reg_data[i]);
                }
                printf("]\n");
            #endif
            memcpy(wbuf, &reg_addr, sizeof(uint8_t));
            memcpy(wbuf+1, reg_data, len*sizeof(uint8_t));
            res_tmp2 = write(i2cHandle, wbuf, len+1);
            #ifdef BMP3XX_DEBUG
                printf("addr+data to write: [");
                for (int i = 0; i < len+1; i++) {
                    printf("%d,", wbuf[i]);
                }
                printf("]\n");
                printf("[bmp3 i2c write] res_tmp2: %d\n", res_tmp2);
            #endif
            if (res_tmp2 != len+1) {
                printf("[bmp3_i2c]Fail to write %d bytes\n", len - res_tmp2);
                return 1;
            }
        }
    }
    // Close the i2c device bus
    close(*I2CDeviceName);
    return 0;
}