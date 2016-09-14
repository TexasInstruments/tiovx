/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

static tivx_target_kernel_t g_target_kernel_table[TIVX_TARGET_KERNEL_MAX];
static tivx_mutex g_target_kernel_lock;

vx_status tivxTargetKernelInit()
{
    uint32_t i;
    vx_status status;

    for(i=0; i<dimof(g_target_kernel_table); i++)
    {
        g_target_kernel_table[i].kernel_id = TIVX_TARGET_KERNEL_ID_INVALID;
    }

    status = tivxMutexCreate(&g_target_kernel_lock);

    return status;
}

void tivxTargetKernelInstanceDeInit()
{
    tivxMutexDelete(&g_target_kernel_lock);
}


VX_API_ENTRY tivx_target_kernel VX_API_CALL tivxAddTargetKernel(
                             vx_enum kernel_id,
                             char *target_name,
                             tivx_target_kernel_f process_func,
                             tivx_target_kernel_f create_func,
                             tivx_target_kernel_f delete_func,
                             tivx_target_kernel_f control_func)
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
                    g_target_kernel_table[i].target_id)
                {
                    g_target_kernel_table[i].kernel_id = kernel_id;
                    g_target_kernel_table[i].target_id =
                        tivxPlatformGetTargetId(target_name);
                    g_target_kernel_table[i].process_func = process_func;
                    g_target_kernel_table[i].create_func = create_func;
                    g_target_kernel_table[i].delete_func = delete_func;
                    g_target_kernel_table[i].control_func = control_func;

                    knl = &g_target_kernel_table[i];

                    break;
                }
            }

            tivxMutexUnlock(g_target_kernel_lock);
        }
    }

    return (knl);
}

tivx_target_kernel tivxTargetKernelGet(vx_enum kernel_id, vx_enum target_id)
{
    uint32_t i;
    tivx_target_kernel knl = NULL;
    vx_status status;

    status = tivxMutexLock(g_target_kernel_lock);

    if (VX_SUCCESS == status)
    {
        for(i=0; i<dimof(g_target_kernel_table); i++)
        {
            if ((kernel_id == g_target_kernel_table[i].kernel_id) &&
                (target_id == g_target_kernel_table[i].target_id))
            {
                knl = &g_target_kernel_table[i];
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
                target_kernel_instance, obj_desc, num_params);
        }
        else
        {
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
                target_kernel_instance, obj_desc, num_params);
        }
        else
        {
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
            status = knl->process_func(
                target_kernel_instance, obj_desc, num_params);
        }
        else
        {
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
                target_kernel_instance, obj_desc, num_params);
        }
        else
        {
            status = VX_FAILURE;
        }
    }

    return (status);
}

