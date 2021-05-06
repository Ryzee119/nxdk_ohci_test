// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "cdc.h"

#define BAUD_RATE 115200
#define TX_BUFF_LEN 128
#define RX_BUFF_LEN 32

typedef struct cdc_info
{
    LINE_CODING_T *line_code;
    uint8_t rdata[RX_BUFF_LEN];
    uint8_t rdata_pos;
    uint8_t rstatus[RX_BUFF_LEN];
    uint8_t *tdata;
} cdc_info, *pcdc_info;

static void cdc_data_in_callback(CDC_DEV_T *cdev, uint8_t *rdata, int data_len)
{
    cdc_info *cdata = (cdc_info *)cdev->user_data;

    //Simple ring buffer
    for (uint32_t i = cdata->rdata_pos; i < (cdata->rdata_pos + data_len); i++)
    {
        cdata->rdata[i % RX_BUFF_LEN] = rdata[i - cdata->rdata_pos];
    }
    cdata->rdata_pos = cdata->rdata_pos + data_len - RX_BUFF_LEN;
}

static void cdc_status_in_callback(CDC_DEV_T *cdev, uint8_t *rdata, int data_len)
{
    cdc_info *cdata = (cdc_info *)cdev->user_data;
    uint32_t capped_len = (data_len < RX_BUFF_LEN) ? data_len : RX_BUFF_LEN;
    memcpy(cdata->rstatus, rdata, capped_len);
}

uint32_t cdc_init_device(CDC_DEV_T *cdev)
{
    //Get some memory
    if (cdev->user_data == NULL)
        cdev->user_data = malloc(sizeof(cdc_info));
    assert(cdev->user_data != NULL);
    memset(cdev->user_data, 0x00, sizeof(cdc_info));

    cdc_info *cdata = (cdc_info *)cdev->user_data;
    //Get a DMA friendly memory ranges
    cdata->line_code = usbh_alloc_mem(sizeof(LINE_CODING_T));
    cdata->tdata = usbh_alloc_mem(TX_BUFF_LEN);

    cdata->line_code->baud = BAUD_RATE;
    cdata->line_code->parity = 0;
    cdata->line_code->data_bits = 8;
    cdata->line_code->stop_bits = 0;

    int ret = 0;
    ret |= usbh_cdc_set_line_coding(cdev, cdata->line_code);
    ret |= usbh_cdc_set_control_line_state(cdev, 1, 1);
    ret |= usbh_cdc_start_polling_status(cdev, cdc_status_in_callback);
    ret |= usbh_cdc_start_to_receive_data(cdev, cdc_data_in_callback);

    return ret;
}

void cdc_connection_callback(CDC_DEV_T *cdev, int status)
{
    log_print("CDC device connected\n");
    cdc_init_device(cdev);
}

void cdc_disconnect_callback(CDC_DEV_T *cdev, int status)
{
    log_print("CDC device disconnected\n");
    if (cdev->user_data)
    {
        cdc_info *cdata = (cdc_info *)cdev->user_data;
        usbh_free_mem(cdata->line_code, sizeof(LINE_CODING_T));
        usbh_free_mem(cdata->tdata, TX_BUFF_LEN);

        if (cdev->user_data)
            free(cdev->user_data);
    }
}

uint32_t cdc_write_data(CDC_DEV_T *cdev, uint8_t *tdata, uint32_t data_len)
{
    if (cdev == NULL)
        return USBH_ERR_INVALID_PARAM;

    cdc_info *cdata = (cdc_info *)cdev->user_data;
    data_len = (data_len < TX_BUFF_LEN) ? data_len : TX_BUFF_LEN;
    memcpy(cdata->tdata, tdata, data_len);
    return usbh_cdc_send_data(cdev, cdata->tdata, data_len);
}

void cdc_print_all_rxdata()
{
    CDC_DEV_T *cdev = usbh_cdc_get_device_list();
    if (cdev == NULL)
        return;
    if (cdev->user_data == NULL)
        return;

    int32_t cdc_num = 0;

    //Create output test message
    static uint16_t j = 0;
    char msg[32];
    sprintf(msg, "Hello from nxdk %d\n", j++);

    while (cdev != NULL)
    {
        cdc_info *cdata = (cdc_info *)cdev->user_data;
        textview_print("CDC %s #%d. Sending periodic data... Rx data:\n", LV_SYMBOL_USB LV_SYMBOL_LOOP, cdc_num++);
        for (uint32_t i = 0; i < RX_BUFF_LEN; i++)
        {
            textview_print("%c", cdata->rdata[i]);
        }
        textview_print("\n\n");
        cdc_write_data(cdev, (uint8_t *)msg, strlen(msg));
        if (!cdev->rx_busy)
            usbh_cdc_start_to_receive_data(cdev, cdc_data_in_callback);
        cdev = cdev->next;
    }
}
