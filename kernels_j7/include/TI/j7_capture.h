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

#ifndef J7_CAPTURE_H_
#define J7_CAPTURE_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Capture kernels in this kernel extension.
 */

/*! \brief capture kernel name
 *  \ingroup group_vision_function_capture
 */
#define TIVX_KERNEL_CAPTURE_NAME          "com.ti.capture"


/*********************************
 *      Capture Control Commands
 *********************************/


/* None */

/*********************************
 *      Capture Defines
 *********************************/

/*! \brief Maximum number of channels supported in the capture node.
 *
 *  \ingroup group_vision_function_capture
 */
#define TIVX_CAPTURE_MAX_CH                                 (16U)


/*********************************
 *      Capture STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the TIVX_KERNEL_CAPTURE kernel.
 *
 * \ingroup group_vision_function_capture
 */
typedef struct
{
    uint32_t instId;                /*!< CSI2Rx Instance Id, 0:CSIRx0 1:CSIRx0 */
    uint32_t enableCsiv2p0Support;  /*!< Flag indicating CSIV2P0 support */
    uint32_t numDataLanes;          /*!< Number of CSIRX data lanes */
    uint32_t dataLanesMap[4];       /*!< Data Lanes map array; note: size from CSIRX_CAPT_DATA_LANES_MAX */
    uint32_t vcNum[TIVX_CAPTURE_MAX_CH]; /*!< Virtual Channel Number for each channel */
} tivx_capture_params_t;


/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the capture Target
 * \ingroup group_vision_function_capture
 */
void tivxRegisterHwaTargetCaptureKernels(void);


/*!
 * \brief Function to un-register HWA Kernels on the capture Target
 * \ingroup group_vision_function_capture
 */
void tivxUnRegisterHwaTargetCaptureKernels(void);


/*! \brief [Graph] Creates a capture Node. The capture node takes in a user data object of type <tt>\ref tivx_capture_params_t</tt> to
                   configure the sensors. The outputs are of type object array which contain vx_image.
 * \param [in] graph The reference to the graph.
 * \param [in] input The input user data object of a single capture params structure of type <tt>\ref tivx_capture_params_t</tt>.
 * \param [out] output Object array output which has been created from an exemplar of vx_image's. The input MUST be made from format
                <tt>\ref VX_DF_IMAGE_RGBX</tt>.
 * \see <tt>TIVX_KERNEL_CAPTURE_NAME</tt>
 * \ingroup group_vision_function_capture
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxCaptureNode(vx_graph graph,
                                      vx_user_data_object  input,
                                      vx_object_array      output);

/*!
 * \brief Function to initialize Capture Parameters
 *
 * \param prms  [in] Pointer to H3A aew config
 *
 * \ingroup group_vision_function_vpac_viss
 */
void tivx_capture_params_init(tivx_capture_params_t *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_CAPTURE_H_ */

