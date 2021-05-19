XBE_TITLE = nxdk\ ohci\ test
GEN_XISO = $(XBE_TITLE).iso
NXDK_SDL = y

SRCS += \
	main.c \
	hid.c \
	xid.c \
	uac.c \
	cdc.c \
	uvc.c \
	msc.c \
	gui.c

NXDK_USB_ENABLE_HID=y
NXDK_USB_ENABLE_CDC=y
NXDK_USB_ENABLE_UAC=y
NXDK_USB_ENABLE_UVC=y
NXDK_USB_ENABLE_MSC=y

#Sample uses fatfs for Mass Storage Testing
SRCS += \
	$(CURDIR)/fatfs/diskio.c \
	$(CURDIR)/fatfs/ff.c \
	$(CURDIR)/fatfs/ffsystem.c \
	$(CURDIR)/fatfs/ffunicode.c

CFLAGS += \
	-I$(CURDIR) \
	-I$(CURDIR)/fatfs -O2 \
	-DUSB_MEMORY_POOL_SIZE=1024*1204 \
	-DMEM_POOL_UNIT_NUM=512

#Include lvgl main library
SRCS +=	\
	$(CURDIR)/lvgl-sdl/lv_drv/lv_sdl_drv_input.c \
	$(CURDIR)/lvgl-sdl/lv_drv/lv_sdl_drv_display.c \
	$(CURDIR)/lvgl-sdl/lv_drv/lv_if_drv_filesystem.c \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_core/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_draw/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_font/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_gpu/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_hal/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_misc/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_themes/*.c) \
	$(wildcard $(CURDIR)/lvgl-sdl/lvgl/src/lv_widgets/*.c)
CFLAGS += \
	-I$(CURDIR)/lvgl-sdl/ \
	-I$(CURDIR)/lvgl-sdl/lv_drv \
	-I$(CURDIR)/lvgl-sdl/lvgl/ \
	-I$(CURDIR)/lvgl-sdl/lvgl/src

include $(NXDK_DIR)/Makefile
