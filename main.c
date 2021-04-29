#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <usbh_lib.h>
#include <usb.h>

#include "hid.h"
#include "uac.h"
#include "cdc.h"
#include "uvc.h"
#include "msc.h"

/*
 * This demo will initialise the usb stack, detect events on the usb ports
 * and print the device PID and VID.
*/
void device_connection_callback(UDEV_T *udev, int status)
{
    debugPrint("Device connected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
               udev->descriptor.idProduct,
               udev->descriptor.idVendor);

    //Handle any new UVC devices at the device level.
    UVC_DEV_T *vdev = usbh_uvc_get_device_list();
    while (vdev != NULL)
    {
        if (vdev->user_data == NULL)
        {
            uvc_init_device(vdev);
        }
        vdev = vdev->next;
    }

    //Handle any new UAC devices at the device level.
    UAC_DEV_T *adev = usbh_uac_get_device_list();
    while (adev != NULL)
    {
        if (adev->user_data == NULL)
        {
            uac_init_device(adev);
        }
        adev = adev->next;
    }
}

void device_disconnect_callback(UDEV_T *udev, int status)
{
    debugPrint("Device disconnected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
               udev->descriptor.idProduct,
               udev->descriptor.idVendor);
}

int main(void)
{
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    usbh_core_init();
    usbh_hid_init();
    usbh_umas_init();
    usbh_cdc_init();
    usbh_uac_init();
    usbh_uvc_init();

    //USB device connection callbacks
    usbh_install_conn_callback(device_connection_callback, device_disconnect_callback);

    //HID interface detected
    usbh_install_hid_conn_callback(hid_connection_callback, hid_disconnect_callback);

    //Communication Device Class interface detected (Virtual Comport)
    usbh_install_cdc_conn_callback(cdc_connection_callback, cdc_disconnect_callback);

    //Mass Storage Device interface detected
    usbh_install_msc_conn_callback(msc_connection_callback, msc_disconnect_callback);

    //USB Audio Class interface detected. Only register a disconnect callback.
    //We handle the connection at the device level as these are complex devices with many interfaces.
    usbh_install_uac_conn_callback(NULL, uac_disconnect_callback);

    //USB Video Class interface detected. Only register a disconnect callback.
    //We handle the connection at the device level as these are complex devices with many interfaces.
    usbh_install_uvc_conn_callback(NULL, uvc_disconnect_callback);

    while (1)
    {
        usbh_pooling_hubs();
        XVideoWaitForVBlank();
        debugClearScreen();
        hid_print_all_rxdata(32);
        uac_print_all_devices();
        cdc_print_all_rxdata();
        msc_print_all_directories();
        Sleep(50);
    }

    usbh_core_deinit();
    return 0;
}
