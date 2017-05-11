/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_equalize_histogram.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

#define SCRATCH_BUFFER_SIZE         (1024)

typedef struct
{
    tivx_bam_graph_handle hist_graph_handle;
    tivx_bam_graph_handle lut_graph_handle;

    uint32_t *scratch;
    uint32_t  scratch_size;

    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
} tivxEqHistParams;

static tivx_target_kernel vx_eq_hist_target_kernel = NULL;

static vx_status VX_CALLBACK tivxBamKernelEqHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxBamKernelEqHistProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxEqHistParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    vx_uint8 *src_addr, *dst_addr;
    vx_rectangle_t rect;
    uint32_t size;
    BAM_VXLIB_histogramSimple_i8u_o32u_params hist_params;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxEqHistParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];

        src->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0U].shared_ptr, src->mem_ptr[0U].mem_type);
        dst->mem_ptr[0U].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0U].shared_ptr, dst->mem_ptr[0U].mem_type);

        tivxMemBufferMap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);

        /* Get the correct offset of the images from the valid roi parameter,
           Assuming valid Roi is same for src0 and src1 images */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        rect = dst->valid_roi;
        dst_addr = (uint8_t *)((uintptr_t)dst->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0]));

        img_ptrs[0] = src_addr;
        img_ptrs[1] = dst_addr;

        /* Update the pointers in histogram and Lut kernels */
        tivxBamUpdatePointers(prms->hist_graph_handle, 1U, 0U, img_ptrs);
        tivxBamUpdatePointers(prms->lut_graph_handle, 1U, 1U, img_ptrs);

        hist_params.dist = prms->scratch;
        hist_params.minValue = 0xffffffffu;
        tivxBamControlNode(prms->hist_graph_handle, 0,
                           VXLIB_HISTOGRAMSIMPLE_I8U_O32U_CMD_SET_PARAMS,
                           &hist_params);
        status  = tivxBamProcessGraph(prms->hist_graph_handle);
        tivxBamControlNode(prms->hist_graph_handle, 0,
                           VXLIB_HISTOGRAMSIMPLE_I8U_O32U_CMD_GET_PARAMS,
                           &hist_params);

        status = VXLIB_histogramCdfLut_i32u_o8u(prms->scratch, NULL,
            (uint8_t*)prms->scratch,
            prms->vxlib_src.dim_x * prms->vxlib_src.dim_y,
            hist_params.minValue);

        status  = tivxBamProcessGraph(prms->lut_graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0U].target_ptr, src->mem_size[0],
            src->mem_ptr[0U].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0U].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0U].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelEqHistCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    tivxEqHistParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;
    BAM_VXLIB_tableLookup_i8u_o8u_params lut_kernel_params;
    BAM_VXLIB_histogramSimple_i8u_o32u_params hist_kernel_params;
    VXLIB_bufParams2D_t *buf_params[2];
    uint32_t lut_kern_create = 0, hist_kern_create = 0;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_EQUALIZE_HISTOGRAM_OUT_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxEqHistParams));

            prms->scratch = tivxMemAlloc(SCRATCH_BUFFER_SIZE *
                sizeof(uint32_t), TIVX_MEM_EXTERNAL);

            if (NULL == prms->scratch)
            {
                status = VX_ERROR_NO_MEMORY;
                tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
                prms = NULL;
            }
            else
            {
                memset(prms->scratch, 0, SCRATCH_BUFFER_SIZE *
                    sizeof(uint32_t));
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            prms->vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            prms->vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            prms->vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            prms->vxlib_src.data_type = VXLIB_UINT8;

            prms->vxlib_dst.dim_x = dst->imagepatch_addr[0].dim_x;
            prms->vxlib_dst.dim_y = dst->imagepatch_addr[0].dim_y;
            prms->vxlib_dst.stride_y = dst->imagepatch_addr[0].stride_y;
            prms->vxlib_dst.data_type = VXLIB_UINT8;

            buf_params[0] = &prms->vxlib_src;
            buf_params[1] = &prms->vxlib_dst;
        }

        if (VX_SUCCESS == status)
        {
            lut_kernel_params.lut    = (uint8_t *)prms->scratch;
            lut_kernel_params.count  = 256U;

            kernel_details.compute_kernel_params = (void*)&lut_kernel_params;

            BAM_VXLIB_tableLookup_i8u_o8u_getKernelInfo(
                &lut_kernel_params, &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_TABLELOOKUP_I8U_O8U,
                buf_params, &kernel_details, &prms->lut_graph_handle);

            if (VX_SUCCESS == status)
            {
                lut_kern_create = 1;
            }
        }
        if (VX_SUCCESS == status)
        {
            hist_kernel_params.dist    = prms->scratch;
            hist_kernel_params.minValue  = 0xffffffffu;

            kernel_details.compute_kernel_params = (void*)&hist_kernel_params;

            BAM_VXLIB_histogramSimple_i8u_o32u_getKernelInfo(
                &hist_kernel_params, &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_HISTOGRAMSIMPLE_I8U_O32U,
                buf_params, &kernel_details,
                &prms->hist_graph_handle);
            if (VX_SUCCESS == status)
            {
                hist_kern_create = 1;
            }
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxEqHistParams));
        }
        else
        {
            if (lut_kern_create)
            {
                tivxBamDestroyHandle(prms->lut_graph_handle);
            }
            if (hist_kern_create)
            {
                tivxBamDestroyHandle(prms->hist_graph_handle);
            }
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxBamKernelEqHistDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxEqHistParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_EQUALIZE_HISTOGRAM_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxEqHistParams) == size))
        {
            tivxBamDestroyHandle(prms->hist_graph_handle);
            tivxBamDestroyHandle(prms->lut_graph_handle);
            tivxMemFree(prms->scratch, SCRATCH_BUFFER_SIZE *
                    sizeof(uint32_t), TIVX_MEM_EXTERNAL);
            tivxMemFree(prms, sizeof(tivxEqHistParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxBamKernelEqHistControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamEqHist(void)
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

        vx_eq_hist_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_EQUALIZE_HISTOGRAM,
            target_name,
            tivxBamKernelEqHistProcess,
            tivxBamKernelEqHistCreate,
            tivxBamKernelEqHistDelete,
            tivxBamKernelEqHistControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamEqHist(void)
{
    tivxRemoveTargetKernel(vx_eq_hist_target_kernel);
}
