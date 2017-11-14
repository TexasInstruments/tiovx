

ifeq ($(TARGET_PLATFORM),PC)
ifeq ($(TARGET_OS),LINUX)

include $(PRELUDE)
TARGET      := vx_utils
TARGETTYPE  := library


CSOURCES    := \
	tivx_utils_png_rd_wr.c \

IDIRS += $(TIOVX_PATH)/utils/include

include $(FINALE)

endif
endif
