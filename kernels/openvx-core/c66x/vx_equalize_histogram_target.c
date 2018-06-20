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
#include <tivx_kernel_equalize_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

#define SCRATCH_BUFFER_SIZE 1024

static tivx_target_kernel vx_equalize_histogram_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelEqualizeHistogramProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
    void *scratch;
    uint32_t scratch_size;

    if (num_params != TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *dst_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_EQUALIZE_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_EQUALIZE_HISTOGRAM_OUT_IMG_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        dst_target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_heap_region);

        tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

        tivxInitBufParams(src, &vxlib_src);
        tivxInitBufParams(dst, &vxlib_dst);

        status = tivxGetTargetKernelInstanceContext(kernel, &scratch, &scratch_size);

        if (VXLIB_SUCCESS == status)
        {
            status = VXLIB_equalizeHist_i8u_o8u(src_addr, &vxlib_src,
                    dst_addr, &vxlib_dst, (uint32_t*)scratch);
        }
        else
        {
            status = VX_FAILURE;
        }

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelEqualizeHistogramCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;

    if (num_params != TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {

        temp_ptr = tivxMemAlloc(SCRATCH_BUFFER_SIZE *
            sizeof(uint32_t), TIVX_MEM_EXTERNAL);

        if (NULL == temp_ptr)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            memset(temp_ptr, 0, SCRATCH_BUFFER_SIZE *
                sizeof(uint32_t));
            tivxSetTargetKernelInstanceContext(kernel, temp_ptr,
                             SCRATCH_BUFFER_SIZE * sizeof(uint32_t));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelEqualizeHistogramDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    void *temp_ptr;
    uint32_t temp_ptr_size;

    if (num_params != TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS; i ++)
        {
            if (NULL == obj_desc[i])
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {

        status = tivxGetTargetKernelInstanceContext(kernel, &temp_ptr, &temp_ptr_size);

        if (VXLIB_SUCCESS != status)
        {
            status = VX_ERROR_NO_MEMORY;
        }
        else
        {
            tivxMemFree(temp_ptr, temp_ptr_size, TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelEqualizeHistogramControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelEqualizeHistogram(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP1) || (self_cpu == TIVX_CPU_ID_DSP2))
    {
        if (self_cpu == TIVX_CPU_ID_DSP1)
        {
            strncpy(target_name, TIVX_TARGET_DSP1,
                TIVX_TARGET_MAX_NAME);
        }
        else
        {
            strncpy(target_name, TIVX_TARGET_DSP2,
                TIVX_TARGET_MAX_NAME);
        }

        vx_equalize_histogram_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_EQUALIZE_HISTOGRAM,
            target_name,
            tivxKernelEqualizeHistogramProcess,
            tivxKernelEqualizeHistogramCreate,
            tivxKernelEqualizeHistogramDelete,
            tivxKernelEqualizeHistogramControl,
            NULL);
    }
}


void tivxRemoveTargetKernelEqualizeHistogram(void)
{
    tivxRemoveTargetKernel(vx_equalize_histogram_target_kernel);
}
