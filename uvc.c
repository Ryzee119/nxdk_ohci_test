// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "uvc.h"

#define BUFF_CNT 2

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
    uint8_t *voice_data;
    IMAGE_FORMAT_E format;
    int32_t width;
    int32_t height;
    int32_t current_buffer;
    frame_buff_data frame_buff[BUFF_CNT];
} video_stream_data, *pvideo_stream_data;

int uvc_video_in_callback(UVC_DEV_T *vdev, uint8_t *data, int len)
{
    video_stream_data *vdata = (video_stream_data *)vdev->user_data;

    uint32_t cb = vdata->current_buffer;
    uint32_t nb = (cb + 1) % BUFF_CNT;
    debugPrint("uvc_video_in_callback\n");    
    if (vdata->frame_buff[nb].state != IMAGE_BUFF_FREE)
    {
        /*
         *  Next image buffer is in used.
         *  Just drop this newly received image and reuse the same image buffer.
         */
        debugPrint("Drop!\n");
    }
    else
    {
        vdata->frame_buff[cb].state = IMAGE_BUFF_READY; /* mark the current buffer as ready for decode/display */
        vdata->frame_buff[cb].rx_len = len;             /* length of this newly received image   */

        /* proceed to the next image buffer */
        vdata->current_buffer = nb;
        vdata->frame_buff[nb].state = IMAGE_BUFF_USB; /* mark the next image as used by USB    */

        /* assign the next image buffer to receive next image from USB */
        usbh_uvc_set_video_buffer(vdev, vdata->frame_buff[nb].buffer, vdata->frame_buff[nb].max_len);
    }
    debugPrint("Current buffer: %d, %d\n", cb, len);
    return 0;
}

uint32_t uvc_init_device(UVC_DEV_T *vdev)
{
    if (vdev->user_data == NULL)
        vdev->user_data = malloc(sizeof(video_stream_data));

    video_stream_data *vdata = (video_stream_data *)vdev->user_data;
    int ret, i = 0;

    for (i = 0;; i++)
    {
        ret = usbh_get_video_format(vdev, i, &vdata->format, &vdata->width, &vdata->height);
        if (ret != 0)
            break;

        debugPrint("[%d] %s, %d x %d\n", i, (vdata->format == UVC_FORMAT_MJPEG ? "MJPEG" : "YUYV"), vdata->width, vdata->height);
    }
    ret = usbh_get_video_format(vdev, i - 1, &vdata->format, &vdata->width, &vdata->height);

    debugPrint("Setting video to format: %s %d %d\n", (vdata->format == UVC_FORMAT_MJPEG ? "MJPEG" : "YUYV"), vdata->width, vdata->height);
    ret = usbh_set_video_format(vdev, vdata->format, vdata->width, vdata->height);
    if (ret != 0)
    {
        debugPrint("usbh_set_video_format failed! - 0x%x\n", ret);
        return ret;
    }

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
    vdata->current_buffer = 0;
    vdata->frame_buff->rx_len = 0;
    vdata->frame_buff->state = IMAGE_BUFF_USB;

    usbh_uvc_set_video_buffer(vdev, vdata->frame_buff[0].buffer, vdata->frame_buff[0].max_len);
    ret = usbh_uvc_start_streaming(vdev, uvc_video_in_callback);
    if (ret != 0)
    {
        debugPrint("usbh_uvc_start_streaming failed! - %d\n", ret);
        debugPrint("Please re-connect UVC device...\n");
    }
    return ret;
}

void uvc_connection_callback(UVC_DEV_T *vdev, int status)
{
    //Dont handle uvc init at interface level.
}

void uvc_disconnect_callback(UVC_DEV_T *vdev, int status)
{
    debugPrint("USB Video device disconnected\n");
    if (vdev->user_data)
       free(vdev->user_data);
}

void uvc_print_framebuffer()
{
}