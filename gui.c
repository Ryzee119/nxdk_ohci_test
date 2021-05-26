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
lv_style_t log_view_style;

static lv_obj_t *image;

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

void gui_toggle_view()
{
    static int toggle = 0;
    if (toggle)
    {
        lv_obj_fade_out(text_view, 0, 0);
        lv_obj_fade_in(log_view, 0, 0);
    }
    else
    {
        lv_obj_fade_out(log_view, 0, 0);
        lv_obj_fade_in(text_view, 0, 0);
    }
    toggle ^= 1;
}

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
static long U[256], V[256], Y1[256], Y2[256];
void init_yuv422p_table(void)
{
    int i;
    for (i = 0; i < 256; i++)
    {
        V[i] = 15938 * i - 2221300;
        U[i] = 20238 * i - 2771300;
        Y1[i] = 11644 * i;
        Y2[i] = 19837 * i - 311710;
    }
}

void yuy2_to_rgb32(unsigned char *yuv422p, unsigned char *rgb, int width, int height)
{
    int y, cb, cr;
    int r, g, b;
    int i = 0;
    unsigned char *p_y;
    unsigned char *p_u;
    unsigned char *p_v;
    unsigned char *p_rgb;
    static int init_yuv422p = 0;

    p_y = yuv422p;
    p_u = p_y + 1;
    p_v = p_y + 3;
    p_rgb = rgb;

    if (init_yuv422p == 0)
    {
        init_yuv422p_table();
        init_yuv422p = 1;
    }

    for (i = 0; i < width * height / 2; i++)
    {
        y  = p_y[0];
        cb = p_u[0];
        cr = p_v[0];

        r = MAX(0, MIN(255, (V[cr] + Y1[y]) / 10000));                 //R value
        b = MAX(0, MIN(255, (U[cb] + Y1[y]) / 10000));                 //B value
        g = MAX(0, MIN(255, (Y2[y] - 5094 * (r)-1942 * (b)) / 10000)); //G value

        p_rgb[0] = b;
        p_rgb[1] = g;
        p_rgb[2] = r;
        p_rgb[3] = 0;

        y = p_y[2];
        cb = p_u[0];
        cr = p_v[0];
        r = MAX(0, MIN(255, (V[cr] + Y1[y]) / 10000));                 //R value
        b = MAX(0, MIN(255, (U[cb] + Y1[y]) / 10000));                 //B value
        g = MAX(0, MIN(255, (Y2[y] - 5094 * (r)-1942 * (b)) / 10000)); //G value

        p_rgb[4] = b;
        p_rgb[5] = g;
        p_rgb[6] = r;
        p_rgb[7] = 0;

        p_y += 4;
        p_u = p_y + 1;
        p_v = p_y + 3;
        p_rgb += 8;
    }
}

uint8_t rgb32[640 * 480 * 4]; //FIXME. dynamically allocate?
void gui_draw_image(uint8_t index, uint8_t *buff, int32_t width, int32_t height)
{
    static lv_img_dsc_t fb;
    fb.header.always_zero = 0;
    fb.header.w = width;
    fb.header.h = height;
    fb.data_size = width * height * 4;
    fb.header.cf = LV_IMG_CF_TRUE_COLOR;

    //FIXME: All webcams probably dont output YUY2 format.
    yuy2_to_rgb32(buff, rgb32, width, height);
    fb.data = (uint8_t*)rgb32;
    lv_img_set_src(image, &fb);
    lv_obj_align(image, NULL, LV_ALIGN_CENTER, 0, -20);
    lv_obj_move_foreground(image);
}

void gui_hide_image(uint8_t index)
{
    lv_obj_move_background(image);
}

void create_gui()
{
    int screen_w = lv_obj_get_width(lv_scr_act());
    int screen_h = lv_obj_get_height(lv_scr_act());

    image = lv_img_create(lv_scr_act(), NULL);

    //Create a text area
    text_view = lv_textarea_create(lv_scr_act(), NULL);
    lv_textarea_set_text(text_view, "");
    lv_textarea_set_cursor_hidden(text_view, true);
    lv_obj_set_width(text_view, screen_w - (XMARGIN * 2));
    lv_obj_set_height(text_view, screen_h - (YMARGIN * 2));
    lv_obj_align(text_view, NULL, LV_ALIGN_CENTER, 0, 0);
    //Set a font for the text area
    lv_style_init(&text_view_style);
    lv_style_set_text_font(&text_view_style, LV_STATE_DEFAULT, &lv_font_montserrat_12);
    lv_obj_add_style(text_view, LV_LABEL_PART_MAIN, &text_view_style);
    lv_textarea_set_cursor_pos(text_view, 0);

    //Create a text log area
    log_view = lv_textarea_create(lv_scr_act(), NULL);
    lv_textarea_set_text(log_view, "");
    lv_textarea_set_cursor_hidden(log_view, true);
    lv_obj_set_width(log_view, screen_w - (XMARGIN * 2) / 2);
    lv_obj_set_height(log_view, screen_h - (YMARGIN * 2) / 2);
    lv_obj_align(log_view, NULL, LV_ALIGN_CENTER, 0, 0);
    //Set a font for the log area
    lv_style_init(&log_view_style);
    lv_style_set_text_font(&log_view_style, LV_STATE_DEFAULT, &lv_font_montserrat_8);
    lv_obj_add_style(log_view, LV_LABEL_PART_MAIN, &log_view_style);
    lv_textarea_set_cursor_pos(log_view, 0);

    //Make sure the text view is showing by default
    lv_obj_fade_out(log_view, 0, 0);
    lv_obj_fade_in(text_view, 0, 0);
}
