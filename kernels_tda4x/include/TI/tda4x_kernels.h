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
    /*! \brief The dmpac_sde kernel
     * \see group_vision_function_hwa
     */
    TIVX_KERNEL_DMPAC_SDE = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_HWA_BASE) + 2,
    /*! \brief The vpac_ldc kernel
     * \see group_vision_function_hwa
     */
    TIVX_KERNEL_VPAC_LDC = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_HWA_BASE) + 3,
    /*! \brief The dmpac_dof kernel
     * \see group_vision_function_hwa
     */
    TIVX_KERNEL_DMPAC_DOF = VX_KERNEL_BASE(VX_ID_DEFAULT, TIVX_LIBRARY_HWA_BASE) + 4,
    TIVX_KERNEL_HWA_MAX_1_0, /*!< \internal Used for bounds checking in the conformance test. */
};

/*********************************
 *      VPAC_NF STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VPAC_NF_GENERIC and TIVX_KERNEL_VPAC_NF_BILATERAL
 *         kernels.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  input_interleaved;       /*!< 0: NonInterleaved input mode; 1: Interleaved input mode (i.e. chroma plane of NV12) */
    int16_t   output_downshift;        /*!< Indicates the down-shift value to apply to the output before offset [Range (-8) - 7] */
    uint16_t  output_offset;           /*!< Indicates the offset value to add after shift [Range (0 - 4095)] */
    uint16_t  output_pixel_skip;       /*!< Horizontal output pixel skipping  0: disabled, 1: enabled */
    uint16_t  output_pixel_skip_odd;   /*!< If outputPixelSkip == 1, then skip 0: even pixel, 1: odd pixel */
    uint16_t  kern_ln_offset;          /*!< kernel line offset [Range (0 - 4)] (LSE) */
    uint16_t  kern_sz_height;          /*!< kernel height [Range (1 - 5)] (LSE) */
    uint16_t  src_ln_inc_2;            /*!< 0: Off, 1: vertical skip input lines (LSE) */
} tivx_vpac_nf_common_params_t;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VPAC_NF_BILATERAL kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    tivx_vpac_nf_common_params_t params;    /*!< Common parameters for configuring vpac nf */
    /*! Adaptive mode 0: disabled, 1: enabled
     *    Adaptive mode is when the sub-table sigma is automatically chosen on a per-pixel basis based on the
     *    average intensity of local 4x4 neighborhood of the center pixel.
     */
    uint16_t  adaptive_mode;
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

/*********************************
 *      DMPAC_SDE STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_DMPAC_SDE kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  median_filter_enable;         /*!< 0: Disabled; 1: Enable post-processing 5x5 median filter */
    uint16_t  reduced_range_search_enable;  /*!< 0: Disabled; 1: Enable reduced range search on pixels near right margin */
    uint16_t  disparity_min;                /*!< 0: minimum disparity == 0; 1: minimum disparity == -3 */
    uint16_t  disparity_max;                /*!< 0: disparity_min + 63; 1: disparity_min + 127; 2: disparity_min + 191 */
    uint16_t  threshold_left_right;         /*!< Left-right consistency check threshold in pixels [Range (0 - 255)] */
    uint16_t  texture_filter_enable;        /*!< 0: Disabled; 1: Enable texture based filtering */
    /*! If texture_filter_enable == 1, Scaled texture threshold [Range (0 - 255)]
     *  Any pixel whose texture metric is lower than threshold_texture is considered to be low texture.  It is specified as
     *  normalized texture threshold times 1024.  For instance, if threshold_texture == 204, the normalized texture threshold
     *  is 204/1024 = 0.1992.
     */
    uint16_t  threshold_texture;
    uint16_t  aggregation_penalty_p1;       /*!< SDE aggragation penalty P1. Optimization penalty constant for small disparity change. P1<=127 */
    uint16_t  aggregation_penalty_p2;       /*!< SDE aggragation penalty P2. Optimization penalty constant for large disparity change. P2<=255 */
    /*! Defines custom ranges for mapping internal confidence score to one of 8 levels. [Range (0 - 4095)]
     *    The confidence score will map to level N if it is less than confidence_score_map[N] but greater than or equal to confidence_score_map[N-1]
     *    For example, to map internal confidence scores from 0 to 50 to level 0, and confidence scores from 51 to 108 to level 1,
     *    then set confidence_score_map[0] = 51 and confidence_score_map[1] = 109
     *    NOTE: Each mapping value must be greater than the values from lower indices of the array */
    uint16_t  confidence_score_map[8];
} tivx_dmpac_sde_params_t;

/*********************************
 *      VPAC_LDC STRUCTURES
 *********************************/

/*!
 * \brief The convert depth structure in the configuration data structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  in_bits;          /*!< Input bit depth [Range (8 - 12)] */
    uint16_t  out_bits;         /*!< Output bit depth [Range (8 - 12)] */
} tivx_vpac_ldc_convert_depth_params_t;

/*!
 * \brief The mesh data structure in the configuration data structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  frame_width;       /*!< Mesh table full-frame width before subsampling */
    uint16_t  frame_height;      /*!< Mesh table full-frame height before subsampling */
    uint16_t  subsample_factor;  /*!< Mesh table subsample factor in horz and vert = 2^subsample_factor */
} tivx_vpac_ldc_mesh_params_t;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  luma_interpolation_type;          /*!< Luma Interpolation Type, 0: Bicubic; 1: Bilinear */
    uint16_t  init_x;                           /*!< Output starting x-coordinate (must be multiple of 8) [Range (0 - 8191)] */
    uint16_t  init_y;                           /*!< Output starting y-coordinate (must be multiple of 2) [Range (0 - 8191)] */
    uint16_t  input_align_12bit;                            /*!< Optional: When input 16-bit unpacked, alignment of 12-bit pixel, 0: LSB, 1:MSB */
    tivx_vpac_ldc_convert_depth_params_t  out_2_luma;   /*!< Optional: When 'out_2_luma_lut' is used, configures input/output bit depths */
    tivx_vpac_ldc_convert_depth_params_t  out_3_chroma; /*!< Optional: When 'out_3_chroma_lut' is used, configures input/output bit depths */
    tivx_vpac_ldc_mesh_params_t             mesh;        /*!< Optional: When 'mesh_table' is used, configures mesh parameters */
} tivx_vpac_ldc_params_t;


/*!
 * \brief The region_params data structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  out_block_width;              /*!< Output block width (must be multiple of 8) [Range (8 - 255)] */
    uint16_t  out_block_height;             /*!< Output block height (must be multiple of 2) [Range (2 - 255)] */
    uint16_t  pixel_pad;                    /*!< Pixel Padding [Range (0 - 15)] */
} tivx_vpac_ldc_region_params_t;

/*!
 * \brief The alternative region_params data structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t column_width[3];               /*!< Width of each column of sub-frames [Range (1 - 8191)] */
    uint16_t row_height[3];                 /*!< Height of each row of sub-frames [Range (1 - 8191)] */
    uint16_t enable[9];                     /*!< Subframe enable, 0: Disabled, 1: Enabled */
    uint16_t out_block_width[9];            /*!< Output block width (must be multiple of 8) [Range (0 - 255)] */
    uint16_t out_block_height[9];           /*!< Output block height (must be multiple of 2) [Range (0 - 255)] */
    uint16_t pixel_pad[9];                  /*!< Pixel Padding [Range (0 - 15)] */
} tivx_vpac_ldc_subregion_params_t;

/*!
 * \brief The bandwidth params structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    /*! Limits the mean bandwidth (computed over one block) that the LDC module can request for read from system memory. [Range (0 - 4095)]
     *    0: (Default) the bandwidth limiter is bypassed
     *    1~4095: Maximum number of bytes per 256 cycles.
     *
     *    Examples:
     *      1 : 1.17 MBytes/s @ 300 MHz
     *      4095 : ~4.8 GBytes/s @300 MHz
     */
    uint16_t  bandwidth_control;
    /*! Limits the maximum number of outstanding LDC requests to TAG_CNT+1. [Range (0 - 31)]
     *    (Default): 31
     */
    uint16_t  tag_count;
    /*! Limits the maximum burst length that could be used by LDC. [Range (0 - 3)]
     *    Each burst is of 16 bytes.  Hardware breaks the command at max_burst_length boundary.
     *    0: 16 (16*16 bytes = 256 bytes)
     *    1:  8 (8*16 bytes  = 128 bytes) (Default)
     *    2:  4 (4*16 bytes  =  64 bytes)
     *    3:  2 (2*16 bytes  =  32 bytes)
     */
    uint16_t  max_burst_length;
} tivx_vpac_ldc_bandwidth_params_t;

/*********************************
 *      VPAC_MSC STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    /*! Single-phase horizontal filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient set 0 (specified using 'horz_coef_sel')
     */
    uint16_t  horz_coef_src;
    /*! Single-phase horizontal filter coefficient selection
     *      if (horz_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient set 0 [Range (0-31)]
     */
    uint16_t  horz_coef_sel;
    /*! Single-phase vertical filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient set 0 (specified using 'vert_coef_sel')
     */
    uint16_t  vert_coef_src;
    /*! Single-phase vertical filter coefficient selection
     *      if (vert_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient set 0 [Range (0-31)]
     */
    uint16_t  vert_coef_sel;
} tivx_vpac_msc_single_phase_params_t;

/*!
 * \brief The multi_phase data structure in the configuration data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  phase_mode;           /*!< Multi-phase Mode, 0: 64 phases, 1: 32 phases */
    /*! Multi-phase horizontal coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint16_t  horz_coef_sel;
    /*! Multi-phase vertical coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint16_t  vert_coef_sel;
    uint16_t  init_phase_x;         /*!< Multi-phase initial horizontal resize phase (U12Q12) [Range (0-4095)] */
    uint16_t  init_phase_y;         /*!< Multi-phase initial vertical resize phase (U12Q12) [Range (0-4095)] */
} tivx_vpac_msc_multi_phase_params_t;

/*!
 * \brief The output config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  input_map;          /*!< Specified which input port this output is mapped to [Range (0-1)] */
    uint16_t  signed_data;        /*!< Integer type of input and output frame data, 0: Unsigned 12-bit, 1: Signed 12-bit */
    uint16_t  input_interleaved;  /*!< 0: NonInterleaved input mode; 1: Interleaved input mode (i.e. chroma plane of NV12) */
    uint16_t  filter_mode;        /*!< 0: Single Phase Filter (integer ratios: 1x, 1/2x, 1/4x); 1: Multi-phase scaling filter */
    uint16_t  coef_shift;         /*!< Sets the fractional bit precision of the 10-bit filter coefficient [Range (5-9)] */
    uint16_t  saturation_mode;    /*!< Filter output saturation mdoe, 0: [0..4095] clipping; 1: [-2048..2047] clip followed by + 2048 */
    uint16_t  offset_x;           /*!< Source region of interest X offset [Range (0-8191)] */
    uint16_t  offset_y;           /*!< Source region of interest Y offset [Range (0-8191)] */
    /*! Horizontal scaling ratio value (U15Q12) [Range (4096-16384)]
     *      Output Width = 4096 / scale_x
     *      When 'filter_mode' == 0: Single-phase, only valid values are 4096 (1x), 8192 (1/2x), or 16384 (1/4x)
     */
    uint16_t  scale_x;
    /*! Vertical scaling ratio value (U15Q12) [Range (4096-16384)]
     *      Output Width = 4096 / scale_x
     *      When 'filter_mode' == 0: Single-phase, only valid values are 4096 (1x), 8192 (1/2x), or 16384 (1/4x)
     */
    uint16_t  scale_y;
    uint16_t  pixel_width;              /*!< Output pixel width, 0: 8-bit, 1: 12-bit, 3: 16-bit */
    uint16_t  output_align_12bit;       /*!< Optional: When input 16-bit unpacked, alignment of 12-bit pixel, 0: LSB, 1:MSB */
    tivx_vpac_msc_single_phase_params_t single_phase;   /*!< Optional: When 'filter_mode' == 0: Single-phase */
    tivx_vpac_msc_multi_phase_params_t  multi_phase;    /*!< Optional: When 'filter_mode' == 1: Multi-phase */
} tivx_vpac_msc_output_params_t;

/*!
 * \brief The input config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  pixel_width;             /*!< Input pixel width, 0: 8-bit, 1: 12-bit, 3: 16-bit */
    uint16_t  kern_tpad_sz;            /*!< kernel top padding lines [Range (0 - 4)] (LSE) */
    uint16_t  kern_bpad_sz;            /*!< kernel bottom padding lines [Range (0 - 4)] (LSE) */
    uint16_t  kern_ln_offset;          /*!< kernel line offset [Range (0 - 4)] (LSE) */
    uint16_t  kern_sz_height;          /*!< kernel height [Range (1 - 5)] (LSE) */
    uint16_t  src_ln_inc_2;            /*!< 0: Off, 1: vertical skip input lines (LSE) */
    uint16_t  input_align_12bit;       /*!< Optional: When input 16-bit unpacked, alignment of 12-bit pixel, 0: LSB, 1:MSB */
} tivx_vpac_msc_input_params_t;

/*!
 * \brief The coefficients input data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    int16_t  single_phase_0[5];             /*!< Single phase coefficient set 0, signed 10-bit */
    int16_t  single_phase_1[5];             /*!< Single phase coefficient set 1, signed 10-bit */
    int16_t  multi_phase_0[5*32];           /*!< Multi phase coefficient set 0, signed 10-bit */
    int16_t  multi_phase_1[5*32];           /*!< Multi phase coefficient set 1, signed 10-bit */
    int16_t  multi_phase_2[5*32];           /*!< Multi phase coefficient set 2, signed 10-bit */
    int16_t  multi_phase_3[5*32];           /*!< Multi phase coefficient set 3, signed 10-bit */
} tivx_vpac_msc_coefficients_t;


/*********************************
 *      DMPAC_DOF STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_DMPAC_DOF kernel.
 *
 * \ingroup group_kernel
 */
typedef struct {
    uint16_t  vertical_search_range[2];     /*!< Range [0, 63] 	Recommended = 48, 48 (vertical_search_range[0] (upward) + vertical_search_range[1] (downward) <= 96) */
    uint16_t  horizontal_search_range;      /*!< Range [0, 191] Recommended value = 191 */
    uint16_t  median_filter_enable;         /*!< 0: Disabled; 1: Enable post-processing median filter [recommended] */
    uint16_t  motion_smoothness_factor;     /*!< Range [0, 31] 	recommended = 24 */
    uint16_t  motion_direction;             /*!< 0 = Motion neutral 1 = Forward motion 2 = Reverse motion  */
} tivx_dmpac_dof_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

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


