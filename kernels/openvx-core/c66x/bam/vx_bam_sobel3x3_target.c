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
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernel_utils.h>
#include <vx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
} tivxSobelParams;

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

static vx_status VX_CALLBACK tivxKernelSobelControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelSobelProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivxSobelParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dstx, *dsty;
    uint8_t *src_addr;
    int16_t *dstx_addr, *dsty_addr;
    vx_rectangle_t rect;
    uint32_t size;

    if (num_params != TIVX_KERNEL_SOBEL_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Check for NULL */
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX];
        dstx = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
        dsty = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxSobelParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        void *img_ptrs[3];

        src->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src->mem_ptr[0].shared_ptr, src->mem_ptr[0].mem_type);

        tivxMemBufferMap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);

        /* Get the correct offset of the images from the valid roi parameter */
        rect = src->valid_roi;

        src_addr = (uint8_t *)((uintptr_t)src->mem_ptr[0U].target_ptr +
            ownComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        if(dstx != NULL)
        {
            dstx->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                dstx->mem_ptr[0].shared_ptr, dstx->mem_ptr[0].mem_type);

            tivxMemBufferMap(dstx->mem_ptr[0].target_ptr, dstx->mem_size[0],
                dstx->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            /* TODO: Do we require to move pointer even for destination image */
            dstx_addr = (int16_t *)((uintptr_t)dstx->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x + 1U, rect.start_y + 1U,
                &dstx->imagepatch_addr[0]));
        }

        if(dsty != NULL)
        {
            dsty->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
                dsty->mem_ptr[0].shared_ptr, dsty->mem_ptr[0].mem_type);

            tivxMemBufferMap(dsty->mem_ptr[0].target_ptr, dsty->mem_size[0],
                dsty->mem_ptr[0].mem_type, VX_WRITE_ONLY);

            /* TODO: Do we require to move pointer even for destination image */
            dsty_addr = (int16_t *)((uintptr_t)dsty->mem_ptr[0U].target_ptr +
                ownComputePatchOffset(rect.start_x + 1U, rect.start_y + 1U,
                &dsty->imagepatch_addr[0]));
        }

        if ((dstx != NULL) && (dsty != NULL))
        {
            img_ptrs[0] = src_addr;
            img_ptrs[1] = (uint8_t*)dstx_addr;
            img_ptrs[2] = (uint8_t*)dsty_addr;
            tivxBamUpdatePointers(prms->graph_handle, 1U, 2U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);

        }
        else if (dstx != NULL)
        {
            img_ptrs[0] = src_addr;
            img_ptrs[1] = (uint8_t*)dstx_addr;
            tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else if (dsty != NULL)
        {
            img_ptrs[0] = src_addr;
            img_ptrs[1] = (uint8_t*)dsty_addr;
            tivxBamUpdatePointers(prms->graph_handle, 1U, 1U, img_ptrs);

            status  = tivxBamProcessGraph(prms->graph_handle);
        }
        else
        {
            status = VX_ERROR_INVALID_PARAMETERS;
        }

        if(dstx != NULL)
        {
            tivxMemBufferUnmap(dstx->mem_ptr[0].target_ptr, dstx->mem_size[0],
                dstx->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        }

        if(dsty != NULL)
        {
            tivxMemBufferUnmap(dsty->mem_ptr[0].target_ptr, dsty->mem_size[0],
                dsty->mem_ptr[0].mem_type, VX_WRITE_ONLY);
        }

        tivxMemBufferUnmap(src->mem_ptr[0].target_ptr, src->mem_size[0],
            src->mem_ptr[0].mem_type, VX_READ_ONLY);

    }
    else
    {
        status = VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSobelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dstx, *dsty;
    tivxSobelParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_SOBEL_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_IN_IMG_IDX];
        dstx = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT0_IMG_IDX];
        dsty = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL_OUT1_IMG_IDX];

        prms = tivxMemAlloc(sizeof(tivxSobelParams), TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            tivx_bam_kernel_details_t kernel_details;
            VXLIB_bufParams2D_t vxlib_src, vxlib_dstx, vxlib_dsty;
            VXLIB_bufParams2D_t *buf_params[3];

            memset(prms, 0, sizeof(tivxSobelParams));

            vxlib_src.dim_x = src->imagepatch_addr[0].dim_x;
            vxlib_src.dim_y = src->imagepatch_addr[0].dim_y;
            vxlib_src.stride_y = src->imagepatch_addr[0].stride_y;
            vxlib_src.data_type = VXLIB_UINT8;

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dstx;
            buf_params[2] = &vxlib_dsty;

            kernel_details.compute_kernel_params = NULL;

            if (dstx != NULL)
            {
                vxlib_dstx.dim_x = dstx->imagepatch_addr[0].dim_x - 2u;
                vxlib_dstx.dim_y = dstx->imagepatch_addr[0].dim_y - 2u;
                vxlib_dstx.stride_y = dstx->imagepatch_addr[0].stride_y;
                vxlib_dstx.data_type = VXLIB_UINT16;
            }
            
            if (dsty != NULL)
            {
                vxlib_dsty.dim_x = dsty->imagepatch_addr[0].dim_x - 2u;
                vxlib_dsty.dim_y = dsty->imagepatch_addr[0].dim_y - 2u;
                vxlib_dsty.stride_y = dsty->imagepatch_addr[0].stride_y;
                vxlib_dsty.data_type = VXLIB_UINT16;
            }

            if ((dstx != NULL) && (dsty != NULL))
            {
                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S,
                                                       buf_params, &kernel_details,
                                                       &prms->graph_handle);

            }
            else if (dstx != NULL)
            {
                buf_params[2] = NULL;

                BAM_VXLIB_sobelX_3x3_i8u_o16s_getKernelInfo( NULL,
                                                             &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S,
                                                       buf_params, &kernel_details,
                                                       &prms->graph_handle);
            }
            else
            {
                buf_params[1] = &vxlib_dsty;
                buf_params[2] = NULL;

                BAM_VXLIB_sobelY_3x3_i8u_o16s_getKernelInfo( NULL,
                                                             &kernel_details.kernel_info);

                status = tivxBamCreateHandleSingleNode(BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S,
                                                       buf_params, &kernel_details,
                                                       &prms->graph_handle);
            }
        }
        else
        {
            status = VX_ERROR_NO_MEMORY;
        }

        if (VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxSobelParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxSobelParams), TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelSobelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t size;
    tivxSobelParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_SOBEL_MAX_PARAMS)
    {
        status = VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL_IN_IMG_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT0_IMG_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL_OUT1_IMG_IDX])))
        {
            status = VX_ERROR_NO_MEMORY;
        }
    }

    if (VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if ((VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxSobelParams) == size))
        {
            tivxBamDestroyHandle(prms->graph_handle);
            tivxMemFree(prms, sizeof(tivxSobelParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSobelControl(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    return (VX_SUCCESS);
}

void tivxAddTargetKernelBamSobel3x3(void)
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

        vx_sobel_target_kernel = tivxAddTargetKernel(
            VX_KERNEL_SOBEL_3x3,
            target_name,
            tivxKernelSobelProcess,
            tivxKernelSobelCreate,
            tivxKernelSobelDelete,
            tivxKernelSobelControl,
            NULL);
    }
}


void tivxRemoveTargetKernelBamSobel3x3(void)
{
    tivxRemoveTargetKernel(vx_sobel_target_kernel);
}
