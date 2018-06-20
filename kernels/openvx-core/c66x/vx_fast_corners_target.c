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
#include <tivx_kernel_fast_corners.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

static tivx_target_kernel vx_fast_corners_target_kernel = NULL;

typedef struct
{
    uint32_t *corners;
    uint8_t  *strength;
    uint8_t  *scratch;
    uint32_t scratch_size;
} tivxFastCornersParams;

static vx_status VX_CALLBACK tivxKernelFastCProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxFastCornersParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_array_t *arr;
    uint8_t *src_addr;
    VXLIB_bufParams2D_t vxlib_src;
    uint32_t size, num_corners;
    tivx_obj_desc_scalar_t *sc_thr, *sc_nms, *sc_cnt;
    vx_keypoint_t *kp;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_FASTC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_FASTC_OUT_ARR_IDX];
        sc_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_SC_THR_IDX];
        sc_nms = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_NMS_IDX];
        sc_cnt = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxFastCornersParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *arr_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_heap_region);
        arr_target_ptr = tivxMemShared2TargetPtr(
            arr->mem_ptr.shared_ptr, arr->mem_ptr.mem_heap_region);

        tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferMap(arr_target_ptr, arr->mem_size,
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        tivxInitBufParams(src, &vxlib_src);
        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        status = VXLIB_fastCorners_i8u(
            src_addr, &vxlib_src, prms->corners, prms->strength,
            arr->capacity, (uint8_t)sc_thr->data.f32, &num_corners,
            sc_nms->data.boolean, prms->scratch, prms->scratch_size);

        if (status != VXLIB_SUCCESS)
        {
            status = VX_FAILURE;
        }
        else
        {
            if (NULL != sc_cnt)
            {
                sc_cnt->data.u32 = num_corners;
            }

            /* Copy array and other parameters */
            if (num_corners > arr->capacity)
            {
                num_corners = arr->capacity;
            }
            arr->num_items = num_corners;

            kp = (vx_keypoint_t *)arr_target_ptr;
            for (i = 0; i < num_corners; i ++)
            {
                kp->x = prms->corners[i] & 0xFFFFu;
                kp->y = (prms->corners[i] & 0xFFFF0000u) >> 16u;
                kp->strength = prms->strength[i];
                kp->scale = 0.0f;
                kp->orientation = 0.0f;
                kp->tracking_status = 1;
                kp->error = 0.0f;

                kp ++;
            }
        }

        tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(arr_target_ptr, arr->mem_size,
            VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_image_t *img;
    tivx_obj_desc_array_t *arr;
    tivxFastCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_FASTC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_ARR_IDX];

        prms = tivxMemAlloc(sizeof(tivxFastCornersParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxFastCornersParams));

            prms->corners = tivxMemAlloc(arr->capacity * 4U, TIVX_MEM_EXTERNAL);
            if (NULL == prms->corners)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->strength = tivxMemAlloc(arr->capacity, TIVX_MEM_EXTERNAL);
                if (NULL == prms->strength)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                if (((img->imagepatch_addr[0].dim_x*4u) + arr->capacity + 30u) >
                        512u)
                {
                    size = img->imagepatch_addr[0].dim_x*4u +
                        arr->capacity + 30u;
                }
                else
                {
                    size = 512u;
                }

                prms->scratch_size = size;

                prms->scratch = tivxMemAlloc(size, TIVX_MEM_EXTERNAL);
                if (NULL == prms->scratch)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxFastCornersParams));
        }
        else
        {
            if (NULL != prms)
            {
                if (prms->corners)
                {
                    tivxMemFree(prms->corners, arr->capacity * 4U,
                        TIVX_MEM_EXTERNAL);
                    prms->corners = NULL;
                }
                if (prms->strength)
                {
                    tivxMemFree(prms->strength, arr->capacity,
                        TIVX_MEM_EXTERNAL);
                    prms->strength = NULL;
                }
                if (prms->scratch)
                {
                    tivxMemFree(prms->scratch, prms->scratch_size,
                        TIVX_MEM_EXTERNAL);
                    prms->scratch = NULL;
                }

                tivxMemFree(prms, sizeof(tivxFastCornersParams),
                    TIVX_MEM_EXTERNAL);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivx_obj_desc_array_t *arr;
    tivxFastCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_FASTC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_FASTC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_FASTC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        arr = (tivx_obj_desc_array_t *)obj_desc[
            TIVX_KERNEL_FASTC_OUT_ARR_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxFastCornersParams) == size))
        {
            if (prms->corners)
            {
                tivxMemFree(prms->corners, arr->capacity * 4U,
                    TIVX_MEM_EXTERNAL);
                prms->corners = NULL;
            }
            if (prms->strength)
            {
                tivxMemFree(prms->strength, arr->capacity,
                    TIVX_MEM_EXTERNAL);
                prms->strength = NULL;
            }
            if (prms->scratch)
            {
                tivxMemFree(prms->scratch, prms->scratch_size,
                    TIVX_MEM_EXTERNAL);
                prms->scratch = NULL;
                prms->scratch_size = 0u;
            }

            tivxMemFree(prms, size, TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelFastCControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelFastCorners(void)
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

        vx_fast_corners_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_FAST_CORNERS,
            target_name,
            tivxKernelFastCProcess,
            tivxKernelFastCCreate,
            tivxKernelFastCDelete,
            tivxKernelFastCControl,
            NULL);
    }
}


void tivxRemoveTargetKernelFastCorners(void)
{
    tivxRemoveTargetKernel(vx_fast_corners_target_kernel);
}
