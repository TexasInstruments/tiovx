include $(PRELUDE)
TARGET      := vx_target_kernels_c66_bam
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/include
IDIRS       += $(HOST_ROOT)/conformance_tests/kernels/test_kernels/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(ALGFRAMEWORK_PATH)/inc
IDIRS       += $(ALGFRAMEWORK_PATH)/src/bam_dma_nodes
IDIRS       += $(DMAUTILS_PATH)/inc
IDIRS       += $(DMAUTILS_PATH)
IDIRS       += $(DMAUTILS_PATH)/inc/edma_utils
IDIRS       += $(DMAUTILS_PATH)/inc/edma_csl
IDIRS       += $(DMAUTILS_PATH)/inc/baseaddress/vayu/dsp
# < DEVELOPER_TODO: Add any custom include paths using 'IDIRS' >
# < DEVELOPER_TODO: Add any custom preprocessor defines or build options needed using
#                   'CFLAGS'. >
# < DEVELOPER_TODO: Adjust which cores this library gets built on using 'SKIPBUILD'. >

DEFS += CORE_DSP

ifeq ($(BUILD_BAM),yes)
DEFS += BUILD_BAM
endif

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
DEFS += _HOST_BUILD _TMS320C6600 TMS320C66X HOST_EMULATION
endif

ifeq ($(BUILD_BAM),yes)
SKIPBUILD=0
else
SKIPBUILD=1
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

