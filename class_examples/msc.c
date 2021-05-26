// Copyright 2021, Ryan Wendland
// SPDX-License-Identifier: MIT
#include <assert.h>
#include "msc.h"

//This example uses FATFS by Chan
#include "ff.h"

#define MAX_DRIVES 4     //Max number of MSC devices and LUNS combined
#define MAX_PATH_LEN 256 //For root mounting point

static uint32_t list_directory()
{
    FRESULT res;
    DIR dir;
    UINT file_count = 0;
    static FILINFO fno;

    res = f_opendir(&dir, "");
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            textview_print("%s %s\n", (fno.fattrib & AM_DIR) ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE, fno.fname);
            file_count++;
        }
        f_closedir(&dir);
    }

    if (file_count == 0)
        textview_print("No files\n");

    return file_count;
}

static FRESULT write_to_file(char *filename, uint8_t *data, uint32_t len)
{
    FRESULT res;
    UINT br;
    FIL fil;

    //FA_CREATE_NEW only writes if file doesnt already exist.
    res = f_open(&fil, (const TCHAR *)filename, FA_WRITE | FA_CREATE_NEW);
    if (res != FR_OK)
    {
        if (res != FR_EXIST)
            log_print("ERROR: Could not open %s for WRITE, error %d\n", filename, res);
        return res;
    }

    res = f_write(&fil, data, len, &br);

    f_close(&fil);
    if (res != FR_OK)
    {
        log_print("ERROR: Could not write %s with error %i\n", filename, res);
        return FR_DISK_ERR;
    }
    else
    {
        log_print("Writing %s for %u bytes ok!\n", filename, br);
    }
    return FR_OK;
}

//These are used to record and access each drive.
//Use f_chdrive("0:") to set first drive, "1:" to set next drive etc.
uint8_t logical_drives[MAX_DRIVES] = {0};
typedef struct msc_filesystem
{
    FATFS fatfs_vol;   /* FATFS volume associated with this instance   */
    char *logical_vol; /* 0: for first drive, 1: for second drive etc  */
} msc_filesystem;

uint32_t msc_init_device(MSC_T *msc_dev)
{
    msc_dev->user_data = malloc(sizeof(msc_filesystem));
    assert(msc_dev->user_data != NULL);

    msc_filesystem *fdata = (msc_filesystem *)msc_dev->user_data;
    fdata->logical_vol = malloc(MAX_PATH_LEN);
    assert(fdata->logical_vol != NULL);
    memcpy(fdata->logical_vol, "0:\0", 3);

    //Claim next available logical drive number
    int drv;
    for (drv = 0; drv < MAX_DRIVES; drv++)
    {
        if (logical_drives[drv] == 0)
        {
            logical_drives[drv] = 1;
            break;
        }
    }
    //No more slots available.
    if (drv == MAX_DRIVES)
        return UMAS_ERR_NO_DEVICE;

    msc_dev->drv_no = drv;

    //Convert number to equivalent ascii char
    fdata->logical_vol[0] = msc_dev->drv_no + '0';
    if (f_mount(&fdata->fatfs_vol, fdata->logical_vol, 1) != FR_OK)
    {
        log_print("MSC: Error, could not mount %s, MSC UID: %d Lun: %d\n", fdata->logical_vol,
                                                                            msc_dev->uid,
                                                                            msc_dev->lun);
        logical_drives[drv] = 0;
        return UMAS_ERR_INIT_DEVICE;
    }
    log_print("MSC: Mounted %s, MSC UID: %d Lun: %d OK\n", fdata->logical_vol,
                                                            msc_dev->uid,
                                                            msc_dev->lun);
    return UMAS_OK;
}

//These callbacks will get called by the backend USB stack for each LUN on the USB MSC device
//You dont need to worry about multiple LUN devices :)
void msc_connection_callback(MSC_T *msc_dev, int status)
{
    msc_init_device(msc_dev);
}

void msc_disconnect_callback(MSC_T *msc_dev, int status)
{
    if (msc_dev->user_data == NULL)
        return;

    msc_filesystem *fdata = (msc_filesystem *)msc_dev->user_data;

    //Free the logical drive number and unmount the drive.
    if (msc_dev->drv_no < MAX_DRIVES)
    {
        f_mount(NULL, fdata->logical_vol, 1);
        logical_drives[msc_dev->drv_no] = 0;
    }

    if (msc_dev->user_data)
        free(msc_dev->user_data);
}

void msc_print_all_directories()
{
    MSC_T *msc_dev = usbh_msc_get_device_list();

    if (msc_dev == NULL)
        return;

    if (msc_dev->user_data == NULL)
        return;

    while (msc_dev != NULL)
    {
        if (msc_dev->user_data == NULL)
            break;

        msc_filesystem *fdata = (msc_filesystem *)msc_dev->user_data;
        if (f_chdrive(fdata->logical_vol) == FR_OK)
        {
            textview_print("MSC %s directory listing %s\n", LV_SYMBOL_USB LV_SYMBOL_DRIVE, fdata->logical_vol);
            list_directory();

            //Save a test file to each drive
            char *msg = "Hello from nxdk!";
            write_to_file("nxdk_test.txt", (uint8_t *)msg, strlen(msg));
        }
        textview_print("\n\n");
        msc_dev = msc_dev->next;
    }
}
