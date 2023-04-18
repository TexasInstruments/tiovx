# This file can optionally be used to define environment variables which
# are needed by the kernel libraries defined in this folder, or can be
# used to overwrite environment variables from the psdk_tools_path.mak
# and vsdk_tools_path.mak files from the tiovx directory.

# set values of below to yes or no to include or exclude the modules from compile and link

ifeq ($(SOC),am62a)
	BUILD_VIDEO_IO_KERNELS=yes
	BUILD_CAPTURE=yes
	BUILD_CSITX=no
	BUILD_DISPLAY=no
else
	BUILD_VIDEO_IO_KERNELS?=yes
	BUILD_CAPTURE?=yes
	BUILD_CSITX?=yes
	BUILD_DISPLAY?=yes
endif
