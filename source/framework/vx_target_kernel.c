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

static tivx_target_kernel_t g_target_kernel_table[TIVX_TARGET_KERNEL_MAX];
static tivx_mutex g_target_kernel_lock;

vx_status tivxTargetKernelInit(void)
{
    uint32_t i;
    vx_status status;

    for(i=0; i<dimof(g_target_kernel_table); i++)
    {
        g_target_kernel_table[i].kernel_id = TIVX_TARGET_KERNEL_ID_INVALID;
        g_target_kernel_table[i].target_id = TIVX_TARGET_KERNEL_ID_INVALID;
    }

    status = tivxMutexCreate(&g_target_kernel_lock);

    return status;
}

void tivxTargetKernelDeInit(void)
{
    tivxMutexDelete(&g_target_kernel_lock);
}



static tivx_target_kernel VX_API_CALL tivxAddTargetKernelInternal(
                             vx_enum kernel_id,
                             char *kernel_name,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func,
                             void *priv_arg)
{
    uint32_t i;
    tivx_target_kernel knl = NULL;
    vx_status status;

    if ((NULL != target_name) &&
        (process_func != NULL) && (create_func != NULL) &&
        (control_func != NULL))
    {
        status = tivxMutexLock(g_target_kernel_lock);

        if (VX_SUCCESS == status)
        {
            for(i=0; i<dimof(g_target_kernel_table); i++)
            {
                if (TIVX_TARGET_KERNEL_ID_INVALID ==
                    g_target_kernel_table[i].kernel_id)
                {
                    g_target_kernel_table[i].kernel_id = kernel_id;
                    g_target_kernel_table[i].kernel_name[0] = 0;
                    if(kernel_name!=NULL)
                    {
                        strncpy(g_target_kernel_table[i].kernel_name, kernel_name, VX_MAX_KERNEL_NAME);
                    }
                    g_target_kernel_table[i].target_id =
                        tivxPlatformGetTargetId(target_name);
                    g_target_kernel_table[i].process_func = process_func;
                    g_target_kernel_table[i].create_func = create_func;
                    g_target_kernel_table[i].delete_func = delete_func;
                    g_target_kernel_table[i].control_func = control_func;
                    g_target_kernel_table[i].caller_priv_arg = priv_arg;

                    knl = &g_target_kernel_table[i];

                    tivxLogResourceAlloc("TIVX_TARGET_KERNEL_MAX", 1);

                    break;
                }
            }

            if (TIVX_TARGET_KERNEL_ID_INVALID ==
                g_target_kernel_table[i].kernel_id)
            {
                VX_PRINT(VX_ZONE_WARNING, "tivxAddTargetKernelInternal: May need to increase the value of TIVX_TARGET_KERNEL_MAX in tiovx/include/tivx_config.h\n");
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }

    return (knl);
}

VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernelByName(
                             char *kernel_name,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func,
                             void *priv_arg)
{
    return tivxAddTargetKernelInternal(
                TIVX_TARGET_KERNEL_ID_NOT_USED,
                kernel_name, target_name, process_func, create_func, delete_func, control_func, priv_arg);
}

VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernel(
                             vx_enum kernel_id,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func,
                             void *priv_arg)
{
    return tivxAddTargetKernelInternal(
                kernel_id,
                NULL, target_name, process_func, create_func, delete_func, control_func, priv_arg);
}

VX_API_ENTRY vx_status VX_API_CALL tivxRemoveTargetKernel(
    tivx_target_kernel target_kernel)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    if (NULL != target_kernel)
    {
        status = tivxMutexLock(g_target_kernel_lock);
        if (VX_SUCCESS == status)
        {
            for(i=0; i<dimof(g_target_kernel_table); i++)
            {
                if (target_kernel ==
                    &g_target_kernel_table[i])
                {
                    g_target_kernel_table[i].kernel_id =
                        TIVX_TARGET_KERNEL_ID_INVALID;
                    g_target_kernel_table[i].target_id =
                        TIVX_TARGET_KERNEL_ID_INVALID;
                    g_target_kernel_table[i].process_func = NULL;
                    g_target_kernel_table[i].create_func = NULL;
                    g_target_kernel_table[i].delete_func = NULL;
                    g_target_kernel_table[i].control_func = NULL;

                    tivxLogResourceFree("TIVX_TARGET_KERNEL_MAX", 1);

                    break;
                }
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }

    return (status);
}

tivx_target_kernel tivxTargetKernelGet(vx_enum kernel_id, char *kernel_name, vx_enum target_id)
{
    uint32_t i;
    tivx_target_kernel knl = NULL;
    vx_status status;

    status = tivxMutexLock(g_target_kernel_lock);

    if (VX_SUCCESS == status)
    {
        tivx_target_kernel tmp_knl = NULL;

        for(i=0; i<dimof(g_target_kernel_table); i++)
        {
            tmp_knl = &g_target_kernel_table[i];
            if(tmp_knl->kernel_name[0]==0)
            {
                /* kernel is registered using kernel_id only */
                if ((kernel_id == tmp_knl->kernel_id) &&
                    (target_id == tmp_knl->target_id))
                {
                    knl = tmp_knl;
                }
            }
            else
            if(kernel_name!=NULL)
            {
                /* kernel registered using name, compare using kernel_name string */
                if( (strncmp(kernel_name, tmp_knl->kernel_name, VX_MAX_KERNEL_NAME)==0)
                    && (target_id == tmp_knl->target_id)
                    )
                {
                    /* copy kernel_id into tmp_knl->kernel_id since it will used
                     * later during tivxTargetKernelInstanceGet as a additional check
                     * to make the correct target kernel instance is being referenced
                     */
                    tmp_knl->kernel_id = kernel_id;
                    knl = tmp_knl;
                }
            }
            if(knl!=NULL)
            {
                /* kernel found */
                break;
            }
        }

        tivxMutexUnlock(g_target_kernel_lock);
    }

    return (knl);
}

vx_status tivxTargetKernelCreate(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->create_func))
        {
            status = knl->create_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetKernelCreate: Kernel create function is NULL\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxTargetKernelDelete(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->delete_func))
        {
            status = knl->delete_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetKernelDelete: Kernel delete function is NULL\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxTargetKernelExecute(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->process_func))
        {
            tivxPlatformActivate();
            status = knl->process_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);
            tivxPlatformDeactivate();
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetKernelExecute: Kernel process function is NULL\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

vx_status tivxTargetKernelControl(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->control_func))
        {
            status = knl->control_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxTargetKernelControl: Kernel control function is NULL\n");
            status = VX_FAILURE;
        }
    }

    return (status);
}

