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
#include "usbh_msc.h"
#include "usb.h"
#include "diskio.h" /* Declarations of disk functions */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static MSC_T *find_msc_by_drive(int drv_no)
{
	MSC_T *msc = usbh_msc_get_device_list();

	while (msc != NULL)
	{
		if (msc->drv_no == drv_no)
			return msc;
		msc = msc->next;
	}
	return NULL;
}

static int umas_disk_status(int drv_no)
{
	if (find_msc_by_drive(drv_no) == NULL)
		return STA_NODISK;

	return RES_OK;
}

DSTATUS disk_status(
	BYTE pdrv /* Physical drive nmuber to identify the drive */
)
{
	if (umas_disk_status(pdrv) == STA_NODISK)
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
	if (umas_disk_status(pdrv) == STA_NODISK)
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
	DRESULT ret;
	INT sec_size = 512;

	MSC_T *msc = find_msc_by_drive(pdrv);

	if (msc == NULL)
		return RES_NOTRDY;

	uint8_t *rx_buff = usbh_alloc_mem(count * sec_size);

	ret = (DRESULT)usbh_umas_read(msc, sector, count, rx_buff);

	memcpy(buff, rx_buff, count * sec_size);

	usbh_free_mem(rx_buff, count * sec_size);

	if (ret == UMAS_OK)
		return RES_OK;

	if (ret == UMAS_ERR_NO_DEVICE)
		return RES_NOTRDY;

	if (ret == UMAS_ERR_IO)
		return RES_ERROR;

	return ret;
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
	INT ret;
	INT sec_size = 512;

	MSC_T *msc = find_msc_by_drive(pdrv);

	if (msc == NULL)
		return RES_NOTRDY;

	uint8_t *tx_buff = usbh_alloc_mem(count * sec_size);
	memcpy(tx_buff, buff, count * sec_size);

	ret = usbh_umas_write(msc, sector, count, tx_buff);

	usbh_free_mem(tx_buff, count * sec_size);

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

	MSC_T *msc = find_msc_by_drive(pdrv);

	if (msc == NULL)
		return RES_NOTRDY;

	switch (cmd)
	{
	case GET_SECTOR_SIZE:
		*(WORD *)buff = msc->nSectorSize;
		break;
	case GET_BLOCK_SIZE:
		*(DWORD *)buff = msc->nSectorSize;
		break;
	case GET_SECTOR_COUNT:
		*(LBA_t *)buff = msc->uTotalSectorN;
		break;
	default:
		break;
	}

	return RES_PARERR;
}
