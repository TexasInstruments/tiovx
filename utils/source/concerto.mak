ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_utils
TARGETTYPE  := library

CSOURCES :=

ifeq ($(TARGET_PLATFORM),PC)
ifeq ($(TARGET_OS),LINUX)
CSOURCES += tivx_utils_png_rd_wr.c
PNGVERSION := $(shell pkg-config --modversion libpng)
ifeq ($(findstring 1.2.,$(PNGVERSION)),1.2.)
DEFS += USING_LIBPNG_1_2
endif
endif
endif

CSOURCES += tivx_utils_bmp_rd_wr.c tivx_utils_graph_perf.c tivx_utils_checksum.c tivx_utils_bmp.c tivx_utils.c

ifneq ($(BUILD_SDK), $(filter $(BUILD_SDK), vsdk psdk))
CSOURCES += tivx_utils_tidl_trace.c
endif

ifneq ($(TARGET_PLATFORM),PC)
CSOURCES += tivx_utils_png_rd_wr_null.c
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A72))
CSOURCES += tivx_utils_ipc_ref_xfer.c
endif

IDIRS += $(TIOVX_PATH)/utils/include
IDIRS += $(TIOVX_PATH)/source/include
IDIRS += $(CUSTOM_KERNEL_PATH)/tidl/include

include $(FINALE)

endif
