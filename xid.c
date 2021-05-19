// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "xid.h"

#define MAX_PACKET 32

static void xid_int_read_callback(UTR_T *utr)
{
    //log_print("xid_int_read_callback\n");
    xid_dev_t *xid_dev = (xid_dev_t *)utr->context;
    uint32_t data_len = utr->xfer_len;
    memcpy(xid_dev->user_data, utr->buff, data_len <= MAX_PACKET ? data_len : MAX_PACKET);
    usbh_xid_read(xid_dev, 0, xid_int_read_callback);
}

uint32_t xid_init_device(xid_dev_t *xid_dev)
{
    xid_dev->user_data = malloc(MAX_PACKET);
    assert(xid_dev->user_data != NULL);
    return usbh_xid_read(xid_dev, 0, xid_int_read_callback);
}

void xid_connection_callback(xid_dev_t *xid_dev, int status)
{
    log_print("XID connected: VID: %04x PID: %04X\n", xid_dev->idVendor, xid_dev->idProduct);
    xid_init_device(xid_dev);
}

void xid_disconnect_callback(xid_dev_t *xid_dev, int status)
{
    log_print("XID disconnected: VID: %04x PID: %04X\n", xid_dev->idVendor, xid_dev->idProduct);
    if (xid_dev->user_data)
        free(xid_dev->user_data);
}

uint32_t xid_write_data(xid_dev_t *xid_dev, uint8_t *tdata, uint32_t data_len)
{
    return usbh_xid_write(xid_dev, 0, tdata, data_len, NULL);
}

void xid_print_all_rxdata(int32_t max_len)
{
    xid_dev_t *xid_dev = usbh_xid_get_device_list();
    if (xid_dev == NULL)
        return;
    if (xid_dev->user_data == NULL)
        return;

    int32_t i = 0;
    static int toggle = 0;
    while (xid_dev != NULL)
    {
        
        uint8_t buttons = ((uint8_t*)xid_dev->user_data)[2];
        if (buttons & 0x10 && buttons & 0x20 && !toggle)
        {
            toggle = 1;
            gui_toggle_view();
        }
        else
        {
            toggle = 0;
        }
        textview_print("XID %s #%d: ", LV_SYMBOL_USB LV_SYMBOL_KEYBOARD, i++);
        for (int i = 0; i < max_len; i++)
        {
            textview_print("%02x ", ((uint8_t*)xid_dev->user_data)[i]);
        }
        textview_print("\n\n");
        xid_dev = xid_dev->next;
    }
}