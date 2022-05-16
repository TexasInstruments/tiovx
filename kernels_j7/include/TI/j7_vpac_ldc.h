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

#ifndef J7_VPAC_LDC_H_
#define J7_VPAC_LDC_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The LDC kernels in this kernel extension.
 */

/*! \brief vpac_ldc kernel name
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_KERNEL_VPAC_LDC_NAME     "com.ti.hwa.vpac_ldc"


/*********************************
 *      VPAC_LDC Control Commands
 *********************************/

/*! \brief Control Command to set LDC Read Bandwidth limit parameters
 *         User data object tivx_vpac_ldc_bandwidth_params_t is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS         (0x10000000u)

/*! \brief Control Command to set LDC Lut parameters
 *         This LUT is used in bit depth conversion for output 2 & 3.
 *         An array of tivx_vpac_ldc_bit_depth_conv_lut_params_t is
 *         used an an argument to this command.
 *         Params at index-0 is used for setting Luma output (output2) and
 *         Params at index-1 is used for setting Chroma output (output3).
 *         Both entries are optional and can be null if no change is required.
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS     (0x10000001u)

/*! \brief Control Command to get the error status
 *         Returns the error status of the last processed frame.
 *         Reference to vx_scalar is passed as argument with
 *         this control command.
 *         Node returns bit-mask of error status in u32 variable of vx_scalar.
 *
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CMD_GET_ERR_STATUS                    (0x10000002u)

/*! \brief Control Command to set the LDC Parameters
 *
 *         This command takes an array of vx_references as an argument,
 *         where each index contains a reference to a parameter to update.
 *
 *         The following are the supported paramters:
 *         index 0 : vx_matrix reference for updating warp matrix.
 *
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CMD_SET_LDC_PARAMS                    (0x10000003u)

/*! \brief Control Command to set the DCC (Dynamic Camera Configuration)
 *         information to the given LDC Node.
 *
 *         Ldc node gets the pointer to DCC buffer containing
 *         LDC configuration. It uses DCC parser to parse and
 *         map DCC parameters into LDC configuration and sets it
 *         in the driver.
 *
 *         User data object containing DCC buffer is passed
 *         as argument with this control command.
 *
 *         The ID of this command must be the same as TIVX_VPAC_VISS_CMD_SET_DCC_PARAMS
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS                  (0x30000000u)

/*********************************
 *      VPAC_LDC Defines
 *********************************/

/*!
 * \defgroup group_vision_function_vpac_ldc_enums Enumerations
 * \brief Enumerations for configuration parameters in VPAC LDC structures
 * \ingroup group_vision_function_vpac_ldc
 * @{*/

/*! \see tivx_vpac_ldc_params_t::input_align_12bit */
#define TIVX_VPAC_LDC_ALIGN_LSB        (0U)
/*! \see tivx_vpac_ldc_params_t::input_align_12bit */
#define TIVX_VPAC_LDC_ALIGN_MSB        (1U)

/*! \see tivx_vpac_ldc_params_t::luma_interpolation_type */
#define TIVX_VPAC_LDC_INTERPOLATION_BICUBIC        (0U)
/*! \see tivx_vpac_ldc_params_t::input_aluma_interpolation_typelign_12bit */
#define TIVX_VPAC_LDC_INTERPOLATION_BILINEAR       (1U)

/*! \see tivx_vpac_ldc_params_t::yc_mode */
#define TIVX_VPAC_LDC_MODE_LUMA_ONLY        (0U)
/*! \see tivx_vpac_ldc_params_t::yc_mode */
#define TIVX_VPAC_LDC_MODE_CHROMA_ONLY      (1U)

/*@}*/

/*! \brief Macro to calculate mesh line offset for the given width and
 *         down scaling factor.
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_LDC_CALC_MESH_LINE_OFFSET(width, subsample_factor)      \
    ((((((width)/(subsample_factor))+1) + 15U) & ~15U) * (4U))

/*! Size of the Remap LUT, used for 12bit to 8bit conversion */
#define TIVX_VPAC_LDC_BIT_DEPTH_CONV_LUT_SIZE           (513u)

/*! Error status for pixel co-ordinate goes out of the
 *  pre-computed input pixel bounding box */
#define TIVX_VPAC_LDC_PIX_BLK_OUTOFBOUND_ERR_STATUS    (0x1u)
/*! Error status for Block mesh co-ordinate goes out of the pre-computed
 *  mesh bounding box */
#define TIVX_VPAC_LDC_MESH_BLK_OUTOFBOUND_ERR_STATUS   (0x2u)
/*! Input Pixel block memory overflow */
#define TIVX_VPAC_LDC_PIX_MEM_OVF_ERR_STATUS           (0x4u)
/*! Mesh block memory overflow */
#define TIVX_VPAC_LDC_MESH_MEM_OVF_ERR_STATUS          (0x8u)
/*! Back mapped input co-ordinate goes out of input frame range */
#define TIVX_VPAC_LDC_PIX_FRM_OUTOFBOUND_ERR_STATUS    (0x10u)
/*! Affine and perspective transform precision overflow error */
#define TIVX_VPAC_LDC_SZ_OVF_ERR_STATUS                (0x20u)
/*! Error on SL2 VBSUM Write interface */
#define TIVX_VPAC_LDC_SL2_WR_ERR_STATUS                (0x80u)
/*! Error on Input VBUSM Read interface */
#define TIVX_VPAC_LDC_RD_ERR_STATUS                    (0x100u)

#define TIVX_VPAC_LDC_SET_PARAMS_WARP_MATRIX_IDX        (0u)

#define TIVX_VPAC_LDC_DEF_BLOCK_WIDTH                  (64u)
#define TIVX_VPAC_LDC_DEF_BLOCK_HEIGHT                 (64u)
#define TIVX_VPAC_LDC_DEF_PIXEL_PAD                    (0u)

/*********************************
 *      VPAC_LDC STRUCTURES
 *********************************/

/*!
 * \brief The region_params data structure used by the
 *        TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {
    /*! Region enable, 0: Disabled, 1: Enabled */
    uint32_t                                enable;
    /*! Output block width (must be multiple of 8) [Range (8 - 255)] */
    uint32_t                                out_block_width;
    /*! Output block height (must be multiple of 2) [Range (2 - 255)] */
    uint32_t                                out_block_height;
    /*! Pixel Padding [Range (0 - 15)] */
    uint32_t                                pixel_pad;
} tivx_vpac_ldc_region_params_t;

/*!
 * \brief The alternative region_params data structure used by
 *        the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {
    /*! Width of each column of sub-frames [Range (1 - 8191)] */
    uint32_t                                reg_width[3];
    /*! Height of each row of sub-frames [Range (1 - 8191)] */
    uint32_t                                reg_height[3];
    /*! Each Region Parameters Parameters */
    tivx_vpac_ldc_region_params_t           reg_params[3][3];
} tivx_vpac_ldc_multi_region_params_t;

/*!
 * \brief The mesh data structure in the configuration data
 *        structure used by the TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {

    /*! Mesh table full-frame width before subsampling */
    uint32_t                                mesh_frame_width;
    /*! Mesh table full-frame height before subsampling */
    uint32_t                                mesh_frame_height;

    /*! Lut down sampling factor,
     *  Valid value is in the range [0,7]. */
    uint32_t                                subsample_factor;
} tivx_vpac_ldc_mesh_params_t;

/*!
 * \brief The configuration data structure used by the
 *        TIVX_KERNEL_VPAC_LDC kernel.
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {
    /*! Optional: When input 16-bit unpacked, alignment of 12-bit pixel,
     *  0: LSB, 1:MSB */
    uint32_t                                input_align_12bit;
    /*! Luma Interpolation Type, 0: Bicubic; 1: Bilinear */
    uint32_t                                luma_interpolation_type;

    /*! Output starting x-coordinate (must be multiple of 8)
     *  [Range (0 - 8191)] */
    uint32_t                                init_x;
    /*! Output starting y-coordinate (must be multiple of 2)
     *  [Range (0 - 8191)] */
    uint32_t                                init_y;

    /*! When LDC is used to operate on single plane mode, ie by
     *  settings output format to U8/U16/P12, this flag can be
     *  used to configure LDC in Luma only or Chroma Only mode
     *
     *  0: Luma only mode, 1: Chroma Only mode
     *
     *  Note: In case of chroma only mode, LDC assumes chroma is from
     *  YUV420 input and expects the frame size
     *  to be for YUV420 frame, ie frame size of luma, where chroma
     *  height is half of luma height.
     *  Node internally takes care of multiplying height by 2. */
    uint32_t                                yc_mode;

    /*! Optional: dcc camera id when dcc is used for LDC settings */
    uint32_t                                dcc_camera_id;
} tivx_vpac_ldc_params_t;


/*!
 * \brief The bandwidth params structure used by the
 *        TIVX_KERNEL_VPAC_LDC kernel.
 *        Passed as argument to TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS
 *        command.
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {
    /*! Limits the mean bandwidth (computed over one block) that
     *  the LDC module can request for read from system memory.
     *  [Range (0 - 4095)]
     *    0: (Default) the bandwidth limiter is bypassed
     *    1~4095: Maximum number of bytes per 256 cycles.
     *
     *    Examples:
     *      1 : 1.17 MBytes/s @ 300 MHz
     *      4095 : ~4.8 GBytes/s @300 MHz
     */
    uint32_t                                bandwidth_control;
    /*! Limits the maximum number of outstanding LDC requests to TAG_CNT+1.
     *  [Range (0 - 31)]
     *    (Default): 31
     */
    uint32_t                                tag_count;
    /*! Limits the maximum burst length that could be used by LDC.
     *  [Range (0 - 3)]
     *    Each burst is of 16 bytes.  Hardware breaks the command
     *    at max_burst_length boundary.
     *    0: 16 (16*16 bytes = 256 bytes)
     *    1:  8 (8*16 bytes  = 128 bytes) (Default)
     *    2:  4 (4*16 bytes  =  64 bytes)
     *    3:  2 (2*16 bytes  =  32 bytes)
     */
    uint32_t                                max_burst_length;
} tivx_vpac_ldc_bandwidth_params_t;

/*!
 * \brief The remap params structure used by the
 *        TIVX_KERNEL_VPAC_LDC kernel.
 *        Passed as argument to TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS
 *        command.
 *        Used to set Remap/Tone Map Lut in LDC
 *
 * \ingroup group_vision_function_vpac_ldc
 */
typedef struct {
    /*! Number of input bits to remap/tone map module.
     *  [Range (8 - 12)]
     */
    uint32_t                                input_bits;
    /*! Number of output bits from remap/tone map module.
     *  [Range (8 - 12)]
     */
    uint32_t                                output_bits;
    /*! 513 entry tone mapping curve
     */
    uint16_t                                lut[TIVX_VPAC_LDC_BIT_DEPTH_CONV_LUT_SIZE];
} tivx_vpac_ldc_bit_depth_conv_lut_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_ldc Target
 * \ingroup group_vision_function_vpac_ldc
 */
void tivxRegisterHwaTargetVpacLdcKernels(void);


/*!
 * \brief Function to un-register HWA Kernels on the vpac_ldc Target
 * \ingroup group_vision_function_vpac_ldc
 */
void tivxUnRegisterHwaTargetVpacLdcKernels(void);


/*! \brief [Graph] Creates a VPAC_LDC Node.
 *
 *  <b> Valid input/output format combinations: </b>
 *
 *  Input Format           | Output Format
 *  -----------------------|--------------
 *  VX_DF_IMAGE_U8         | VX_DF_IMAGE_U8
 *  ^                      | TIVX_DF_IMAGE_P12
 *  VX_DF_IMAGE_U16        | VX_DF_IMAGE_U16
 *  TIVX_DF_IMAGE_P12      | TIVX_DF_IMAGE_P12
 *  ^                      | VX_DF_IMAGE_U8
 *  VX_DF_IMAGE_NV12       | VX_DF_IMAGE_NV12
 *  ^                      | TIVX_DF_IMAGE_NV12_P12
 *  TIVX_DF_IMAGE_NV12_P12 | TIVX_DF_IMAGE_NV12_P12
 *  ^                      | VX_DF_IMAGE_NV12
 *  VX_DF_IMAGE_UYVY       | VX_DF_IMAGE_UYVY
 *  ^                      | VX_DF_IMAGE_YUYV
 *  ^                      | VX_DF_IMAGE_NV12
 *  ^                      | TIVX_DF_IMAGE_NV12_P12
 *
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input object of a single params
 *             structure of type <tt>\ref tivx_vpac_ldc_params_t</tt>.
 * \param [in] warp_matrix (optional) Input warp_matrix of type
 *             <tt>\ref vx_matrix</tt> for affine or
 *             perspective transform configuration.
 *             Must be 2x3 (affine) or 3x3 (perspective), and of type
 *             <tt>\ref VX_TYPE_INT16</tt> if using HW register values,
 *             or <tt>\ref VX_TYPE_FLOAT32</tt> if using matrix values
 *             defined in OpenVX warp functions ( \ref vxWarpAffineNode,
 *             \ref vxWarpPerspectiveNode).
 * \param [in] region_prms (optional) The input object of a single params
 *             structure of type <tt>\ref tivx_vpac_ldc_region_params_t</tt>
 *             or <tt>\ref tivx_vpac_ldc_multi_region_params_t</tt>.
 *             If set to null, default block size
 *             (TIVX_VPAC_LDC_DEF_BLOCK_WIDTH x
 *             TIVX_VPAC_LDC_DEF_BLOCK_HEIGHT) and pixel padding
 *             (TIVX_VPAC_LDC_DEF_PIXEL_PAD) is used.
 * \param [in] mesh_prms (optional) Mesh Configuration, the input object of type
 *             <tt>\ref tivx_vpac_ldc_mesh_params_t</tt>. It is used to
 *             provide frame size and downsampling factor of mesh_img.
 *             If set to null, back mapping is disabled.
 * \param [in] mesh_img (optional) Mesh image containing mesh 2D coordinate offset lookup table.
 *             This can be a full remap table per output pixel, but is typically sub-sampled by a power
 *             of 2 as per tivx_vpac_ldc_mesh_params_t.subsample_factor to save memory
 *             footprint and bandwidth. The pitch/line offset for the mesh can be calculated using
 *             TIVX_VPAC_LDC_CALC_MESH_LINE_OFFSET.
 *             The coordinate offsets are of type S16Q3, and are stored as X,Y pairs
 *             of type <tt>\ref VX_DF_IMAGE_U32</tt>.
 *             - 32bit encoding format (X,Y) is as follows:
 *               - Y offset (16 bit, 13bit signed integer, 3 bit fractional)
 *                  - [2:0]   Fractional is 3 bits (support 1/8th pixel of precision)
 *                  - [15:3]  Signed integer offset in verical direction
 *               - X offset (16 bit, 13bit signed integer, 3 bit fractional)
 *                  - [18:16] Fractional is 3 bits (support 1/8th pixel of precision)
 *                  - [31:19] Signed integer offset in horizontal direction
 *             If set to null, back mapping is disabled.
 * \param [in] dcc_db (optional) DCC tuning database for the given sensor
 *             <tt>\ref vx_user_data_object </tt>
 * \param [in] in_img The input image in
 *             <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *             <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>,
 *             <tt>\ref VX_DF_IMAGE_UYVY</tt>,
 *             <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *             <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *             <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *             format.
 * \param [out] out0_img The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>,
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt>,
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only),
 *              <tt>\ref VX_DF_IMAGE_U16</tt> (12bit in 16bit container Luma only), or
 *              <tt>\ref TIVX_DF_IMAGE_P12</tt> (12bit packed Luma only)
 *              format.
 * \param [out] out1_img (optional) The output image in
 *              <tt>\ref VX_DF_IMAGE_NV12</tt>,
 *              <tt>\ref VX_DF_IMAGE_UYVY</tt>,
 *              <tt>\ref VX_DF_IMAGE_YUYV</tt>,
 *              <tt>\ref VX_DF_IMAGE_U8</tt> (8bit Luma only)
 *              format.
 *
 * \see <tt>TIVX_KERNEL_VPAC_LDC_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_ldc
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *  Any possible errors preventing a successful creation should be
 *  checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacLdcNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_matrix            warp_matrix,
                                      vx_user_data_object  region_prms,
                                      vx_user_data_object  mesh_prms,
                                      vx_image             mesh_img,
                                      vx_user_data_object  dcc_db,
                                      vx_image             in_img,
                                      vx_image             out0_img,
                                      vx_image             out1_img);

/*!
 * \brief Function to initialize LDC Region parameters
 *
 * \param prms  [IN] Pointer to LDC region parameter structure
 *
 * \ingroup group_vision_function_vpac_ldc
 */
void tivx_vpac_ldc_region_params_init(tivx_vpac_ldc_region_params_t *prms);

/*!
 * \brief Function to initialize LDC Multi Region parameters
 *
 * \param prms  [IN] Pointer to LDC multi region parameter structure
 *
 * \ingroup group_vision_function_vpac_ldc
 */
void tivx_vpac_ldc_multi_region_params_init(tivx_vpac_ldc_multi_region_params_t *prms);

/*!
 * \brief Function to initialize LDC Mesh parameters
 *
 * \param prms  [IN] Pointer to LDC mesh parameter structure
 *
 * \ingroup group_vision_function_vpac_ldc
 */
void tivx_vpac_ldc_mesh_params_init(tivx_vpac_ldc_mesh_params_t *prms);

/*!
 * \brief Function to initialize Bandwidth limiter parameters
 *
 * \param prms  [IN] Pointer to LDC bandwidth limiter parameter structure
 *
 * \ingroup group_vision_function_vpac_ldc
 */
void tivx_vpac_ldc_bandwidth_params_init(
    tivx_vpac_ldc_bandwidth_params_t *prms);

/*!
 * \brief Function to initialize LDC parameters with the default values
 *
 * \param prms  [IN] Pointer to LDC parameter structure
 *
 * \ingroup group_vision_function_vpac_ldc
 */
void tivx_vpac_ldc_params_init(tivx_vpac_ldc_params_t *prms);

#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_LDC_H_ */


