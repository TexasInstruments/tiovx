# This file contains a list of extension kernel specific static libraries
# to be included in the PC executables.  It is put in this separate file
# to make it easier to add/extend kernels without needing to modify
# several concerto.mak files which depend on kernel libraries.

ifeq ($(TARGET_CPU),x86_64)

STATIC_LIBS += vx_vxu

ifeq ($(BUILD_HWA_KERNELS),yes)

STATIC_LIBS += vx_kernels_hwa_tests vx_kernels_hwa vx_vxu

STATIC_LIBS += vx_target_kernels_dmpac_sde
STATIC_LIBS += sde_hw

STATIC_LIBS += vx_target_kernels_vpac_ldc
STATIC_LIBS += ldc

ifeq ($(BUILD_HWA_DMPAC_DOF),yes)
    STATIC_LIBS += vx_target_kernels_dmpac_dof
    STATIC_LIBS += vx_kernels_hwa_tests
endif

STATIC_LIBS += vx_target_kernels_vpac_msc
STATIC_LIBS += scalar

STATIC_LIBS += vx_target_kernels_vpac_nf
STATIC_LIBS += bl_filter_lib

STATIC_LIBS += vx_target_kernels_vpac_viss
STATIC_LIBS += rawfe nsf4 flexcc h3a ee utils glbce
SYS_SHARED_LIBS += glbce

ifeq ($(BUILD_VPAC3),yes)
    STATIC_LIBS += cac RawHistogram nsf4_wb flexcfa_vpac3
else
    STATIC_LIBS += flexcfa
endif

STATIC_LIBS += vx_target_kernels_j7_arm

endif #ifeq ($(BUILD_HWA_KERNELS),yes)

STATIC_LIBS += vx_utils
STATIC_LIBS += vx_tiovx_tidl_tests
STATIC_LIBS += vx_kernels_tidl vx_target_kernels_tidl vx_target_kernels_ivision_common tidl_algo tidl_priv_algo tidl_obj_algo tidl_custom
STATIC_LIBS += C7100-host-emulation

# Uncomment below to link to TIDL/MMALIB in host emulation mode instead of natural C mode on PC
# STATIC_LIBS += mmalib_x86_64 mmalib_cn_x86_64 common_x86_64
# ADDITIONAL_STATIC_LIBS += dmautils.lib udma.lib sciclient.lib ti.csl.lib ti.osal.lib

SYS_SHARED_LIBS += rt dl png z

LDIRS+=$(J7_C_MODELS_PATH)/lib/PC/x86_64/LINUX/release
LDIRS+=$(CGT7X_ROOT)/host_emulation
LDIRS+=$(MMALIB_PATH)/lib/$(TARGET_BUILD)
LDIRS+=$(TIDL_PATH)/lib/PC/dsp/algo/$(TARGET_BUILD)

LDIRS+= $(PDK_PATH)/packages/ti/drv/udma/lib/j721e_hostemu/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/csl/lib/j721e/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/drv/sciclient/lib/j721e_hostemu/c7x-hostemu/$(TARGET_BUILD)
LDIRS+= $(PDK_PATH)/packages/ti/osal/lib/nonos/j721e/c7x-hostemu/$(TARGET_BUILD)


LDIRS       += $(IMAGING_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += ti_imaging_dcc

LDIRS       += $(VISION_APPS_PATH)/lib/PC/$(TARGET_CPU)/$(TARGET_OS)/$(TARGET_BUILD)
STATIC_LIBS += app_utils_iss

PDK_LIBS =
PDK_LIBS += dmautils.lib
PDK_LIBS += udma.lib
PDK_LIBS += sciclient.lib
PDK_LIBS += ti.csl.lib
PDK_LIBS += ti.osal.lib

MMA_LIBS =
MMA_LIBS += mmalib_cn_x86_64
MMA_LIBS += mmalib_x86_64
MMA_LIBS += common_x86_64

ADDITIONAL_STATIC_LIBS += $(PDK_LIBS)

STATIC_LIBS += $(MMA_LIBS)

endif #ifeq ($(TARGET_CPU),x86_64)
