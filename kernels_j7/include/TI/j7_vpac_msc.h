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

#ifndef J7_VPAC_MSC_H_
#define J7_VPAC_MSC_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The MSC kernels in this kernel extension.
 */

/*! \brief vpac_msc scaler kernel name
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME "com.ti.hwa.vpac_msc_multi_scale"

/*! \brief vpac_msc pyramid kernel name
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME   "com.ti.hwa.vpac_msc_pyramid"

/*********************************
 *      VPAC_MSC Control Commands
 *********************************/

/*! \brief Control Command to set MSC Filter Coefficients
 *         User data object tivx_vpac_msc_coefficients_t is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_COEFF                    (0x20000000u)

/*! \brief Control Command to set MSC input parameters.
 *
 *         These parameters are common for all scaler outputs.
 *         Used to configure/select the number of taps to be
 *         used for scaling operation, line increment by 2 to improve
 *         HW throughput for 1/2 scaling.
 *
 *         This control command uses pointer to structure
 *         tivx_vpac_msc_input_params_t as an input argument.
 *
 *         The index0 of the vx_reference passed to the control command is
 *         used to specify user object containing input parameters.
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS             (0x20000001u)

/*! \brief Control Command to set MSC output scaling parameters.
 *
 *         This command takes an array of vx_references as an argument,
 *         where each index contains output parameters for the
 *         corresponding scaler. For example, the output parameters at
 *         the index0 contains output parameters for the scaler output0
 *         and so on. If the reference is set to null for a scaler, default
 *         scaler parameters or previously set/configured parameters
 *         are used for that scaler.
 *
 *         Used to configure/select number of phases for the
 *         scaling operation, the coefficients and other scaler parameters.
 *
 *         This command takes an array of user objects containing
 *         tivx_vpac_msc_output_params_t parameters..
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS            (0x20000002u)

/*! \brief Control Command to set MSC output croping parameters.
 *
 *         This command takes an array of vx_references as an argument,
 *         where each index contains crop parameters for the
 *         corresponding scaler. For example, the crop parameters at
 *         the index0 contains crop parameters required by output0 and so on.
 *         The crop parameters are applied on the corrosponding input image.
 *         If the reference is set to null for a scaler, default
 *         scaler parameters or previously set/configured parameters
 *         are used for that scaler. (Default: no crop)
 *
 *         Used to configure/select number of phases for the
 *         scaling operation, the coefficients and other scaler parameters.
 *
 *         This command takes an array of user objects containing
 *         tivx_vpac_msc_crop_params_t parameters.
 *
 *         Node: Applies only to \ref tivxVpacMscScaleNode
 *
 *
 *  \ingroup group_vision_function_vpac_msc
 */
#define TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS            (0x20000003u)

/*********************************
 *      VPAC_MSC Defines
 *********************************/

/*! Maximum number of Multi Phase Coefficient Set */
#define TIVX_VPAC_MSC_MAX_MP_COEFF_SET                (4U)

/*! Maximum number of Single Phase Coefficient Set */
#define TIVX_VPAC_MSC_MAX_SP_COEFF_SET                (2U)

/*! Maximum number of Scaler output supported by Scaler,
 *  Used by the sim target kernel */
#define TIVX_VPAC_MSC_MAX_OUTPUT                      (10U)

/*! Maximum number of filter taps supported in Scaler */
#define TIVX_VPAC_MSC_MAX_TAP                         (5U)

/*! 32 Phase coefficients for Scaler */
#define TIVX_VPAC_MSC_32_PHASE_COEFF                  (32U)

/*! Autocompute: Allow the node to autocompute instead of specifying */
#define TIVX_VPAC_MSC_AUTOCOMPUTE                     (0xFFFFFFFFU)

/*! Specify the 32 phase gaussian interpolation filter coeficients */
#define TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE (0)

/*********************************
 *      VPAC_MSC STRUCTURES
 *********************************/

/*!
 * \brief The coefficients input data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! 2 Sets of Single phase coefficient set 0, signed 10-bit */
    int32_t  single_phase[TIVX_VPAC_MSC_MAX_SP_COEFF_SET][TIVX_VPAC_MSC_MAX_TAP];
    /*! 4 Sets of Multi phase coefficient, signed 10-bit */
    int32_t  multi_phase[TIVX_VPAC_MSC_MAX_MP_COEFF_SET]
        [TIVX_VPAC_MSC_MAX_TAP*TIVX_VPAC_MSC_32_PHASE_COEFF];
} tivx_vpac_msc_coefficients_t;

/*!
 * \brief The configuration data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Single-phase horizontal filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient
     *         set 0 (specified using 'horz_coef_sel')
     */
    uint32_t  horz_coef_src;
    /*! Single-phase horizontal filter coefficient selection
     *      if (horz_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient
     *          set 0 [Range (0-31)]
     */
    uint32_t  horz_coef_sel;
    /*! Single-phase vertical filter coefficient source
     *     0 : Use one of the dedicated single-phase coefficient sets
     *     1 : Use one of the phases from the multi-phase coefficient
     *         set 0 (specified using 'vert_coef_sel')
     */
    uint32_t  vert_coef_src;
    /*! Single-phase vertical filter coefficient selection
     *      if (vert_coef_src == 0)
     *          Choose which single-phase coefficient set to use [Range (0-1)]
     *      else
     *          Choose which phase to use among multi-phase coefficient
     *          set 0 [Range (0-31)]
     */
    uint32_t  vert_coef_sel;
} tivx_vpac_msc_single_phase_params_t;

/*!
 * \brief The multi_phase data structure in the configuration data
 *        structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Multi-phase Mode, 0: 64 phases, 1: 32 phases */
    uint32_t  phase_mode;
    /*! Multi-phase horizontal coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint32_t  horz_coef_sel;
    /*! Multi-phase vertical coefficient set selection
     *      if (phase_mode == 0)
     *          0: 32 phase coefficient sets 0 & 1
     *          2: 32 phase coefficient sets 2 & 3
     *          1&3 : reserved
     *      else
     *          32 phase coefficient set [Range (0-3)]
     */
    uint32_t  vert_coef_sel;
    /*! Multi-phase initial horizontal resize phase (U12Q12):
     *  Used to align center tap if filter to appropriate input for first output [Range (0-4095, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_output_params_t::offset_x
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  init_phase_x;
    /*! Multi-phase initial vertical resize phase (U12Q12):
     *  Used to align center tap if filter to appropriate input for first output [Range (0-4095, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_output_params_t::offset_y
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  init_phase_y;
} tivx_vpac_msc_multi_phase_params_t;

/*!
 * \brief The crop config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Source region of interest X offset [Range (0-width)] */
    uint32_t  crop_start_x;
    /*! Source region of interest Y offset [Range (0-height)] */
    uint32_t  crop_start_y;
    /*! Source region of interest X width [Range (0-(width-startx))] */
    uint32_t  crop_width;
    /*! Source region of interest Y height [Range (0-(height-starty))] */
    uint32_t  crop_height;
} tivx_vpac_msc_crop_params_t;

/*!
 * \brief The output config data structure used by the TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! Integer type of input and output frame data,
     *  0: Unsigned 12-bit, 1: Signed 12-bit */
    uint32_t  signed_data;
    /*! 0: Single Phase Filter (integer ratios: 1x, 1/2x, 1/4x);
     *  1: Multi-phase scaling filter */
    uint32_t  filter_mode;
    /*! Sets the fractional bit precision of the 10-bit filter
     *  coefficient [Range (5-9)] */
    uint32_t  coef_shift;
    /*! Filter output saturation mdoe,
     *   0: [0..4095] clipping;
     *   1: [-2048..2047] clip followed by + 2048 */
    uint32_t  saturation_mode;
    /*! Source X offset: used to align center tap if filter to appropriate input for first output [Range (0,1, or TIVX_VPAC_MSC_AUTOCOMPUTE)]
     *  \see tivx_vpac_msc_multi_phase_params_t::init_phase_x
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  offset_x;
    /*! Source Y offset: used to align center tap if filter to appropriate input for first output [Range (0,1, or TIVX_VPAC_MSC_AUTOCOMPUTE))]
     *  \see tivx_vpac_msc_multi_phase_params_t::init_phase_y
     *  \note Using TIVX_VPAC_MSC_AUTOCOMPUTE aligns center of output to match center of input cropped region for each output */
    uint32_t  offset_y;
    /*! Optional: When input 16-bit unpacked, alignment of 12-bit pixel,
     *  0: LSB, 1:MSB */
    uint32_t  output_align_12bit;

    /*! Optional: When 'filter_mode' == 0: Single-phase */
    tivx_vpac_msc_single_phase_params_t single_phase;
    /*! Optional: When 'filter_mode' == 1: Multi-phase */
    tivx_vpac_msc_multi_phase_params_t  multi_phase;
} tivx_vpac_msc_output_params_t;

/*!
 * \brief The input config data structure used by the
 *        TIVX_KERNEL_VPAC_MSC kernel.
 *
 * \ingroup group_vision_function_vpac_msc
 */
typedef struct {
    /*! kernel size [Range (3 - 5)] */
    uint32_t  kern_sz;
    /*! 0: Off, 1: vertical skip input lines (LSE) */
    uint32_t  src_ln_inc_2;
    /*! Optional: When input 16-bit unpacked,
     *  alignment of 12-bit pixel, 0: LSB, 1:MSB */
    uint32_t  input_align_12bit;
} tivx_vpac_msc_input_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_msc Target
 * \ingroup group_vision_function_vpac_msc
 */
void tivxRegisterHwaTargetVpacMscKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_msc Target
 * \ingroup group_vision_function_vpac_msc
 */
void tivxUnRegisterHwaTargetVpacMscKernels(void);


/*! \brief Creates a VPAC_MSC Node with multi-scale outputs.
 * \param [in] graph The reference to the graph.
 * \param [in] in_img The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 * \param [out] out0_img The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 * \param [out] out1_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 * \param [out] out2_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 * \param [out] out3_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 * \param [out] out4_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *
 * \see <tt>TIVX_KERNEL_VPAC_MSC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_msc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscScaleNode(vx_graph graph,
                                      vx_image             in_img,
                                      vx_image             out0_img,
                                      vx_image             out1_img,
                                      vx_image             out2_img,
                                      vx_image             out3_img,
                                      vx_image             out4_img);

/*! \brief Creates a VPAC_MSC Node with multi-scale pyramid output.
 *
 *         By default, a separable 5-tap gaussian filter is used with following
 *         coefficients for half scale pyramid : [ 16, 64, 96, 64, 16 ], and a 32 phase
 *         5-tap gaussian filter is used for non-half scale pyramids.
 *         These can be customized using \ref TIVX_VPAC_MSC_CMD_SET_COEFF command.
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_img The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 * \param [out] out_pyramid The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 *
 * \see <tt>TIVX_KERNEL_VPAC_MSC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_msc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacMscPyramidNode(vx_graph graph,
                                      vx_image             in_img,
                                      vx_pyramid           out_pyramid);


/*!
 * \brief Function to initialize MSC Coefficients Parameters
 *        This initializes Coefficients to default values.
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_COEFF
 *
 * \param coeff          [IN] Pointer to MSC coefficient structure
 * \param interpolation  [IN] Indicates interpolation method to initialize coefficients to
 *                            ( \ref VX_INTERPOLATION_BILINEAR or
 *                              \ref VX_INTERPOLATION_NEAREST_NEIGHBOR or
 *                              \ref TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE)
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_coefficients_params_init(tivx_vpac_msc_coefficients_t *coeff, vx_enum interpolation);

/*!
 * \brief Function to initialize MSC input Parameters
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS
 *
 * \param prms  [IN] Pointer to MSC input parameters
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_input_params_init(tivx_vpac_msc_input_params_t *prms);

/*!
 * \brief Function to initialize MSC output Parameters
 *        This is used for control command: \ref TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS
 *
 * \param prms  [IN] Pointer to MSC output parameters
 *
 * \ingroup group_vision_function_vpac_msc
 */
void tivx_vpac_msc_output_params_init(tivx_vpac_msc_output_params_t *prms);

#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_MSC_H_ */

