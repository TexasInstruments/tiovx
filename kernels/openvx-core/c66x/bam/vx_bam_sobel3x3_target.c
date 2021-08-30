/*
*
* Copyright (c) 2017-2019 Texas Instruments Incorporated
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
#include <tivx_kernel_sobel3x3.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
    uint8_t switch_buffers;
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

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelSobelCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelSobelGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);


static vx_status VX_CALLBACK tivxKernelSobelProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxSobelParams *prms = NULL;
    tivx_obj_desc_image_t *src, *dstx, *dsty;
    uint8_t *src_addr;
    int16_t *dstx_addr, *dsty_addr;
    uint32_t size;

    if (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* Check for NULL */
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX])))
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX];
        dstx = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        dsty = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxSobelParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[3];
        void *src_target_ptr;
        void *dstx_target_ptr = NULL;
        void *dsty_target_ptr = NULL;

        src_target_ptr = tivxMemShared2TargetPtr(&src->mem_ptr[0]);

        tivxSetPointerLocation(src, &src_target_ptr, &src_addr);

        if(dstx != NULL)
        {
            dstx_target_ptr = tivxMemShared2TargetPtr(&dstx->mem_ptr[0]);

            tivxSetPointerLocation(dstx, &dstx_target_ptr, (uint8_t **)&dstx_addr);
        }

        if(dsty != NULL)
        {
            dsty_target_ptr = tivxMemShared2TargetPtr(&dsty->mem_ptr[0]);

            tivxSetPointerLocation(dsty, &dsty_target_ptr, (uint8_t **)&dsty_addr);
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
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

    }
    else
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelSobelCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src, *dstx, *dsty;
    tivxSobelParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX])))
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_INPUT_IDX];
        dstx = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        dsty = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];

        prms = tivxMemAlloc(sizeof(tivxSobelParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            VXLIB_bufParams2D_t vxlib_src, vxlib_dstx, vxlib_dsty;
            VXLIB_bufParams2D_t *buf_params[3];

            memset(prms, 0, sizeof(tivxSobelParams));

            tivxInitBufParams(src, &vxlib_src);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src;
            buf_params[1] = &vxlib_dstx;
            buf_params[2] = &vxlib_dsty;

            if (dstx != NULL)
            {
                tivxInitBufParams(dstx, &vxlib_dstx);
            }

            if (dsty != NULL)
            {
                tivxInitBufParams(dsty, &vxlib_dsty);
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
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxSobelParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxSobelParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelSobelDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxSobelParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX])))
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxSobelParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxSobelParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamSobel3x3(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_sobel_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_SOBEL_3x3,
            target_name,
            tivxKernelSobelProcess,
            tivxKernelSobelCreate,
            tivxKernelSobelDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_sobel_target_kernel,
            tivxKernelSobelCreateInBamGraph,
            tivxKernelSobelGetNodePort,
            NULL,
            NULL,
            NULL,
            0,
            NULL);
    }
}


void tivxRemoveTargetKernelBamSobel3x3(void)
{
    tivxRemoveTargetKernel(vx_sobel_target_kernel);
}

static vx_status VX_CALLBACK tivxKernelSobelCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivxSobelParams *prms = NULL;
    tivx_obj_desc_image_t *dstx, *dsty;

    /* Check number of buffers and NULL pointers */
    if (num_params != TIVX_KERNEL_SOBEL3X3_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Obj_desc param count doesn't match TIVX_KERNEL_SOBEL_MAX_PARAMS\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        if ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_INPUT_IDX]) ||
            ((NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX]) &&
             (NULL == obj_desc[TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX])))
        {
            VX_PRINT(VX_ZONE_ERROR, "required Obj_descs are NULL\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        dstx = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX];
        dsty = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX];

        prms = tivxMemAlloc(sizeof(tivxSobelParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxSobelParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

            if ((dstx != NULL) && (dsty != NULL))
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBEL_3X3_I8U_O16S_O16S;

                BAM_VXLIB_sobel_3x3_i8u_o16s_o16s_getKernelInfo( NULL,
                                                                 &kernel_details[*bam_node_cnt].kernel_info);
            }
            else if (dstx != NULL)
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBELX_3X3_I8U_O16S;

                BAM_VXLIB_sobelX_3x3_i8u_o16s_getKernelInfo( NULL,
                                                             &kernel_details[*bam_node_cnt].kernel_info);
            }
            else
            {
                node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_SOBELY_3X3_I8U_O16S;

                BAM_VXLIB_sobelY_3x3_i8u_o16s_getKernelInfo( NULL,
                                                             &kernel_details[*bam_node_cnt].kernel_info);
                prms->switch_buffers = 1;
            }

            prms->bam_node_num = (uint8_t)*bam_node_cnt;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "prms mem allocation failed\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxSobelParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxSobelParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelSobelGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxSobelParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxSobelParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_SOBEL3X3_INPUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_INPUT_IMAGE_PORT;
                break;
            case TIVX_KERNEL_SOBEL3X3_OUTPUT_X_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_OUTPUT_X_PORT;
                break;
            case TIVX_KERNEL_SOBEL3X3_OUTPUT_Y_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_OUTPUT_Y_PORT;
                if (prms->switch_buffers != 0U) {
                    *bam_port = (uint8_t)BAM_VXLIB_SOBEL_3X3_I8U_O16S_O16S_OUTPUT_X_PORT;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}
