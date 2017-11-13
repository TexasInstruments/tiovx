# This file can optionally be used to define environment variables which
# are needed by the kernel libraries defined in this folder, or can be
# used to overwrite environment variables from the psdk_tools_path.mak
# and vsdk_tools_path.mak files from the tiovx directory.

TDA4x_C_MODELS_PATH ?= $(CUSTOM_KERNEL_PATH)/../tda4x_c_models/

# set values of below to yes or no to include or exclude the modules from compile and link
BUILD_HWA_VPAC_LDC=yes
BUILD_HWA_VPAC_NF=yes
BUILD_HWA_VPAC_MSC=yes
BUILD_HWA_VPAC_VISS=no
BUILD_HWA_DMPAC_SDE=yes
BUILD_HWA_DMPAC_DOF=yes

