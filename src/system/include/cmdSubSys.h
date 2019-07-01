//
// Created by kaminari on 10-07-18.
//

#ifndef SUCHAI_FLIGHT_SOFTWARE_CMDSUBSYS_H
#define SUCHAI_FLIGHT_SOFTWARE_CMDSUBSYS_H

#include "config.h"
#include "repoCommand.h"

/**
 * Register on board computer related (OBC) commands
 */
void cmd_subsys_init(void);

int get_gps_data(char *fmt, char *params, int nparams);
int get_dpl_data(char *fmt, char *params, int nparams);
int get_prs_data(char *fmt, char *params, int nparams);
int open_dpl_1(char *fmt, char *params, int nparams);
int open_dpl_2(char *fmt, char *params, int nparams);
int open_dpl_sm(char *fmt, char *params, int nparams);
int close_dpl_sm(char *fmt, char *params, int nparams);
int send_iridium_data(char *fmt, char *params, int nparams);
int send_iridium_msg1(char *fmt, char *params, int nparams);
int send_iridium_msg2(char *fmt, char *params, int nparams);
int run_sstv(char *fmt, char *params, int nparams);
int send_xbee_data(char *fmt, char *params, int nparams);
int send_sun_data(char *fmt, char *params, int nparams);

#endif //SUCHAI_FLIGHT_SOFTWARE_CMDSUBSYS_H
