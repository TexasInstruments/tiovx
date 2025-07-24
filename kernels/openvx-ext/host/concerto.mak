
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 A72 A53 R5F))

	include $(PRELUDE)
	TARGET      := vx_kernels_openvx_ext
	TARGETTYPE  := library
	CSOURCES    := $(call all-c-files)
	IDIRS       := $(HOST_ROOT)/kernels/include
	IDIRS       += $(HOST_ROOT)/kernels/openvx-ext/include

	include $(FINALE)
endif

