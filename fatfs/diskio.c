/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h" /* Obtains integer types */
#include "usbh_lib.h"
#include "usb.h"
#include "diskio.h" /* Declarations of disk functions */

/* FATFS window buffer is cachable. Must not use it directly. */
BYTE *fatfs_win_buff;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	usbh_pooling_hubs();
	if (usbh_umas_disk_status(pdrv) == UMAS_ERR_NO_DEVICE)
		return STA_NODISK;
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	if (fatfs_win_buff == NULL)
	{
		fatfs_win_buff = usbh_alloc_mem(FF_MAX_SS);
	}
	usbh_pooling_hubs();
	if (usbh_umas_disk_status(pdrv) == UMAS_ERR_NO_DEVICE)
		return STA_NODISK;
	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
	BYTE pdrv,	  /* Physical drive nmuber to identify the drive */
	BYTE *buff,	  /* Data buffer to store read data */
	LBA_t sector, /* Start sector in LBA */
	UINT count	  /* Number of sectors to read */
)
{
	int ret;
	int sec_size = 512;

	if (count * sec_size > FF_MAX_SS)
		return RES_ERROR;

	if (fatfs_win_buff == NULL)
		return RES_NOTRDY;

	ret = (DRESULT)usbh_umas_read(pdrv, sector, count, fatfs_win_buff);
	memcpy(buff, fatfs_win_buff, count * sec_size);

	if (ret == UMAS_OK)
		return RES_OK;

	if (ret == UMAS_ERR_NO_DEVICE)
		return RES_NOTRDY;

	if (ret == UMAS_ERR_IO)
		return RES_ERROR;

	return (DRESULT)ret;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(
	BYTE pdrv,		  /* Physical drive nmuber to identify the drive */
	const BYTE *buff, /* Data to be written */
	LBA_t sector,	  /* Start sector in LBA */
	UINT count		  /* Number of sectors to write */
)
{
	int ret;
	int sec_size = 512;

	if (count * sec_size > FF_MAX_SS)
		return RES_ERROR;

	if (fatfs_win_buff == NULL)
		return RES_NOTRDY;

	memcpy(fatfs_win_buff, buff, count * sec_size);
	ret = usbh_umas_write(pdrv, sector, count, fatfs_win_buff);

	if (ret == UMAS_OK)
		return RES_OK;

	if (ret == UMAS_ERR_NO_DEVICE)
		return RES_NOTRDY;

	if (ret == UMAS_ERR_IO)
		return RES_ERROR;

	return (DRESULT)ret;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
	BYTE pdrv, /* Physical drive nmuber (0..) */
	BYTE cmd,  /* Control code */
	void *buff /* Buffer to send/receive control data */
)
{
	int ret = usbh_umas_ioctl(pdrv, cmd, buff);

	if (ret == UMAS_OK)
		return RES_OK;

	if (ret == UMAS_ERR_IVALID_PARM)
		return RES_PARERR;

	if (ret == UMAS_ERR_NO_DEVICE)
		return RES_NOTRDY;

	return RES_PARERR;
}
