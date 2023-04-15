# currently required to be set to yes
BUILD_CONFORMANCE_TEST?=yes
# currently required to be set to yes
BUILD_TUTORIAL?=yes
# currently required to be set to no
BUILD_BAM?=no

# Flags to enable disable, groups of conformance tests (CT)
BUILD_CT_KHR=yes
BUILD_CT_TIOVX=yes
BUILD_CT_TIOVX_TEST_KERNELS=yes
BUILD_CT_TIOVX_IVISION=yes

BUILD_CT_TIOVX_TIDL=yes
BUILD_CT_TIOVX_TVM=yes

BUILD_CT_TIOVX_HWA=yes
BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS=yes

BUILD_CT_TIOVX_VIDEO_IO=yes
BUILD_CT_TIOVX_VIDEO_IO_CAPTURE_TESTS=no
BUILD_CT_TIOVX_VIDEO_IO_DISPLAY_TESTS=no

# Applied to target mode only
BUILD_VLAB?=no

