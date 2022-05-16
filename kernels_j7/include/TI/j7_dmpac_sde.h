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

#ifndef J7_DMPAC_SDE_H_
#define J7_DMPAC_SDE_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The SDE kernels in this kernel extension.
 */

/*! \brief dmpac_sde kernel name
 *  \ingroup group_vision_function_dmpac_sde
 */
#define TIVX_KERNEL_DMPAC_SDE_NAME     "com.ti.hwa.dmpac_sde"


/*********************************
 *      DMPAC_SDE Control Commands
 *********************************/

/*! \brief Control Command to set TODO
 *
 *
 */
#define TIVX_DMPAC_SDE_CMD_SET_HTS_LIMIT                     (0x10000000u)

/*! \brief Control Command to get the error status
 *         Returns the error status of the last processed frame.
 *         Reference to vx_scalar is passed as argument with
 *         this control command.
 *         Node returns bit-mask of error status in u32 variable of vx_scalar.
 */
#define TIVX_DMPAC_SDE_CMD_GET_ERR_STATUS                    (0x10000002u)

/*********************************
 *      DMPAC_SDE Defines
 *********************************/

/* TODO */

/*********************************
 *      DMPAC_SDE STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_DMPAC_SDE kernel.
 *
 * \ingroup group_vision_function_dmpac_sde
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
    /*! Defines custom ranges for mapping 7-bit confidence score to one of 8 levels (3-bit confidence value) in each disparity output. ([Range (0 - 127)]
     *    The confidence score will map to level N if it is less than confidence_score_map[N] but greater than or equal to confidence_score_map[N-1]
     *    For example, to map internal confidence scores from 0 to 50 to level 0, and confidence scores from 51 to 108 to level 1,
     *    then set confidence_score_map[0] = 51 and confidence_score_map[1] = 109
     *    \note Each mapping value must be greater than the values from lower indices of the array
     *    \note confidence_score_map indices 0 - 6 must be less than 127
     */
    uint16_t  confidence_score_map[8];
} tivx_dmpac_sde_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the dmpac_sde Target
 * \ingroup group_vision_function_dmpac_sde
 */
void tivxRegisterHwaTargetDmpacSdeKernels(void);


/*!
 * \brief Function to un-register HWA Kernels on the dmpac_sde Target
 * \ingroup group_vision_function_dmpac_sde
 */
void tivxUnRegisterHwaTargetDmpacSdeKernels(void);


/*! \brief [Graph] Creates a DMPAC_SDE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input object of a single params structure of type <tt>\ref tivx_dmpac_sde_params_t</tt>.
 * \param [in] left The left input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_NV12</tt> (Y plane only), <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [in] right The right input image in <tt>\ref VX_DF_IMAGE_U8</tt>, <tt>\ref VX_DF_IMAGE_NV12</tt> (Y plane only), <tt>\ref VX_DF_IMAGE_U16</tt>, or <tt>\ref TIVX_DF_IMAGE_P12</tt> format.
 * \param [out] output The output image in <tt>\ref VX_DF_IMAGE_S16</tt> format. Bit packing format: Sign[15], Integer[14:7], Fractional[6:3], Confidence[2:0]. <br>
 *                     The 3 bit 'Confidence' value is the result of passing the 7-bit confidence score through the user configured
 *                     \ref tivx_dmpac_sde_params_t::confidence_score_map range table to produce a 3-bit mapping.
 * \param [out] confidence_histogram (optional) Histogram of the full 7-bit confidence scores.  Must be configured to 128 bins.
 * \see <tt>TIVX_KERNEL_DMPAC_SDE_NAME</tt>
 * \ingroup group_vision_function_dmpac_sde
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDmpacSdeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             left,
                                      vx_image             right,
                                      vx_image             output,
                                      vx_distribution      confidence_histogram);

/*!
 * \brief Function to initialize SDE Parameters
 *
 * \param prms  [IN] Pointer to SDE parameters
 *
 * \ingroup group_vision_function_dmpac_sde
 */
 void tivx_dmpac_sde_params_init(tivx_dmpac_sde_params_t *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_DMPAC_SDE_H_ */
