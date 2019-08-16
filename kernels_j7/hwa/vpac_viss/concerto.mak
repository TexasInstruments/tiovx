
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_viss
TARGETTYPE  := library
CSOURCES    := vx_kernels_hwa_target.c

ifeq ($(TARGET_CPU),R5F)
  ifeq ($(BUILD_VLAB),yes)
    CSOURCES    += vx_vpac_viss_target_sim_dcc.c vx_vpac_viss_vlab_target.c
    IDIRS       += $(J7_C_MODELS_PATH)/include
  else
    CSOURCES    += vx_vpac_viss_target.c vx_vpac_viss_target_dcc.c vx_vpac_viss_target_drv.c
    IDIRS       += $(PDK_PATH)/packages
    IDIRS       += $(VISION_APPS_PATH)/
    IDIRS       += $(XDCTOOLS_PATH)/packages
    IDIRS       += $(BIOS_PATH)/packages
    DEFS        += SOC_J721E
  endif
else
  CSOURCES    += vx_vpac_viss_target_sim_dcc.c vx_vpac_viss_target_sim.c
  IDIRS       += $(J7_C_MODELS_PATH)/include
endif

IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(HOST_ROOT)/kernels/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(IMAGING_PATH)/algos/dcc/include
IDIRS       += $(IMAGING_PATH)/algos/awb/include

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-result
endif

include $(FINALE)

endif
