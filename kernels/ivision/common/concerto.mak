
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64 C66 C71 C7120 C7504 C7524))


include $(PRELUDE)
TARGET      := vx_target_kernels_ivision_common
TARGETTYPE  := library
CSOURCES    := $(call all-c-files)

IDIRS       += $(HOST_ROOT)/kernels/ivision/include
IDIRS       += $(IVISION_PATH)/

ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), x86_64))
CFLAGS      += -D_HOST_BUILD -D_TMS320C6600 -DTMS320C66X -DHOST_EMULATION
endif

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-switch
endif


include $(FINALE)

endif
