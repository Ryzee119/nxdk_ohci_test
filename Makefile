XBE_TITLE = nxdk\ ohci\ test
GEN_XISO = $(XBE_TITLE).iso
NXDK_SDL = y

SRCS += \
	main.c \
	gui.c \
	class_examples/hid.c \
	class_examples/xid.c \
	class_examples/uac.c \
	class_examples/cdc.c \
	class_examples/uvc.c \
	class_examples/msc.c
	

NXDK_USB_ENABLE_HID=y
NXDK_USB_ENABLE_CDC=y
NXDK_USB_ENABLE_UAC=y
NXDK_USB_ENABLE_UVC=y
NXDK_USB_ENABLE_MSC=y

#Sample uses fatfs for Mass Storage Testing
SRCS += \
	$(CURDIR)/lib/fatfs/diskio.c \
	$(CURDIR)/lib/fatfs/ff.c \
	$(CURDIR)/lib/fatfs/ffsystem.c \
	$(CURDIR)/lib/fatfs/ffunicode.c

CFLAGS += \
	-I$(CURDIR) \
	-I$(CURDIR)/class_examples \
	-I$(CURDIR)/lib/fatfs \
	-O2 \
	-DUSB_MEMORY_POOL_SIZE=1024*1204 \
	-DMEM_POOL_UNIT_NUM=512

#Include lvgl main library
SRCS +=	\
	$(CURDIR)/lib/lvgl-sdl/lv_drv/lv_sdl_drv_input.c \
	$(CURDIR)/lib/lvgl-sdl/lv_drv/lv_sdl_drv_display.c \
	$(CURDIR)/lib/lvgl-sdl/lv_drv/lv_if_drv_filesystem.c \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_core/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_draw/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_font/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_gpu/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_hal/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_misc/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_themes/*.c) \
	$(wildcard $(CURDIR)/lib/lvgl-sdl/lvgl/src/lv_widgets/*.c)
CFLAGS += \
	-I$(CURDIR)/lib/lvgl-sdl/ \
	-I$(CURDIR)/lib/lvgl-sdl/lv_drv \
	-I$(CURDIR)/lib/lvgl-sdl/lvgl/ \
	-I$(CURDIR)/lib/lvgl-sdl/lvgl/src

include $(NXDK_DIR)/Makefile
