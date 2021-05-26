// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _CDC_H
#define _CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include "gui.h"
#include "usbh_lib.h"
#include "usbh_cdc.h"

void cdc_connection_callback(CDC_DEV_T *cdev, int status);
void cdc_disconnect_callback(CDC_DEV_T *cdev, int status);
uint32_t cdc_init_device(CDC_DEV_T *cdev);
uint32_t cdc_write_data(CDC_DEV_T *cdev, uint8_t *tdata, uint32_t data_len);
void cdc_print_all_rxdata();

#ifdef __cplusplus
}
#endif

#endif