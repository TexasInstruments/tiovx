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

#ifndef J7_VPAC_VISS_FCP_H_
#define J7_VPAC_VISS_FCP_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The VISS FCP structures in this kernel extension.
 */

/*!
 * \defgroup group_vision_function_vpac_viss_fcp Enumerations
 * \brief Enumerations for configuration parameters in VPAC VISS FCP structure
 * \ingroup group_vision_function_vpac_viss
 * @{*/

/*! \see tivx_vpac_viss_fcp_params_t::mux_output0 */
#define TIVX_VPAC_VISS_MUX0_Y12        (0U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output0 */
#define TIVX_VPAC_VISS_MUX0_VALUE12    (3U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output0 */
#define TIVX_VPAC_VISS_MUX0_NV12_P12   (4U)
#ifdef VPAC3L
/*! \see tivx_vpac_viss_fcp_params_t::mux_output0 */
#define TIVX_VPAC_VISS_MUX0_IR8        (6U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output0 */
#define TIVX_VPAC_VISS_MUX0_IR12_P12   (7U)
#endif

/*! \see tivx_vpac_viss_fcp_params_t::mux_output1 */
#define TIVX_VPAC_VISS_MUX1_UV12       (0U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output1 */
#define TIVX_VPAC_VISS_MUX1_C1         (2U)

/*! \see tivx_vpac_viss_fcp_params_t::mux_output2*/
#define TIVX_VPAC_VISS_MUX2_Y8         (0U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_RED        (1U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_C2         (2U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_VALUE8     (3U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_NV12       (4U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_YUV422     (5U)
#ifdef VPAC3L
/*! \see tivx_vpac_viss_fcp_params_t::mux_output2 */
#define TIVX_VPAC_VISS_MUX2_IR12_U16   (6U)
#endif

/*! \see tivx_vpac_viss_fcp_params_t::mux_output3 */
#define TIVX_VPAC_VISS_MUX3_UV8        (0U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output3 */
#define TIVX_VPAC_VISS_MUX3_GREEN      (1U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output3 */
#define TIVX_VPAC_VISS_MUX3_C3         (2U)

/*! \see tivx_vpac_viss_fcp_params_t::mux_output4 */
#define TIVX_VPAC_VISS_MUX4_BLUE       (1U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output4 */
#define TIVX_VPAC_VISS_MUX4_C4         (2U)
/*! \see tivx_vpac_viss_fcp_params_t::mux_output4 */
#define TIVX_VPAC_VISS_MUX4_SAT        (3U)

/*! \see tivx_vpac_viss_fcp_params_t::ee_mode */
#define TIVX_VPAC_VISS_EE_MODE_OFF     (0U)
/*! \see tivx_vpac_viss_fcp_params_t::ee_mode */
#define TIVX_VPAC_VISS_EE_MODE_Y12     (1U)
/*! \see tivx_vpac_viss_fcp_params_t::ee_mode */
#define TIVX_VPAC_VISS_EE_MODE_Y8      (2U)

/*! \see tivx_vpac_viss_fcp_params_t::chroma_mode */
#define TIVX_VPAC_VISS_CHROMA_MODE_420 (0U)
/*! \see tivx_vpac_viss_fcp_params_t::chroma_mode */
#define TIVX_VPAC_VISS_CHROMA_MODE_422 (1U)

/*@}*/

/*********************************
 *      VPAC_VISS FCP STRUCTURE
 *********************************/

/*!
 * \brief The Flex Color Processing (FCP) data structure used in the \ref tivx_vpac_viss_params_t
 *        structure for programming the TIVX_KERNEL_VISS kernel.
 *
 *  \details Below table provides output format supported on different
 *  outputs of each FCP unit in the VISS for the corresponding mux value in this structure.
 *
 *  |val|  mux_output0   |   mux_output1  |   mux_output2  |   mux_output3  |   mux_output4   |
 *  |:-:|:--------------:|:--------------:|:--------------:|:--------------:|:---------------:|
 *  | 0 |  Y12(P12/U16)  | UV12(P12/U16)  |     Y8(U8)     |   UV8(P12/U16) |     Invalid     |
 *  | 1 |     Invalid    |     Invalid    |     R8(U8)     |     G8(U8)     |     B8(U8)      |
 *  | 2 |     Invalid    |  C1(P12/U16)   |  C2(P12/U16)   |  C3(P12/U16)   |  C4(P12/U16)    |
 *  | 3 | Value(P12/U16) |     Invalid    |    Value(U8)   |     Invalid    |  Saturation(U8) |
 *  | 4 |    NV12_P12    |     Invalid    |      NV12      |     Invalid    |     Invalid     |
 *  | 5 |     Invalid    |     Invalid    |     YUV422     |     Invalid    |     Invalid     |
 *
 * \ingroup group_vision_function_vpac_viss
 */
typedef struct {
    /*! Mux to select the output format for the \a output0 parameter of an FCP.
     *
     *  Valid values for this mux are:
     *  |          Enum                       |   Description                                  |
     *  |:------------------------------------|:-----------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_MUX0_Y12        | Luma 12bit output either in U16 or P12 format  |
     *  | \ref TIVX_VPAC_VISS_MUX0_VALUE12    | Value output form the HSV module either in U16 or P12 format  |
     *  | \ref TIVX_VPAC_VISS_MUX0_NV12_P12   | NV12 output format in P12 format.              |
     *
     *  \note If this mux is set to \ref TIVX_DF_IMAGE_NV12_P12, output1
     *  must be disabled and \ref mux_output1 is ignored.
     */
    uint32_t                    mux_output0;

    /*! Mux to select the output format for the \a output1 parameter of an FCP.
     *
     *  Valid values for this mux are:
     *  |          Enum                       |   Description                                   |
     *  |:------------------------------------|:------------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_MUX1_UV12       | Chroma 12bit output either in U16 or P12 format |
     *  | \ref TIVX_VPAC_VISS_MUX1_C1         | C1 output from CFA either in U16 or P12 format  |
     *
     *  \note This mux is ignored if output0 is set to output NV12 data.
     */
    uint32_t                    mux_output1;

    /*! Mux to select the output format for the \a output2 parameter of an FCP.
     *
     *  Valid values for this mux are:
     *  |          Enum                       |   Description                                  |
     *  |:------------------------------------|:-----------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_MUX2_Y8         | Luma in U8 format                              |
     *  | \ref TIVX_VPAC_VISS_MUX2_RED        | Red color component in U8 format               |
     *  | \ref TIVX_VPAC_VISS_MUX2_C2         | C2 output from CFA either in U16 or P12 format |
     *  | \ref TIVX_VPAC_VISS_MUX2_VALUE8     | Value output form the HSV module in U8 format  |
     *  | \ref TIVX_VPAC_VISS_MUX2_NV12       | NV12 output format in 8bit format              |
     *  | \ref TIVX_VPAC_VISS_MUX2_YUV422     | YUV422 interleaved in UYVY or YUYV format      |
     *
     *  \note If this mux is set to VX_DF_IMAGE_NV12 or
     *  VX_DF_IMAGE_YUYV or VX_DF_IMAGE_UYVY, output3
     *  must be disabled by passing NULL to output3 input parameter
     *  and \ref mux_output3 is ignored.
     *  Also if the output0 is selected for NV12_P12 format, output2
     *  cannot be YUV422 (UYVY or YUYV).
     */
    uint32_t                    mux_output2;

    /*! Mux to select the output format for the \a output3 parameter of an FCP.
     *
     * Valid values for this mux are:
     *  |          Enum                       |   Description                                  |
     *  |:------------------------------------|:-----------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_MUX3_UV8        | Chroma in U8 format                            |
     *  | \ref TIVX_VPAC_VISS_MUX3_GREEN      | Green color component in U8 format             |
     *  | \ref TIVX_VPAC_VISS_MUX3_C3         | C3 output from CFA either in U16 or P12 format |
     *
     *  \note This mux is ignored if output2 is set to output NV12 data.
     */
    uint32_t                    mux_output3;

    /*! Mux to select the output format for the \a output4 parameter of an FCP.
     *
     * Valid values for this mux are:
     *  |          Enum                       |   Description                                  |
     *  |:------------------------------------|:-----------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_MUX4_BLUE       | Blue color component in U8 format              |
     *  | \ref TIVX_VPAC_VISS_MUX4_C4         | C4 output from CFA either in U16 or P12 format |
     *  | \ref TIVX_VPAC_VISS_MUX4_SAT        | Saturation from HSV module in U8 format        |
     */
    uint32_t                    mux_output4;

    /*! Enables/Disables Edge Enhancer (EE) and also selects the Luma
     *  channel on which the EE to be enabled.
     *
     *  Valid values are
     *  |          Enum                       |   Description                                     |
     *  |:------------------------------------|:--------------------------------------------------|
     *  | \ref TIVX_VPAC_VISS_EE_MODE_OFF     | Edge Enhancer is disabled                         |
     *  | \ref TIVX_VPAC_VISS_EE_MODE_Y12     | Edge Enhancer is enabled on Y12 output (output0)  |
     *  | \ref TIVX_VPAC_VISS_EE_MODE_Y8      | Edge Enhancer is enabled on Y8 output (output2)   |
     */
    uint32_t                    ee_mode;

    /*! Selects the chroma output format, when chroma only output is
     *  selected in \ref mux_output1 or \ref mux_output3.
     *
     *  Valid Values are
     *  |          Enum                        |   Description                    |
     *  |:-------------------------------------|:---------------------------------|
     *  | \ref TIVX_VPAC_VISS_CHROMA_MODE_420  | 420 Mode                         |
     *  | \ref TIVX_VPAC_VISS_CHROMA_MODE_422  | 422 Mode                         |
     *
     *  \note There is only one chroma down sampler, so if
     *  one of output0 or output2 is selected as NV12, the other output,
     *  if selected as chroma only, is fixed to 420 output.
     *  Also if chroma only output is selected in both \ref mux_output1
     *  and \ref mux_output3, chroma format must be same for both outputs.
     *
     */
    uint32_t                    chroma_mode;
} tivx_vpac_viss_fcp_params_t;

#ifdef __cplusplus
}
#endif

#endif /* J7_VPAC_VISS_FCP_H_ */
