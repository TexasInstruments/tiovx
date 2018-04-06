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


#ifndef TIVX_KERNELS_HOST_UTILS_
#define TIVX_KERNELS_HOST_UTILS_

#include <TI/tivx_debug.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface file for utility functions for the Kernel
 */

typedef vx_status (*tivxHostKernel_Fxn) (vx_context context);

typedef struct {
    tivxHostKernel_Fxn    add_kernel;
    tivxHostKernel_Fxn    remove_kernel;
} Tivx_Host_Kernel_List;

/*!
 * \brief Publishes the kernels list on the host side
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxPublishKernels(vx_context context, Tivx_Host_Kernel_List *kernel_list, uint32_t num_kernels);

/*!
 * \brief Unpublishes the kernels list on the host side
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxUnPublishKernels(vx_context context, Tivx_Host_Kernel_List *kernel_list, uint32_t num_kernels);


/*!
 * \brief Maximum number of images (input/output) supported in
 *        calculating valid rectangles
 *
 * \ingroup group_tivx_ext_common_kernel
 */
#define TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE        (5u)

typedef struct
{
    /*! \brief List of input images */
    vx_image in_img[TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE];
    /*! \brief number Valid entries in in_img array */
    vx_uint32 num_input_images;

    /*! \brief List of output images */
    vx_image out_img[TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE];
    /*! \brief number of Valid entries in out_img array */
    vx_uint32 num_output_images;

    /*! \brief Padding requied by the kernel */
    vx_uint32 top_pad, bot_pad, right_pad, left_pad;

    /*! \brief Input Border Mode */
    vx_enum border_mode;
} tivxKernelValidRectParams;

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelValidateParametersNotNull(const vx_reference *parameters, vx_uint8 maxParams);

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelValidateInputSize(vx_uint32 inputWidth0, vx_uint32 inputWidth1,
                            vx_uint32 inputHeight0, vx_uint32 inputHeight1);

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelValidatePossibleFormat(vx_df_image inputFormat, vx_df_image possibleFormat);

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelValidateScalarType(vx_enum scalarType, vx_enum expectedScalarType);

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelValidateOutputSize(vx_uint32 expectedWidth, vx_uint32 outputWidth, vx_uint32 expectedHeight,
                             vx_uint32 outputHeight, vx_image outputImage);

/*!
 * \ingroup group_tivx_ext_host_kernel
 */
void tivxKernelSetMetas(vx_meta_format *metas, vx_uint8 maxParams, vx_df_image fmt, vx_uint32 width, vx_uint32 height);

/*!
 * \brief Function to initialize Valid Rect Parameter structure
 *        Currently the entire structure is memset to 0
 *
 * \param prms [in] Valid Rectange Parameters
 *
 * \ingroup group_tivx_ext_host_kernel
 */
static inline void tivxKernelValidRectParams_init(
    tivxKernelValidRectParams *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0, sizeof(tivxKernelValidRectParams));
        prms->border_mode = VX_BORDER_UNDEFINED;
    }
}

/*!
 * \brief Function to set the status variable equal to status_temp
 *        if the status_temp variable is not VX_SUCCESS.
 *
 * \param status [in,out] status variable
 * \param status_temp [in] temporary status variable
 *
 * \ingroup group_tivx_ext_host_kernel
 */
static inline void tivxCheckStatus(vx_status *status, vx_status status_temp)
{
    if(VX_SUCCESS != status_temp) {
        *status = status_temp;
    }
}

/*!
 * \brief Function to calculate and configure valid region
 *        This API loops over all the input and output image's valid
 *        rectangles and figures out overlapping rectangle and sets it
 *        as valid rectangle in the output image.
 *
 *        Each kernel may also require few lines/pixels of padding on
 *        each side of the image. Host side kernel can provide this
 *        information to this API and it will adjust valid
 *        rectangle considering padding requirement also.
 *
 *        This is utility API.
 *
 * \param prms [in] Valid Rectange Parameters
 *
 * \ingroup group_tivx_ext_host_kernel
 */
vx_status tivxKernelConfigValidRect(tivxKernelValidRectParams *prms);

#ifdef __cplusplus
}
#endif

#endif
