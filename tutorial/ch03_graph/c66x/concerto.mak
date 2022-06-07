

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 C66))

include $(PRELUDE)
TARGET      := vx_target_kernels_tutorial
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/tutorial/ch03_graph/c66x
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
IDIRS       += $(VXLIB_PATH)/packages

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

include $(FINALE)

endif

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), J721S2 J784S4 AM62A))

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7120 C7504))

include $(PRELUDE)
TARGET      := vx_target_kernels_tutorial
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)
IDIRS       += $(HOST_ROOT)/tutorial/ch03_graph/c66x
IDIRS       += $(TIOVX_PATH)/tutorial/ch01_common
IDIRS       += $(VXLIB_PATH)/packages

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), C71 C7100 C7120 C7504))
DEFS += C6X_MIGRATION _TMS320C6600
endif

include $(FINALE)

endif

endif
