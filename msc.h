// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _MSC_H
#define _MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include "gui.h"
#include "usbh_lib.h"
#include "usbh_msc.h"

void msc_connection_callback(MSC_T *msc_dev, int status);
void msc_disconnect_callback(MSC_T *msc_dev, int status);
uint32_t msc_init_device(MSC_T *msc_dev);
uint32_t msc_write_data(MSC_T *msc_dev, uint8_t *tdata, uint32_t data_len);
void msc_print_all_directories();

#ifdef __cplusplus
}
#endif

#endif