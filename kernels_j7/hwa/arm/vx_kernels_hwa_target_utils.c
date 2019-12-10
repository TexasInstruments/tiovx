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
#include <TI/tivx_target_kernel.h>
#include "tivx_hwa_kernels.h"
#include "tivx_kernels_target_utils.h"
#include "vx_kernels_hwa_target.h"

void lse_reformat_in(tivx_obj_desc_image_t *src, void* src_target_ptr, uint16_t src16[], uint8_t ch, uint8_t out_bit_alignment)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = src->imagepatch_addr[ch].dim_x;
    uint32_t h = src->imagepatch_addr[ch].dim_y / src->imagepatch_addr[ch].step_y;
    uint32_t stride = src->imagepatch_addr[ch].stride_y;

    if ((VX_DF_IMAGE_U8 == src->format) || (VX_DF_IMAGE_NV12 == src->format))
    {
        uint8_t *src_addr8 = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[ch]));

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                if(out_bit_alignment == 0)
                {
                    /* Put 8 bits to 12 */
                    src16[(j*w)+i] = (uint16_t)src_addr8[(j*stride)+i] << 4;
                }
                else
                {
                    /* Put 8 bits as is */
                    src16[(j*w)+i] = src_addr8[(j*stride)+i];
                }

            }
        }
    }
    else if((VX_DF_IMAGE_U16 == src->format) || (VX_DF_IMAGE_S16 == src->format))
    {
        uint16_t *src_addr16 = (uint16_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[ch]));
        stride /= 2;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take 16 as is */
                src16[(j*w)+i] = src_addr16[(j*stride)+i];
            }
        }
    }
    else /* TIVX_DF_IMAGE_P12*/
    {
        uint32_t *src_addr32 = (uint32_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[ch]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            int32_t k = 0;
            /* This assumes that width is a multiple of 8 for now */
            for(i=0; i < w; i+=8)
            {
                /* 8 pixels are packed among 3 32-bit words */
                src16[(j*w)+i+0]  = src_addr32[(j*stride)+k] & 0xFFF;
                src16[(j*w)+i+1]  = (src_addr32[(j*stride)+k] >> 12) & 0xFFF;
                src16[(j*w)+i+2]  = (src_addr32[(j*stride)+k] >> 24) | ((src_addr32[(j*stride)+k+1] & 0xF) << 8);
                src16[(j*w)+i+3]  = (src_addr32[(j*stride)+k+1] >> 4) & 0xFFF;
                src16[(j*w)+i+4]  = (src_addr32[(j*stride)+k+1] >> 16) & 0xFFF;
                src16[(j*w)+i+5]  = (src_addr32[(j*stride)+k+1] >> 28) | ((src_addr32[(j*stride)+k+2] & 0xFF) << 4);
                src16[(j*w)+i+6]  = (src_addr32[(j*stride)+k+2] >> 8) & 0xFFF;
                src16[(j*w)+i+7]  = (src_addr32[(j*stride)+k+2] >> 20);
                k+=3;
            }
        }
    }
}

void lse_reformat_out(tivx_obj_desc_image_t *src, tivx_obj_desc_image_t *dst, void *dst_target_ptr, uint16_t dst16[], uint16_t input_bits, uint8_t ch)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = dst->imagepatch_addr[ch].dim_x;
    uint32_t h = dst->imagepatch_addr[ch].dim_y / dst->imagepatch_addr[ch].step_y;
    uint32_t stride = dst->imagepatch_addr[ch].stride_y;
    uint16_t downshift = input_bits-8;
    uint16_t upshift = 12-input_bits;

    if ((VX_DF_IMAGE_U8 == dst->format) || (VX_DF_IMAGE_NV12 == dst->format))
    {
        uint8_t *dst_addr8 = (uint8_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[ch]));

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Downshift bits to align msb to bit 7 */
                dst_addr8[(j*stride)+i] = dst16[(j*w)+i] >> downshift;
            }
        }
    }
    else if((VX_DF_IMAGE_U16 == dst->format) || (VX_DF_IMAGE_S16 == dst->format))
    {
        uint16_t *dst_addr16 = (uint16_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[ch]));
        stride /= 2;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Upshift bits to align msb to bit 11 */
                dst_addr16[(j*stride)+i] = dst16[(j*w)+i] << upshift;
            }
        }
    }
    else /* TIVX_DF_IMAGE_P12*/
    {
        uint32_t *dst_addr32 = (uint32_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[ch]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            int32_t k = 0;
            /* This assumes that width is a multiple of 8 for now */
            for(i=0; i < w; i+=8)
            {
                /* 8 pixels are packed among 3 32-bit words */
                dst_addr32[(j*stride)+k+0] =   ((uint32_t)dst16[(j*w)+i+0] << upshift) |
                                            (((uint32_t)dst16[(j*w)+i+1] << upshift) << 12) |
                                           ((((uint32_t)dst16[(j*w)+i+2] << upshift) & 0xFF) << 24);
                dst_addr32[(j*stride)+k+1] =  (((uint32_t)dst16[(j*w)+i+2] << upshift) >> 8) |
                                            (((uint32_t)dst16[(j*w)+i+3] << upshift) << 4) |
                                            (((uint32_t)dst16[(j*w)+i+4] << upshift) << 16) |
                                           ((((uint32_t)dst16[(j*w)+i+5] << upshift) & 0xF) << 28);
                dst_addr32[(j*stride)+k+2] =  (((uint32_t)dst16[(j*w)+i+5] << upshift) >> 4) |
                                            (((uint32_t)dst16[(j*w)+i+6] << upshift) << 8) |
                                            (((uint32_t)dst16[(j*w)+i+7] << upshift) << 20);
                k+=3;
            }
        }
    }
}

void lse_interleave_422(tivx_obj_desc_image_t *src, tivx_obj_desc_image_t *dst, void *dst_target_ptr, uint16_t dst16_0[], uint16_t dst16_1[], uint16_t input_bits)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = dst->imagepatch_addr[0].dim_x;
    uint32_t h = dst->imagepatch_addr[0].dim_y / dst->imagepatch_addr[0].step_y;
    uint32_t stride = dst->imagepatch_addr[0].stride_y;
    uint16_t downshift = input_bits-8;
    uint32_t y,u;

    uint8_t *dst_addr8 = (uint8_t *)((uintptr_t)dst_target_ptr +
        tivxComputePatchOffset(rect.start_x, rect.start_y,
        &dst->imagepatch_addr[0]));

    if (VX_DF_IMAGE_YUYV == dst->format)
    {
        y=0;
        u=1;
    }
    else
    {
        y=1;
        u=0;
    }

    for(j = 0; j < h; j++)
    {
        for(i=0; i < w; i++)
        {
            /* Downshift bits to align msb to bit 7 */
            dst_addr8[(j*stride)+(i*2)+y] = dst16_0[(j*w)+i] >> downshift;
            dst_addr8[(j*stride)+(i*2)+u] = dst16_1[(j*w)+i] >> downshift;
        }
    }
}

void lse_deinterleave_422(tivx_obj_desc_image_t *src, void *src_target_ptr, uint16_t src16_0[], uint16_t src16_1[], uint16_t input_bits)
{
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = src->imagepatch_addr[0].dim_x;
    uint32_t h = src->imagepatch_addr[0].dim_y / src->imagepatch_addr[0].step_y;
    uint32_t stride = src->imagepatch_addr[0].stride_y;
    uint16_t downshift = input_bits-8;
    uint32_t y,u;

    uint8_t *src_addr8 = (uint8_t *)((uintptr_t)src_target_ptr +
        tivxComputePatchOffset(rect.start_x, rect.start_y,
        &src->imagepatch_addr[0]));

    if (VX_DF_IMAGE_YUYV == src->format)
    {
        y=0;
        u=1;
    }
    else
    {
        y=1;
        u=0;
    }

    for(j = 0; j < h; j++)
    {
        for(i=0; i < w; i++)
        {
            /* Downshift bits to align msb to bit 7 */
            src16_0[(j*w)+i] = (uint16_t)src_addr8[(j*stride)+(i*2)+y] >> downshift;
            src16_1[(j*w)+i] = (uint16_t)src_addr8[(j*stride)+(i*2)+u] >> downshift;
        }
    }
}

void lse_reformat_in_dof(tivx_obj_desc_image_t *src, void *src_target_ptr, int *src32)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = src->imagepatch_addr[0].dim_x;
    uint32_t h = src->imagepatch_addr[0].dim_y;
    uint32_t stride = src->imagepatch_addr[0].stride_y;

    if (VX_DF_IMAGE_U8 == src->format)
    {
        uint8_t *src_addr8 = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Put 8 bits to 12 and then into 32b container */
                src32[(j*w)+i] = src_addr8[(j*stride)+i] << 4;
            }
        }
    }
    else if((VX_DF_IMAGE_U16 == src->format) || (VX_DF_IMAGE_S16 == src->format))
    {
        uint16_t *src_addr16 = (uint16_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        stride /= 2;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take 16 as is */
                src32[(j*w)+i] = src_addr16[(j*stride)+i];
            }
        }
    }
    else if((VX_DF_IMAGE_U32 == src->format) || (VX_DF_IMAGE_S32 == src->format))
    {
        uint32_t *src_addr32 = (uint32_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take 32 as is */
                src32[(j*w)+i] = src_addr32[(j*stride)+i];
            }
        }
    }
    else /* TIVX_DF_IMAGE_P12*/
    {
        uint32_t *src_addr32 = (uint32_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[0U]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            int32_t k = 0;
            /* This assumes that width is a multiple of 8 for now */
            for(i=0; i < w; i+=8)
            {
                /* 8 pixels are packed among 3 32-bit words */
                src32[(j*w)+i+0]  = src_addr32[(j*stride)+k] & 0xFFF;
                src32[(j*w)+i+1]  = (src_addr32[(j*stride)+k] >> 12) & 0xFFF;
                src32[(j*w)+i+2]  = (src_addr32[(j*stride)+k] >> 24) | ((src_addr32[(j*stride)+k+1] & 0xF) << 8);
                src32[(j*w)+i+3]  = (src_addr32[(j*stride)+k+1] >> 4) & 0xFFF;
                src32[(j*w)+i+4]  = (src_addr32[(j*stride)+k+1] >> 16) & 0xFFF;
                src32[(j*w)+i+5]  = (src_addr32[(j*stride)+k+1] >> 28) | ((src_addr32[(j*stride)+k+2] & 0xFF) << 4);
                src32[(j*w)+i+6]  = (src_addr32[(j*stride)+k+2] >> 8) & 0xFFF;
                src32[(j*w)+i+7]  = (src_addr32[(j*stride)+k+2] >> 20);
                k+=3;
            }
        }
    }
}

void lse_reformat_out_dof(tivx_obj_desc_image_t *src, tivx_obj_desc_image_t *dst, void *dst_target_ptr, int32_t *dst32)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = dst->imagepatch_addr[0].dim_x;
    uint32_t h = dst->imagepatch_addr[0].dim_y;
    uint32_t stride = dst->imagepatch_addr[0].stride_y;

    if((VX_DF_IMAGE_U32 == dst->format) || (VX_DF_IMAGE_S32 == dst->format))
    {
        uint32_t *dst_addr32 = (uint32_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take 32b as is */
                dst_addr32[(j*stride)+i] = dst32[(j*w)+i];
            }
        }
    }
    else if((VX_DF_IMAGE_U16 == dst->format) || (VX_DF_IMAGE_S16 == dst->format))
    {
        uint16_t *dst_addr16 = (uint16_t *)((uintptr_t)dst_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &dst->imagepatch_addr[0U]));
        stride /= 2;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take lower 16 bits */
                dst_addr16[(j*stride)+i] = dst32[(j*w)+i] & 0x0000FFFF;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "DOF output format not supported\n");
    }
}

void lse_reformat_in_viss(tivx_obj_desc_raw_image_t *src, void* src_target_ptr, uint16_t src16[], uint32_t exposure)
{
    /* Get the correct offset of the images from the valid roi parameter,
       Assuming valid Roi is same images */
    vx_rectangle_t rect = src->valid_roi;
    int32_t i, j;
    uint32_t w = src->imagepatch_addr[exposure].dim_x;
    uint32_t h = src->imagepatch_addr[exposure].dim_y;
    uint32_t stride = src->imagepatch_addr[exposure].stride_y;
    uint32_t idx = exposure;

    if(src->params.line_interleaved)
    {
        idx = 0;
    }

    if (TIVX_RAW_IMAGE_8_BIT == src->params.format[idx].pixel_container)
    {
        uint8_t *src_addr8 = (uint8_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[exposure]));

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Put 8 bits to 12 */
                src16[(j*w)+i] = (uint16_t)src_addr8[(j*stride)+i] << 4;
            }
        }
    }
    else if(TIVX_RAW_IMAGE_16_BIT == src->params.format[idx].pixel_container)
    {
        uint16_t *src_addr16 = (uint16_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[exposure]));
        stride /= 2;

        for(j = 0; j < h; j++)
        {
            for(i=0; i < w; i++)
            {
                /* Take 16 as is */
                src16[(j*w)+i] = src_addr16[(j*stride)+i];
            }
        }
    }
    else /* TIVX_RAW_IMAGE_P12_BIT */
    {
        uint32_t *src_addr32 = (uint32_t *)((uintptr_t)src_target_ptr +
            tivxComputePatchOffset(rect.start_x, rect.start_y,
            &src->imagepatch_addr[exposure]));
        stride /= 4;

        for(j = 0; j < h; j++)
        {
            int32_t k = 0;
            /* This assumes that width is a multiple of 8 for now */
            for(i=0; i < w; i+=8)
            {
                /* 8 pixels are packed among 3 32-bit words */
                src16[(j*w)+i+0]  = src_addr32[(j*stride)+k] & 0xFFF;
                src16[(j*w)+i+1]  = (src_addr32[(j*stride)+k] >> 12) & 0xFFF;
                src16[(j*w)+i+2]  = (src_addr32[(j*stride)+k] >> 24) | ((src_addr32[(j*stride)+k+1] & 0xF) << 8);
                src16[(j*w)+i+3]  = (src_addr32[(j*stride)+k+1] >> 4) & 0xFFF;
                src16[(j*w)+i+4]  = (src_addr32[(j*stride)+k+1] >> 16) & 0xFFF;
                src16[(j*w)+i+5]  = (src_addr32[(j*stride)+k+1] >> 28) | ((src_addr32[(j*stride)+k+2] & 0xFF) << 4);
                src16[(j*w)+i+6]  = (src_addr32[(j*stride)+k+2] >> 8) & 0xFFF;
                src16[(j*w)+i+7]  = (src_addr32[(j*stride)+k+2] >> 20);
                k+=3;
            }
        }
    }
}

void lse_reformat_out_viss(tivx_obj_desc_raw_image_t *src, tivx_obj_desc_image_t *dst, void *dst0_target_ptr, void *dst1_target_ptr, uint16_t dst16_0[], uint16_t dst16_1[], uint16_t input_bits)
{
    uint32_t format;
    tivx_obj_desc_image_t tmp;

    memcpy(&tmp.valid_roi, &src->valid_roi, sizeof(src->valid_roi));

    if ((TIVX_DF_IMAGE_NV12_P12 == dst->format) ||
        (VX_DF_IMAGE_NV12 == dst->format))
    {
        format = dst->format;

        if (TIVX_DF_IMAGE_NV12_P12 == dst->format)
        {
            dst->format = TIVX_DF_IMAGE_P12;
        }
        else
        {
            dst->format = VX_DF_IMAGE_U8;
        }

        lse_reformat_out(&tmp, dst, dst0_target_ptr, dst16_0, input_bits, 0);
        lse_reformat_out(&tmp, dst, dst1_target_ptr, dst16_1, input_bits, 1);

        dst->format = format;
    }
    else if ((VX_DF_IMAGE_YUYV == dst->format) ||
             (VX_DF_IMAGE_UYVY == dst->format))
    {
        lse_interleave_422(&tmp, dst, dst0_target_ptr, dst16_0, dst16_1, input_bits);
    }
    else
    {
        lse_reformat_out(&tmp, dst, dst0_target_ptr, dst16_0, input_bits, 0);
    }
}


#ifdef VLAB_HWA
vx_status vlab_hwa_process(uint32_t base_address, char *kernel_prefix, uint32_t config_size, void *pConfig)
{
    uint32_t data;
    uint32_t *base_addr = (uint32_t*)base_address;
    volatile uint32_t* const regs = base_addr;
    vx_status status = (vx_status)VX_SUCCESS;

    /* Check if config size is as expected */
    data = GET_REG(REG_STATUS);
    if(( data >> 16 )  != config_size)
    {
        VX_PRINT(VX_ZONE_ERROR, "%s: vlab model expecting different config size: model=%d, local=%d\n", kernel_prefix, data >> 16, config_size);
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set config address, and start processing */
        SET_REG(REG_ADDR_LO, (uintptr_t)pConfig);
        SET_REG(REG_STATUS, 1);

        /* Read STATUS register; DONE bit should be cleared before checking output */
        do {
            data = GET_REG(REG_STATUS);
        } while(data & 1);

        data = GET_REG(REG_ERROR);

        if( data )
        {
            if( VLAB_MODEL_ERROR == data )
            {
                data = GET_REG(REG_MODEL_ERROR);
                VX_PRINT(VX_ZONE_ERROR, "%s: vlab model returned error VLAB_MODEL_ERROR: %d\n", kernel_prefix, data);
            }
            else if ( VLAB_BUS_ERROR == data )
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: vlab model returned error VLAB_BUS_ERROR\n", kernel_prefix);
            }
            else if ( VLAB_MALLOC_ERROR == data )
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: vlab model returned error VLAB_MALLOC_ERROR\n", kernel_prefix);
            }
            else if ( VLAB_PARAM_ERROR == data )
            {
                VX_PRINT(VX_ZONE_ERROR, "%s: vlab model returned error VLAB_PARAM_ERROR\n", kernel_prefix);
            }

            status = (vx_status)VX_FAILURE;
        }
    }
    return status;
}
#endif
