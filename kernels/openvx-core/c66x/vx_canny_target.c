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
#include <tivx_kernel_canny.h>
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


    /* Parameters for Norm */
    uint8_t *norm;
    uint32_t norm_size;
    VXLIB_bufParams2D_t vxlib_norm;

    /* Parameters for canny NMS */
    uint8_t *nms_edge;
    uint32_t nms_edge_size;
    VXLIB_bufParams2D_t vxlib_edge;

    /* Parameters for threshold */
    uint8_t *nms_edge2;
    uint32_t nms_edge_size2;
    VXLIB_bufParams2D_t vxlib_edge2;

    uint32_t *edge_list;
    uint32_t edge_list_size;
    uint32_t gs;

    VXLIB_bufParams2D_t vxlib_src;
    VXLIB_bufParams2D_t vxlib_dst;
} tivxCannyParams;

static vx_status tivxCannyCalcSobel(const tivxCannyParams *prms,
    const uint8_t *src_addr, int32_t gs);
static vx_status tivxCannyCalcNorm(const tivxCannyParams *prms, vx_enum norm,
    int32_t gs);
static vx_status tivxCannyCalcNms(const tivxCannyParams *prms, int32_t gs);
static vx_status tivxCannyCalcDblThr(tivxCannyParams *prms,
    uint32_t *num_items, int32_t lower, int32_t upper, int32_t gs);
static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelCannyDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);
static void tivxCannyFreeMem(tivxCannyParams *prms);

static tivx_target_kernel vx_harris_corners_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelCannyProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    tivx_obj_desc_threshold_t *thr;
    uint8_t *src_addr, *dst_addr;
    uint8_t *border_addr_tl, *border_addr_tr, *border_addr_bl;
    uint32_t i;
    tivx_obj_desc_scalar_t *sc_gs, *sc_norm;
    vx_rectangle_t rect;
    uint32_t size, num_dbl_thr_items = 0, num_edge_trace_out = 0;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_CANNY_OUTPUT_IDX];
        thr = (tivx_obj_desc_threshold_t *)obj_desc[
            TIVX_KERNEL_CANNY_HYST_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_GRADIENT_SIZE_IDX];
        sc_norm = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_NORM_TYPE_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCannyParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *src_target_ptr;
        void *dst_target_ptr;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);
        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);

        tivxCheckStatus(&status, tivxMemBufferMap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        rect = dst->valid_roi;

        dst_addr = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));

        prms->gs = (uint32_t)sc_gs->data.s32;

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        border_addr_tl = (uint8_t *)((uintptr_t)prms->nms_edge +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_tr = (uint8_t *)((uintptr_t)prms->nms_edge +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_x, rect.start_y + (prms->gs / 2U),
            &dst->imagepatch_addr[0U]));
        border_addr_bl = (uint8_t *)((uintptr_t)prms->nms_edge +
            tivxComputePatchOffset(rect.start_x + (prms->gs / 2U), rect.start_y + (prms->gs / 2U) + 1U + prms->vxlib_dst.dim_y,
            &dst->imagepatch_addr[0U]));

        tivxInitBufParams(src, &prms->vxlib_src);
        tivxInitBufParams(dst, &prms->vxlib_dst);

        status = tivxCannyCalcSobel(prms, src_addr, sc_gs->data.s32);

        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcNorm(prms, sc_norm->data.enm,
                sc_gs->data.s32);
        }
        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcNms(prms, sc_gs->data.s32);
        }
        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = tivxCannyCalcDblThr(prms, &num_dbl_thr_items, thr->lower,
                thr->upper, sc_gs->data.s32);
        }

        /* Edge Tracing requires 1 pixel border of zeros */
        memset(border_addr_tl, 0, (size_t)prms->vxlib_dst.dim_x + (size_t)2);
        memset(border_addr_bl, 0, (size_t)prms->vxlib_dst.dim_x + (size_t)2);
        for(i=0; i<(prms->vxlib_dst.dim_y+2U); i++)
        {
            border_addr_tl[(int32_t)i*prms->vxlib_dst.stride_y] = 0;
            border_addr_tr[(int32_t)i*prms->vxlib_dst.stride_y] = 0;
        }

        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_edgeTracing_i8u(prms->nms_edge, &prms->vxlib_edge,
                prms->edge_list, prms->edge_list_size, num_dbl_thr_items,
                &num_edge_trace_out);
        }
        if ((vx_status)VXLIB_SUCCESS == status)
        {
            status = (vx_status)VXLIB_thresholdBinary_i8u_o8u(prms->nms_edge2,
                &prms->vxlib_edge2, dst_addr, &prms->vxlib_dst, 128, 255, 0);
        }

        if (status != (vx_status)VXLIB_SUCCESS)
        {
            status = (vx_status)VX_FAILURE;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(src_target_ptr, src->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(dst_target_ptr, dst->mem_size[0],
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivxCannyParams *prms = NULL;
    tivx_obj_desc_scalar_t *sc_gs;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CANNY_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_CANNY_OUTPUT_IDX];
        sc_gs = (tivx_obj_desc_scalar_t *)obj_desc[
            TIVX_KERNEL_CANNY_GRADIENT_SIZE_IDX];

        prms = tivxMemAlloc(sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);
        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxCannyParams));

            tivxInitBufParams(src, &prms->vxlib_src);

            tivxInitBufParams(dst, &prms->vxlib_dst);

            prms->vxlib_sobx.dim_x = prms->vxlib_src.dim_x;
            prms->vxlib_sobx.dim_y = prms->vxlib_src.dim_y -
                ((uint32_t)sc_gs->data.s32 - 1U);
            prms->vxlib_sobx.stride_y = (prms->vxlib_src.stride_y * 2);
            prms->vxlib_sobx.data_type = (uint32_t)VXLIB_UINT16;

            prms->vxlib_soby.dim_x = prms->vxlib_src.dim_x;
            prms->vxlib_soby.dim_y = prms->vxlib_src.dim_y -
                ((uint32_t)sc_gs->data.s32 - 1U);
            prms->vxlib_soby.stride_y = (prms->vxlib_src.stride_y * 2);
            prms->vxlib_soby.data_type = (uint32_t)VXLIB_UINT16;

            prms->sobel_size = (uint32_t)prms->vxlib_sobx.stride_y *
                prms->vxlib_src.dim_y;

            prms->sobel_x = tivxMemAlloc(prms->sobel_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            if (NULL == prms->sobel_x)
            {
                VX_PRINT(VX_ZONE_ERROR, "sobel_x mem allocation failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->sobel_y = tivxMemAlloc(prms->sobel_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->sobel_y)
                {
                    VX_PRINT(VX_ZONE_ERROR, "sobel_y mem allocation failed\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->vxlib_norm.dim_x = prms->vxlib_src.dim_x;
                prms->vxlib_norm.dim_y = prms->vxlib_src.dim_y -
                    ((uint32_t)sc_gs->data.s32 - 1U);
                prms->vxlib_norm.stride_y = (prms->vxlib_src.stride_y *
                    2);
                prms->vxlib_norm.data_type = (uint32_t)VXLIB_UINT16;

                prms->norm_size = (uint32_t)prms->vxlib_norm.stride_y *
                    prms->vxlib_src.dim_y;

                prms->norm = tivxMemAlloc(prms->norm_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->norm)
                {
                    VX_PRINT(VX_ZONE_ERROR, "norm mem allocation failed\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                vx_rectangle_t rect;

                prms->vxlib_edge.dim_x = prms->vxlib_src.dim_x;
                prms->vxlib_edge.dim_y = prms->vxlib_src.dim_y -
                    ((uint32_t)sc_gs->data.s32 - 1U) - 2U;
                prms->vxlib_edge.stride_y = prms->vxlib_src.stride_y;
                prms->vxlib_edge.data_type = (uint32_t)VXLIB_UINT8;

                prms->nms_edge_size = (uint32_t)prms->vxlib_edge.stride_y *
                    prms->vxlib_src.dim_y;

                prms->nms_edge = tivxMemAlloc(prms->nms_edge_size,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->nms_edge)
                {
                    VX_PRINT(VX_ZONE_ERROR, "nms_edge mem allocation failed\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }

                prms->vxlib_edge2.dim_x = prms->vxlib_dst.dim_x;
                prms->vxlib_edge2.dim_y = prms->vxlib_dst.dim_y;
                prms->vxlib_edge2.stride_y = prms->vxlib_edge.stride_y;
                prms->vxlib_edge2.data_type = (uint32_t)VXLIB_UINT8;

                rect = dst->valid_roi;

                prms->nms_edge2 = (uint8_t *)((uintptr_t)prms->nms_edge +
                    tivxComputePatchOffset(rect.start_x, rect.start_y,
                    &src->imagepatch_addr[0U]));
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                prms->edge_list_size = prms->vxlib_dst.dim_x * prms->vxlib_dst.dim_y;

                prms->edge_list = tivxMemAlloc(prms->edge_list_size * 4u,
                    (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
                if (NULL == prms->edge_list)
                {
                    VX_PRINT(VX_ZONE_ERROR, "edge_list mem allocation failed\n");
                    status = (vx_status)VX_ERROR_NO_MEMORY;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "struct mem allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxCannyParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxCannyFreeMem(prms);
            }
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelCannyDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxCannyParams *prms = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_CANNY_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxCannyParams) == size))
        {
            tivxCannyFreeMem(prms);
        }
    }

    return (status);
}

void tivxAddTargetKernelCannyEd(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_harris_corners_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_CANNY_EDGE_DETECTOR,
            target_name,
            tivxKernelCannyProcess,
            tivxKernelCannyCreate,
            tivxKernelCannyDelete,
            NULL,
            NULL);
    }
}


void tivxRemoveTargetKernelCannyEd(void)
{
    tivxRemoveTargetKernel(vx_harris_corners_target_kernel);
}

static void tivxCannyFreeMem(tivxCannyParams *prms)
{
    if (NULL != prms)
    {
        if (NULL != prms->sobel_x)
        {
            tivxMemFree(prms->sobel_x, prms->sobel_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->sobel_x = NULL;
        }
        if (NULL != prms->sobel_y)
        {
            tivxMemFree(prms->sobel_y, prms->sobel_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->sobel_y = NULL;
        }
        if (NULL != prms->norm)
        {
            tivxMemFree(prms->norm, prms->norm_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->norm = NULL;
        }
        if (NULL != prms->nms_edge)
        {
            tivxMemFree(prms->nms_edge, prms->nms_edge_size, (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->nms_edge = NULL;
        }
        if (NULL != prms->edge_list)
        {
            tivxMemFree(prms->edge_list, prms->edge_list_size * 4u,
                (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
            prms->edge_list = NULL;
        }

        tivxMemFree(prms, sizeof(tivxCannyParams), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static vx_status tivxCannyCalcSobel(const tivxCannyParams *prms,
    const uint8_t *src_addr, int32_t gs)
{
    vx_status status = (vx_status)VX_FAILURE;

    int16_t *temp1_16, *temp2_16;

    if (NULL != prms)
    {
        switch(gs)
        {
            case 3:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    (uint32_t)prms->vxlib_sobx.stride_y + 2U);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    (uint32_t)prms->vxlib_soby.stride_y + 2U);

                status = (vx_status)VXLIB_sobel_3x3_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            case 5:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    ((uint32_t)prms->vxlib_sobx.stride_y*2U) + 4U);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    ((uint32_t)prms->vxlib_soby.stride_y*2U) + 4U);

                status = (vx_status)VXLIB_sobel_5x5_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
            case 7:
            default:
                temp1_16 = (int16_t *)((uintptr_t)prms->sobel_x +
                    ((uint32_t)prms->vxlib_sobx.stride_y*3U) + 6U);

                temp2_16 = (int16_t *)((uintptr_t)prms->sobel_y +
                    ((uint32_t)prms->vxlib_soby.stride_y*3U) + 6U);

                status = (vx_status)VXLIB_sobel_7x7_i8u_o16s_o16s(src_addr,
                    &prms->vxlib_src, temp1_16, &prms->vxlib_sobx,
                    temp2_16, &prms->vxlib_soby);
                break;
        }
    }

    if (status != (vx_status)VXLIB_SUCCESS)
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcNorm(const tivxCannyParams *prms, vx_enum norm_enm,
    int32_t gs)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint16_t *norm;
    int16_t *sobx, *soby;

    if (NULL != prms)
    {
        sobx = (int16_t *)((uintptr_t)prms->sobel_x + ((uint32_t)prms->vxlib_sobx.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));
        soby = (int16_t *)((uintptr_t)prms->sobel_y + ((uint32_t)prms->vxlib_soby.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));
        norm = (uint16_t *)((uintptr_t)prms->norm + ((uint32_t)prms->vxlib_norm.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));

        if ((vx_enum)VX_NORM_L1 == norm_enm)
        {
            status = (vx_status)VXLIB_normL1_i16s_i16s_o16u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm);
        }
        else
        {
            status = (vx_status)VXLIB_normL2_i16s_i16s_o16u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm);
        }
    }

    if (status != (vx_status)VXLIB_SUCCESS)
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcNms(const tivxCannyParams *prms, int32_t gs)
{
    vx_status status = (vx_status)VX_FAILURE;
    int16_t *sobx, *soby;
    uint16_t *norm;
    uint8_t *edge;

    if (NULL != prms)
    {
        sobx = (int16_t *)((uintptr_t)prms->sobel_x + ((uint32_t)prms->vxlib_sobx.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));
        soby = (int16_t *)((uintptr_t)prms->sobel_y + ((uint32_t)prms->vxlib_soby.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));
        norm = (uint16_t *)((uintptr_t)prms->norm + ((uint32_t)prms->vxlib_norm.stride_y *
            ((uint32_t)gs / 2U)) + (((uint32_t)gs / 2U) * 2U));
        edge = (uint8_t *) ((uintptr_t)prms->nms_edge + ((uint32_t)prms->vxlib_edge.stride_y *
            (((uint32_t)gs / 2U) + 1U)) + ((uint32_t)gs / 2U) + 1U);

        status = (vx_status)VXLIB_cannyNMS_i16s_i16s_i16u_o8u(sobx, &prms->vxlib_sobx,
                soby, &prms->vxlib_soby, norm, &prms->vxlib_norm, edge,
                &prms->vxlib_edge);
    }
    if (status != (vx_status)VXLIB_SUCCESS)
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxCannyCalcDblThr(tivxCannyParams *prms,
    uint32_t *num_items, int32_t lower, int32_t upper, int32_t gs)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint32_t start_pos;
    uint16_t *norm;
    uint8_t *edge;
    VXLIB_bufParams2D_t vxlib_prms;

    if (NULL != prms)
    {
        start_pos = ((uint32_t)prms->vxlib_edge.stride_y * (((uint32_t)gs / 2U) + 1U)) +
            (((uint32_t)gs / 2U) +1U);
        norm = (uint16_t*)((uintptr_t)prms->norm + ((uint32_t)prms->vxlib_norm.stride_y *
            (((uint32_t)gs/2U)+1U)) + ((((uint32_t)gs/2U)+1U)*2U));
        edge = (uint8_t*)((uintptr_t)prms->nms_edge + start_pos);

        vxlib_prms.dim_x = prms->vxlib_edge.dim_x - (uint32_t)gs;
        vxlib_prms.dim_y = prms->vxlib_edge.dim_y;
        vxlib_prms.stride_y = prms->vxlib_edge.stride_y;
        vxlib_prms.data_type = prms->vxlib_edge.data_type;

        status = (vx_status)VXLIB_doubleThreshold_i16u_i8u(norm, &prms->vxlib_norm,
            edge, &vxlib_prms, (uint16_t)prms->vxlib_edge.stride_y, prms->edge_list,
            prms->edge_list_size, num_items, start_pos, (uint32_t)lower, (uint32_t)upper);
    }
    if (status != (vx_status)VXLIB_SUCCESS)
    {
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}
