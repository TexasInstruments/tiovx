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
#include <tivx_kernel_and.h>
#include <TI/tivx_target_kernel.h>
#include <ti/vxlib/vxlib.h>
#include <tivx_kernels_target_utils.h>
#include <tivx_bam_kernel_wrapper.h>
#include "tivx_target_kernels_priv.h"

typedef struct
{
    tivx_bam_graph_handle graph_handle;
    uint8_t bam_node_num;
} tivxAndParams;

static tivx_target_kernel vx_and_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelAndProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAndCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

static vx_status VX_CALLBACK tivxKernelAndDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg);

/* Supernode Callbacks */
static vx_status VX_CALLBACK tivxKernelAndCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size);

static vx_status VX_CALLBACK tivxKernelAndGetNodePort(
    tivx_target_kernel_instance kernel, uint8_t ovx_port, uint8_t plane,
    uint8_t *bam_node, uint8_t *bam_port);

static vx_status VX_CALLBACK tivxKernelAndProcess(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxAndParams *prms = NULL;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    uint8_t *src0_addr, *src1_addr, *dst_addr;
    uint32_t size;

    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_AND_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_AND_IN1_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_AND_IN2_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_AND_OUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxAndParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        void *img_ptrs[3];
        void *src0_target_ptr;
        void *src1_target_ptr;
        void *dst_target_ptr;

        src0_target_ptr = tivxMemShared2TargetPtr(&src0->mem_ptr[0]);

        src1_target_ptr = tivxMemShared2TargetPtr(&src1->mem_ptr[0]);

        tivxSetTwoPointerLocation(src0, src1, &src0_target_ptr, &src1_target_ptr, &src0_addr, &src1_addr);

        dst_target_ptr = tivxMemShared2TargetPtr(&dst->mem_ptr[0]);
        tivxSetPointerLocation(dst, &dst_target_ptr, &dst_addr);

        img_ptrs[0] = src0_addr;
        img_ptrs[1] = src1_addr;
        img_ptrs[2] = dst_addr;
        tivxBamUpdatePointers(prms->graph_handle, 2U, 1U, img_ptrs);

        status  = tivxBamProcessGraph(prms->graph_handle);
    }
    else
    {
        status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelAndCreate(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_image_t *src0, *src1, *dst;
    tivxAndParams *prms = NULL;
    tivx_bam_kernel_details_t kernel_details;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_AND_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxBamInitKernelDetails(&kernel_details, 1, kernel);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        src0 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_AND_IN1_IDX];
        src1 = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_AND_IN2_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[
            TIVX_KERNEL_AND_OUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxAndParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            VXLIB_bufParams2D_t vxlib_src0, vxlib_src1, vxlib_dst;
            VXLIB_bufParams2D_t *buf_params[3];

            memset(prms, 0, sizeof(tivxAndParams));

            tivxInitTwoBufParams(src0, src1, &vxlib_src0, &vxlib_src1);
            tivxInitBufParams(dst, &vxlib_dst);

            /* Fill in the frame level sizes of buffers here. If the port
             * is optionally disabled, put NULL */
            buf_params[0] = &vxlib_src0;
            buf_params[1] = &vxlib_src1;
            buf_params[2] = &vxlib_dst;

            BAM_VXLIB_and_i8u_i8u_o8u_getKernelInfo(NULL,
                &kernel_details.kernel_info);

            status = tivxBamCreateHandleSingleNode(
                BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U, buf_params,
                &kernel_details, &prms->graph_handle);
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxAndParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxAndParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelAndDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t size;
    tivxAndParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
            TIVX_KERNEL_AND_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
            (sizeof(tivxAndParams) == size))
        {
            if(NULL != prms->graph_handle)
            {
                tivxBamDestroyHandle(prms->graph_handle);
            }
            tivxMemFree(prms, sizeof(tivxAndParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

void tivxAddTargetKernelBamAnd(void)
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_and_target_kernel = tivxAddTargetKernel(
            (vx_enum)VX_KERNEL_AND,
            target_name,
            tivxKernelAndProcess,
            tivxKernelAndCreate,
            tivxKernelAndDelete,
            NULL,
            NULL);

        tivxEnableKernelForSuperNode(vx_and_target_kernel,
            tivxKernelAndCreateInBamGraph,
            tivxKernelAndGetNodePort,
            NULL,
            NULL,
            NULL,
            0,
            NULL);
    }
}


void tivxRemoveTargetKernelBamAnd(void)
{
    tivxRemoveTargetKernel(vx_and_target_kernel);
}


static vx_status VX_CALLBACK tivxKernelAndCreateInBamGraph(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg, BAM_NodeParams node_list[],
    tivx_bam_kernel_details_t kernel_details[],
    int32_t * bam_node_cnt, void * scratch, int32_t *size)
{

    vx_status status = (vx_status)VX_SUCCESS;
    tivxAndParams *prms = NULL;

    /* Check number of buffers and NULL pointers */
    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_AND_MAX_PARAMS);

    if ((vx_status)VX_SUCCESS == status)
    {
        prms = tivxMemAlloc(sizeof(tivxAndParams), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != prms)
        {
            memset(prms, 0, sizeof(tivxAndParams));

            node_list[*bam_node_cnt].nodeIndex = (uint8_t)*bam_node_cnt;
            node_list[*bam_node_cnt].kernelId = (uint32_t)BAM_KERNELID_VXLIB_AND_I8U_I8U_O8U;
            node_list[*bam_node_cnt].kernelArgs = NULL;

            BAM_VXLIB_and_i8u_i8u_o8u_getKernelInfo(NULL,
                &kernel_details[*bam_node_cnt].kernel_info);

            kernel_details[*bam_node_cnt].compute_kernel_params = NULL;

            prms->bam_node_num = (uint8_t)*bam_node_cnt;
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, prms,
                sizeof(tivxAndParams));
        }
        else
        {
            if (NULL != prms)
            {
                tivxMemFree(prms, sizeof(tivxAndParams), (vx_enum)TIVX_MEM_EXTERNAL);
            }
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxKernelAndGetNodePort(
    tivx_target_kernel_instance kernel,
    uint8_t ovx_port, uint8_t plane, uint8_t *bam_node, uint8_t *bam_port)
{
    tivxAndParams *prms = NULL;
    uint32_t size;

    vx_status status = tivxGetTargetKernelInstanceContext(kernel,
                        (void **)&prms, &size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != prms) &&
        (sizeof(tivxAndParams) == size))
    {
        switch (ovx_port)
        {
            case TIVX_KERNEL_AND_IN1_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_AND_I8U_I8U_O8U_INPUT0_IMAGE_PORT;
                break;
            case TIVX_KERNEL_AND_IN2_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_AND_I8U_I8U_O8U_INPUT1_IMAGE_PORT;
                break;
            case TIVX_KERNEL_AND_OUT_IDX:
                *bam_node = prms->bam_node_num;
                *bam_port = (uint8_t)BAM_VXLIB_AND_I8U_I8U_O8U_OUTPUT_PORT;
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "non existing index queried by tivxKernelSupernodeCreate.tivxGetNodePort()\n");
                status = (vx_status)VX_FAILURE;
                break;
        }
    }

    return status;
}

