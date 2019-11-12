/**
 * @file  cmdRW.h
 * @author Gustavo Díaz H - g.hernan.diaz@ing.uchile.cl
 * @date 2019
 * @copyright GNU GPL v3
 *
 * This header have definitions of commands related to reaction wheel driver
 * (RW) features.
 */

#ifndef CMD_RW_H
#define CMD_RW_H

#include "config.h"
#include "osDelay.h"
#include "i2c.h"

#ifdef LINUX
    #include <stdint.h>
    #include <fcntl.h>
    #include <sys/ioctl.h>
    #include <linux/i2c-dev.h>
    #include <errno.h>
    #include <unistd.h>
#endif

#define MAX_LEN 32
#define READ_LEN 2
#define OBC_ADDR 0x10
#define BIuC_ADDR 0x11

#define SAMPLE_SPEED_CODE_MOTOR1 21
#define SAMPLE_CURRENT_CODE_MOTOR1 22
#define SET_SPEED_CODE_MOTOR1 23

#define SAMPLE_SPEED_CODE_MOTOR2 24
#define SAMPLE_CURRENT_CODE_MOTOR2 25
#define SET_SPEED_CODE_MOTOR2 26

#define SAMPLE_SPEED_CODE_MOTOR3 27
#define SAMPLE_CURRENT_CODE_MOTOR3 28
#define SET_SPEED_CODE_MOTOR3 29

#define MOTOR1_ID 1
#define MOTOR2_ID 2
#define MOTOR3_ID 3

#define DATA_PACKET_SZ 8
typedef struct{
    float speed;
    float current;
}motor_data_t;

motor_data_t motor_data_;

#define CMD_PACKET_SZ 2
typedef struct{
    uint16_t data;
}data_t;

data_t data_;

#include "repoCommand.h"
#include "os/os.h"

/**
 * Register reaction whee related (rw) commands
 */
void cmd_rw_init(void);

int rw_sample_speed(char *fmt, char *params, int nparams);

/**
 * Request speed to the driver and store in their internal registers
 *
 * @param fmt Str. Parameters format ""
 * @param params Str. Parameters as string ""
 * @param nparams Int. Number of parameters 0
 * @return  CMD_OK if executed correctly or CMD_FAIL in case of errors
 */
uint8_t i2c_init(void);

/**
 * Init i2c library for raspberry
 */

int rw_sample_current(char *fmt, char *params, int nparams);
int rw_get_speed(char *fmt, char *params, int nparams);
int rw_get_current(char *fmt, char *params, int nparams);
int rw_set_speed(char *fmt, char *params, int nparams);

#endif /* CMD_RW_H */