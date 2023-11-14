# TIOVX Supported Kernels {#SUPPORTED_KERNELS}

[TOC]

# Legend

Meaning of terms in the following tables, 

- **DMA**:     Kernel implemented and memory access done using DMA
- **Cache**:   Kernel implemented and memory access done using CPU Cache
- **[empty]**: Kernel is not supported on target

---

# OpenVX Standard Kernels

This table lists the mapping of standard OpenVX kernels to compute targets
on the TDA2/3 platform.  When mapped to C66x DSP, it indicates if it is implemented
using BAM DMA acceleration, or cache only.

Kernel | C66x | EVE |
-------|------|-----|
[Absolute Difference](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/ddf/group__group__vision__function__absdiff.html)              | DMA | |
[Accumulate](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d77/group__group__vision__function__accumulate.html)                    | DMA | |
[Accumulate Squared](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d2c/group__group__vision__function__accumulate__square.html)    | DMA | |
[Accumulate Weighted](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d2/d2e/group__group__vision__function__accumulate__weighted.html) | DMA | |
[Arithmetic Addition](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/db0/group__group__vision__function__add.html)                  | DMA | |
[Arithmetic Subtraction](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/d6d/group__group__vision__function__sub.html)               | DMA | |
[Bitwise AND](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d4/d4e/group__group__vision__function__and.html)                          | DMA | |
[Bitwise EXCLUSIVE OR](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/dd1/group__group__vision__function__xor.html)                 | DMA | |
[Bitwise INCLUSIVE OR](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d2/d5d/group__group__vision__function__or.html)                  | DMA | |
[Bitwise NOT](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d06/group__group__vision__function__not.html)                          | DMA | |
[Box Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d7c/group__group__vision__function__box__image.html)                    | DMA | |
[Canny Edge Detector](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/d71/group__group__vision__function__canny.html)                | DMA | |
[Channel Combine](https://www.khronos.org/registry/OpenVX/specs/1.1/html/de/df2/group__group__vision__function__channelcombine.html)           | DMA | |
[Channel Extract](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/dc1/group__group__vision__function__channelextract.html)           | DMA | |
[Color Convert](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/dc2/group__group__vision__function__colorconvert.html)               | DMA | |
[Convert Bit depth](https://www.khronos.org/registry/OpenVX/specs/1.1/html/de/d73/group__group__vision__function__convertdepth.html)           | DMA | |
[Custom Convolution](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d3b/group__group__vision__function__custom__convolution.html)   | DMA | |
[Dilate Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d73/group__group__vision__function__dilate__image.html)               | DMA | |
[Equalize Histogram](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/d70/group__group__vision__function__equalize__hist.html)        | DMA | |
[Erode Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/dff/group__group__vision__function__erode__image.html)                 | DMA | |
[Fast Corners](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/d22/group__group__vision__function__fast.html)                        | Cache   |  |
[Gaussian Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/d58/group__group__vision__function__gaussian__image.html)          | DMA   |  |
[Non Linear Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/dc5/group__group__vision__function__nonlinear__filter.html)      | DMA   |  |
[Harris Corners](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/d5f/group__group__vision__function__harris.html)                    | DMA   |  |
[Histogram](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d6/dcb/group__group__vision__function__histogram.html)                      | DMA   |  |
[Gaussian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d15/group__group__vision__function__gaussian__pyramid.html) | Cache |  |
[Laplacian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dc/d60/group__group__vision__function__laplacian__pyramid.html) | Cache   |  |
[Reconstruction from a Laplacian Image Pyramid](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/daa/group__group__vision__function__laplacian__reconstruct.html) | Cache   |  |
[Integral Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d7b/group__group__vision__function__integral__image.html)           | DMA   |  |
[Magnitude](https://www.khronos.org/registry/OpenVX/specs/1.1/html/dd/df2/group__group__vision__function__magnitude.html)                      | DMA   |  |
[Mean and Standard Deviation](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d8/d85/group__group__vision__function__meanstddev.html)   | DMA   |  |
[Median Filter](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d77/group__group__vision__function__median__image.html)              | DMA   |  |
[Min, Max Location](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d8/d05/group__group__vision__function__minmaxloc.html)              | DMA   |  |
[Optical Flow Pyramid (LK)](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d0/d0c/group__group__vision__function__opticalflowpyrlk.html) | Cache   |  |
[Phase](https://www.khronos.org/registry/OpenVX/specs/1.1/html/db/d4e/group__group__vision__function__phase.html)                              | DMA   |  |
[Pixel-wise Multiplication](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d7/dae/group__group__vision__function__mult.html)           | DMA   |  |
[Remap](https://www.khronos.org/registry/OpenVX/specs/1.1/html/df/dca/group__group__vision__function__remap.html)                              | Cache   |  |
[Scale Image](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d1/d26/group__group__vision__function__scale__image.html)                 | Cache   |  |
[Sobel 3x3](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d4b/group__group__vision__function__sobel3x3.html)                       | DMA   |  |
[TableLookup](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/d4e/group__group__vision__function__lut.html)                          | DMA   |  |
[Thresholding](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d3/d1e/group__group__vision__function__threshold.html)                   | DMA   |  |
[Warp Affine](https://www.khronos.org/registry/OpenVX/specs/1.1/html/d5/d5f/group__group__vision__function__warp__affine.html)                 | Cache   |  |
[Warp Perspective](https://www.khronos.org/registry/OpenVX/specs/1.1/html/da/d6a/group__group__vision__function__warp__perspective.html)       | Cache   |  |

---

# Additional TI Kernels

Kernel                            | DSP | EVE
----------------------------------|-----|-------
Harris Corners (IVision)          |     | DMA
TIDL                              | DMA | DMA
