# TIOVX Build & Run Instructions {#BUILD_INSTRUCTIONS}

[TOC]

# Build instructions for x86_64 Linux Platform (HOST Emulation Mode) {#BUILD_LINUX}

## Requirements
  - Tested on Ubuntu Linux x86_64 machine, v22.04 LTS
  - Tested with GCC 11
  - Download and Run PSDK RTOS Add-on package installer file **ti-processor-sdk-rtos-${SOC}-evm-xx_xx_xx_xx-linux-x64-installer.run** on a Ubuntu 22.04 (x86_64). This package
    contains performance datasheet documentation as well as PC compiled cmodel libraries for VPAC and DMPAC OpenVX nodes.
    \attention The installer is available only through TI mySecure login.
    \attention To Request access for PSDK RTOS Add-on Packages the first time: [<a href="https://www.ti.com/licreg/docs/swlicexportcontrol.tsp?form_id=276074&prod_no=PSDK-RTOS-AUTO&ref_url=adas">LINK</a>]
  \attention mySecure SW Access Link (after access has been granted) : [<a href="https://www.ti.com/securesoftware/docs/securesoftwarehome.tsp">LINK</a>]
  - Do below to download and install additional dependencies needed to build TIOVX
    \code
    cd ${PSDKR_PATH}
    ./sdk_builder/scripts/setup_psdk_rtos.sh
    \endcode

## Setup
  - Edit sdk_builder/tools_path.mak
    - Modify "GCC_LINUX_ROOT" to match your environment if needed

## Build
  - Open a command prompt at $TIOVX_PATH
  - Type "make" to build TIOVX and associated libraries
    - This builds the TIOVX libraries, conformance tests, and tutorial executables
    - This uses pre-built library for VXLIB, from "$TIOVX_PATH\lib\PC\x86_64\LINUX\$PROFILE\*.a"
    - The pre-built libraries are compiled in 64bit x86 host emulation mode
  - Conformance test, tutorial executable is output at
    - "$TIOVX_PATH\out\PC\x86_64\LINUX\debug\vx_conformance_tests_exe"
    - "$TIOVX_PATH\out\PC\x86_64\LINUX\debug\vx_tutorial_exe"

## Running the conformance test and tutorial
  - Open a command prompt at the folder "$TIOVX_PATH/out/PC/x86_64/LINUX/$PROFILE/"
  - Download and untar the test_data package from the associated release page
  - Set the environment variable below to specify the path of the test data. This is used by both
    the conformance test and tutorial executables
    \code
    export VX_TEST_DATA_PATH=<path_to_untarred_test_data>
    \endcode
  - Execute "./vx_conformance_tests_exe" to run the conformance test
  - Execute "./vx_tutorial_exe" to run the tutorial
  - NOTE: Output .bmp/.png files generated by the tutorial will be present in the folder $VX_TEST_DATA_PATH

# Build instructions for J7 EVM Platform (Target Mode) {#BUILD_EVM}

  - TIOVX is built for the J7 SoC as a part of the SDK Builder "make sdk" command.  Please refer to the Vision Apps User Guide for more details.

---

# Build options (found in Makefile, build_flags.mak, or sdk_builder/build_flags.mak) {#BUILD_OPTIONS}

Build Option                         | Description | Default Setting |
-------------------------------------|-------------|-----------------|
PROFILE                        | Determines which profile to build for. Valid values are: release / debug / all  | all |
BUILD_EMULATION_MODE           | Builds PC emulation mode | yes |
BUILD_EMULATION_ARCH           | PC emulation architecture. Valid values are: X86 / x86_64 / all | x86_64 |
BUILD_TARGET_MODE              | Builds for target SoC platform like TDAxx | yes |
BUILD_CONFORMANCE_TEST  | Builds entire test suite executable  | yes |
BUILD_IVISION_KERNELS   | Builds iVision kernels  | no |
BUILD_TUTORIAL          | Builds OpenVX tutorial  | yes |
BUILD_LINUX_MPU         | Builds for A72 Linux target (NOT used in PC HOST emulation mode | yes |
BUILD_SDK               | Builds for SDK SW platform. Valid values are: psdkra (for Processor SDK RTOS J7) / platform | psdkra |
BUILD_IGNORE_LIB_ORDER  | When set to yes, it ignores the static library order listed in makefiles when building on the PC. | yes |
BUILD_CT_KHR  | Builds and includes the Khronos OpenVX 1.1 conformance tests suite.  | yes |
BUILD_CT_TIOVX  | Builds and includes the TI-added tests suite (for TI extensions and additional rohbustness testing).  | yes |
BUILD_CT_TIOVX_IVISION  | Builds and includes the tests for IVISION kernels test suite.       | no |
BUILD_CT_TIOVX_TIDL  | Builds and includes the tests for TIDL kernel test suite.     | yes |
BUILD_CT_TIOVX_HWA  | Builds and includes the tests for HWA kernels test suite.      | yes |
BUILD_CT_TIOVX_HWA_NEGATIVE_TESTS | Builds and includes a large set of negative tests for HWA kernels | yes |
BUILD_CT_TIOVX_HWA_DISPLAY_TESTS | Builds and includes display test cases <BR> Note: in order to run on J7 platform, a display must be connected | no |
BUILD_CT_TIOVX_HWA_CAPTURE_TESTS | Builds and includes a large set of negative tests for HWA kernels <BR> Note: in order to run on J7 platform, 4 IMX390 cameras must be connected to a Fusion board which is connected to the EVM| no |
BUILD_CT_TIOVX_HWA_CSITX_TESTS | Builds and includes csitx test cases <BR> Note: in order to run on J7 platform, the following setup is required: <ul><li> EVM Board Configuration: By default DPHY is connected to FPD Panel (DSI-TX), it has to be changed to DSI FPC(CSI-TX). </li><li> J7X LI(Leopard Imaging) Serial Capture Board </li><li> FPC Cable: Connect Csitx to Csirx. </li></ul> This test uses CSIRX to receive the data transmitted by CSITX, hence lane speed for both modules should be same. This tests confgiures the CSITX lane speed to 800 Mbps.| no |
BUILD_TYPE              | Specifies the build configuration as development or production. Valid values are: dev / prod | dev |
BUILD_CORE_KERNELS      | Builds and loads all core kernels and associated test cases. This flag is enabled when BUILD_TYPE is "dev" and disabled when BUILD_TYPE is "prod". | yes |
BUILD_EXT_KERNELS       | Builds and loads all extension kernels and associated test cases. This flag is enabled when BUILD_TYPE is "dev" and disabled when BUILD_TYPE is "prod". | yes |
BUILD_TEST_KERNELS      | Builds and loads all test kernels and associated test cases. This flag is enabled when BUILD_TYPE is "dev" and disabled when BUILD_TYPE is "prod". | yes |

---


# Deleting all generated files {#BUILD_CLEAN}

 - To do a clean build, do \code make clean \endcode
 - To delete both "out" and "lib" output directories, do \code make scrub \endcode
 - To delete the "out" directory for a specific core, do the following and specify a core:
   \code make clean_[core] \endcode
   ex. \code make clean_r5f \endcode
