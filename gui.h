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
void gui_toggle_view();

void log_print(const char *format, ...);
void log_clear(void);

void gui_draw_image(uint8_t index, uint8_t *buff, int32_t width, int32_t height);
void gui_hide_image(uint8_t index);
#ifdef __cplusplus
}
#endif

#endif