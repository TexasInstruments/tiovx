
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 A15 M4 A72 R5F))

include $(PRELUDE)
TARGET      := vx_kernels_openvx_core
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/kernels/openvx-core/include

ifneq ($(BUILD_SDK), $(filter $(BUILD_SDK), vsdk psdk))
IDIRS += $(CUSTOM_KERNEL_PATH)/include
endif

include $(FINALE)

endif
