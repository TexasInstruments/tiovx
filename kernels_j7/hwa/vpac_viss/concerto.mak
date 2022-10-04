
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)
ifeq ($(BUILD_VPAC_VISS),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_viss
TARGETTYPE  := library
CSOURCES    := vx_kernels_hwa_target.c vx_vpac_viss_target_defaults.c vx_vpac_viss_target_dcc.c

ifeq ($(TARGET_CPU),R5F)
  ifeq ($(BUILD_VLAB),yes)
    CSOURCES    += vx_vpac_viss_target_sim_dcc.c vx_vpac_viss_vlab_target.c
    IDIRS       += $(J7_C_MODELS_PATH)/include
  else
    CSOURCES    += vx_vpac_viss_target.c vx_vpac_viss_target_drv.c
    CSOURCES    += viss_srvr_remote.c
  endif
else
  CSOURCES    += vx_vpac_viss_target_sim.c vx_vpac_viss_target_sim_priv.c
  IDIRS       += $(J7_C_MODELS_PATH)/include
  DEFS        += __aarch64__
endif

IDIRS       += $(PDK_PATH)/packages
IDIRS       += $(VISION_APPS_PATH)/

ifeq ($(TARGET_OS),SYSBIOS)
  IDIRS       += $(XDCTOOLS_PATH)/packages
  IDIRS       += $(BIOS_PATH)/packages
endif

IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(IMAGING_PATH)/algos/dcc/include
IDIRS       += $(IMAGING_PATH)/algos/awb/include

IDIRS       += $(VISION_APPS_PATH)/utils/remote_service/include
IDIRS       += $(VISION_APPS_PATH)/utils/ipc/include

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

ifeq ($(HOST_COMPILER),GCC_LINUX)
CFLAGS += -Wno-unused-result
endif

ifeq ($(SOC)$(TARGET_CPU),am62ax86_64)
SKIPBUILD=1
endif

include $(FINALE)

endif
endif
endif