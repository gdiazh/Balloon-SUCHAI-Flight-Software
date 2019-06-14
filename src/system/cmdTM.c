/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2018, Carlos Gonzalez Cortes, carlgonz@uchile.cl
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

#include "cmdTM.h"

static const char *tag = "cmdTM";

void cmd_tm_init(void)
{
    cmd_add("tm_parse_status", tm_parse_status, "", 0);
    cmd_add("tm_send_status", tm_send_status, "%d", 1);
    cmd_add("tm_send_last", tm_send_last, "%u %u", 2);
    cmd_add("tm_send_all", tm_send_all, "%u %u", 2);
    cmd_add("tm_set_ack", tm_set_ack, "%u %u", 2);
}

int tm_send_status(char *fmt, char *params, int nparams)
{
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    int dest_node;
    //Format: <node>
    if(nparams == sscanf(params, fmt, &dest_node))
    {
        com_data_t data;
        memset(&data, 0, sizeof(data));
        data.node = (uint8_t)dest_node;
        data.frame.nframe = 0;
        data.frame.type = TM_TYPE_STATUS;
        data.frame.ndata = 1;

        // Pack status variables to a structure
        dat_status_t status;
        dat_status_to_struct(&status);
        if(LOG_LEVEL >= LOG_LVL_DEBUG)
        {
            LOGD(tag, "Sending system status to node %d", dest_node)
            dat_print_status(&status);
        }

        // The total amount of status variables must fit inside a nframe
        LOGD(tag, "sizeof(status) = %lu", sizeof(status));
        LOGD(tag, "sizeof(data.nframe) = %lu", sizeof(data.frame));
        LOGD(tag, "sizeof(data.nframe.data) = %lu", sizeof(data.frame.data));
        assert(sizeof(status) < sizeof(data.frame.data));
        memcpy(data.frame.data.data8, &status, sizeof(status));

        return com_send_data("", (char *)&data, 0);
    }
    else
    {
        LOGW(tag, "Invalid args!");
        return CMD_FAIL;
    }
}

int tm_parse_status(char *fmt, char *params, int nparams)
{
    if(params == NULL)
        return CMD_ERROR;

    dat_status_t *status = (dat_status_t *)params;
    dat_print_status(status);

    return CMD_OK;
}

int tm_send_last(char *fmt, char *params, int nparams)
{
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    uint32_t dest_node;
    uint32_t payload;
    if(nparams == sscanf(params, fmt, &dest_node, &payload))
    {
        if(payload >= last_sensor) {
            return CMD_FAIL;
        }

        com_data_t data;
        memset(&data, 0, sizeof(data));
        data.node = (uint8_t)dest_node;
        data.frame.type = (uint16_t)(TM_TYPE_PAYLOAD + payload);

        int n_structs = (COM_FRAME_MAX_LEN) / data_map[payload].size;
        int index_pay = dat_get_system_var(data_map[payload].sys_index);
        data.frame.nframe = (uint16_t) (index_pay-n_structs);

        LOGI(tag, "index_payload: %d", index_pay);
        if(index_pay < n_structs) {
            n_structs = index_pay;
            data.frame.nframe = 0;
        }

        LOGI(tag, "Sending %d structs of payload %d", n_structs, (int)payload);
        data.frame.ndata = (uint32_t)n_structs;

        int i;
        int mem_delay;
        uint16_t payload_size = data_map[payload].size;
        for(i=0; i < n_structs; ++i) {
            char buff[data_map[payload].size];
            dat_get_recent_payload_sample(buff, payload, n_structs-1-i);
            mem_delay = (i*payload_size);
            memcpy(data.frame.data.data8+mem_delay, buff, payload_size);
        }
        print_buff(data.frame.data.data8, payload_size*n_structs);

        return com_send_data("", (char *)&data, 0);
    }
    else
    {
        LOGW(tag, "Invalid args!");
        return CMD_FAIL;
    }
}

int tm_send_all(char *fmt, char *params, int nparams)
{
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    uint32_t dest_node;
    uint32_t payload;

    if(nparams == sscanf(params, fmt, &payload, &dest_node)) {

        if(payload >= last_sensor) {
            return CMD_FAIL;
        }

        int index_pay = dat_get_system_var(data_map[payload].sys_index);
        int structs_per_frame = (COM_FRAME_MAX_LEN) / data_map[payload].size;
        uint16_t payload_size = data_map[payload].size;

        int n_frames = (index_pay/structs_per_frame);
        if(index_pay % structs_per_frame != 0) {
            n_frames += 1;
        }
        int i;
        for(i=0; i < n_frames; ++i) {
            com_data_t data;
            memset(&data, 0, sizeof(data));
            data.node = (uint8_t)dest_node;
            data.frame.nframe = (uint16_t) i;
            data.frame.type = (uint16_t)(TM_TYPE_PAYLOAD + payload);
            data.frame.ndata = (uint32_t)structs_per_frame;

            int j;
            for(j=0; j < structs_per_frame; ++j) {
                if(i*structs_per_frame+j >= index_pay) {
                    data.frame.ndata = (uint32_t) j;
                    break;
                }

                char buff[payload_size];
                dat_get_recent_payload_sample(buff, payload, i*structs_per_frame+j);
                int mem_delay = (j*payload_size);
                memcpy(data.frame.data.data8+mem_delay, buff, payload_size);
            }

            LOGI(tag, "Sending %d structs of payload %d", data.frame.ndata, (int)payload);
            com_send_data("", (char *)&data, 0);

            print_buff(data.frame.data.data8, payload_size*structs_per_frame);
        }
        return CMD_OK;
    }
}

int tm_set_ack(char *fmt, char *params, int nparams) {
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    uint32_t payload;
    uint32_t k_samples;

    if(nparams == sscanf(params, fmt, &payload, &k_samples)) {

        if(payload >= last_sensor) {
            LOGE(tag, "payload not found")
            return CMD_FAIL;
        }

        if(k_samples < 1) {
            LOGE(tag, "could not acknowledge %d", k_samples)
            return CMD_FAIL;
        }

        int ack_pay = dat_get_system_var(data_map[payload].sys_ack);
        int index_pay = dat_get_system_var(data_map[payload].sys_index);

        if(ack_pay== -1 || index_pay==-1) {
            LOGE(tag, "something bad happen");
            return CMD_FAIL;
        }

        ack_pay += k_samples;
        if(ack_pay > index_pay) {
            ack_pay = index_pay;
        }

        dat_set_system_var(data_map[payload].sys_ack ,ack_pay);
        CMD_OK;
    }
}
