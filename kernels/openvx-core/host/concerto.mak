
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 R5F))

	include $(PRELUDE)
	TARGET      := vx_kernels_openvx_core
	TARGETTYPE  := library
	CSOURCES    := $(call all-c-files)
	IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include
	IDIRS       += $(CUSTOM_KERNEL_PATH)/include
	IDIRS       += $(TIOVX_PATH)/source/include


	include $(FINALE)

endif
