ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66))

	include $(PRELUDE)
	TARGET      := vx_target_kernels_tutorial
	TARGETTYPE  := library
	CSOURCES    := $(call all-c-files)
	IDIRS       += $(HOST_ROOT)/tutorial/ch03_graph/c66x
	IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
	IDIRS       += $(VXLIB_PATH)/packages

	ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
		CFLAGS  += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
	endif

	include $(FINALE)

endif

ifneq ($(TARGET_PLATFORM),J721E)

	ifeq ($(TARGET_FAMILY),DSP)

		ifneq ($(TARGET_CPU),C66)

			include $(PRELUDE)
			TARGET      := vx_target_kernels_tutorial
			TARGETTYPE  := library
			CSOURCES    := $(call all-c-files)
			IDIRS       += $(HOST_ROOT)/tutorial/ch03_graph/c66x
			IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
			IDIRS       += $(VXLIB_PATH)/packages

			DEFS        += C6X_MIGRATION _TMS320C6600

			include $(FINALE)

		endif # ifneq ($(TARGET_CPU),C66)

	endif # ifeq ($(TARGET_FAMILY),DSP)

endif # ifneq ($(TARGET_PLATFORM),J721E)
