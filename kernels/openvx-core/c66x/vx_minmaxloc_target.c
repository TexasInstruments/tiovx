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
#include <tivx_kernel_minmaxloc.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_mml_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelMmlProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src;
    vx_uint8 *src_addr;
    VXLIB_bufParams2D_t vxlib_src;
    tivx_obj_desc_scalar_t *sc[4U];
    tivx_obj_desc_array_t *arr[2U];
    uint32_t min_cnt = 0, max_cnt = 0, min_cap = 0, max_cap = 0;
    uint32_t *min_loc = NULL, *max_loc = NULL;

    if (num_params != TIVX_KERNEL_MML_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_MML_IN_IMG_IDX]) ||
            (NULL == obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_IDX]) ||
            (NULL == obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_IDX]))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *arr0_target_ptr;
        void *arr1_target_ptr;

        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_MML_IN_IMG_IDX];
        sc[0U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_IDX];
        sc[1U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_IDX];
        sc[2U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MIN_SC_C_IDX];
        sc[3U] = (tivx_obj_desc_scalar_t*)obj_desc[TIVX_KERNEL_MML_OUT_MAX_SC_C_IDX];
        arr[0U] = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_MML_OUT_MIN_ARR_IDX];
        arr[1U] = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_MML_OUT_MAX_ARR_IDX];

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        if (NULL != arr[0u])
        {
            arr0_target_ptr = tivxMemShared2TargetPtr(
                arr[0U]->mem_ptr.shared_ptr, arr[0U]->mem_ptr.mem_heap_region);
            tivxMemBufferMap(arr0_target_ptr, arr[0U]->mem_size,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

            min_loc = arr0_target_ptr;
            min_cap = arr[0U]->mem_size / arr[0u]->item_size;
        }

        if (NULL != arr[1u])
        {
            arr1_target_ptr = tivxMemShared2TargetPtr(
                arr[1U]->mem_ptr.shared_ptr, arr[1U]->mem_ptr.mem_heap_region);
            tivxMemBufferMap(arr1_target_ptr, arr[1U]->mem_size,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

            max_loc = arr1_target_ptr;
            max_cap = arr[1U]->mem_size / arr[1u]->item_size;
        }

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);
        tivxInitBufParams(src, &vxlib_src);

        if (VX_DF_IMAGE_U8 == src->format)
        {
            sc[0U]->data.u08 = 0xFF;
            sc[1U]->data.u08 = 0;
            status = VXLIB_minMaxLoc_i8u(src_addr, &vxlib_src,
                &sc[0U]->data.u08, &sc[1U]->data.u08,
                &min_cnt, &max_cnt,
                min_cap, max_cap,
                min_loc, max_loc, 0, 0);
        }
        else
        {
            sc[0U]->data.s16 = 0x7fff;
            sc[1U]->data.s16 = 0x8000;
            status = VXLIB_minMaxLoc_i16s((int16_t*)src_addr, &vxlib_src,
                &sc[0U]->data.s16, &sc[1U]->data.s16,
                &min_cnt, &max_cnt,
                min_cap, max_cap,
                min_loc, max_loc, 0, 0);
        }
        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }

        if (NULL != sc[2U])
        {
             sc[2U]->data.u32 = min_cnt;
        }
        if (NULL != sc[3U])
        {
             sc[3U]->data.u32 = max_cnt;
        }
        if (NULL != arr[0u])
        {
            if (min_cnt > min_cap)
            {
                arr[0u]->num_items = min_cap;
            }
            else
            {
                arr[0u]->num_items = min_cnt;
            }
        }
        if (NULL != arr[1u])
        {
            if (max_cnt > max_cap)
            {
                arr[1u]->num_items = max_cap;
            }
            else
            {
                arr[1u]->num_items = max_cnt;
            }
        }

        tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (NULL != arr[0u])
        {
            tivxMemBufferUnmap(arr0_target_ptr, arr[0U]->mem_size,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
        if (NULL != arr[1u])
        {
            tivxMemBufferUnmap(arr1_target_ptr, arr[1U]->mem_size,
                VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelMmlCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelMmlDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelMmlControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelMinMaxLoc(void)
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

        vx_mml_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_MINMAXLOC,
            target_name,
            tivxKernelMmlProcess,
            tivxKernelMmlCreate,
            tivxKernelMmlDelete,
            tivxKernelMmlControl,
            NULL);
    }
}


void tivxRemoveTargetKernelMinMaxLoc(void)
{
    tivxRemoveTargetKernel(vx_mml_target_kernel);
}
