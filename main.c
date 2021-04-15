#include <hal/debug.h>
#include <hal/video.h>
#include <windows.h>
#include <usbh_lib.h>

#include "hid.h"
#include "uac.h"
#include "cdc.h"

/*
 * This demo will initialise the usb stack, detect events on the usb ports
 * and print the device PID and VID.
*/
void device_connection_callback(UDEV_T *udev, int status)
{
    debugPrint("Device connected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
               udev->descriptor.idProduct,
               udev->descriptor.idVendor);
}

void device_disconnect_callback(UDEV_T *udev, int status)
{
    debugPrint("Device disconnected on port %u (PID: %04x VID: %04x)\n", udev->port_num,
               udev->descriptor.idProduct,
               udev->descriptor.idVendor);
}

int main(void)
{
    XVideoSetMode(1280, 720, 32, REFRESH_DEFAULT);

    usbh_core_init();
    usbh_hid_init();
    usbh_umas_init();
    usbh_cdc_init();
    usbh_uac_init();
    usbh_uvc_init();

    //Generic USB device callbacks
    usbh_install_conn_callback(&device_connection_callback, &device_disconnect_callback);

    //HID class detected
    usbh_install_hid_conn_callback(hid_connection_callback, hid_disconnect_callback);

    //USB Audio Class device detected
    usbh_install_uac_conn_callback(uac_connection_callback, uac_disconnect_callback);

    //Communication Device Class device detected (Virtual Comport)
    usbh_install_cdc_conn_callback(cdc_connection_callback, cdc_disconnect_callback);

    while (1)
    {
        usbh_pooling_hubs();
        XVideoWaitForVBlank();
        debugClearScreen();
        hid_print_all_rxdata(32);
        uac_print_all_devices();
        cdc_print_all_rxdata();
        Sleep(50);
    }

    usbh_core_deinit();
    return 0;
}
