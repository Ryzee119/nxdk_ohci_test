// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "hid.h"

#define MAX_PACKET 128

static void hid_int_read_callback(HID_DEV_T *hdev, uint16_t ep_addr, int32_t status, uint8_t *rdata, uint32_t data_len)
{
    memcpy(hdev->user_data, rdata, data_len <= MAX_PACKET ? data_len : MAX_PACKET);
}

uint32_t hid_init_device(HID_DEV_T *hdev)
{
    hdev->user_data = malloc(MAX_PACKET);
    assert(hdev->user_data != NULL);
    return usbh_hid_start_int_read(hdev, 0, hid_int_read_callback);
}

void hid_connection_callback(HID_DEV_T *hdev, int status)
{
    log_print("HID connected: VID: %04x PID: %04X\n", hdev->idVendor, hdev->idProduct);
    hid_init_device(hdev);
}

void hid_disconnect_callback(HID_DEV_T *hdev, int status)
{
    log_print("HID disconnected: VID: %04x PID: %04X\n", hdev->idVendor, hdev->idProduct);
    if (hdev->user_data)
        free(hdev->user_data);
}

uint32_t hid_write_data(HID_DEV_T *hdev, uint8_t *tdata, uint32_t data_len)
{
    return usbh_hid_int_write(hdev, 0, tdata, data_len, NULL);
}

void hid_print_all_rxdata(int32_t max_len)
{
    HID_DEV_T *hdev = usbh_hid_get_device_list();
    if (hdev == NULL)
        return;
    if (hdev->user_data == NULL)
        return;

    int32_t i = 0;
    static int toggle = 0;
    while (hdev != NULL)
    {
        uint8_t buttons = ((uint8_t*)hdev->user_data)[2];
        if (buttons & 0x10 && buttons & 0x20 && !toggle)
        {
            toggle = 1;
            gui_toggle_view();
        }
        else
        {
            toggle = 0;
        }
        textview_print("HID %s #%d: ", LV_SYMBOL_USB LV_SYMBOL_KEYBOARD, i++);
        for (int i = 0; i < max_len; i++)
        {
            textview_print("%02x ", ((uint8_t*)hdev->user_data)[i]);
        }
        textview_print("\n\n");
        hdev = hdev->next;
    }
}