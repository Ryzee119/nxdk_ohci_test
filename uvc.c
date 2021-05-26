// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "uvc.h"

#define BUFF_CNT 2 //FIXME: Dont change

typedef enum
{
    IMAGE_BUFF_FREE,
    IMAGE_BUFF_USB,
    IMAGE_BUFF_READY,
    IMAGE_BUFF_POST
} buffer_state;

typedef struct frame_buff_data
{
    uint8_t *buffer;
    uint32_t max_len;
    int rx_len;
    buffer_state state;
} frame_buff_data, *pframe_buff_data;

typedef struct video_stream_data
{
    IMAGE_FORMAT_E format;
    int32_t width;
    int32_t height;
    int32_t current_buffer; //The buffer the USB backend will write to
    frame_buff_data frame_buff[BUFF_CNT];
} video_stream_data, *pvideo_stream_data;

int uvc_video_in_callback(UVC_DEV_T *vdev, uint8_t *data, int len)
{
    video_stream_data *vdata = (video_stream_data *)vdev->user_data;

    uint32_t usb_buffer = vdata->current_buffer;
    uint32_t user_buffer = (usb_buffer + 1) % BUFF_CNT;
    if (vdata->frame_buff[user_buffer].state != IMAGE_BUFF_FREE)
    {
        log_print("Got a new image, but user hasnt processed the finished buffer. Dropping backbuffer!\n");
        vdata->frame_buff[user_buffer].state = IMAGE_BUFF_USB;
    }
    else
    {
        //Mark the current buffer as read for decode/display
        vdata->frame_buff[usb_buffer].state = IMAGE_BUFF_READY;
        vdata->frame_buff[usb_buffer].rx_len = len;

        //Swap the frame buffer
        vdata->current_buffer = user_buffer;
        vdata->frame_buff[user_buffer].state = IMAGE_BUFF_USB;
        usbh_uvc_set_video_buffer(vdev, vdata->frame_buff[user_buffer].buffer, vdata->frame_buff[user_buffer].max_len);
    }
    return 0;
}

uint32_t uvc_init_device(UVC_DEV_T *vdev)
{
    if (vdev->user_data == NULL)
        vdev->user_data = malloc(sizeof(video_stream_data));

    memset(vdev->user_data, 0x00, sizeof(video_stream_data));

    video_stream_data *vdata = (video_stream_data *)vdev->user_data;
    int ret, i = 0;

    //Print a list of supported video formats
    for (i = 0;; i++)
    {
        ret = usbh_get_video_format(vdev, i, &vdata->format, &vdata->width, &vdata->height);
        if (ret != 0)
            break;

        log_print("[%d] %s, %d x %d %d\n", i, (vdata->format == UVC_FORMAT_MJPEG ? "MJPEG" : "YUYV"), vdata->width, vdata->height, vdata->format);
    }

    //Find the lowest quality stream with UVC_FORMAT_YUY2 format.
    int j;
    for (j = i; j >=0; j--)
    {
        ret = usbh_get_video_format(vdev, i - 1, &vdata->format, &vdata->width, &vdata->height);
        if (vdata->format == UVC_FORMAT_YUY2)
            break;
    }
    if (j < 0)
    {
        log_print("Error: Video device does not support UVC_FORMAT_YUY2!\n");
        return UVC_RET_DRV_NOT_SUPPORTED;
    }

    //Set the video format
    log_print("Setting video to format: %s %d %d\n", (vdata->format == UVC_FORMAT_MJPEG ? "MJPEG" : "YUYV"), vdata->width, vdata->height);
    ret = usbh_set_video_format(vdev, vdata->format, vdata->width, vdata->height);
    if (ret != 0)
    {
        log_print("usbh_set_video_format failed! - 0x%x\n", ret);
        return ret;
    }

    //Allocate some framebuffers. We double buffer
    vdata->frame_buff[0].max_len = vdata->width * vdata->height * 4;
    vdata->frame_buff[0].buffer = (uint8_t *)MmAllocateContiguousMemoryEx(vdata->frame_buff[0].max_len,
                                                                          0,
                                                                          64 * 1024 * 1024,
                                                                          4096,
                                                                          PAGE_READWRITE | PAGE_NOCACHE);

    vdata->frame_buff[1].max_len = vdata->width * vdata->height * 4;
    vdata->frame_buff[1].buffer = (uint8_t *)MmAllocateContiguousMemoryEx(vdata->frame_buff[1].max_len,
                                                                          0,
                                                                          64 * 1024 * 1024,
                                                                          4096,
                                                                          PAGE_READWRITE | PAGE_NOCACHE);
    assert(vdata->frame_buff[0].buffer != NULL);
    assert(vdata->frame_buff[1].buffer != NULL);
    memset(vdata->frame_buff[0].buffer, 0x00, vdata->frame_buff[0].max_len);
    memset(vdata->frame_buff[1].buffer, 0x00, vdata->frame_buff[1].max_len);
    vdata->current_buffer = 0;
    vdata->frame_buff->rx_len = 0;
    vdata->frame_buff->state = IMAGE_BUFF_USB;

    //Set the first buffer and start streaming
    usbh_uvc_set_video_buffer(vdev, vdata->frame_buff[0].buffer, vdata->frame_buff[0].max_len);
    ret = usbh_uvc_start_streaming(vdev, uvc_video_in_callback);
    if (ret != 0)
    {
        log_print("usbh_uvc_start_streaming failed! - %d\n", ret);
        log_print("Please re-connect UVC device...\n");
    }
    return ret;
}

void uvc_connection_callback(UVC_DEV_T *vdev, int status)
{
    //Dont handle uvc init at interface level.
}

void uvc_disconnect_callback(UVC_DEV_T *vdev, int status)
{
    log_print("USB Video device disconnected\n");
    if (vdev->user_data)
    {
        video_stream_data *vdata = (video_stream_data *)vdev->user_data;

        if(vdata->frame_buff[0].buffer)
            MmFreeContiguousMemory(vdata->frame_buff[0].buffer);

        if(vdata->frame_buff[1].buffer)
            MmFreeContiguousMemory(vdata->frame_buff[1].buffer);

        free(vdev->user_data);
    }
}

//FIXME: Only first video device will print.
void uvc_print_framebuffers()
{
    UVC_DEV_T *vdev = usbh_uvc_get_device_list();

    if (vdev == NULL || vdev->user_data == NULL)
    {
        gui_hide_image(0);
        return;
    }

    video_stream_data *vdata = (video_stream_data *)vdev->user_data;
    //Get the inactive buffer.
    uint32_t complete_buffer = (vdata->current_buffer + 1) % BUFF_CNT;

    //Draw it to the gui
    gui_draw_image(0, vdata->frame_buff[complete_buffer].buffer, vdata->width, vdata->height);
    vdata->frame_buff[complete_buffer].state = IMAGE_BUFF_FREE;
}