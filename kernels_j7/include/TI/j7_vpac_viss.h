/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef J7_VPAC_VISS_H_
#define J7_VPAC_VISS_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS kernels in this kernel extension.
 */

/*! \brief vpac_viss kernel name
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_KERNEL_VPAC_VISS_NAME     "com.ti.hwa.vpac_viss"

/*********************************
 *      VPAC_VISS Control Commands
 *********************************/

/*! \brief Control Command to set the DCC (Dynamic Camera Configuration)
 *         information to the given VISS Node.
 *
 *         Viss node gets the pointer to DCC buffer containing
 *         VISS configuration. It uses DCC parser to parse and
 *         map DCC parameters into VISS configuration and sets it
 *         in the driver.
 *
 *         Note that if the module is bypassed in tivx_vpac_viss_params_t
 *         during node creating, the parameter will not be set for
 *         this module, even through DCC profile has config for
 *         the same module.
 *
 *         User data object containing DCC buffer is passed
 *         as argument with this control command.
 *
 *  \ingroup group_vision_function_vpac_ldc
 */
#define TIVX_VPAC_VISS_SET_DCC_PARAMS                   (0x30000000u)

/*********************************
 *      VPAC_VISS Defines
 *********************************/

/*! \brief Maximum H3A number of bytes in statistics data array
 *
 *   Allocating 24KB buffer which is large enough for 32x32 windows
 *
 *  \ingroup group_vision_function_vpac_viss
 */
#define TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES            (24576U)



/*********************************
 *      VPAC_VISS STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Identifier for DCC profile, this directly is passed to DCC parser
     *  to get the VISS configuration */
    uint32_t                    sensor_dcc_id;
    /*! Identifier corresponding to the sub-setting within the
     *  sensor configuration of the DCC file,
     *  This directly is passed to DCC parser for getting VISS configuration */
    uint32_t                    use_case;

    /**
     *  Below table provides output format supported on different
     *  output for the corresponding mux value.
     *  Note that mux value is used only if the corresponding output
     *  image is set to non-null. Otherwise mux value is ignored.
     *
     *  |val|  mux_output0   |   mux_output1  |   mux_output2  |   mux_output3  |   mux_output4   |
     *  | 0 |  Y12(P12/U16)  | UV12(P12/U16)  |     Y8(U8)     |   UV8(P12/U16) |     Invalid     |
     *  | 1 |     Invalid    |     Invalid    |     R8(U8)     |     G8(U8)     |     B8(U8)      |
     *  | 2 |     Invalid    |  C1(P12/U16)   |  C2(P12/U16)   |  C3(P12/U16)   |  C4(P12/U16)    |
     *  | 3 | Value(P12/U16) |     Invalid    |    Value(U8)   |     Invalid    |  Saturation(U8) |
     *  | 4 |    NV12_P12    |     Invalid    |      NV12      |     Invalid    |     Invalid     |
     *  | 5 |     Invalid    |     Invalid    |     YUV422     |     Invalid    |     Invalid     |
     */

    /*! Mux to select the output for the output0.
     *
     *  Valid values for this mux are
     *  0: Luma 12bit output either in U16 or P12 format
     *  3: Value output form the HSV module either in U16 or P12 format
     *  4: NV12 output format in P12 format.
     *
     *  Note that if this mux is set to TIVX_DF_IMAGE_NV12_P12, output1
     *  must be disabled and mux_output1 is ignored.
     */
    uint32_t                    mux_output0;

    /*! Mux to select the output for the output1.
     *
     *  Valid values for this mux are
     *  0: Chroma 12bit output either in U16 or P12 format
     *  2: C1 output from CFA either in U16 or P12 format
     *
     *  Note: this mux is ignored if output0 is set to output NV12 data.
     */
    uint32_t                    mux_output1;

    /*! Mux to select the output for the output2.
     *
     *  Valid values for this mux are
     *  0: Luma in U8 format
     *  1: Red color component in U8 format
     *  2: C2 output from CFA either in U16 or P12 format
     *  3: Value output form the HSV module in U8 format
     *  4: NV12 output format in 8bit format.
     *  5: YUV422 interleaved in UYVY or YUYV format.
     *
     *  Note that if this mux is set to VX_DF_IMAGE_NV12 or
     *  VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY, output3
     *  must be disabled by passing NULL to output3 input parameter
     *  and mux_output3 is ignored.
     *  Also if the output0 is selected for NV12_P12 format, output2
     *  cannot be YUV422 (UYVY or YUYV).
     */
    uint32_t                    mux_output2;

    /*! Mux to select the output for the output3.
     *
     * Valid values for this mux are
     *  0: Chroma in U8 format
     *  1: Green color component in U8 format
     *  2: C3 output from CFA either in U16 or P12 format
     *
     *  Note: this mux is ignored if output2 is set to output NV12 data.
     */
    uint32_t                    mux_output3;

    /*! Mux to select the output for the output4.
     *
     *  1: Blue color component in U8 format
     *  2: C4 output from CFA either in U16 or P12 format
     *  3: Saturation from HSV module in U8 format
     */
    uint32_t                    mux_output4;

    /*! Flag to enable/bypass GLBCE processing:
     *  1: Bypasses GLBCE,  0: Enables GLBCE */
    uint32_t                    bypass_glbce;
    /*! Flag to enable/bypass NSF4 processing:
     *  1: Bypasses NSF4,  0: Enables NSF4 */
    uint32_t                    bypass_nsf4;

    /* May change in between frames */
    /*!< If h3a_aew_af output port is not NULL, this provides
     *   input source of h3a
     *
     *   Valid values for this input are
     *   0: H3A Input is from RAW0
     *   1: H3A Input is from RAW1
     *   2: H3A Input is from RAW2
     *   3: H3A Input is from LSC
     */
    uint32_t                    h3a_in;

    /*!< If h3a_aew_af output port is not NULL, this variable
     *   selects the h3a module to be enabled
     *
     *   Valid values are 0: Disabled 1: AEWB 2:AF
     */
    uint32_t                    h3a_aewb_af_mode;

    /*! Enables/Disables Edge Enhancer (EE) and also selects the Luma
     *  channel on which the EE to be enabled
     *
     *  Valid values are
     *  0: Edge Enhancer is disabled
     *  1: Edge Enhancer is enabled on Y12 output (output0)
     *  2: Edge Enhancer is enabled on Y8 output (output2)
     */
    uint32_t                    ee_mode;

    /*! Selects the chroma output format, when chroma only output is
     *  selected in mux_output1 or mux_output3.
     *
     *  Note that there is only one chroma down sampler, so if
     *  one of output0 or output2 is selected as NV12, the other output,
     *  if selected as chroma only, is fixed to 420 output..
     *  Also if chroma only output is selected in both mux_output1
     *  and mux_output3, chroma format must be same for both outputs.
     *
     *  Valid Values are
     *  0:420 Mode
     *  1:422 Mode
     */
    uint32_t                    chroma_mode;
} tivx_vpac_viss_params_t;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /* Data corresponding to result of 2A algorithm ...
     * used to index into the appropriate photospace in the DCC files (resv) */

    /*! Indicates the source data corresponding to this data:
     *   0: RAW0, 1: RAW1, 2: RAW2, 3: LSC */
    uint32_t                    h3a_source_data;
    /*! Measured in micro seconds (us) */
    uint32_t                    exposure_time;
    /*! Analog Gain */
    uint32_t                    analog_gain;
    /*! Is AE output valid */
    uint32_t                    ae_valid;
    /*! Is AE converged */
    uint32_t                    ae_converged;
    /*! Digital Gain */
    uint32_t                    digital_gain;
    /*! White Balance Gains */
    uint32_t                    wb_gains[4];
    /*! White Balance Offsets */
    int32_t                     wb_offsets[4];
    /*! Color Temperature (K) */
    uint32_t                    color_temperature;
    /*! Is AWB output valid */
    uint32_t                    awb_valid;
    /*! Is AWB converged */
    uint32_t                    awb_converged;
} tivx_ae_awb_params_t;

/*!
 * \brief H3A AEW configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct
{
    /*! AEW Window Height */
    uint16_t                    aewwin1_WINH;
    /*! AEW Window Width */
    uint16_t                    aewwin1_WINW;
    /*! AEW Vertical Count */
    uint16_t                    aewwin1_WINVC;
    /*! AEW Horizontal Count */
    uint16_t                    aewwin1_WINHC;
    /*! AEW Subwindow Vertical Increment Value */
    uint16_t                    aewsubwin_AEWINCV;
    /*! AEW Subwindow Horizontal Increment Value */
    uint16_t                    aewsubwin_AEWINCH;
} tivx_h3a_aew_config;

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_VISS kernel.
 *        Must be allocated at 64 byte boundary.
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Indicates the contents of this buffer:
     *   1: AEW data, 2:AF data */
    uint32_t                    aew_af_mode;
    /*! Indicates the source data corresponding to this data:
     *   0: RAW0, 1: RAW1, 2: RAW2, 3: LSC */
    uint32_t                    h3a_source_data;
    /*! If aew_af_mode == 1, this is the aew HW configuration used */
    tivx_h3a_aew_config         aew_config;
    /*! Total used size of the data buffer in bytes */
    uint32_t                    size;
    /*! Reserved dummy field to make data to be 64 byte aligned */
    uint32_t                    resv[(64U-(sizeof(tivx_h3a_aew_config)+12U))/4U ];
    /*! Payload of the AEW or AF data */
    uint8_t                     data[TIVX_VPAC_VISS_MAX_H3A_STAT_NUMBYTES];
} tivx_h3a_data_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_hwa
 */
void tivxRegisterHwaTargetVpacVissKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the vpac_viss Target
 * \ingroup group_vision_function_hwa
 */
void tivxUnRegisterHwaTargetVpacVissKernels(void);

/*! \brief [Graph] Creates a VPAC_VISS Node.
 *
 *
 * VISS node supports 5 optional outputs (output0 to output4),
 * atleast one of the outputs must be enabled.
 *
 * The resolution of all the image ports should have the same width
 * and height. The only exception to this is if the
 * output1 and/or output3 is used to output chroma alone by selecting
 * appropriate mux_output1 and mux_output3, then the height is
 * half of the input for these 2 ports if the chroma_mode is
 * selected as 0U (420 Mode).
 *
 * \param [in] graph The reference to the graph.
 * \param [in] configuration             The input object of a single params structure of type
 *                                       <tt>\ref tivx_vpac_viss_params_t</tt>.
 *                                       These parameters essentially defines path inside
 *                                       VISS and are used to select output format.
 * \param [in] ae_awb_result (optional)  The input object of a single params structure of
 *                                       type <tt>\ref tivx_ae_awb_params_t</tt>.
 *                                       Typically this input parameter would come
 *                                       from a 2A algorithm node.
 * \param [in] dcc_buf (optional)        DCC tuning database for the given sensor
 *                                       <tt>\ref vx_user_data_object </tt>
 * \param [in] raw                       The RAW input image (can contain up to 3 exposures
 *                                       plus meta data) in P12 or U16 or U8 format.
 *                                       RAW Image at index 0 is used for single exposure
 *                                          processing and also for Linear mode processing.
 *                                       For two exposure WDR merge mode, RAW Image at
 *                                          index 0 is used for short exposure and image
 *                                          at index1 is used for long exposure.
 *                                       For three exposure WDR merge mode, RAW Image at
 *                                          index 0 is used for very short exposure,
 *                                          image at index1 for short exposure and image
 *                                          at index2 for long exposure.
 * \param [out] output0 (optional)       Output0 for 12bit output.
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 12bit format, or
 *                                          Luma plane of the YUV420 frame or
 *                                          Value from the HSV module
 *                                       mux_output0 in <tt>\ref tivx_vpac_viss_params_t</tt>
 *                                          is used to select the output format when
 *                                          data format is set to U16 or P12
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>.
 * \param [out] output1 (optional)       Output1 for 12bit output.
 *                                       Typically this output is used to get
 *                                          Chroma plane of YUV420 frame in 12bit format or
 *                                          One of the CFA output  in 12bit format
 *                                       mux_output1 in <tt>\ref tivx_vpac_viss_params_t</tt>
 *                                          is used to select the output format
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output0 is not set
 *                                          to YUV420 output format.
 * \param [out] output2  (optional)      Output2 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 8bit format or
 *                                          Luma portion of the YUV420 frame in 8bit or
 *                                          Red color plane in 8bit or
 *                                          YUV422 (YUYV or UYVY) in 8bit format or
 *                                          One of the color output CFA in 12bit or
 *                                          Value output from HSV module in 8bit
 *                                       mux_output2 in <tt>\ref tivx_vpac_viss_params_t</tt>
 *                                          is used to select the output format when
 *                                          data format is set to U8 or U16 or P12
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref VX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] output3  (optional)      Output2 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          Chroma portion of the YUV420 frame or
 *                                          One of the color output CFA in 12bit
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref VX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output2 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] output4  (optional)      Output2 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          Saturation from HSV block in 8bit or
 *                                          One of the color output CFA in 12bit
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref VX_DF_IMAGE_P12</tt>
 * \param [out] h3a_output (optional)    AEWB/AF output.
 *                                       This output is used to get AEWB/AF output.
 *                                       User data object of type tivx_h3a_data_t
 *                                       is used to AEWB/AF output.
 *                                       Only one of AEWB & AF can be enabled and
 *                                       outputted at a time.
 * \param [out] histogram (optional)     The output histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bits.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 *
 * \see <tt>TIVX_KERNEL_VPAC_VISS_NAME</tt>
 *
 * \ingroup group_vision_function_vpac_viss
 *
 * \return <tt>\ref vx_node</tt>.
 *
 * \retval vx_node A node reference.
 *         Any possible errors preventing a successful creation should
 *         be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVpacVissNode(vx_graph  graph,
                                      vx_user_data_object   configuration,
                                      vx_user_data_object   ae_awb_result,
                                      vx_user_data_object   dcc_buf,
                                      tivx_raw_image        input_raw_images,
                                      vx_image              output0,
                                      vx_image              output1,
                                      vx_image              output2,
                                      vx_image              output3,
                                      vx_image              output4,
                                      vx_user_data_object   h3a_output,
                                      vx_distribution       histogram);

/*!
 * \brief Function to initialize VISS Parameters
 *
 * \param prms  [in] Pointer to VISS parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_vpac_viss_params_init(tivx_vpac_viss_params_t *prms);

/*!
 * \brief Function to initialize H3A data Parameters
 *
 * \param prms  [in] Pointer to H3A data parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_h3a_data_init(tivx_h3a_data_t *prms);

/*!
 * \brief Function to initialize AEWB Output Parameters
 *        These parameters come from the AEWB algorithm.
 *
 * \param prms  [in] Pointer to AEWB output parameters
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_ae_awb_params_init(tivx_ae_awb_params_t *prms);

#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_VISS_H_ */

