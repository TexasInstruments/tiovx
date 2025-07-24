
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66))

	include $(PRELUDE)
	TARGET      := vx_target_kernels_openvx_core
	TARGETTYPE  := library
	CSOURCES    := $(call all-c-files)
	IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include
	IDIRS       += $(HOST_ROOT)/kernels/include
	IDIRS       += $(VXLIB_PATH)/packages
	IDIRS       += $(HOST_ROOT)/kernels/openvx-core/c66x
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

	DEFS += CORE_DSP CORE_C6XX

	ifeq ($(BUILD_BAM),yes)
		DEFS += BUILD_BAM
	endif

	ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
		CFLAGS  += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
	endif

	include $(FINALE)

endif

ifneq ($(TARGET_PLATFORM),J721E)

	ifeq ($(TARGET_FAMILY),DSP)

		ifneq ($(TARGET_CPU),C66)

		include $(PRELUDE)
		TARGET      := vx_target_kernels_openvx_core
		TARGETTYPE  := library
		CSOURCES    := $(call all-c-files)
		IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include
		IDIRS       += $(HOST_ROOT)/kernels/include
		IDIRS       += $(VXLIB_PATH)/packages
		IDIRS       += $(HOST_ROOT)/kernels/openvx-core/c66x
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

		DEFS += CORE_DSP

		ifeq ($(BUILD_BAM),yes)
			DEFS += BUILD_BAM
		endif

		DEFS += C6X_MIGRATION _TMS320C6600

		include $(FINALE)

		endif # ifneq ($(TARGET_CPU),C66)

	endif # ifeq ($(TARGET_FAMILY),DSP)

endif # ifneq ($(TARGET_PLATFORM),J721E)