// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _UVC_H
#define _UVC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include <xboxkrnl/xboxkrnl.h>
#include "usbh_lib.h"
#include "usbh_uvc.h"

void uvc_connection_callback(UVC_DEV_T *vdev, int status);
void uvc_disconnect_callback(UVC_DEV_T *vdev, int status);
void uvc_print_framebuffer();
uint32_t uvc_init_device(UVC_DEV_T *vdev);

#ifdef __cplusplus
}
#endif

#endif