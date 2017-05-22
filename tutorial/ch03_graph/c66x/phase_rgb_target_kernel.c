/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <TI/tivx.h>
#include <VX/vx.h>
#include <TI/tivx_target_kernel.h>
#include <vx_tutorial_kernels.h>

#define PHASE_RGB_IN0_IMG_IDX   (0u)
#define PHASE_RGB_OUT0_IMG_IDX  (1u)
#define PHASE_RGB_MAX_PARAMS    (2u)

static tivx_target_kernel phase_rgb_target_kernel = NULL;

vx_status VX_CALLBACK vxTutotrialPhaseRgb(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    tivx_obj_desc_image_t *src_desc, *dst_desc;

    if ((num_params != PHASE_RGB_MAX_PARAMS)
        || (NULL == obj_desc[PHASE_RGB_IN0_IMG_IDX])
        || (NULL == obj_desc[PHASE_RGB_OUT0_IMG_IDX])
        )
    {
        status = VX_FAILURE;
    }
    else
    {
        /* Get the Src and Dst descriptors */
        src_desc = (tivx_obj_desc_image_t *)obj_desc[PHASE_RGB_IN0_IMG_IDX];
        dst_desc = (tivx_obj_desc_image_t *)obj_desc[PHASE_RGB_OUT0_IMG_IDX];

        /* Get the target pointer from the shared pointer for all
           buffers */
        src_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            src_desc->mem_ptr[0].shared_ptr, src_desc->mem_ptr[0].mem_type);
        dst_desc->mem_ptr[0].target_ptr = tivxMemShared2TargetPtr(
            dst_desc->mem_ptr[0].shared_ptr, dst_desc->mem_ptr[0].mem_type);

        /* Map all buffers, which invalidates the cache */
        tivxMemBufferMap(src_desc->mem_ptr[0].target_ptr,
            src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferMap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

        /* Run function for complete image, ROI is ignored in this example */
        {
            vx_uint32 x, y, width, height;
            vx_uint8 *in_data_ptr, *out_data_ptr;
            vx_uint8 *in_pixel, *out_pixel;

            in_data_ptr = src_desc->mem_ptr[0].target_ptr;
            out_data_ptr = dst_desc->mem_ptr[0].target_ptr;

            width = dst_desc->imagepatch_addr[0U].dim_x;
            height = dst_desc->imagepatch_addr[0U].dim_y;

            for(y=0; y<height; y++)
            {
                in_pixel = in_data_ptr;
                out_pixel = out_data_ptr;

                for(x=0; x<width; x++)
                {
                    if(in_pixel[0]<64)
                    {
                        out_pixel[0] = 0xFF;
                        out_pixel[1] = 0x00;
                        out_pixel[2] = 0x00;
                    }
                    else
                    if(in_pixel[0]<128)
                    {
                        out_pixel[0] = 0x00;
                        out_pixel[1] = 0xFF;
                        out_pixel[2] = 0x00;
                    }
                    else
                    if(in_pixel[0]<192)
                    {
                        out_pixel[0] = 0x00;
                        out_pixel[1] = 0x00;
                        out_pixel[2] = 0xFF;
                    }
                    else
                    {
                        out_pixel[0] = 0xFF;
                        out_pixel[1] = 0xFF;
                        out_pixel[2] = 0xFF;
                    }
                    in_pixel++;
                    out_pixel += 3;
                }
                in_data_ptr += src_desc->imagepatch_addr[0U].stride_y;
                out_data_ptr += dst_desc->imagepatch_addr[0U].stride_y;
            }
        }

        tivxMemBufferUnmap(src_desc->mem_ptr[0].target_ptr,
            src_desc->mem_size[0], src_desc->mem_ptr[0].mem_type,
            VX_READ_ONLY);
        tivxMemBufferUnmap(dst_desc->mem_ptr[0].target_ptr,
            dst_desc->mem_size[0], dst_desc->mem_ptr[0].mem_type,
            VX_WRITE_ONLY);

    }

    return (status);
}

vx_status VX_CALLBACK vxTutotrialPhaseRgbCreate(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK vxTutotrialPhaseRgbDelete(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}

vx_status VX_CALLBACK vxTutotrialPhaseRgbControl(tivx_target_kernel_instance kernel, tivx_obj_desc_t *param_obj_desc[], uint16_t num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;

    return status;
}


void vxTutorialAddTargetKernelPhaseRgb(void)
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

        phase_rgb_target_kernel = tivxAddTargetKernel(
                    TIVX_TUTORIAL_KERNEL_PHASE_RGB,
                    target_name,
                    vxTutotrialPhaseRgb,
                    vxTutotrialPhaseRgbCreate,
                    vxTutotrialPhaseRgbDelete,
                    vxTutotrialPhaseRgbControl,
                    NULL);
    }
}

void vxTutorialRemoveTargetKernelPhaseRgb(void)
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(phase_rgb_target_kernel);

    if (VX_SUCCESS == status)
    {
        phase_rgb_target_kernel = NULL;
    }
}

