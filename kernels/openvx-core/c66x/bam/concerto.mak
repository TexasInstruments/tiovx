


include $(PRELUDE)
TARGET      := vx_target_kernels_openvx_core_bam
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/include \
               $(HOST_ROOT)/kernels/openvx-core/include \
               $(HOST_ROOT)/kernels/openvx-core/c66x \
               $(VXLIB_PATH)/packages \
               $(ALGFRAMEWORK_PATH)/inc \
               $(ALGFRAMEWORK_PATH)/src/bam_dma_nodes \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH) \
               $(DMAUTILS_PATH)/inc \
               $(DMAUTILS_PATH)/inc/edma_utils \
               $(DMAUTILS_PATH)/inc/edma_csl \
               $(DMAUTILS_PATH)/inc/baseaddress/vayu/dsp \
               $(EDMA3_LLD_PATH) \
               $(EDMA3_LLD_PATH)/packages/ti/sdo/edma3/rm \
               $(EDMA3_LLD_PATH)/packages

DEFS += CORE_C6XX

ifeq ($(BUILD_BAM),yes)
SKIPBUILD=0
else
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),X86)
CFLAGS		+= -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION 
endif

ifeq ($(TARGET_CPU),C66)
endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif


include $(FINALE)
