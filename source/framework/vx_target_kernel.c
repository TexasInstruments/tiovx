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

static tivx_target_kernel VX_API_CALL ownAddTargetKernelInternal(
                             vx_enum kernel_id,
                             const char *kernel_name,
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
                             void *priv_arg);

static tivx_target_kernel_t g_target_kernel_table[TIVX_TARGET_KERNEL_MAX];
static vx_uint32 g_num_target_kernel;
static tivx_mutex g_target_kernel_lock;

vx_status ownTargetKernelInit(void)
{
    uint32_t i;
    vx_status status;

    for(i=0; i<dimof(g_target_kernel_table); i++)
    {
        g_target_kernel_table[i].kernel_id = (vx_int32)TIVX_TARGET_KERNEL_ID_INVALID;
        g_target_kernel_table[i].target_id = (vx_int32)TIVX_TARGET_KERNEL_ID_INVALID;
    }

    g_num_target_kernel = 0U;

    status = tivxMutexCreate(&g_target_kernel_lock);

    return status;
}

void ownTargetKernelDeInit(void)
{
    tivxMutexDelete(&g_target_kernel_lock);
}

static tivx_target_kernel VX_API_CALL ownAddTargetKernelInternal(
                             vx_enum kernel_id,
                             const char *kernel_name,
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
                             void *priv_arg)
{
    uint32_t i;
    tivx_target_kernel knl = NULL;
    vx_status status;
    vx_bool resource_added = (vx_bool)vx_false_e;

    if ((NULL != target_name) &&
        (process_func != NULL) && (create_func != NULL))
    {
        status = tivxMutexLock(g_target_kernel_lock);

        if ((vx_status)VX_SUCCESS == status)
        {
            for(i=0; i<dimof(g_target_kernel_table); i++)
            {
                if ((vx_int32)TIVX_TARGET_KERNEL_ID_INVALID ==
                    g_target_kernel_table[i].kernel_id)
                {
                    g_target_kernel_table[i].kernel_id = kernel_id;
                    g_target_kernel_table[i].kernel_name[0] = '\0';
                    if(kernel_name!=NULL)
                    {
                        strncpy(g_target_kernel_table[i].kernel_name, kernel_name, VX_MAX_KERNEL_NAME-1U);
                        g_target_kernel_table[i].kernel_name[VX_MAX_KERNEL_NAME-1U] = '\0';
                        VX_PRINT(VX_ZONE_INFO, "registered kernel %s on target %s\n", kernel_name, target_name);
                    }
                    g_target_kernel_table[i].target_id =
                        ownPlatformGetTargetId(target_name);
                    g_target_kernel_table[i].process_func = process_func;
                    g_target_kernel_table[i].create_func = create_func;
                    g_target_kernel_table[i].delete_func = delete_func;
                    g_target_kernel_table[i].control_func = control_func;
                    g_target_kernel_table[i].caller_priv_arg = priv_arg;
                    g_target_kernel_table[i].num_pipeup_bufs = 1;

                    knl = &g_target_kernel_table[i];

                    g_num_target_kernel++;

                    ownLogResourceAlloc("TIVX_TARGET_KERNEL_MAX", 1);
                    resource_added = (vx_bool)vx_true_e;

                    break;
                }
            }

            if ((vx_bool)vx_false_e == resource_added)
            {
                VX_PRINT(VX_ZONE_WARNING, "May need to increase the value of TIVX_TARGET_KERNEL_MAX in tiovx/include/TI/tivx_config.h\n");
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters given to function\n");
    }

    return (knl);
}

VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernelByName(
                             const char *kernel_name,
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
                             void *priv_arg)
{
    return ownAddTargetKernelInternal(
                (vx_int32)TIVX_TARGET_KERNEL_ID_NOT_USED,
                kernel_name, target_name, process_func, create_func, delete_func, control_func, priv_arg);
}

VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernel(
                             vx_enum kernel_id,
                             const char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_control_f control_func,
                             void *priv_arg)
{
    return ownAddTargetKernelInternal(
                kernel_id,
                NULL, target_name, process_func, create_func, delete_func, control_func, priv_arg);
}

VX_API_ENTRY vx_status VX_API_CALL tivxRemoveTargetKernel(
    tivx_target_kernel target_kernel)
{
    vx_status status = (vx_status)VX_FAILURE;
    vx_status mutex_status = (vx_status)VX_FAILURE;
    uint32_t i;

    if (NULL != target_kernel)
    {
        mutex_status = tivxMutexLock(g_target_kernel_lock);
        if ((vx_status)VX_SUCCESS == mutex_status)
        {
            for(i=0; i<dimof(g_target_kernel_table); i++)
            {
                if (target_kernel ==
                    &g_target_kernel_table[i])
                {
                    g_target_kernel_table[i].kernel_id =
                        (vx_int32)TIVX_TARGET_KERNEL_ID_INVALID;
                    g_target_kernel_table[i].target_id =
                        (vx_int32)TIVX_TARGET_KERNEL_ID_INVALID;
                    g_target_kernel_table[i].process_func = NULL;
                    g_target_kernel_table[i].create_func = NULL;
                    g_target_kernel_table[i].delete_func = NULL;
                    g_target_kernel_table[i].control_func = NULL;

                    ownLogResourceFree("TIVX_TARGET_KERNEL_MAX", 1);

                    g_num_target_kernel--;

                    status = (vx_status)VX_SUCCESS;
                    break;
                }
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }

    return (status);
}

tivx_target_kernel ownTargetKernelGet(vx_enum kernel_id, volatile char *kernel_name, vx_enum target_id)
{
    uint32_t i;
    tivx_target_kernel knl = NULL;
    vx_status status;

    status = tivxMutexLock(g_target_kernel_lock);

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_target_kernel tmp_knl = NULL;

        for(i=0; i<dimof(g_target_kernel_table); i++)
        {
            tmp_knl = &g_target_kernel_table[i];
            if(tmp_knl->kernel_name[0]==(char)0)
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
                if( (tivx_obj_desc_strncmp_delim(kernel_name, tmp_knl->kernel_name, VX_MAX_KERNEL_NAME, ':')==0)
                    && (target_id == tmp_knl->target_id)
                    )
                {
                    /* copy kernel_id into tmp_knl->kernel_id since it will used
                     * later during ownTargetKernelInstanceGet as a additional check
                     * to make the correct target kernel instance is being referenced
                     */
                    tmp_knl->kernel_id = kernel_id;
                    knl = tmp_knl;
                }
            }
            else
            {
                /* do nothing */
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

vx_status ownTargetKernelCreate(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->create_func))
        {
            VX_PRINT(VX_ZONE_INFO, "Executing create callback for kernel [%s]\n", knl->kernel_name);

            status = knl->create_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);

            VX_PRINT(VX_ZONE_INFO, "Done executing create callback for kernel [%s]\n", knl->kernel_name);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel create function is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

vx_status ownTargetKernelDelete(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->delete_func))
        {
            VX_PRINT(VX_ZONE_INFO, "Executing delete callback for kernel [%s]\n", knl->kernel_name);

            status = knl->delete_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);

            VX_PRINT(VX_ZONE_INFO, "Done executing delete callback for kernel [%s]\n", knl->kernel_name);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel delete function is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

vx_status ownTargetKernelExecute(
    tivx_target_kernel_instance target_kernel_instance,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->process_func))
        {
            VX_PRINT(VX_ZONE_INFO, "Executing process callback for kernel [%s]\n", knl->kernel_name);

            ownPlatformActivate();
            status = knl->process_func(
                target_kernel_instance, obj_desc, num_params,
                knl->caller_priv_arg);
            ownPlatformDeactivate();

            VX_PRINT(VX_ZONE_INFO, "Done executing process callback for kernel [%s]\n", knl->kernel_name);

            if((vx_status)VX_SUCCESS != status)
            {
                /* making info since on a valid kernel process error, it will continously print errors  */
                VX_PRINT(VX_ZONE_INFO, "Kernel process function for [%s] returned error code: %d\n", knl->kernel_name, status);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel process function is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

vx_status ownTargetKernelControl(
    tivx_target_kernel_instance target_kernel_instance,
    uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[], uint16_t num_params)
{
    vx_status status = (vx_status)VX_FAILURE;
    tivx_target_kernel knl = NULL;

    if ((NULL != target_kernel_instance) && (NULL != obj_desc))
    {
        /* Check if the kernel is valid */
        knl = tivxTargetKernelInstanceGetKernel(target_kernel_instance);

        if ((NULL != knl) && (NULL != knl->control_func))
        {
            VX_PRINT(VX_ZONE_INFO, "Executing control callback for kernel [%s]\n", knl->kernel_name);

            status = knl->control_func(
                target_kernel_instance, node_cmd_id, obj_desc, num_params,
                knl->caller_priv_arg);

            VX_PRINT(VX_ZONE_INFO, "Done executing control callback for kernel [%s]\n", knl->kernel_name);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel control function is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    return (status);
}

VX_API_ENTRY vx_status VX_API_CALL tivxQueryNumTargetKernel(vx_uint32 *ptr)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (VX_CHECK_PARAM(ptr, sizeof(vx_uint32), vx_uint32, 0x3U))
    {
        *(vx_uint32 *)ptr = g_num_target_kernel;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"query number of target kernels failed\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

#if defined(BUILD_BAM)

VX_API_ENTRY vx_status VX_API_CALL tivxEnableKernelForSuperNode(
                             tivx_target_kernel target_kernel,
                             tivx_target_kernel_create_in_bam_graph_f   create_in_bam_func,
                             tivx_target_kernel_get_node_port_f         get_node_port_func,
                             tivx_target_kernel_append_internal_edges_f append_internal_edges_func,
                             tivx_target_kernel_pre_post_process_f      preprocess_func,
                             tivx_target_kernel_pre_post_process_f      postprocess_func,
                             int32_t                                    kernel_params_size,
                             void *priv_arg)
{
    uint32_t i;
    vx_status status;

    if ((NULL != target_kernel) &&
        (create_in_bam_func != NULL) && (get_node_port_func != NULL))
    {
        status = tivxMutexLock(g_target_kernel_lock);

        if ((vx_status)VX_SUCCESS == status)
        {
            for(i=0; i<dimof(g_target_kernel_table); i++)
            {
                if (target_kernel ==
                    &g_target_kernel_table[i])
                {
                    g_target_kernel_table[i].create_in_bam_func = create_in_bam_func;
                    g_target_kernel_table[i].get_node_port_func = get_node_port_func;
                    g_target_kernel_table[i].append_internal_edges_func = append_internal_edges_func;
                    g_target_kernel_table[i].preprocess_func = preprocess_func;
                    g_target_kernel_table[i].postprocess_func = postprocess_func;
                    g_target_kernel_table[i].kernel_params_size = kernel_params_size;
                    break;
                }
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters given to function\n");
    }

    return status;
}

#endif
