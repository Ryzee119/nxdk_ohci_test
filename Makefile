XBE_TITLE = nxdk\ ohci\ test
GEN_XISO = $(XBE_TITLE).iso

SRCS += \
	main.c \
	hid.c \
	uac.c \
	cdc.c \
	uvc.c
	#msc.c (todo)

#NXDK_USB_DISABLE_HID=y
NXDK_USB_ENABLE_CDC=y
NXDK_USB_ENABLE_UAC=y
NXDK_USB_ENABLE_UVC=y
NXDK_USB_ENABLE_MSC=y

#Manually include MSC lib with fatfs
SRCS += \
	$(NXDK_DIR)/lib/usb/libusbohci/src_msc/msc_driver.c \
	$(NXDK_DIR)/lib/usb/libusbohci/src_msc/msc_xfer.c \
	$(CURDIR)/fatfs/diskio.c \
	$(CURDIR)/fatfs/ff.c \
	$(CURDIR)/fatfs/ffsystem.c \
	$(CURDIR)/fatfs/ffunicode.c

CFLAGS += -I$(CURDIR)/fatfs -O2 -DUSB_MEMORY_POOL_SIZE=1024*1204 -DMEM_POOL_UNIT_NUM=512

include $(NXDK_DIR)/Makefile
