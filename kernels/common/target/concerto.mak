
include $(PRELUDE)
TARGET      := vx_kernels_target_utils
TARGETTYPE  := library
CSOURCES    := tivx_kernels_target_utils.c
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(ALGFRAMEWORK_PATH)/inc \
               $(ALGFRAMEWORK_PATH)/src/bam_dma_nodes \
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

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION

SKIPBUILD=0
DEFS += CORE_DSP CORE_C6XX

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
CSOURCES    += tivx_bam_kernel_wrapper.c \
               tivx_bam_kernel_database.c
endif

endif

ifeq ($(TARGET_CPU),C66)
SKIPBUILD=0
DEFS += CORE_DSP CORE_C6XX

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
CSOURCES    += tivx_bam_kernel_wrapper.c \
               tivx_bam_kernel_database.c
endif

endif

ifeq ($(TARGET_CPU),EVE)
SKIPBUILD=0
endif

ifeq ($(TARGET_CPU),A15)
SKIPBUILD=1
endif

ifeq ($(TARGET_CPU),M4)
SKIPBUILD=1
endif

include $(FINALE)
