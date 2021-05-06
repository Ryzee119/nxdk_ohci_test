// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT

#ifndef _GUI_H
#define _GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lvgl.h"

void create_gui(void);

void textview_print(const char *format, ...);
void textview_clear(void);

void log_print(const char *format, ...);
void log_clear(void);

#ifdef __cplusplus
}
#endif

#endif