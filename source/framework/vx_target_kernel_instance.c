/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

static tivx_target_kernel_instance_t g_target_kernel_instance_table[TIVX_TARGET_KERNEL_INSTANCE_MAX];
static tivx_mutex g_target_kernel_instance_lock;

vx_status tivxTargetKernelInstanceInit()
{
    uint16_t i;
    vx_status status;

    for(i=0; i<dimof(g_target_kernel_instance_table); i++)
    {
        g_target_kernel_instance_table[i].kernel_id = TIVX_TARGET_KERNEL_ID_INVALID;
        g_target_kernel_instance_table[i].index = i;
    }

    status = tivxMutexCreate(&g_target_kernel_instance_lock);

    return status;
}

void tivxTargetKernelInstanceDeInit()
{
    tivxMutexDelete(&g_target_kernel_instance_lock);
}

tivx_target_kernel_instance tivxTargetKernelInstanceAlloc(vx_enum kernel_id, vx_enum target_id)
{
    uint16_t i;
    tivx_target_kernel_instance kernel_instance = NULL, tmp_kernel_instance = NULL;
    tivx_target_kernel kernel;
    vx_status status;

    kernel = tivxTargetKernelGet(kernel_id, target_id);

    if(kernel==NULL)
    {
        /* there is no kernel registered with this kernel ID on this CPU, hence return
        NULL */
    }
    else
    {
        status = tivxMutexLock(g_target_kernel_instance_lock);

        if(status == VX_SUCCESS)
        {
            for(i=0; i<dimof(g_target_kernel_instance_table); i++)
            {
                tmp_kernel_instance = &g_target_kernel_instance_table[i];
                if(tmp_kernel_instance->kernel_id == TIVX_TARGET_KERNEL_ID_INVALID)
                {
                    /* free entry found */
                    tmp_kernel_instance->kernel_id = kernel_id;
                    tmp_kernel_instance->index = i;
                    tmp_kernel_instance->kernel_context = NULL;
                    tmp_kernel_instance->kernel_context_size = 0;
                    tmp_kernel_instance->kernel = kernel;
                    tmp_kernel_instance->priv_args = NULL;

                    kernel_instance = tmp_kernel_instance;
                    break;
                }
            }
            tivxMutexUnlock(g_target_kernel_instance_lock);
        }
    }
    return kernel_instance;
}

vx_status tivxTargetKernelInstanceFree(tivx_target_kernel_instance *target_kernel_instance)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(target_kernel_instance && *target_kernel_instance)
    {
        (*target_kernel_instance)->kernel_id = TIVX_TARGET_KERNEL_ID_INVALID;
        *target_kernel_instance = NULL;

        status = VX_SUCCESS;
    }

    return status;
}

uint32_t tivxTargetKernelInstanceGetIndex(tivx_target_kernel_instance target_kernel_instance)
{
    uint32_t index = TIVX_TARGET_KERNEL_INSTANCE_MAX;

    if(target_kernel_instance &&
        target_kernel_instance->index < TIVX_TARGET_KERNEL_INSTANCE_MAX)
    {
        index = target_kernel_instance->index;
    }

    return index;
}

tivx_target_kernel tivxTargetKernelInstanceGetKernel(tivx_target_kernel_instance target_kernel_instance)
{
    tivx_target_kernel kernel = NULL;

    if(target_kernel_instance
        && target_kernel_instance->kernel_id != TIVX_TARGET_KERNEL_ID_INVALID)
    {
        kernel = target_kernel_instance->kernel;
    }

    return kernel;
}

tivx_target_kernel_instance tivxTargetKernelInstanceGet(uint16_t target_kernel_index, vx_enum kernel_id)
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
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(target_kernel_instance)
    {
        target_kernel_instance->kernel_context = kernel_context;
        target_kernel_instance->kernel_context_size = kernel_context_size;

        status = VX_SUCCESS;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxGetTargetKernelInstanceContext(
            tivx_target_kernel_instance target_kernel_instance,
            void **kernel_context,
            uint32_t *kernel_context_size)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;

    if(target_kernel_instance)
    {
        *kernel_context = target_kernel_instance->kernel_context;
        *kernel_context_size = target_kernel_instance->kernel_context_size;

        status = VX_SUCCESS;
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

VX_API_ENTRY void tivxSetTargetKernelInstanceCustomArgs(
    tivx_target_kernel_instance target_kernel_instance,
    void *priv_args)
{
    if (NULL != target_kernel_instance)
    {
        target_kernel_instance->priv_args = priv_args;
    }
}

VX_API_ENTRY void *tivxGetTargetKernelInstanceCustomArgs(
    tivx_target_kernel_instance target_kernel_instance)
{
    void *ret_val = NULL;

    if (NULL != target_kernel_instance)
    {
        ret_val = target_kernel_instance->priv_args;
    }

    return (ret_val);
}

