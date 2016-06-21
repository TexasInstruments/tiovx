/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

static tivx_module_t modules[TIVX_MAX_MODULES];

static void ownCheckAndInitModule()
{
    static vx_bool is_init = vx_false_e;
    uint32_t idx;

    if(!is_init)
    {
        for(idx=0; idx<dimof(modules); idx++)
        {
            modules[idx].publish = NULL;
            modules[idx].unpublish = NULL;
        }
        is_init = vx_true_e;
    }
}

uint32_t ownGetModuleCount()
{
    uint32_t count=0, idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(modules); idx++)
    {
        if( modules[idx].publish != NULL
            &&
            modules[idx].unpublish != NULL
          )
        {
            count++;
        }
    }
    return count;
}

VX_API_ENTRY vx_status VX_API_CALL vxRegisterModule(char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish)
{
    uint32_t idx;
    vx_status status = VX_FAILURE;

    ownCheckAndInitModule();

    if(publish != NULL && unpublish != NULL)
    {
        for(idx=0; idx<dimof(modules); idx++)
        {
            if(modules[idx].publish == NULL
                &&
                modules[idx].unpublish == NULL
              )
            {
                strncpy(modules[idx].name, name, TIVX_MAX_MODULE_NAME);
                modules[idx].publish = publish;
                modules[idx].unpublish = unpublish;
                status = VX_SUCCESS;
                break;
            }
        }
        if(idx>=dimof(modules))
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

VX_API_ENTRY vx_status vxUnRegisterModule(char *name)
{
    vx_status status = VX_FAILURE;
    uint32_t idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(modules); idx++)
    {
        if( modules[idx].publish != NULL
            &&
            modules[idx].unpublish != NULL
            &&
            (strncmp(modules[idx].name, name, TIVX_MAX_MODULE_NAME) == 0)
          )
        {
            modules[idx].publish = NULL;
            modules[idx].unpublish = NULL;
            status = VX_SUCCESS;
            break;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    uint32_t idx;
    vx_status status = VX_FAILURE;

    ownCheckAndInitModule();
    for(idx=0; idx<dimof(modules); idx++)
    {
        if( modules[idx].publish != NULL
            &&
            modules[idx].unpublish != NULL
            &&
            (strncmp(modules[idx].name, module, TIVX_MAX_MODULE_NAME) == 0)
          )
        {
            status = modules[idx].publish(context);
        }
    }
    if(idx>=dimof(modules))
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
    for(idx=0; idx<dimof(modules); idx++)
    {
        if( modules[idx].publish != NULL
            &&
            modules[idx].unpublish != NULL
            &&
            (strncmp(modules[idx].name, module, TIVX_MAX_MODULE_NAME) == 0)
          )
        {
            status = modules[idx].unpublish(context);
        }
    }
    if(idx>=dimof(modules))
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}
