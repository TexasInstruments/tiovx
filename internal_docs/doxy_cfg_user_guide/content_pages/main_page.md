# Overview {#mainpage}

[TOC]

# Introduction

TIOVX is TI's implementation of OpenVX Standard.

TIOVX allows users to create vision and compute applications using OpenVX API.
These OpenVX applications can be executed on TI SoCs like TDA2x, TDA3x and TDA4x. TIOVX is fully conformant
to OpenVX v1.1 specification. TIOVX also provides optimized OpenVX kernels for C66x DSP. An extension API
allows users to integrate their own natively developed custom kernels and call them using OpenVX APIs.
Examples showing usage of TIOVX as part of the larger system, ex, camera input and display output are also
provided as part of Processor SDK Vision.

Top level block diagram of TIOVX is shown below.

\if DOCS_J7
\image html tiovx_block_diagram_j7.png "TIOVX Block Diagram" width=600
\else
\image html tiovx_block_diagram.png "TIOVX Block Diagram" width=600
\endif


The components of this diagram are described below

TIOVX Module                         | Description
-------------------------------------|------------
Khronos Conformance Test             | OpenVX conformance test from Khronos to make sure an implementation implements OpenVX according to specification
TI Extension Conformance Test        | Additional test cases from TI to test TI extention interfaces
OpenVX API                           | OpenVX API as defined by Khronos
TIOVX API                            | TI extensions and additional APIs in order to efficiently use OpenVX on TI platforms
TIOVX Framework                      | TI's implementation of OpenVX spec. This layer is agnostic of underlying SoC, OS platform
TIOVX Platform                       | This layer binds TIOVX framework to a specific platform. Ex, Processor SDK platform for TDA4x SOCs. This layer also binds TIOVX framework to a specific OS like Linux or TI-RTOS.
TIOVX Kernel Wrapper                 | Kernel wrappers allow TI and customers to integrate a natively implemented kernel into the TIOVX framework.
Examples                             | These are examples which show usage of TIOVX with other system level compoenents. These are not included in TIOVX package. Users should refer to SDK for these examples.
User Kernels / Target Kernels        | User kernels is an interface to integrate user kernels on HOST CPU using standard Khronos OpenVX APIs. Target kernels is a TI specific interface to integrate user kernels on target CPU like DSP


---

# Directory Structure {#TIOVX_PACKAGE_CONTENTS}

The following describes the contents of the package.

Folder | Description
-------|-----
conformance_tests/kernels           | Kernels added only for test purposes (to test target kernel APIs)
conformance_tests/test_conformance  | Khronos OpenVX conformance test
conformance_tests/test_engine       | ^
conformance_tests/test_data         | Data files used for Khronos Conformance test, TI test suite, and tutorial input data files
conformance_tests/test_executable   | Khronos OpenVX conformance test executable (PC HOST emulation mode only)
conformance_tests/test_tiovx        | Additional test cases for Khronos OpenVX APIs and TI extention APIs
docs                | User documentation
include/VX          | Khronos OpenVX interface
include/TI          | TI OpenVX extension interface
kernels/openvx-core | OpenVX defined kernels
kernels*            | TI vendor-specific kernels, including unit test for the kernels
source/framework    | TI OpenVX implementation
source/include      | ^
source/vxu          | ^
source/platform     | TI OpenVX platform adaptation layer
tools               | OpenVX use case and kernel wrapper code generation tool
tutorial            | TI OpenVX tutorials
utils               | Helpful utilities like image readers
out                 | Build generated files and executables
lib                 | Pre-built dependency libraries (PC) and tiovx libraries copied from out folder at end of build


Note: since the 8.6 release the below folders have been moved to the following locations:


8.6 Location          | 9.0 (and beyond) Location
----------------------|--------------------------
tiovx/concerto        | sdk_builder/concerto
tiovx/kernels_j7/hwa  | imaging/kernels/hwa
tiovx/kernels_j7/tidl | c7x-mma-tidl/arm-tidl/tiovx_kernels/tidl
