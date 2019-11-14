/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2018, Carlos Gonzalez Cortes, carlgonz@ug.uchile.cl
 *      Copyright 2018, Tomas Opazo Toro, tomas.opazo.t@gmail.com
 *      Copyright 2018, Matias Ramirez Martinez, nicoram.mt@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmdMCP9808.h"

static const char* tag = "cmdMCP9808";

void cmd_mcp9808_init(void)
{
    #ifdef LINUX
        cmd_add("mcp9808_init", mcp9808_init, "", 0);
    #endif

    #ifdef NANOMIND
        //TO DO
    #endif
}

int mcp9808_init(char *fmt, char *params, int nparams)
{
    LOGI(tag, "mcp9808 Init");

    if (read16(MCP9808_REG_MANUF_ID) != 0x0054)
        return 0;
    if (read16(MCP9808_REG_DEVICE_ID) != 0x0400)
        return 0;

    write16(MCP9808_REG_CONFIG, 0x0);
    return 1;
}

/*!
 *   @brief  Reads the 16-bit temperature register and returns the Centigrade
 *           temperature as a float.
 *   @return Temperature in Centigrade.
 */
float mcp9808_readTempC() {
    float temp = strtod("NaN", NULL);
    uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

    if (t != 0xFFFF) {
        temp = t & 0x0FFF;
        temp /= 16.0;
        if (t & 0x1000)
            temp -= 256;
    }

    return temp;
}

/*!
 *   @brief  Reads the 16-bit temperature register and returns the Fahrenheit
 *           temperature as a float.
 *   @return Temperature in Fahrenheit.
 */
float mcp9808_readTempF() {
    float temp = strtod("NaN", NULL);
    uint16_t t = read16(MCP9808_REG_AMBIENT_TEMP);

    if (t != 0xFFFF) {
        temp = t & 0x0FFF;
        temp /= 16.0;
        if (t & 0x1000)
            temp -= 256;

        temp = temp * 9.0 / 5.0 + 32;
    }

    return temp;
}

/*!
 *   @brief  Set Sensor to Shutdown-State or wake up (Conf_Register BIT8)
 *   @param  sw true = shutdown / false = wakeup
 */
void mcp9808_shutdown_wake(uint8_t sw) {
    uint16_t conf_shutdown;
    uint16_t conf_register = read16(MCP9808_REG_CONFIG);
    if (sw == 1) {
        conf_shutdown = conf_register | MCP9808_REG_CONFIG_SHUTDOWN;
        write16(MCP9808_REG_CONFIG, conf_shutdown);
    }
    if (sw == 0) {
        conf_shutdown = conf_register & ~MCP9808_REG_CONFIG_SHUTDOWN;
        write16(MCP9808_REG_CONFIG, conf_shutdown);
    }
}

/*!
 *   @brief  Shutdown MCP9808
 */
void mcp9808_shutdown(){
    mcp9808_shutdown_wake(1);
}

/*!
 *   @brief  Wake up MCP9808
 */
void mcp9808_wake() {
    mcp9808_shutdown_wake(0);
    osDelay(250);
}

/*!
 *   @brief  Get Resolution Value
 *   @return Resolution value
 */
uint8_t mcp9808_getResolution() {
    return read8(MCP9808_REG_RESOLUTION);
}

/*!
 *   @brief  Set Resolution Value
 *   @param  value
 */
void mcp9808_setResolution(uint8_t value) {
    write8(MCP9808_REG_RESOLUTION, value & 0x03);
}
