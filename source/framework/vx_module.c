/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

static tivx_module_t g_module_table[TIVX_MODULE_MAX];

static void ownCheckAndInitModule()
{
    static vx_bool is_init = vx_false_e;
    uint32_t idx;

    if(!is_init)
    {
        for(idx=0; idx<dimof(g_module_table); idx++)
        {
            g_module_table[idx].publish = NULL;
            g_module_table[idx].unpublish = NULL;
            g_module_table[idx].is_loaded = vx_false_e;
        }
        is_init = vx_true_e;
    }
}

uint32_t ownGetModuleCount()
{
    uint32_t count=0, idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( g_module_table[idx].publish != NULL
            &&
            g_module_table[idx].unpublish != NULL
            &&
            g_module_table[idx].is_loaded
          )
        {
            count++;
        }
    }
    return count;
}

VX_API_ENTRY vx_status VX_API_CALL tivxRegisterModule(char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish)
{
    uint32_t idx;
    vx_status status = VX_FAILURE;

    ownCheckAndInitModule();

    if(publish != NULL && unpublish != NULL)
    {
        for(idx=0; idx<dimof(g_module_table); idx++)
        {
            if(g_module_table[idx].publish == NULL
                &&
                g_module_table[idx].unpublish == NULL
              )
            {
                strncpy(g_module_table[idx].name, name, TIVX_MODULE_MAX_NAME);
                g_module_table[idx].publish = publish;
                g_module_table[idx].unpublish = unpublish;
                status = VX_SUCCESS;
                break;
            }
        }
        if(idx>=dimof(g_module_table))
        {
            status = VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(char *name)
{
    vx_status status = VX_FAILURE;
    uint32_t idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( g_module_table[idx].publish != NULL
            &&
            g_module_table[idx].unpublish != NULL
            &&
            (strncmp(g_module_table[idx].name, name, TIVX_MODULE_MAX_NAME) == 0)
          )
        {
            g_module_table[idx].publish = NULL;
            g_module_table[idx].unpublish = NULL;
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    uint32_t idx, kernels_loaded = 0;
    vx_status status = VX_FAILURE;

    ownCheckAndInitModule();
    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( g_module_table[idx].publish != NULL
            &&
            g_module_table[idx].unpublish != NULL
            &&
            (strncmp(g_module_table[idx].name, module, TIVX_MODULE_MAX_NAME) == 0)
          )
        {
            status = g_module_table[idx].publish(context);

            if (VX_SUCCESS == status)
            {
                g_module_table[idx].is_loaded = vx_true_e;
                kernels_loaded ++;
                break;
            }
        }
    }
    if((idx>=dimof(g_module_table)) && (0 == kernels_loaded))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnloadKernels(vx_context context, const vx_char *module)
{
    uint32_t idx;
    vx_status status = VX_FAILURE;

    ownCheckAndInitModule();
    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( g_module_table[idx].publish != NULL
            &&
            g_module_table[idx].unpublish != NULL
            &&
            (strncmp(g_module_table[idx].name, module, TIVX_MODULE_MAX_NAME) == 0)
            &&
            g_module_table[idx].is_loaded
          )
        {
            status = g_module_table[idx].unpublish(context);
            g_module_table[idx].is_loaded = vx_false_e;
            break;
        }
    }
    if(idx>=dimof(g_module_table))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}
