/*
 *
 * Copyright (c) 2019-2021 Texas Instruments Incorporated
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

#ifndef J7_VPAC_NF_H_
#define J7_VPAC_NF_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The NF kernels in this kernel extension.
 */

/*! \brief vpac_nf_generic kernel name
 *  \ingroup group_vision_function_vpac_nf
 */
#define TIVX_KERNEL_VPAC_NF_GENERIC_NAME "com.ti.hwa.vpac_nf_generic"

/*! \brief vpac_nf_bilateral kernel name
 *  \ingroup group_vision_function_vpac_nf
 */
#define TIVX_KERNEL_VPAC_NF_BILATERAL_NAME     "com.ti.hwa.vpac_nf_bilateral"


/*********************************
 *      VPAC_NF Control Commands
 *********************************/

/*! \brief Control Command to set TODO
 *
 *
 */
#define TIVX_VPAC_NF_CMD_SET_HTS_LIMIT                     (0x10000000u)

/*! \brief Control Command to set TODO
 *
 *
 */
#define TIVX_VPAC_NF_CMD_SET_COEFF                         (0x10000001u)

/*! \brief Control Command to get the error status
 *         Returns the error status of the last processed frame.
 *         Reference to vx_scalar is passed as argument with
 *         this control command.
 *         Node returns bit-mask of error status in u32 variable of vx_scalar.
 *
 *
 */
#define TIVX_VPAC_NF_CMD_GET_ERR_STATUS                    (0x10000002u)

/*********************************
 *      VPAC_NF Defines
 *********************************/

/* TODO */

/*********************************
 *      VPAC_NF STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VPAC_NF_GENERIC and TIVX_KERNEL_VPAC_NF_BILATERAL
 *         kernels.
 *
 * \ingroup group_vision_function_vpac_nf
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
 * \ingroup group_vision_function_vpac_nf
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
 * \ingroup group_vision_function_vpac_nf
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
    vx_float64  sigma_space[8];  /*!< Array of space sigmas used to generate a 5x5 gaussian filter around the center pixel */
    vx_float64  sigma_range[8];  /*!< Array of range sigmas used to weight the neigborhood pixels according to their absolute difference in value from the center pixel */
} tivx_vpac_nf_bilateral_sigmas_t;

/*!
 * \brief Configuration parameters used to set HTS BW limit used by
          TIVX_KERNEL_VPAC_NF kernel.
 *
 * \ingroup group_vision_function_vpac_nf
 */
typedef struct {
    /*! 0 = Disable HTS BW limiter, 1 = Enable */
    uint32_t  enable_hts_bw_limit;
    /*! Cycle count between each HTS trigger */
    uint32_t  cycle_cnt;
    /*! Count for which cycle_cnt average is calculated */
    uint32_t  token_cnt;
} tivx_vpac_nf_hts_bw_limit_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_nf Target
 * \ingroup group_vision_function_vpac_nf
 */
void tivxRegisterHwaTargetVpacNfKernels(void);


/*!
 * \brief Function to un-register HWA Kernels on the vpac_nf Target
 * \ingroup group_vision_function_vpac_nf
 */
void tivxUnRegisterHwaTargetVpacNfKernels(void);


/*! \brief [Graph] Creates a VPAC_NF_GENERIC Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input object of a single params structure of type <tt>\ref tivx_vpac_nf_common_params_t</tt>.
 * \param [in] input The input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] conv The input convolution matrix. Max columns or rows supported is 5.  Scale value is ignored.  Coefficients are 9-bit signed.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \see <tt>TIVX_KERNEL_VPAC_NF_GENERIC_NAME</tt>
 * \ingroup group_vision_function_vpac_nf
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfGenericNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_convolution       conv,
                                      vx_image             output);

/*! \brief [Graph] Creates a VPAC_NF_BILATERAL Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input object of a single params structure of type <tt>\ref tivx_vpac_nf_bilateral_params_t</tt>.
 * \param [in] input The input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] sigmas The input object of a single params structure of type <tt>\ref tivx_vpac_nf_bilateral_sigmas_t</tt>.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \see <tt>TIVX_KERNEL_VPAC_NF_BILATERAL_NAME</tt>
 * \ingroup group_vision_function_vpac_nf
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacNfBilateralNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input,
                                      vx_user_data_object  sigmas,
                                      vx_image             output);

/*!
 * \brief Function to initialize NF common Parameters
 *
 * \param prms  [IN] Pointer to NF common parameters
 *
 * \ingroup group_vision_function_vpac_nf
 */
void tivx_vpac_nf_common_params_init(tivx_vpac_nf_common_params_t *prms);

/*!
 * \brief Function to initialize NF HTS bandwidth limit Parameters
 *
 * \param prms  [IN] Pointer to NF HTS bandwidth limit parameters
 *
 * \ingroup group_vision_function_vpac_nf
 */
void tivx_vpac_nf_hts_bw_limit_params_init(tivx_vpac_nf_hts_bw_limit_params_t *prms);

/*!
 * \brief Function to initialize NF bilateral Parameters
 *
 * \param prms  [IN] Pointer to NF bilateral parameters
 *
 * \ingroup group_vision_function_vpac_nf
 */
void tivx_vpac_nf_bilateral_params_init(tivx_vpac_nf_bilateral_params_t *prms);

/*!
 * \brief Function to initialize NF bilateral Sigmas
 *
 * \param sigmas  [IN] Pointer to NF bilateral sigmas
 *
 * \ingroup group_vision_function_vpac_nf
 */
void tivx_vpac_nf_bilateral_sigmas_init(tivx_vpac_nf_bilateral_sigmas_t *sigmas);


#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_NF_H_ */

