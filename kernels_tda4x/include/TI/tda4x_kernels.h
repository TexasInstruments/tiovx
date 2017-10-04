/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef TDA4X_KERNELS_H_
#define TDA4X_KERNELS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The list of supported kernels in this kernel extension.
 */

/*! \brief Name for OpenVX Extension kernel module: hwa
 * \ingroup group_tivx_ext
 */
#define TIVX_MODULE_NAME_HWA    "hwa"

/*! \brief The list of available libraries in this extension */
enum tda4x_library_e {
   /*! \brief The set of kernels supported in hwa module  */
   TIVX_LIBRARY_HWA_BASE = 0,
};

/*!
 * \brief The list of kernels supported in hwa module
 *
 * Each kernel listed here can be used with the <tt>\ref vxGetKernelByEnum</tt> call.
 * When programming the parameters, use
 * \arg <tt>\ref VX_INPUT</tt> for [in]
 * \arg <tt>\ref VX_OUTPUT</tt> for [out]
 * \arg <tt>\ref VX_BIDIRECTIONAL</tt> for [in,out]
 *
 * When programming the parameters, use
 * \arg <tt>\ref VX_TYPE_IMAGE</tt> for a <tt>\ref vx_image</tt> in the size field of <tt>\ref vxGetParameterByIndex</tt> or <tt>\ref vxSetParameterByIndex</tt>
 * \arg <tt>\ref VX_TYPE_ARRAY</tt> for a <tt>\ref vx_array</tt> in the size field of <tt>\ref vxGetParameterByIndex</tt> or <tt>\ref vxSetParameterByIndex</tt>
 * \arg or other appropriate types in \ref vx_type_e.
 * \ingroup group_kernel
 */
enum tivx_kernel_hwa_e {
    /*! \brief The vpac_nf_generic kernel
     * \see group_vision_function_hwa
     */
    TIVX_KERNEL_VPAC_NF_GENERIC = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_HWA_BASE) + 0,
    /*! \brief The vpac_nf_bilateral kernel
     * \see group_vision_function_hwa
     */
    TIVX_KERNEL_VPAC_NF_BILATERAL = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_HWA_BASE) + 1,
    TIVX_KERNEL_HWA_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */
};

/*!
 * \brief The parameters data structure used by the TIVX_KERNEL_VPAC_NF_GENERIC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  input_interleaved;       /*!< 0: NonInterleaved mode; 1: Interleaved mode */
    int16_t   output_downshift;        /*!< Indicates the down shift value to apply to the output before offset [Range (-8) - 7] */
    uint16_t  output_offset;           /*!< Indicates the offset value to add after shift [Range (0 - 4095)] */
    uint16_t  output_pixel_skip;       /*!< Horizontal output pixel skipping  0: disabled, 1: enabled */
    uint16_t  output_pixel_skip_odd;   /*!< If outputPixelSkip == 1, then skip 0: even pixel, 1: odd pixel */
    uint16_t  kern_ln_offset;          /*!< kernel line offset [Range (0 - 4)] (LSE) */
    uint16_t  kern_sz_height;          /*!< kernel height [Range (1 - 5)] (LSE) */
    uint16_t  src_ln_inc_2;            /*!< 0: Off, 1: vertical skip input lines (LSE) */
} tivx_vpac_nf_common_params_t;

/*!
 * \brief The parameters data structure used by the TIVX_KERNEL_VPAC_NF_BILATERAL kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    tivx_vpac_nf_common_params_t params;    /*!< Common parameters for configuring vpac nf */
    uint16_t  adaptive_mode;                   /*!< Adaptive mode (automatically choose sub-table based on average intensity)   0: disabled, 1: enabled */
    uint16_t  sub_table_select;                /*!< If adaptive_mode == 0, selects which sub-table to use [0 - 7] */
} tivx_vpac_nf_bilateral_params_t;

/*!
 * \brief The sigmas data structure used by the TIVX_KERNEL_VPAC_NF_BILATERAL kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    /*! The number of sigmas given in each of the sigma_space and sigma_range arrays. Valid supported values are 1, 2, 4, or 8.
     *  Since this corresponds to applying a lookup table in RAM, there are a few different use cases this is intended to enable:
     *  1. The common case is to use a value of 1, applying the same pair of sigmas to the full image.
     *  2. In the case where the VPAC NF will be used to alternate between luma and chroma planes of an image, there are two options:
     *     a. Load one sigma to apply to luma plane, and then overwrite it with a second table to apply to chroma plane
     *        - This has a latency penalty in between each plane to load 5*256 bytes
     *     b. Load a quantized version of both luma and chroma planes at once, and hardware can alternate between each plane using the
     *        sub_table_select identifier.
     *        - This saves latency, but looses one bit of precision for the two sub-tables.
     *  3. Adaptive mode case.  In adaptive mode, the user can load a different sigma value corresponding to the average neigborhood
     *     intensity of the center pixel.  For example, if 4 sigmas are used, then the full dynamic range of the input is divided into
     *     4 equal-sized ranges.  Sigma index '0' is applied to the lowest value range of the average neighborhood intensity, and so on.
     *     - The tradeoff is that the more tables that are used, the less the precision of lookup tables.
     */
    uint16_t  num_sigmas;
    double  sigma_space[8];  /*!< Array of space sigmas used to generate a 5x5 gaussian filter around the center pixel */
    double  sigma_range[8];  /*!< Array of range sigmas used to weight the neigborhood pixels according to their absolute difference in value from the center pixel */
} tivx_vpac_nf_bilateral_sigmas_t;


/*!
 * \brief Used for the Application to load the hwa kernels into the context.
 * \ingroup group_kernel
 */
void hwaLoadKernels(vx_context context);

/*!
 * \brief Used for the Application to unload the hwa kernels from the context.
 * \ingroup group_kernel
 */
void hwaUnLoadKernels(vx_context context);

/*!
 * \brief Used to print the performance of the kernels.
 * \ingroup group_kernel
 */
void hwaPrintPerformance(vx_perf_t performance, uint32_t numPixels, const char* testName);

#ifdef __cplusplus
}
#endif

#endif /* TDA4X_KERNELS_H_ */


