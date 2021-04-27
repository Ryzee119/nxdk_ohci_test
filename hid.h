// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _HID_H
#define _HID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include "usbh_lib.h"
#include "usbh_hid.h"

void hid_connection_callback(HID_DEV_T *hdev, int status);
void hid_disconnect_callback(HID_DEV_T *hdev, int status);
uint32_t hid_init_device(HID_DEV_T *hdev);
uint32_t hid_write_data(HID_DEV_T *hdev, uint8_t *tdata, uint32_t data_len);
void hid_print_all_rxdata(int32_t max_len);

#ifdef __cplusplus
}
#endif

#endif