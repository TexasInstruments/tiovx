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



#include <vx_internal.h>

static tivx_target_kernel_instance_t g_target_kernel_instance_table[TIVX_TARGET_KERNEL_INSTANCE_MAX];
static tivx_mutex g_target_kernel_instance_lock;

vx_status ownTargetKernelInstanceInit(void)
{
    uint16_t i;
    vx_status status;

    for(i=0; i<dimof(g_target_kernel_instance_table); i++)
    {
        g_target_kernel_instance_table[i].kernel_id = (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID;
        g_target_kernel_instance_table[i].index = i;
        g_target_kernel_instance_table[i].state = (vx_enum)VX_NODE_STATE_STEADY;
        g_target_kernel_instance_table[i].is_kernel_instance_replicated = (vx_bool)vx_false_e;
        g_target_kernel_instance_table[i].node_obj_desc = NULL;
    }

    status = tivxMutexCreate(&g_target_kernel_instance_lock);

    return status;
}

void ownTargetKernelInstanceDeInit(void)
{
    tivxMutexDelete(&g_target_kernel_instance_lock);
}

tivx_target_kernel_instance ownTargetKernelInstanceAlloc(vx_enum kernel_id, volatile char *kernel_name, vx_enum target_id)
{
    uint16_t i;
    tivx_target_kernel_instance kernel_instance = NULL, tmp_kernel_instance = NULL;
    tivx_target_kernel kernel = NULL;
    vx_status status;
    volatile char kernel_name_print[VX_MAX_KERNEL_NAME];

    kernel = ownTargetKernelGet(kernel_id, kernel_name, target_id);

    if( kernel_name != NULL )
    {
        tivx_obj_desc_memcpy(kernel_name_print, kernel_name, VX_MAX_KERNEL_NAME);
        VX_PRINT(VX_ZONE_INFO, "attempting to call kernel %s on this CPU\n", kernel_name_print);
    }

    if(kernel==NULL)
    {
        /* there is no kernel registered with this kernel ID on this CPU, hence return
        NULL */
        if( kernel_name != NULL )
        {
            VX_PRINT(VX_ZONE_ERROR, "kernel %s has not been registered on this CPU\n", kernel_name_print);
            VX_PRINT(VX_ZONE_ERROR, "Please register this kernel on the appropriate target core\n");
        }
    }
    else
    {
        status = tivxMutexLock(g_target_kernel_instance_lock);

        if(status == (vx_status)VX_SUCCESS)
        {
            for(i=0; i<dimof(g_target_kernel_instance_table); i++)
            {
                tmp_kernel_instance = &g_target_kernel_instance_table[i];
                if(tmp_kernel_instance->kernel_id == (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID)
                {
                    /* free entry found */
                    tmp_kernel_instance->kernel_id = kernel_id;
                    tmp_kernel_instance->index = i;
                    tmp_kernel_instance->kernel_context = NULL;
                    tmp_kernel_instance->kernel_context_size = 0;
                    tmp_kernel_instance->kernel = kernel;
                    tmp_kernel_instance->is_kernel_instance_replicated = (vx_bool)vx_false_e;
                    tmp_kernel_instance->node_obj_desc = NULL;
                    if (kernel->num_pipeup_bufs > 1U)
                    {
                        tmp_kernel_instance->state = (vx_enum)VX_NODE_STATE_PIPEUP;
                    }
                    else
                    {
                        tmp_kernel_instance->state = (vx_enum)VX_NODE_STATE_STEADY;
                    }

                    kernel_instance = tmp_kernel_instance;

                    ownLogResourceAlloc("TIVX_TARGET_KERNEL_INSTANCE_MAX", 1);

                    break;
                }
            }

            if(tmp_kernel_instance->kernel_id == (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID)
            {
                VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_TARGET_KERNEL_INSTANCE_MAX in tiovx/include/TI/tivx_config.h\n");
            }

            tivxMutexUnlock(g_target_kernel_instance_lock);
        }
    }

    return kernel_instance;
}

vx_status ownTargetKernelInstanceFree(tivx_target_kernel_instance *target_kernel_instance)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if((NULL != target_kernel_instance) && (NULL != *target_kernel_instance))
    {
        (*target_kernel_instance)->kernel_id = (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID;
        *target_kernel_instance = NULL;

        ownLogResourceFree("TIVX_TARGET_KERNEL_INSTANCE_MAX", 1);

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target kernel instance is NULL\n");
    }

    return status;
}

uint32_t ownTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance)
{
    uint32_t index = TIVX_TARGET_KERNEL_INSTANCE_MAX;

    if((NULL != target_kernel_instance) &&
       (target_kernel_instance->index < (uint32_t)TIVX_TARGET_KERNEL_INSTANCE_MAX))
    {
        index = target_kernel_instance->index;
    }

    return index;
}

tivx_target_kernel tivxTargetKernelInstanceGetKernel(tivx_target_kernel_instance target_kernel_instance)
{
    tivx_target_kernel kernel = NULL;

    if((NULL != target_kernel_instance)
        && (target_kernel_instance->kernel_id != (vx_enum)TIVX_TARGET_KERNEL_ID_INVALID))
    {
        kernel = target_kernel_instance->kernel;
    }

    return kernel;
}

tivx_target_kernel_instance ownTargetKernelInstanceGet(uint16_t target_kernel_index, vx_enum kernel_id)
{
    tivx_target_kernel_instance target_kernel_instance = NULL, tmp_target_kernel_instance = NULL;

    if(target_kernel_index < TIVX_TARGET_KERNEL_INSTANCE_MAX)
    {
        tmp_target_kernel_instance = &g_target_kernel_instance_table[target_kernel_index];

        if(tmp_target_kernel_instance->kernel_id == kernel_id)
        {
            target_kernel_instance = tmp_target_kernel_instance;
        }
    }
    return target_kernel_instance;
}


VX_API_ENTRY vx_status VX_API_CALL tivxSetTargetKernelInstanceContext(
            tivx_target_kernel_instance target_kernel_instance,
            void *kernel_context, uint32_t kernel_context_size)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(NULL != target_kernel_instance)
    {
        target_kernel_instance->kernel_context = kernel_context;
        target_kernel_instance->kernel_context_size = kernel_context_size;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target kernel instance is NULL\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelInstanceState(
            tivx_target_kernel_instance target_kernel_instance,
            vx_enum *state)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(NULL != target_kernel_instance)
    {
        *state = target_kernel_instance->state;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target kernel instance is NULL\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelTargetId(
            tivx_target_kernel_instance target_kernel_instance,
            uint32_t *targetId)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(NULL != target_kernel_instance)
    {
        *targetId = target_kernel_instance->node_obj_desc->target_id;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target kernel instance is NULL\n");
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelInstanceContext(
            tivx_target_kernel_instance target_kernel_instance,
            void **kernel_context,
            uint32_t *kernel_context_size)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_PARAMETERS;

    if(NULL != target_kernel_instance)
    {
        *kernel_context = target_kernel_instance->kernel_context;
        *kernel_context_size = target_kernel_instance->kernel_context_size;

        status = (vx_status)VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "target kernel instance is NULL\n");
    }
    return status;
}

VX_API_ENTRY void tivxGetTargetKernelInstanceBorderMode(
    tivx_target_kernel_instance target_kernel_instance,
    vx_border_t *border_mode)
{
    if ((NULL != target_kernel_instance) && (NULL != border_mode))
    {
        memcpy(border_mode, &target_kernel_instance->border_mode,
            sizeof(vx_border_t));
    }
}

VX_API_ENTRY vx_bool tivxIsTargetKernelInstanceReplicated(tivx_target_kernel_instance kernel_instance)
{
    vx_bool retVal = (vx_bool)vx_false_e;

    if (NULL != kernel_instance)
    {
        retVal = (vx_bool)kernel_instance->is_kernel_instance_replicated;
    }

    return retVal;
}
