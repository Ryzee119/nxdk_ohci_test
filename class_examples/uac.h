// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _UAC_H
#define _UAC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <hal/debug.h>
#include "gui.h"
#include "usbh_lib.h"
#include "usbh_uac.h"

void uac_connection_callback(UAC_DEV_T *adev, int status);
void uac_disconnect_callback(UAC_DEV_T *adev, int status);
uint32_t uac_init_device(UAC_DEV_T *adev);
void uac_set_voice_data(UAC_DEV_T *adev, uint8_t *voice_data, uint32_t voice_len);
void uac_print_all_devices();

#ifdef __cplusplus
}
#endif

#endif