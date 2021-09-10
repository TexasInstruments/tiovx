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

#ifndef J7_VPAC_VISS3_H_
#define J7_VPAC_VISS3_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS kernels in this kernel extension.
 */

#define TIVX_VPAC_VISS_FCP_NUM_INSTANCES    (2U)

/*!
 * \defgroup group_vision_function_vpac_viss_mux Enumerations
 * \brief Enumerations for configuration output routing in VPAC VISS structures
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_DISABLED    (0U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_RFE   (1U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_CAC   (2U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_NSF4  (3U)
/*! \see tivx_vpac_viss_params_t::fcp1_config */
#define TIVX_VPAC_VISS_FCP1_INPUT_GLBCE (4U)

/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP0            (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP1            (1U)

/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT0        (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT1        (0U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT2        (1U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT3        (1U)
/*! \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_FCP_OUT4        (0U)

/*! Macro to set the output_fcp_mapping
    \see tivx_vpac_viss_params_t::output_fcp_mapping */
#define TIVX_VPAC_VISS_MAP_FCP_OUTPUT(fcp, output)   (output<<1 | fcp)

/*@}*/


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

    /*! There are two FCP units in the VISS.  FCP[0] is typically used for human
     *  vision, while FCP[1] can be used for machine vision pipeline in parallel.
     *  While the fcp[0] input is dictated by the enabling/bypassing of the upstream
     *  blocks, fcp[1] can be disabled, or configured to use an input from different
     *  tap points in the upstream processing chain.
     *   Valid values for this input are:
     *  |          Enum                          |   Description                    |
     *  |:---------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_FCP1_DISABLED      | FCP1 is disabled                 |
     *  | \ref TIVX_VPAC_VISS_FCP1_INPUT_RFE     | FCP1 input is from RawFE         |
     *  | \ref TIVX_VPAC_VISS_FCP1_INPUT_CAC     | FCP1 input is from CAC           |
     *  | \ref TIVX_VPAC_VISS_FCP1_INPUT_NSF4    | FCP1 input is from NSF4          |
     *  | \ref TIVX_VPAC_VISS_FCP1_INPUT_GLBCE   | FCP1 input is from GLBCE         |
     */
    uint32_t                    fcp1_config;

    /*! There are two FCP units in the VISS.  Each FCP unit has flexible multiplexing
     *  of internal taps/formats to each of their 5 output channels.  This strucuture
     *  can be used to configure the signal for each of the outputs of each fcp unit,
     *  as well as other fcp specific settings. */
    tivx_vpac_viss_fcp_params_t     fcp[TIVX_VPAC_VISS_FCP_NUM_INSTANCES];

    /*! Mapping for FCP output to final VISS output ports.
     *  \details Below table provides output format supported on different
     *  outputs for the corresponding mux value in this structure.
     *  Note that mux value is used only if the corresponding output
     *  image is set to non-null. Otherwise mux value is ignored.
     *
     *  |val|  output_...[0] |  output_...[1] |  output_...[2] |  output_...[3] |  output_...[4]  |
     *  |:-:|:--------------:|:--------------:|:--------------:|:--------------:|:---------------:|
     *  | 0 |  FCP0 output0  |  FCP0 output1  |  FCP0 output0  |  FCP0 output1  |  FCP0 output4   |
     *  | 1 |  FCP1 output0  |  FCP1 output1  |  FCP1 output0  |  FCP1 output1  |  FCP1 output4   |
     *  | 2 |  FCP0 output2  |  FCP0 output3  |  FCP0 output2  |  FCP0 output3  |     Invalid     |
     *  | 3 |  FCP1 output2  |  FCP1 output3  |  FCP1 output2  |  FCP1 output3  |     Invalid     |
     *
     *  \see TIVX_VPAC_VISS_MAP_FCP_OUTPUT() macro to use easy to read enums instead of
     *  integer values.
     */
    uint32_t                    output_fcp_mapping[5];

    /*! Flag to enable/bypass chromatic aberation correction (CAC) processing:
     *  1: Bypasses CAC,  0: Enables CAC */
    uint32_t                    bypass_cac;

    /*! Flag to enable/bypass dynamic white balance (DWB) processing:
     *  1: Bypasses DWB,  0: Enables DWB */
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
 * \param [out] output0 (optional)       Output0.
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 12bit or 8bit format, or
 *                                          Luma plane of the YUV420 frame in 12 or 8 bit, or
 *                                          Value from the HSV module, or
 *                                          Red color plane in 8bit or
 *                                          YUV422 (YUYV or UYVY) in 8bit format or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[0]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>.
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] output1 (optional)       Output1.
 *                                       Typically this output is used to get
 *                                          Chroma plane of YUV420 frame in 12bit or 8bit format or
 *                                          One of the CFA output  in 12bit format
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[1]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output0 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] output2 (optional)       Output2.
 *                                       Typically this output is used to get
 *                                          YUV420 frame in 12bit or 8bit format, or
 *                                          Luma plane of the YUV420 frame in 12 or 8 bit, or
 *                                          Value from the HSV module, or
 *                                          Red color plane in 8bit or
 *                                          YUV422 (YUYV or UYVY) in 8bit format or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[2]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_NV12_P12</tt>.
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_NV12</tt>
 *                                          <tt>\ref VX_DF_IMAGE_YUYV</tt>
 *                                          <tt>\ref VX_DF_IMAGE_UYVY</tt>
 * \param [out] output3 (optional)       Output3.
 *                                       Typically this output is used to get
 *                                          Chroma plane of YUV420 frame in 12bit or 8bit format or
 *                                          One of the CFA output  in 12bit format
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[3]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
 *                                       Supported image format are
 *                                          <tt>\ref VX_DF_IMAGE_U8</tt>
 *                                          <tt>\ref VX_DF_IMAGE_U16</tt>
 *                                          <tt>\ref TIVX_DF_IMAGE_P12</tt>
 *                                       can be enabled only when output0 is not set
 *                                          to YUV420/UYVY/YUYV output formats.
 * \param [out] output4  (optional)      Output4 for 12bit or 8bit output.
 *                                       Typically this output is used to get
 *                                          Saturation from HSV block in 8bit or
 *                                          One of the color output CFA in 12bit
 *                                        <tt>\ref tivx_vpac_viss_params_t::output_fcp_mapping[4]</tt>
 *                                          along with <tt>\ref tivx_vpac_viss_params_t::fcp</tt>
 *                                          is used to select the output format from the appropriate FCP
 *                                          instance.
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
 * \param [out] histogram0 (optional)    The output FCP0 histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 * \param [out] histogram1 (optional)    The output FCP1 histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 256 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 256 x sizeof(uint32_t), which is sufficient
 *                                       for storing 256x20bit histogram.
 * \param [out] raw_histogram (optional) The output raw data histogram.
 *                                       The number of bins for this histogram is
 *                                       fixed to 128 bins.
 *                                       The memory size allocated for this histogram
 *                                       is 128 x sizeof(uint32_t), which is sufficient
 *                                       for storing 128x22bit histogram.
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

#endif /* J7_VPAC_VISS3_H_ */

