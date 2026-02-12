# Build Configuration Examples and Workflows {#BUILD_EXAMPLES}

[TOC]

This page provides practical examples for building TIOVX with various configuration options, including builds with PyTIOVX-generated custom kernels. These examples demonstrate common development scenarios and help you choose the right build flags for your use case.

\note For a complete list of all build options and their descriptions, see \ref BUILD_OPTIONS.

\note For custom kernel development, PyTIOVX is the recommended approach. See \ref PYTIOVX and the \ref BUILD_EXAMPLES_CUSTOM_KERNELS section below.

# Common Build Scenarios {#BUILD_EXAMPLES_COMMON}

## Quick Development Build (PC Emulation) {#BUILD_EXAMPLES_QUICK_DEV}

For rapid development and testing on your development PC without target hardware:

\code
# Build only release profile with core functionality (PC emulation only)
make PROFILE=release BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no
\endcode

**What this does:**
- Builds x86_64 PC emulation binaries only (BUILD_EMULATION_MODE=yes)
- Explicitly excludes target hardware build (BUILD_TARGET_MODE=no)
- Release profile for optimized performance
- Includes conformance tests and tutorials by default

**Output location:** `out/PC/x86_64/LINUX/release/`

**Use when:**
- Initial algorithm development
- Debugging without hardware
- Running automated tests in CI/CD
- Developing custom user kernels

---

## Minimal Build (Libraries Only) {#BUILD_EXAMPLES_MINIMAL}

When you only need TIOVX libraries without tests or tutorials:

\code
# Build libraries only, skip tests and tutorials (PC emulation)
make PROFILE=release BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no \
     BUILD_CONFORMANCE_TEST=no BUILD_TUTORIAL=no
\endcode

**What this does:**
- Builds TIOVX framework libraries for PC emulation
- Excludes conformance test executable
- Excludes tutorial executable
- Explicitly PC-only build (no target hardware)

**Use when:**
- Integrating TIOVX into a larger application
- Building for embedded systems with storage constraints
- Iterating on library changes only

---

## Full Development Build (All Features) {#BUILD_EXAMPLES_FULL_DEV}

Complete build with all tests, tutorials, and features enabled:

\code
# Build everything for development (PC emulation + target hardware)
make PROFILE=all BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=yes \
     BUILD_BAM=yes SOC=j721e
\endcode

**What this does:**
- Builds both debug and release profiles
- Builds for both PC emulation (BUILD_EMULATION_MODE=yes) and target hardware (BUILD_TARGET_MODE=yes)
- Includes all test suites (Khronos, TI, internal)
- Includes BAM/Supernode support
- Maximum validation coverage

**Output locations:**
- PC emulation: `out/PC/x86_64/LINUX/{debug,release}/`
- Target A72 (MPU): `out/<SOC>/A72/LINUX/{debug,release}/`
- Target R5F: `out/<SOC>/R5F/FREERTOS/{debug,release}/`
- Target C7x DSP: `out/<SOC>/<C7X_DIR>/FREERTOS/{debug,release}/`
- Target C66 DSP (J721E only): `out/<SOC>/C66/FREERTOS/{debug,release}/`

\note The C7x directory name varies by SoC (e.g., C7X, C7120). Check the actual `out/<SOC>/` directory for the exact name on your platform.

**Use when:**
- Comprehensive system validation
- Pre-release testing
- Full regression test execution

---

## Production Build {#BUILD_EXAMPLES_PRODUCTION}

Production build that excludes development and debug features:

\code
# Production build (target hardware only)
make PROFILE=release BUILD_TYPE=prod BUILD_EMULATION_MODE=no \
     BUILD_TARGET_MODE=yes SOC=j721e
\endcode

**What this does:**
- Target hardware only (BUILD_TARGET_MODE=yes, BUILD_EMULATION_MODE=no)
- Excludes debug logging functionality (vx_log_resource.c, tivx_perf.c)
- Excludes runtime trace logging (vx_log_rt_trace*.c)
- Excludes graph export to DOT functionality (vx_graph_export_dot.c)
- Excludes VXU functions (source/vxu)
- Excludes Supernode/BAM extensions (if BUILD_BAM=yes)
- Optimized for production deployment

**Limitations in production mode:**
- Debug logging APIs return failure codes
- Performance measurement APIs return failure codes
- Runtime trace APIs return failure codes
- Graph export APIs return failure codes
- VXU immediate-mode APIs return failure codes

**Use when:**
- Deploying to production systems
- Reducing code footprint
- Excluding debug/development features from final build

\note The BUILD_TYPE flag sets the BUILD_DEV compile-time definition when set to "dev" (default). When BUILD_TYPE=prod, BUILD_DEV is not defined, which excludes the features listed above through conditional compilation.

---

## Core-Specific Development Build {#BUILD_EXAMPLES_CORE_SPECIFIC}

### C7x DSP Development Only

When developing and testing kernels specifically for C7x DSP:

\code
# Build only C7x DSP target (no PC emulation)
make BUILD_C7X=1 BUILD_TARGET_MODE=yes BUILD_EMULATION_MODE=no \
     PROFILE=debug SOC=j721e
\endcode

**What this does:**
- Compiles only C7x DSP binaries for target hardware (BUILD_TARGET_MODE=yes)
- Skips PC emulation build (BUILD_EMULATION_MODE=no)
- Skips R5F, MPU (A72), and C66 builds
- Debug symbols enabled for DSP debugging
- Faster iteration for DSP kernel development

**Output location:** `out/j721e/<C7X_DIR>/FREERTOS/debug/`

\note C7x directory name may be C7X or platform-specific (e.g., C7120 on J784S4).

### R5F Real-Time Core Development

When developing real-time control code for R5F:

\code
# Build only R5F targets (no PC emulation)
make BUILD_R5F=1 BUILD_TARGET_MODE=yes BUILD_EMULATION_MODE=no \
     PROFILE=debug SOC=j721e
\endcode

**What this does:**
- Compiles only R5F binaries for target hardware (BUILD_TARGET_MODE=yes)
- Skips PC emulation build (BUILD_EMULATION_MODE=no)

**Output locations:**
- `out/j721e/R5F/FREERTOS/debug/` (MCU2_0)
- `out/j721e/R5F/FREERTOS/debug/` (MCU2_1)

### MPU Linux Development

When developing host application code on the ARM A72 MPU:

\code
# Build only MPU Linux target (no PC emulation)
make BUILD_MPU=1 BUILD_TARGET_MODE=yes BUILD_EMULATION_MODE=no \
     PROFILE=debug SOC=j721e
\endcode

**What this does:**
- Compiles only MPU (A72) binaries for target hardware (BUILD_TARGET_MODE=yes)
- Skips PC emulation build (BUILD_EMULATION_MODE=no)

**Output location:** `out/j721e/A72/LINUX/debug/`

---

## PyTIOVX-Generated Kernel Build {#BUILD_EXAMPLES_CUSTOM_KERNELS}

Building with PyTIOVX-generated custom kernels:

\note PyTIOVX is the recommended approach for creating custom kernels. It automates kernel wrapper generation, node creation APIs, and graph construction code. See \ref PYTIOVX for details.

### PC Emulation Build (Development/Testing)

\code
# Generate kernel wrappers using PyTIOVX
cd tools/sample_kernel_wrappers
python3 my_custom_kernel.py

# Build TIOVX with generated kernel path (PC emulation)
cd $TIOVX_PATH
make CUSTOM_KERNEL_PATH=tools/sample_kernel_wrappers/out \
     BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no PROFILE=debug
\endcode

**What this does:**
- PyTIOVX generates conformant kernel wrapper code
- Builds PC emulation binaries with custom kernels (BUILD_EMULATION_MODE=yes)
- Includes host kernel implementations for x86_64
- Debug profile for kernel validation

### Target Hardware Build (Deployment)

\code
# Build with PyTIOVX kernels for target hardware
make CUSTOM_KERNEL_PATH=tools/sample_kernel_wrappers/out \
     BUILD_EMULATION_MODE=no BUILD_TARGET_MODE=yes \
     PROFILE=release SOC=j721e
\endcode

**What this does:**
- Builds target binaries with custom kernels (BUILD_TARGET_MODE=yes)
- Includes target kernel implementations (ARM, C7x, etc.)
- Release profile for optimized performance

**PyTIOVX workflow:**
1. Define kernel specification in Python (parameters, data types, validation)
2. Run PyTIOVX to generate C code (host, target, node wrappers)
3. Build TIOVX with `CUSTOM_KERNEL_PATH` pointing to generated code
4. Use generated node APIs in your OpenVX application

**Example with both PC and target:**
\code
# Build with PyTIOVX kernels for both PC and target
make CUSTOM_KERNEL_PATH=/path/to/pytiovx/output \
     BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=yes \
     PROFILE=all SOC=j721e
\endcode

---

## Test Suite Selection Builds {#BUILD_EXAMPLES_TEST_SELECTION}

### Khronos Conformance Only

Build only Khronos OpenVX 1.1 conformance tests:

\code
# Khronos tests only
make PROFILE=release BUILD_CT_TIOVX=no
\endcode

### TI Extension Tests Only

Build only TI-specific extension tests:

\code
# TI extension tests only
make PROFILE=release BUILD_CT_KHR=no
\endcode

### All Tests Including Internal

Build with all TIOVX test suites (this is the default):

\code
# All TIOVX tests (default configuration)
make PROFILE=debug
\endcode

---

---

# Platform-Specific Examples {#BUILD_EXAMPLES_PLATFORMS}

## J721E Platform {#BUILD_EXAMPLES_J721E}

J721E (TDA4VM) has the most complete core configuration with C66 DSP support.

### Build All Cores
\code
# Build for J721E with all cores (all cores built by default)
make SOC=j721e PROFILE=release
\endcode

**Cores built:**
- A72 MPU (Linux/QNX host)
- R5F (MCU2_0, MCU2_1, MCU3_0, MCU3_1)
- C7x DSP1
- C66 DSP1, DSP2

### C66 DSP-Specific Build
\code
# Build only C66 DSP targets (J721E only)
make SOC=j721e BUILD_C66=1 PROFILE=debug
\endcode

---

## J721S2 Platform {#BUILD_EXAMPLES_J721S2}

J721S2 has dual C7x DSPs and no C66 DSPs.

\code
# Build for J721S2 (all available cores built by default)
make SOC=j721s2 PROFILE=release
\endcode

**Cores built:**
- A72 MPU
- R5F (MCU2_0, MCU2_1, MCU3_0, MCU3_1)
- C7x DSP1, DSP2

---

## J784S4 Platform {#BUILD_EXAMPLES_J784S4}

J784S4 has quad C7x DSPs for maximum compute performance.

\code
# Build for J784S4 with quad C7x (all cores built by default)
make SOC=j784s4 PROFILE=release
\endcode

**Cores built:**
- A72 MPU
- R5F (MCU2_0, MCU2_1, MCU3_0, MCU3_1, MCU4_0, MCU4_1)
- C7x DSP1, DSP2, DSP3, DSP4

---

## J722S Platform {#BUILD_EXAMPLES_J722S}

J722S is a cost-optimized platform with single C7x and fewer R5F cores.

\code
# Build for J722S (all cores built by default)
make SOC=j722s PROFILE=release
\endcode

**Cores built:**
- A53 MPU
- R5F (MCU1_0)
- C7x DSP1

---

## AM62A Platform {#BUILD_EXAMPLES_AM62A}

AM62A is an entry-level platform focused on edge AI applications.

\code
# Build for AM62A (all cores built by default)
make SOC=am62a PROFILE=release
\endcode

---

# Build Flag Combinations Reference {#BUILD_EXAMPLES_COMBINATIONS}

## Core Selection Matrix {#BUILD_EXAMPLES_CORE_MATRIX}

Core Build Configuration | Command | Use Case |
-------------------------|---------|----------|
All cores (PC emulation) | `make BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no` | PC development |
All cores (target) | `make BUILD_EMULATION_MODE=no BUILD_TARGET_MODE=yes SOC=j721e` | Complete system build |
MPU only | `make BUILD_MPU=1 BUILD_TARGET_MODE=yes SOC=j721e` | Host application development |
R5F only | `make BUILD_R5F=1 BUILD_TARGET_MODE=yes SOC=j721e` | Real-time control development |
C7x only | `make BUILD_C7X=1 BUILD_TARGET_MODE=yes SOC=j721e` | DSP kernel optimization |
C66 only (J721E) | `make BUILD_C66=1 BUILD_TARGET_MODE=yes SOC=j721e` | C66 DSP development |

---

## Profile and Mode Combinations {#BUILD_EXAMPLES_PROFILE_COMBINATIONS}

Build Configuration | Command | Use Case |
--------------------|---------|----------|
Debug PC | `make PROFILE=debug BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no` | Development |
Release PC | `make PROFILE=release BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no` | Performance testing |
Both PC | `make PROFILE=all BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=no` | Complete validation |
Debug Target | `make PROFILE=debug BUILD_EMULATION_MODE=no BUILD_TARGET_MODE=yes SOC=j721e` | Target debugging |
Release Target | `make PROFILE=release BUILD_EMULATION_MODE=no BUILD_TARGET_MODE=yes SOC=j721e` | Deployment |
Both Target | `make PROFILE=all BUILD_EMULATION_MODE=no BUILD_TARGET_MODE=yes SOC=j721e` | Full target validation |
PC + Target | `make PROFILE=all BUILD_EMULATION_MODE=yes BUILD_TARGET_MODE=yes SOC=j721e` | Complete system build |

---

# Build Optimization Tips {#BUILD_EXAMPLES_OPTIMIZATION}

## Faster Incremental Builds {#BUILD_EXAMPLES_FAST_BUILDS}

### Parallel Builds

Enable parallel compilation using make's -j option:

\code
# Use all CPU cores
make -j$(nproc)

# Or specify core count
make -j8
\endcode

**Performance improvement:** ~3-4x faster on multi-core systems

### Skip Unnecessary Components

\code
# Skip tutorials and tests during development
make BUILD_CONFORMANCE_TEST=no BUILD_TUTORIAL=no -j8

# Skip documentation generation
# (documentation only built with 'make doxy_docs')
\endcode

---

## Disk Space Optimization {#BUILD_EXAMPLES_DISK_SPACE}

### Selective Profile Builds

\code
# Build only release (saves ~40% disk space)
make PROFILE=release
\endcode

### Clean Intermediate Files

\code
# Remove object files, keep libraries
make clean

# Remove everything (out/ and lib/)
make scrub
\endcode

### Core-Specific Cleaning

\code
# Clean only C7x builds
make clean_c7x

# Clean only PC builds
make clean_mpu
\endcode

---

# Additional Resources {#BUILD_EXAMPLES_RESOURCES}

For more information on build configuration:

- Build options reference: \ref BUILD_OPTIONS
- Build and run instructions: \ref BUILD_INSTRUCTIONS
- PyTIOVX custom kernel generation: \ref PYTIOVX
- Tutorial examples: \ref TUTORIALS
- Advanced: User target kernels (manual approach): \ref TIOVX_TARGET_KERNEL

For platform-specific information:
- Vision Apps User Guide (target deployment)
- PSDK RTOS Documentation
- TI E2E Forum: https://e2e.ti.com/support/processors-group/processors/f/processors-forum
