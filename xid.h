// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _XID_H
#define _XID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include "gui.h"
#include "usbh_lib.h"
#include "xid_driver.h"

void xid_connection_callback(xid_dev_t *xid_dev, int status);
void xid_disconnect_callback(xid_dev_t *xid_dev, int status);
uint32_t xid_init_device(xid_dev_t *xid_dev);
uint32_t xid_write_data(xid_dev_t *xid_dev, uint8_t *tdata, uint32_t data_len);
void xid_print_all_rxdata(int32_t max_len);

#ifdef __cplusplus
}
#endif

#endif