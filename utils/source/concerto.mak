

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_utils
TARGETTYPE  := library

CSOURCES :=

ifeq ($(TARGET_PLATFORM),PC)
ifeq ($(TARGET_OS),LINUX)
CSOURCES += tivx_utils_png_rd_wr.c
endif
endif

CSOURCES += tivx_utils_bmp_rd_wr.c

IDIRS += $(TIOVX_PATH)/utils/include
IDIRS += $(TIOVX_PATH)/conformance_tests

include $(FINALE)

endif
