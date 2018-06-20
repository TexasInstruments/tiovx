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
#include <tivx_kernel_harris_corners.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>

typedef struct
{
    /* Sobel parameters */
    uint8_t *sobel_x;
    uint8_t *sobel_y;
    uint32_t sobel_size;
    VXLIB_bufParams2D_t vxlib_sobx, vxlib_soby;

    /* Parameters for Harris Corner Score */
    vx_float32 *hcs_score;
    uint32_t hcs_score_size;

    vx_float32 *hcs_scratch;
    uint32_t hcs_scratch_size;
    VXLIB_bufParams2D_t vxlib_score;

    /* Parameters for Harris Corner Detect */
    vx_float32 *hcd_sprs;
    uint32_t hcd_sprs_size;

    uint32_t *hcd_corners;
    uint32_t hcd_corners_size;

    vx_float32 *hcd_strength;
    uint32_t hcd_strength_size;

    /* Parameters for NMS */
    uint32_t *nms_corners;
    uint32_t nms_corners_size;

    vx_float32 *nms_strength;
    uint32_t nms_strength_size;

    int8_t *nms_scratch;
    uint32_t nms_scratch_size;

    VXLIB_bufParams2D_t vxlib_src;

    vx_float32 rad;
} tivxHarrisCornersParams;

static vx_status tivxHarrisCCalcSobel(tivxHarrisCornersParams *prms,
    const uint8_t *src_addr, int32_t gs);
static vx_status tivxHarrisCCalcScore(tivxHarrisCornersParams *prms,
    vx_float32 sensitivity, int32_t gs, int32_t bs);
static vx_status tivxHarrisCCalcDetect(tivxHarrisCornersParams *prms,
    uint32_t *num_corners, vx_float32 threshold, int32_t gs, int32_t bs);
static void tivxHarrisCFreeMem(tivxHarrisCornersParams *prms);

static tivx_target_kernel vx_harris_corners_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelHarrisCProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    tivxHarrisCornersParams *prms = NULL;
    tivx_obj_desc_image_t *src;
    tivx_obj_desc_array_t *arr;
    uint8_t *src_addr;
    uint32_t size, num_corners, num_corners_hcd;
    tivx_obj_desc_scalar_t *sc_thr, *sc_gs, *sc_bs, *sc_sens, *sc_cnt;
    vx_keypoint_t *kp;
    vx_rectangle_t rect;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_HARRISC_IN_IMG_IDX];
        arr = (tivx_obj_desc_array_t *)obj_desc[TIVX_KERNEL_HARRISC_OUT_ARR_IDX];
        sc_thr = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_THR_IDX];
        sc_bs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_BS_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_GS_IDX];
        sc_sens = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_SENS_IDX];
        sc_cnt = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxHarrisCornersParams) != size))
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

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        tivxInitBufParams(src, &prms->vxlib_src);

        status = tivxHarrisCCalcSobel(prms, src_addr, sc_gs->data.s32);

        if (VXLIB_SUCCESS == status)
        {
            status = tivxHarrisCCalcScore(prms, sc_sens->data.f32,
                sc_gs->data.s32, sc_bs->data.s32);
        }
        if (VXLIB_SUCCESS == status)
        {
            status = tivxHarrisCCalcDetect(prms, &num_corners_hcd,
                sc_thr->data.f32, sc_gs->data.s32, sc_bs->data.s32);
        }
        if (VXLIB_SUCCESS == status)
        {
            if(prms->rad >= 2.0f)
            {
                status = VXLIB_harrisCornersNMS_i32f(prms->hcd_corners,
                    prms->hcd_strength, num_corners_hcd, prms->nms_corners,
                    prms->nms_strength, num_corners_hcd, &num_corners,
                    prms->nms_scratch, prms->nms_scratch_size, prms->rad, NULL);
            }
            else
            {
                num_corners = num_corners_hcd;
            }
        }

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
                kp->x = rect.start_x + (prms->nms_corners[i] & 0xFFFFu);
                kp->y = rect.start_y + ((prms->nms_corners[i] & 0xFFFF0000u) >>
                    16u);
                kp->strength = prms->nms_strength[i];
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

static vx_status VX_CALLBACK tivxKernelHarrisCCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t strideshift;
    tivx_obj_desc_image_t *img;
    tivxHarrisCornersParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_dist, *sc_gs, *sc_bs;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        img = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_IMG_IDX];
        sc_bs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_BS_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_GS_IDX];
        sc_dist = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_HARRISC_IN_SC_DIST_IDX];

        prms = tivxMemAlloc(sizeof(tivxHarrisCornersParams), TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxHarrisCornersParams));

            if (7 == sc_gs->data.s32)
            {
                strideshift = 2u;
            }
            else
            {
                strideshift = 1u;
            }

            prms->vxlib_src.dim_x = img->imagepatch_addr[0].dim_x;
            prms->vxlib_src.dim_y = img->imagepatch_addr[0].dim_y;
            prms->vxlib_src.stride_y = img->imagepatch_addr[0].stride_y;
            prms->vxlib_src.data_type = VXLIB_UINT8;

            prms->vxlib_sobx.dim_x = img->imagepatch_addr[0].dim_x;
            prms->vxlib_sobx.dim_y = img->imagepatch_addr[0].dim_y -
                (sc_gs->data.s32 - 1u);
            prms->vxlib_sobx.stride_y =
                (img->imagepatch_addr[0].stride_y * 2u) * strideshift;
            prms->vxlib_sobx.data_type = VXLIB_UINT8;

            prms->vxlib_soby.dim_x = img->imagepatch_addr[0].dim_x;
            prms->vxlib_soby.dim_y = img->imagepatch_addr[0].dim_y -
                (sc_gs->data.s32 - 1u);
            prms->vxlib_soby.stride_y =
                (img->imagepatch_addr[0].stride_y * 2u) * strideshift;
            prms->vxlib_soby.data_type = VXLIB_UINT8;

            prms->sobel_size = prms->vxlib_sobx.stride_y *
                img->imagepatch_addr[0].dim_y;

            prms->sobel_x = tivxMemAlloc(prms->sobel_size, TIVX_MEM_EXTERNAL);
            if (NULL == prms->sobel_x)
            {
                status = VX_ERROR_NO_MEMORY;
            }

            if (VX_SUCCESS == status)
            {
                prms->sobel_y = tivxMemAlloc(prms->sobel_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->sobel_y)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->vxlib_score.dim_x = img->imagepatch_addr[0].dim_x;
                prms->vxlib_score.dim_y = img->imagepatch_addr[0].dim_y -
                    (sc_gs->data.s32 - 1u) - (sc_bs->data.s32 - 1u);
                prms->vxlib_score.stride_y =
                    img->imagepatch_addr[0].stride_y * 4u;

                prms->hcs_score_size = prms->vxlib_score.stride_y *
                    img->imagepatch_addr[0].dim_y;

                prms->hcs_score = tivxMemAlloc(prms->hcs_score_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->hcs_score)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->hcs_scratch_size = 12u * prms->vxlib_sobx.dim_x *
                    (1 + sc_bs->data.s32);

                prms->hcs_scratch = tivxMemAlloc(prms->hcs_scratch_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->hcs_scratch)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->hcd_sprs_size = prms->vxlib_score.stride_y *
                    img->imagepatch_addr[0].dim_y;

                prms->hcd_sprs = tivxMemAlloc(prms->hcd_sprs_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->hcd_sprs)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->hcd_corners_size = img->imagepatch_addr[0].dim_x *
                    img->imagepatch_addr[0].dim_y * sizeof(uint32_t);

                prms->hcd_corners = tivxMemAlloc(prms->hcd_corners_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->hcd_corners)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                prms->hcd_strength_size = img->imagepatch_addr[0].dim_x *
                    img->imagepatch_addr[0].dim_y * sizeof(vx_float32);

                prms->hcd_strength = tivxMemAlloc(prms->hcd_strength_size,
                    TIVX_MEM_EXTERNAL);
                if (NULL == prms->hcd_strength)
                {
                    status = VX_ERROR_NO_MEMORY;
                }
            }

            if (VX_SUCCESS == status)
            {
                vx_float32 radius = sc_dist->data.f32;

                prms->rad = radius;
                if (prms->rad >= 2.0f)
                {
                    /* We need to run the NMS function, so allocate buffers */

                    prms->nms_scratch_size = (img->imagepatch_addr[0].dim_x *
                        img->imagepatch_addr[0].dim_y) +
                            ((((uintptr_t)prms->rad+1)*2)*
                            (((uintptr_t)prms->rad+1)*2)*2);

                    prms->nms_scratch = tivxMemAlloc(prms->nms_scratch_size,
                        TIVX_MEM_EXTERNAL);
                    if (NULL == prms->nms_scratch)
                    {
                        status = VX_ERROR_NO_MEMORY;
                    }

                    if (VX_SUCCESS == status)
                    {
                        prms->nms_corners_size = img->imagepatch_addr[0].dim_x *
                            img->imagepatch_addr[0].dim_y * sizeof(uint32_t);

                        prms->nms_corners = tivxMemAlloc(prms->nms_corners_size,
                            TIVX_MEM_EXTERNAL);
                        if (NULL == prms->nms_corners)
                        {
                            status = VX_ERROR_NO_MEMORY;
                        }
                    }

                    if (VX_SUCCESS == status)
                    {
                        prms->nms_strength_size = img->imagepatch_addr[0].dim_x *
                            img->imagepatch_addr[0].dim_y * sizeof(vx_float32);

                        prms->nms_strength = tivxMemAlloc(prms->nms_strength_size,
                            TIVX_MEM_EXTERNAL);
                        if (NULL == prms->nms_strength)
                        {
                            status = VX_ERROR_NO_MEMORY;
                        }
                    }
                }
                else
                {
                    /* We don't need to run the NMS operation since distance is within 3x3 neighborhood which "detect" already covers */
                    /* Set output pointers to be same as input pointers */

                    prms->nms_corners = prms->hcd_corners;
                    prms->nms_strength = prms->hcd_strength;
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
                sizeof(tivxHarrisCornersParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxHarrisCFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxHarrisCornersParams *prms = NULL;

    if (num_params != TIVX_KERNEL_HARRISC_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        for (i = 0U; i < TIVX_KERNEL_HARRISC_MAX_PARAMS; i ++)
        {
            if ((NULL == obj_desc[i]) &&
                (i != TIVX_KERNEL_HARRISC_OUT_SC_CNT_IDX))
            {
                status = VX_FAILURE;
                break;
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxHarrisCornersParams) == size))
        {
            tivxHarrisCFreeMem(prms);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelHarrisCControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelHarrisCorners(void)
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

        vx_harris_corners_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_HARRIS_CORNERS,
            target_name,
            tivxKernelHarrisCProcess,
            tivxKernelHarrisCCreate,
            tivxKernelHarrisCDelete,
            tivxKernelHarrisCControl,
            NULL);
    }
}


void tivxRemoveTargetKernelHarrisCorners(void)
{
    tivxRemoveTargetKernel(vx_harris_corners_target_kernel);
}

static void tivxHarrisCFreeMem(tivxHarrisCornersParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->sobel_x)
        {
            tivxMemFree(prms->sobel_x, prms->sobel_size, TIVX_MEM_EXTERNAL);
            prms->sobel_x = NULL;
        }
        if (NULL != prms->sobel_y)
        {
            tivxMemFree(prms->sobel_y, prms->sobel_size, TIVX_MEM_EXTERNAL);
            prms->sobel_y = NULL;
        }
        if (NULL != prms->hcs_score)
        {
            tivxMemFree(prms->hcs_score, prms->hcs_score_size,
                TIVX_MEM_EXTERNAL);
            prms->hcs_score = NULL;
        }
        if (NULL != prms->hcs_scratch)
        {
            tivxMemFree(prms->hcs_scratch, prms->hcs_scratch_size,
                TIVX_MEM_EXTERNAL);
            prms->hcs_scratch = NULL;
        }
        if (NULL != prms->hcd_sprs)
        {
            tivxMemFree(prms->hcd_sprs, prms->hcd_sprs_size, TIVX_MEM_EXTERNAL);
            prms->hcd_sprs = NULL;
        }
        if (NULL != prms->hcd_corners)
        {
            tivxMemFree(prms->hcd_corners, prms->hcd_corners_size,
                TIVX_MEM_EXTERNAL);
            prms->hcd_corners = NULL;
        }
        if (NULL != prms->hcd_strength)
        {
            tivxMemFree(prms->hcd_strength, prms->hcd_strength_size,
                TIVX_MEM_EXTERNAL);
            prms->hcd_strength = NULL;
        }
        if(prms->rad >= 2.0f)
        {
            if (NULL != prms->nms_corners)
            {
                tivxMemFree(prms->nms_corners, prms->nms_corners_size,
                    TIVX_MEM_EXTERNAL);
                prms->nms_corners = NULL;
            }
            if (NULL != prms->nms_strength)
            {
                tivxMemFree(prms->nms_strength, prms->nms_strength_size,
                    TIVX_MEM_EXTERNAL);
                prms->nms_strength = NULL;
            }
            if (NULL != prms->nms_scratch)
            {
                tivxMemFree(prms->nms_scratch, prms->nms_scratch_size,
                    TIVX_MEM_EXTERNAL);
                prms->nms_scratch = NULL;
            }
        }

        tivxMemFree(prms, sizeof(tivxHarrisCornersParams), TIVX_MEM_EXTERNAL);
    }
}

static vx_status tivxHarrisCCalcSobel(tivxHarrisCornersParams *prms,
    const uint8_t *src_addr, int32_t gs)
{
    vx_status status = VX_FAILURE;

    int16_t *temp1_16, *temp2_16;
    int32_t *temp1_32, *temp2_32;

    if (NULL != prms)
    {
        switch(gs)
        {
            case 3:
                temp1_16 = (int16_t *)(prms->sobel_x +
                    prms->vxlib_sobx.stride_y + 2u);

                temp2_16 = (int16_t *)(prms->sobel_y +
                    prms->vxlib_soby.stride_y + 2u);

                status = VXLIB_sobel_3x3_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            case 5:
                temp1_16 = (int16_t *)(prms->sobel_x +
                    (prms->vxlib_sobx.stride_y*2u) + 4u);

                temp2_16 = (int16_t *)(prms->sobel_y +
                    (prms->vxlib_soby.stride_y*2u) + 4u);

                status = VXLIB_sobel_5x5_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            default:
                temp1_32 = (int32_t *)(prms->sobel_x +
                    (prms->vxlib_sobx.stride_y*3u) + 12u);

                temp2_32 = (int32_t *)(prms->sobel_y +
                    (prms->vxlib_soby.stride_y*3u) + 12u);

                status = VXLIB_sobel_7x7_i8u_o32s_o32s(src_addr,
                    &prms->vxlib_src, temp1_32, &prms->vxlib_sobx,
                    temp2_32, &prms->vxlib_soby);
                break;
        }
    }

    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxHarrisCCalcScore(tivxHarrisCornersParams *prms,
    vx_float32 sensitivity, int32_t gs, int32_t bs)
{
    vx_status status = VX_FAILURE;
    vx_float32 *score;
    int32_t *sobx_32, *soby_32;
    int16_t *sobx_16, *soby_16;

    if (NULL != prms)
    {
        if (7 == gs)
        {
            sobx_32 = (int32_t*)(prms->sobel_x + (prms->vxlib_sobx.stride_y *
                (gs / 2)) + ((gs / 2) * 4));
            soby_32 = (int32_t*)(prms->sobel_y + (prms->vxlib_soby.stride_y *
                (gs / 2)) + ((gs / 2) * 4));
            score = (vx_float32*)(prms->hcs_score +
                ((prms->vxlib_score.stride_y / 4) * ((gs / 2) + (bs / 2))) +
                ((gs / 2) + (bs / 2)));

            status = VXLIB_harrisCornersScore_i32s_i32s_o32f(
                sobx_32, &prms->vxlib_sobx, soby_32, &prms->vxlib_soby,
                score, &prms->vxlib_score,
                prms->hcs_scratch, sensitivity, gs, bs);
        }
        else
        {
            sobx_16 = (int16_t*)(prms->sobel_x +
                (prms->vxlib_sobx.stride_y * (gs / 2)) +
                ((gs / 2) * 2));
            soby_16 = (int16_t*)(prms->sobel_y +
                (prms->vxlib_soby.stride_y * (gs / 2)) +
                ((gs / 2) * 2));
            score = (vx_float32*)(prms->hcs_score +
                ((prms->vxlib_score.stride_y / 4) * ((gs / 2) + (bs / 2))) +
                ((gs / 2) + (bs / 2)));

            status = VXLIB_harrisCornersScore_i16s_i16s_o32f(
                sobx_16, &prms->vxlib_sobx, soby_16, &prms->vxlib_soby,
                score, &prms->vxlib_score, prms->hcs_scratch, sensitivity, gs, bs);
        }
    }
    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}

static vx_status tivxHarrisCCalcDetect(tivxHarrisCornersParams *prms,
    uint32_t *num_corners, vx_float32 threshold, int32_t gs, int32_t bs)
{
    vx_status status = VX_FAILURE;
    VXLIB_F32 *score_out;
    VXLIB_bufParams2D_t score_prms;

    if (NULL != prms)
    {
        score_prms.dim_x = prms->vxlib_score.dim_x - (gs - 1) - (bs - 1);
        score_prms.dim_y = prms->vxlib_score.dim_y;
        score_prms.stride_y = prms->vxlib_score.stride_y;
        score_prms.data_type = VXLIB_FLOAT32;

        score_out = (VXLIB_F32*)(prms->hcs_score +
            ((score_prms.stride_y / 4) * ((gs / 2) + (bs / 2))) +
            ((gs / 2) + (bs / 2)));

        status = VXLIB_harrisCornersDetect_i32f(score_out, &score_prms,
            prms->hcd_corners, prms->hcd_strength,
            prms->vxlib_src.dim_x * prms->vxlib_src.dim_y,
            num_corners, threshold, (gs/2) + (bs/2), (gs/2) + (bs/2));
    }
    if (status != VXLIB_SUCCESS)
    {
        status = VX_FAILURE;
    }

    return (status);
}
