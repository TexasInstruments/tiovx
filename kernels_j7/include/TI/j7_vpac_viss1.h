/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
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

#ifndef J7_VPAC_VISS1_H_
#define J7_VPAC_VISS1_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS kernels in this kernel extension.
 */

#define TIVX_VPAC_VISS_FCP_NUM_INSTANCES    (1U)

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

    /*! [DONT CARE]: This is not used in TDA4VM
     */
    uint32_t                    fcp1_config;

    /*! There is one FCP unit in the VPAC1 VISS.  Each FCP unit has flexible multiplexing
     *  of internal taps/formats to each of their 5 output channels.  This strucuture
     *  can be used to configure the signal for each of the outputs of each fcp unit,
     *  as well as other fcp specific settings. */
    tivx_vpac_viss_fcp_params_t     fcp[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];

    /*! [DONT CARE]: This is not used in J721E
     */
    uint32_t                    output_fcp_mapping[5];

    /*! [DONT CARE]: This is not used in J721E */
    uint32_t                    bypass_cac;

    /*! [DONT CARE]: This is not used in J721E */
    uint32_t                    bypass_dwb;

    /*! Flag to enable/bypass NSF4 processing:
     *  1: Bypasses NSF4,  0: Enables NSF4 */
    uint32_t                    bypass_nsf4;

    /*! Flag to enable/bypass GLBCE processing:
     *  1: Bypasses GLBCE,  0: Enables GLBCE */
    uint32_t                    bypass_glbce;

    /*!< If \a h3a_aew_af output port of \ref tivxVpacVissNode is not NULL, this provides
     *   input source of h3a
     *
     *   Valid values for this input are:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW0     | H3A Input is from RAW0           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW1     | H3A Input is from RAW1           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_RAW2     | H3A Input is from RAW2           |
     *  | \ref TIVX_VPAC_VISS_H3A_IN_LSC      | H3A Input is from LSC            |
     *
     *  \note May change in between frames
     */
    uint32_t                    h3a_in;

    /*!< If h3a_aew_af output port of \ref tivxVpacVissNode is not NULL, this variable
     *   selects the h3a module to be enabled
     *
     *   Valid values are:
     *  |          Enum                       |   Description                    |
     *  |:------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AEWB   | AEWB                             |
     *  | \ref TIVX_VPAC_VISS_H3A_MODE_AF     | AF                               |
     */
    uint32_t                    h3a_aewb_af_mode;

    /*! Enables/Disables Ctx save and restore.
     *  If enabled, the node restores the context before submitting frame
     *  to the driver and saves the context after frame completion.
     *
     *  Currently, Ctx save/restore is supported only for GLBCE
     *  statistics memory.
     *
     *  Note: It allocates additional memory for CTX and uses UDMA common
     *  channel for R5F for the memcpy.
     *
     *  By default, it is disabled in the init API. Application requires to
     *  enable it based on number of instances of VISS and use of GLBCE
     *  in VISS.
     */
    uint32_t                    enable_ctx;
    /*! Identifier for camera channel ID.
     *  Currently not being used, potentially for future need.
     */
    uint32_t                    channel_id;
} tivx_vpac_viss_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

/*! \brief [Graph] Creates a VPAC_VISS Node.
 *
 * At a high level, VPAC VISS converts RAW image sensor data into
 * processed YUV or RGB images.
 *
 * VISS node supports 5 optional outputs (output0 to output4),
 * and at least one of the outputs must be enabled.
 *
 * The resolution of all the image ports should have the same width
 * and height. The only exception to this is if the
 * output1 and/or output3 is used to output chroma alone by selecting
 * appropriate tivx_vpac_viss_params_t::fcp::mux_output1 and
 * tivx_vpac_viss_params_t::fcp::mux_output3, then the height is
 * half of the input for these 2 ports if the tivx_vpac_viss_params_t::fcp::chroma_mode is
 * selected as TIVX_VPAC_VISS_CHROMA_MODE_420.
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
 *                                        <tt>\ref tivx_vpac_viss_params_t::mux_output0</tt>
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
 *                                        <tt>\ref tivx_vpac_viss_params_t::mux_output1</tt>
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
 *                                        <tt>\ref tivx_vpac_viss_params_t::mux_output2</tt>
 *                                          is used to select the output format when
 *                                          data format is set to U8 or U16 or P12
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] output3  (optional)      Output3 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          Chroma portion of the YUV420 frame or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::mux_output3</tt>
 *                                          is used to select the output format.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output2 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] output4  (optional)      Output4 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          Saturation from HSV block in 8bit or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::mux_output4</tt>
 *                                          is used to select the output format.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 * \param [out] h3a_output (optional)    AEWB/AF output.
 *                                       This output is used to get AEWB/AF output.
 *                                       User data object of type \ref tivx_h3a_data_t
 *                                       is used to AEWB/AF output.
 *                                       Only one of AEWB & AF can be enabled and
 *                                       outputted at a time.
 * \param [out] histogram0 (optional)    The output histogram from the FCP.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 * \param [out] histogram1 (optional)    [SHOULD BE NULL] This is not used in J721E
 * \param [out] raw_histogram (optional) [SHOULD BE NULL] This is not used in J721E
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
                                      tivx_raw_image        raw,
                                      vx_image              output0,
                                      vx_image              output1,
                                      vx_image              output2,
                                      vx_image              output3,
                                      vx_image              output4,
                                      vx_user_data_object   h3a_output,
                                      vx_distribution       histogram0,
                                      vx_distribution       histogram1,
                                      vx_distribution       raw_histogram);

#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_VISS1_H_ */

