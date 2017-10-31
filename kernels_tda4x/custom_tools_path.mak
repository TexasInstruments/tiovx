# This file can optionally be used to define environment variables which
# are needed by the kernel libraries defined in this folder, or can be
# used to overwrite environment variables from the psdk_tools_path.mak
# and vsdk_tools_path.mak files from the tiovx directory.

BL_FILTER_PATH ?= /home/a0323847local/gitrepo/bl_filter
SDE_PATH ?= /home/a0323847local/gitrepo/tda4x_cmodels/sde_tiovx_dv
VPAC_LDC_PATH ?= /home/a0323847local/gitrepo/tda4x_cmodels/ldc/vpac_ldc
DMPAC_DOF_PATH ?= /home/kedarc/code/tda4x_cmodels/dof/tidofimulator1.6.5.alpha

# set values of below to yes or no to include or exclude the modules from compile and link
BUILD_HWA_VPAC_LDC=no
BUILD_HWA_VPAC_NF=no
BUILD_HWA_VPAC_MSC=no
BUILD_HWA_VPAC_VISS=no
BUILD_HWA_DMPAC_SDE=no
BUILD_HWA_DMPAC_DOF=yes

