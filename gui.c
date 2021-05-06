#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "gui.h"
#include "lv_conf.h"
#include "lvgl.h"
#include "lv_sdl_drv_input.h"

static const int XMARGIN = 15;
static const int YMARGIN = 15;
static lv_indev_t *sdl_indev;
static lv_group_t *input_group;
static lv_obj_t *text_view;
static lv_obj_t *log_view;
lv_style_t text_view_style;

void log_clear()
{
    lv_textarea_set_text(log_view, "");
}

void textview_clear()
{
    lv_textarea_set_text(text_view, "");
}

void log_print(const char *format, ...)
{
    char buffer[512];
    unsigned short len;
    va_list argList;
    va_start(argList, format);
    vsprintf(buffer, format, argList);
    va_end(argList);

    lv_textarea_add_text(log_view, buffer);
}

void textview_print(const char *format, ...)
{
    char buffer[512];
    unsigned short len;
    va_list argList;
    va_start(argList, format);
    vsprintf(buffer, format, argList);
    va_end(argList);

    lv_textarea_add_text(text_view, buffer);
}

void create_gui()
{
    int screen_w = lv_obj_get_width(lv_scr_act());
    int screen_h = lv_obj_get_height(lv_scr_act());

    //Create a text area
    text_view = lv_textarea_create(lv_scr_act(), NULL);
    lv_textarea_set_text(text_view, "");
    lv_textarea_set_cursor_hidden(text_view, true);
    lv_obj_set_width(text_view, screen_w - (XMARGIN * 2));
    lv_obj_set_height(text_view, screen_h - (YMARGIN * 2));
    lv_obj_align(text_view, NULL, LV_ALIGN_CENTER, 0, 0);
    //Set a font to the text area
    lv_style_init(&text_view_style);
    lv_style_set_text_font(&text_view_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_add_style(text_view, LV_LABEL_PART_MAIN, &text_view_style);
    lv_textarea_set_cursor_pos(text_view, 0);

    //Create a text log area
    log_view = lv_textarea_create(lv_scr_act(), NULL);
    lv_textarea_set_cursor_hidden(log_view, true);
    lv_obj_set_width(log_view, screen_w - (XMARGIN * 2) / 2);
    lv_obj_set_height(log_view, screen_h - (YMARGIN * 2) / 2);
    lv_obj_align(log_view, NULL, LV_ALIGN_CENTER, 0, 0);
    //Set a font to the text area
    lv_obj_add_style(log_view, LV_LABEL_PART_MAIN, &text_view_style);
    lv_textarea_set_cursor_pos(log_view, 0);
    lv_obj_fade_out(log_view, 0, 0);
}
