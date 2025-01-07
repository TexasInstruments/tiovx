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




#include <TI/tivx.h>
#include <TI/tivx_debug.h>
#include <VX/vx_types.h>
#include <VX/vx_khr_safe_casts.h>
#include <tivx_kernels_host_utils.h>

vx_status tivxPublishKernels(vx_context context, const Tivx_Host_Kernel_List *kernel_list, uint32_t num_kernels)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i < num_kernels; i ++)
    {
        if (kernel_list[i].add_kernel != NULL)
        {
            status = kernel_list[i].add_kernel(context);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

vx_status tivxUnPublishKernels(vx_context context, const Tivx_Host_Kernel_List *kernel_list, uint32_t num_kernels)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i < num_kernels; i ++)
    {
        if (kernel_list[i].remove_kernel != NULL)
        {
            status = kernel_list[i].remove_kernel(context);
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

vx_status tivxKernelValidateParametersNotNull(const vx_reference *parameters, vx_uint8 maxParams)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i;

    for (i = 0U; i < maxParams; i ++)
    {
        /* Check for NULL */
        if (NULL == parameters[i])
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter %d is NULL\n", i);
            break;
        }
    }

    return status;
}

vx_status tivxKernelConfigValidRect(tivxKernelValidRectParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_rectangle_t out_rect, rect;
    vx_uint32 i;

    if (NULL == prms)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if ((prms->num_input_images >
                TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE) ||
            (prms->num_output_images >
                TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE))
        {
                status = (vx_status)VX_FAILURE;
        }

        for (i = 0; i < prms->num_input_images; i ++)
        {
            if (NULL == prms->in_img[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
        for (i = 0; i < prms->num_output_images; i ++)
        {
            if (NULL == prms->out_img[i])
            {
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        out_rect.start_y = 0;
        out_rect.start_x = 0;
        out_rect.end_y = 0xFFFFFFFFU;
        out_rect.end_x = 0xFFFFFFFFU;

        for (i = 0; i < prms->num_input_images; i ++)
        {
            status = vxGetValidRegionImage(prms->in_img[i], &rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            else
            {
                if (rect.start_x > out_rect.start_x)
                {
                    out_rect.start_x = rect.start_x;
                }
                if (rect.start_y > out_rect.start_y)
                {
                    out_rect.start_y = rect.start_y;
                }

                if (rect.end_x < out_rect.end_x)
                {
                    out_rect.end_x = rect.end_x;
                }
                if (rect.end_y < out_rect.end_y)
                {
                    out_rect.end_y = rect.end_y;
                }
            }
        }
        for (i = 0; (i < prms->num_output_images) && ((vx_status)VX_SUCCESS == status); i ++)
        {
            status = vxGetValidRegionImage(prms->out_img[i], &rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            else
            {
                if (rect.start_x > out_rect.start_x)
                {
                    out_rect.start_x = rect.start_x;
                }
                if (rect.start_y > out_rect.start_y)
                {
                    out_rect.start_y = rect.start_y;
                }

                if (rect.end_x < out_rect.end_x)
                {
                    out_rect.end_x = rect.end_x;
                }
                if (rect.end_y < out_rect.end_y)
                {
                    out_rect.end_y = rect.end_y;
                }
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        if ((vx_enum)VX_BORDER_UNDEFINED == prms->border_mode)
        {
            out_rect.start_x += prms->left_pad;
            out_rect.start_y += prms->top_pad;
            out_rect.end_x -= prms->right_pad;
            out_rect.end_y -= prms->bot_pad;
        }

        for (i = 0; i < prms->num_output_images; i ++)
        {
            status = vxSetImageValidRectangle(prms->out_img[i], &out_rect);

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
        }

    }
    return (status);
}

vx_status tivxKernelsHostUtilsAddKernelTargetDsp(vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    status = tivxAddKernelTarget(kernel, TIVX_TARGET_DSP1);
    #if defined(SOC_J721E) || defined(SOC_J722S)
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxAddKernelTarget(kernel, TIVX_TARGET_DSP2);
    }
    #endif
    return status;
}

vx_status tivxKernelsHostUtilsAddKernelTargetMcu(vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    #ifndef SOC_AM62A
    status = tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0);
    #ifndef SOC_J722S
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_1);
    }
    #endif
    #endif
    #if defined(SOC_AM62A) || defined(SOC_J722S)
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxAddKernelTarget(kernel, TIVX_TARGET_MCU1_0);
    }
    #endif
    return status;
}
