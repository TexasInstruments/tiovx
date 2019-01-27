# currently required to be set to yes
BUILD_CONFORMANCE_TEST?=yes
# currently required to be set to yes
BUILD_TUTORIAL?=yes
# currently required to be set to no
BUILD_BAM?=yes
# currently required to be set to yes
BUILD_IGNORE_LIB_ORDER?=yes

# Build for SoC
BUILD_TARGET_MODE?=yes
# Build for x86 PC
BUILD_EMULATION_MODE?=no
# valid values: X86 x86_64 all
BUILD_EMULATION_ARCH?=X86

# Flags to enable disable, groups of conformance tests (CT)
BUILD_CT_KHR=yes
BUILD_CT_TIOVX=yes
BUILD_CT_TIOVX_TEST_KERNELS=yes
BUILD_CT_TIOVX_IVISION=yes
BUILD_CT_TIOVX_TIDL=yes
BUILD_CT_TIOVX_HWA=no

# valid values: release debug all
PROFILE?=debug

BUILD_LINUX_A15?=yes
BUILD_EVE?=yes
BUILD_IVISION_KERNELS?=yes
