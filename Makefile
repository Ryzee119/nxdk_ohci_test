XBE_TITLE = nxdk\ ohci\ test
GEN_XISO = $(XBE_TITLE).iso

SRCS += \
	main.c \
	hid.c \
	uac.c \
	cdc.c
	#msc.c (todo)
	#uvc.c (todo)

#Manually include MSC lib with fatfs
SRCS += \
	$(NXDK_DIR)/lib/usb/ohci_usbhostlib/src_msc/msc_driver.c \
	$(NXDK_DIR)/lib/usb/ohci_usbhostlib/src_msc/msc_xfer.c \
	$(CURDIR)/fatfs/diskio.c \
	$(CURDIR)/fatfs/ff.c \
	$(CURDIR)/fatfs/ffsystem.c \
	$(CURDIR)/fatfs/ffunicode.c

CFLAGS += -I$(CURDIR)/fatfs

include $(NXDK_DIR)/Makefile
