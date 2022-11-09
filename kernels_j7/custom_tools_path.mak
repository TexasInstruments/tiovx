# This file can optionally be used to define environment variables which
# are needed by the kernel libraries defined in this folder, or can be
# used to overwrite environment variables from the psdk_tools_path.mak
# and vsdk_tools_path.mak files from the tiovx directory.

# set values of below to yes or no to include or exclude the modules from compile and link

BUILD_HWA_KERNELS?=yes
BUILD_VPAC_VISS?=yes
BUILD_VPAC_MSC?=yes
BUILD_VPAC_LDC?=yes

ifeq ($(SOC),am62a)
	BUILD_CAPTURE=no
	BUILD_CSITX=no
	BUILD_DISPLAY=no
	BUILD_DMPAC_DOF=no
	BUILD_DMPAC_SDE=no
	BUILD_VPAC_NF=no
else
	BUILD_CAPTURE?=yes
	BUILD_CSITX?=yes
	BUILD_DISPLAY?=yes
	BUILD_DMPAC_DOF?=yes
	BUILD_DMPAC_SDE?=yes
	BUILD_VPAC_NF?=yes
endif
