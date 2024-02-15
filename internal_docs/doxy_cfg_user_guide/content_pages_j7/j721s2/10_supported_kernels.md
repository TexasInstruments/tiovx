# TIOVX Supported Kernels {#SUPPORTED_KERNELS}

[TOC]

# Legend

Meaning of terms in the following tables,

- **DMA**:     Kernel implemented and memory access done using DMA (NOTE: Not enabled in this release)
- **Cache**:   Kernel implemented and memory access done using CPU Cache
- **[empty]**: Kernel is not supported on target

---

# OpenVX Standard Kernels

This table lists the mapping of standard OpenVX kernels to compute targets
on the Jacinto7 platform.  When mapped to C7X DSP, it indicates if it is implemented
using BAM DMA acceleration, or cache only.

All of the below kernels default to running on the standalone C7X DSP.  If a different target
is needed, it can be selected from the available targets indicated below by using the vxSetNodeTarget() API.

Note: all of the standard kernels were initially developed on J721E running on the C66x DSP.  When porting to
the C7x, the VXLIB kernels and TIOVX kernel wrappers were recompiled for the C7x.  If custom kernels need to
be recompiled from C66x to C7x, these kernels in VXLIB and associated kernel wrappers in TIOVX can be referenced
for information on how to recompile these kernels.

Kernel | C7X | HWA | PC Emulation Support |
-------|-----|-----|----------------------|
[Absolute Difference](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/ddf/group__group__vision__function__absdiff.html)              | Cache | | Yes |
[Accumulate](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d77/group__group__vision__function__accumulate.html)                    | Cache | | Yes |
[Accumulate Squared](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d2c/group__group__vision__function__accumulate__square.html)    | Cache | | Yes |
[Accumulate Weighted](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d2/d2e/group__group__vision__function__accumulate__weighted.html) | Cache | | Yes |
[Arithmetic Addition](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/db0/group__group__vision__function__add.html)                  | Cache | | Yes |
[Arithmetic Subtraction](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/d6d/group__group__vision__function__sub.html)               | Cache | | Yes |
[Bitwise AND](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d4/d4e/group__group__vision__function__and.html)                          | Cache | | Yes |
[Bitwise EXCLUSIVE OR](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/dd1/group__group__vision__function__xor.html)                 | Cache | | Yes |
[Bitwise INCLUSIVE OR](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d2/d5d/group__group__vision__function__or.html)                  | Cache | | Yes |
[Bitwise NOT](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d06/group__group__vision__function__not.html)                          | Cache | | Yes |
[Box Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d7c/group__group__vision__function__box__image.html)                    | Cache | | Yes |
[Canny Edge Detector](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/d71/group__group__vision__function__canny.html)                | Cache | | Yes |
[Channel Combine](https://www.khronos.org/registry/OpenVX/specs/1.1/html/de/df2/group__group__vision__function__channelcombine.html)           | Cache | | Yes |
[Channel Extract](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/dc1/group__group__vision__function__channelextract.html)           | Cache | | Yes |
[Color Convert](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/dc2/group__group__vision__function__colorconvert.html)               | Cache | | Yes |
[Convert Bit depth](https://www.khronos.org/registry/OpenVX/specs/1.1/html/de/d73/group__group__vision__function__convertdepth.html)           | Cache | | Yes |
[Custom Convolution](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d3b/group__group__vision__function__custom__convolution.html)   | Cache | | Yes |
[Dilate Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d73/group__group__vision__function__dilate__image.html)               | Cache | | Yes |
[Equalize Histogram](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/d70/group__group__vision__function__equalize__hist.html)        | Cache | | Yes |
[Erode Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/dff/group__group__vision__function__erode__image.html)                 | Cache | | Yes |
[Fast Corners](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/d22/group__group__vision__function__fast.html)                        | Cache | | Yes |
[Gaussian Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/d58/group__group__vision__function__gaussian__image.html)          | Cache | | Yes |
[Non Linear Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/dc5/group__group__vision__function__nonlinear__filter.html)      | Cache | | Yes |
[Harris Corners](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/d5f/group__group__vision__function__harris.html)                    | Cache | | Yes |
[Histogram](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/dcb/group__group__vision__function__histogram.html)                      | Cache | | Yes |
[Gaussian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d15/group__group__vision__function__gaussian__pyramid.html) | Cache | VPAC_MSC* | Yes |
[Laplacian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d60/group__group__vision__function__laplacian__pyramid.html) | Cache   |  | Yes |
[Reconstruction from a Laplacian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/daa/group__group__vision__function__laplacian__reconstruct.html) | Cache |  | Yes |
[Integral Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d7b/group__group__vision__function__integral__image.html)           | Cache |  | Yes |
[Magnitude](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/df2/group__group__vision__function__magnitude.html)                      | Cache |  | Yes |
[Mean and Standard Deviation](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d8/d85/group__group__vision__function__meanstddev.html)   | Cache |  | Yes |
[Median Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d77/group__group__vision__function__median__image.html)              | Cache |  | Yes |
[Min, Max Location](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d8/d05/group__group__vision__function__minmaxloc.html)              | Cache |  | Yes |
[Optical Flow Pyramid (LK)](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d0c/group__group__vision__function__opticalflowpyrlk.html) | Cache   |  | Yes |
[Phase](https://www.khronos.org/registry/OpenVX/specs/1.1/html/db/d4e/group__group__vision__function__phase.html)                              | Cache   |  | Yes |
[Pixel-wise Multiplication](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/dae/group__group__vision__function__mult.html)           | Cache   |  | Yes |
[Remap](https://www.khronos.org/registry/OpenVX/specs/1.1/html/df/dca/group__group__vision__function__remap.html)                              | Cache   |  | Yes |
[Scale Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/d26/group__group__vision__function__scale__image.html)                 | Cache   | VPAC_MSC* | Yes |
[Sobel 3x3](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d4b/group__group__vision__function__sobel3x3.html)                       | Cache   |  | Yes |
[TableLookup](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/d4e/group__group__vision__function__lut.html)                          | Cache   |  | Yes |
[Thresholding](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d1e/group__group__vision__function__threshold.html)                   | Cache   |  | Yes |
[Warp Affine](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/d5f/group__group__vision__function__warp__affine.html)                 | Cache   |  | Yes |
[Warp Perspective](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d6a/group__group__vision__function__warp__perspective.html)       | Cache   |  | Yes |

* Subset of configuration options and or accuracy tradeoff to speed is to be considered for this HWA implementation.

---

# TI Extension Kernels

Note: the below node implementation locations have changed from the 8.6 to 9.0 releases.  The new locations can be
referenced in \ref TIOVX_PACKAGE_CONTENTS document.

Kernel                            | Target    | PC Emulation Support |
----------------------------------|-----------|----------------------|
#tivxCaptureNode                  | CSIRX     | No                   |
#tivxDisplayNode                  | DSS       | No                   |
#tivxTIDLNode                     | C7x + MMA | Yes                  |
#tivxVpacVissNode                 | VPAC_VISS | Yes                  |
#tivxVpacLdcNode                  | VPAC_LDC  | Yes                  |
#tivxVpacNfGenericNode            | VPAC_NF   | Yes                  |
#tivxVpacNfBilateralNode          | VPAC_NF   | Yes                  |
#tivxVpacMscScaleNode             | VPAC_MSC  | Yes                  |
#tivxVpacMscPyramidNode           | VPAC_MSC  | Yes                  |
#tivxDmpacSdeNode                 | DMPAC_SDE | Yes                  |
#tivxDmpacDofNode                 | DMPAC_DOF | Yes                  |
#tivxCsitxNode                    | CSITX     | No                   |
#tivxDisplayM2MNode               | DSS       | No                   |

