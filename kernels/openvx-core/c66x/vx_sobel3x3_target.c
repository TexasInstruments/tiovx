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
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_sobel_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelSobelProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelSobelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelSobelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSobelProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst0, *dst1;
    uint8_t *src_addr;
    int16_t *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;

    if (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* Check for NULL */
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX])))
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *dst0_target_ptr;
        void *dst1_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX];
        dst0 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        dst1 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);

        tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
        vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
        vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
        vxlib_src.data_type = (uint32_t)VXLIB_UINT8;

        if (NULL != dst0)
        {
            dst0_target_ptr = tivxMemShared2TargetPtr(&dst0->mem_ptr[0]);

            tivxCheckStatus(&status, tivxMemBufferMap(dst0_target_ptr, dst0->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            tivxSetPointerLocation(dst0, &dst0_target_ptr, (uint8_t **)&dst_addr);

            tivxInitBufParams(dst0, &vxlib_dst);

            status = (vx_status)VXLIB_sobelX_3x3_i8u_o16s(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst);

            if (status != (vx_status)VXLIB_SUCCESS)
            {
                status = (vx_status)VX_FAILURE;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(dst0_target_ptr, dst0->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != dst1))
        {
            dst1_target_ptr = tivxMemShared2TargetPtr(&dst1->mem_ptr[0]);

            tivxCheckStatus(&status, tivxMemBufferMap(dst1_target_ptr, dst1->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            tivxSetPointerLocation(dst1, &dst1_target_ptr, (uint8_t **)&dst_addr);

            tivxInitBufParams(dst1, &vxlib_dst);

            status = (vx_status)VXLIB_sobelY_3x3_i8u_o16s(
                src_addr, &vxlib_src, dst_addr, &vxlib_dst);

            if (status != (vx_status)VXLIB_SUCCESS)
            {
                status = (vx_status)VX_FAILURE;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(dst1_target_ptr, dst1->mem_size[0],
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxEnableL1DandL2CacheWb();
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSobelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelSobelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return ((vx_status)VX_SUCCESS);
}

void tivxAddTargetKernelSobel3x3(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_sobel_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_SOBEL_3x3,
            target_name,
            tivxKernelSobelProcess,
            tivxKernelSobelCreate,
            tivxKernelSobelDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelSobel3x3(void)
{
    tivxRemoveTargetKernel(vx_sobel_target_kernel);
}
