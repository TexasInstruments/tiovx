/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <tivx_openvx_core_kernels.h>
#include <tivx_target_kernels_priv.h>
#include <tivx_kernel_intgimg.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_target_kernels_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    VXLIB_bufParams2D_t vxlib_src, vxlib_dst;
} tivxIntgImgParams;

static tivx_target_kernel vx_intgimg_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelIntgImgProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelIntgImgCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelIntgImgDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelIntgImgControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelIntgImgProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxIntgImgParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dst;
    uint32_t size;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_INTGIMG_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxIntgImgParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[2];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);
        dst->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst->mem_ptr[0].shared_ptr, dst->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferMap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);

        ownInitBufParams(src, &dst->valid_roi, &prms->vxlib_src,
            (uint8_t **)&img_ptrs[0], 1, 1, 1, 1);
        ownInitBufParams(dst, NULL, &prms->vxlib_dst,
            (uint8_t **)&img_ptrs[1], 0, 0, 0, 0);

        tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);
        tivxMemBufferUnmap(dst->mem_ptr[0].target_ptr, dst->mem_size[0],
            dst->mem_ptr[0].mem_type, VX_WRITE_ONLY);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelIntgImgCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dst;
    uint8_t *addr;
    tivxIntgImgParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_INTGIMG_IN_IMG_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_INTGIMG_OUT_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxIntgImgParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            BAM_VXLIB_integralImage_i8u_o32u_params kernel_params;
            VXLIB_bufParams2D_t *buf_params[2];

            memset(prms, 0, sizeof(tivxIntgImgParams));

            ownInitBufParams(src, NULL, &prms->vxlib_src, &addr, 0, 0, 0, 0);
            ownInitBufParams(dst, NULL, &prms->vxlib_dst, &addr, 0, 0, 0, 0);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &prms->vxlib_src;
            buf_params[1] = &prms->vxlib_dst;

            kernel_params.frameWidth   = prms->vxlib_dst.dim_x;
            kernel_params.frameHeight  = prms->vxlib_dst.dim_y;

            kernel_details.compute_kernel_params = (void*)&kernel_params;

            BAM_VXLIB_integralImage_i8u_o32u_getKernelInfo( &kernel_params,
                                                        &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_INTEGRALIMAGE_I8U_O32U,
                                                   buf_params, &kernel_details,
                                                   &prms->graph_handle);
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxIntgImgParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxIntgImgParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelIntgImgDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxIntgImgParams *prms = NULL;

    status = ownCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_INTGIMG_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxIntgImgParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxIntgImgParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelIntgImgControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamIntegralImage(void)
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

        vx_intgimg_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_INTEGRAL_IMAGE,
            target_name,
            tivxKernelIntgImgProcess,
            tivxKernelIntgImgCreate,
            tivxKernelIntgImgDelete,
            tivxKernelIntgImgControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamIntegralImage(void)
{
    tivxRemoveTargetKernel(vx_intgimg_target_kernel);
}
