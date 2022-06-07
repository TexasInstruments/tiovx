
ifeq ($(TARGET_CPU), $(filter $(TARGET_CPU), X86 x86_64 R5F))
ifeq ($(BUILD_HWA_KERNELS),yes)
ifeq ($(BUILD_VPAC_MSC),yes)

include $(PRELUDE)
TARGET      := vx_target_kernels_vpac_msc
TARGETTYPE  := library
ifeq ($(TARGET_CPU),R5F)
  ifeq ($(BUILD_VLAB),yes)
    CSOURCES    := vx_vpac_msc_scale_target_sim.c vx_vpac_msc_pyramid_target_sim.c vx_vpac_msc_halfscalegaussian_target_sim.c vx_vpac_msc_multi_scale_output_target_sim.c
    IDIRS       += $(J7_C_MODELS_PATH)/include
  else
    CSOURCES    := vx_vpac_msc_multi_scale_output_target.c vx_vpac_msc_pyramid_target.c vx_vpac_msc_scale_target.c
    IDIRS       += $(PDK_PATH)/packages
    IDIRS       += $(VISION_APPS_PATH)/
    ifeq ($(TARGET_OS),SYSBIOS)
      IDIRS       += $(XDCTOOLS_PATH)/packages
      IDIRS       += $(BIOS_PATH)/packages
    endif
  endif
else
  CSOURCES    := vx_vpac_msc_scale_target_sim.c vx_vpac_msc_pyramid_target_sim.c vx_vpac_msc_halfscalegaussian_target_sim.c vx_vpac_msc_multi_scale_output_target_sim.c
  IDIRS       += $(J7_C_MODELS_PATH)/include
endif

CSOURCES    += vx_kernels_hwa_target.c
IDIRS       += $(CUSTOM_KERNEL_PATH)/hwa/include
IDIRS       += $(VXLIB_PATH)/packages
IDIRS       += $(TIOVX_PATH)/kernels/openvx-core/include

ifeq ($(TARGET_CPU)$(BUILD_VLAB),R5Fyes)
DEFS += VLAB_HWA
endif

ifeq ($(SOC)$(TARGET_CPU),am62ax86_64)
SKIPBUILD=1
endif

include $(FINALE)

endif
endif
endif