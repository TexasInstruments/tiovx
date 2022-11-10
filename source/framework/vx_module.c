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

static void ownCheckAndInitModule(void);

static tivx_module_t g_module_table[TIVX_MODULE_MAX];

static void ownCheckAndInitModule(void)
{
    static vx_bool is_init = (vx_bool)(vx_bool)vx_false_e;
    uint32_t idx;

    if(!(is_init != (vx_bool)(vx_bool)vx_false_e))
    {
        for(idx=0; idx<dimof(g_module_table); idx++)
        {
            g_module_table[idx].publish = NULL;
            g_module_table[idx].unpublish = NULL;
            g_module_table[idx].is_loaded = (vx_bool)vx_false_e;
        }
        is_init = (vx_bool)vx_true_e;
    }
}

uint32_t ownGetModuleCount(void)
{
    uint32_t count=0, idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( (g_module_table[idx].publish != NULL)
            &&
            (g_module_table[idx].unpublish != NULL)
            &&
            (g_module_table[idx].is_loaded != (vx_bool)vx_false_e)
          )
        {
            count++;
        }
    }
    return count;
}

VX_API_ENTRY vx_status VX_API_CALL tivxRegisterModule(const char *name, vx_publish_kernels_f publish, vx_unpublish_kernels_f unpublish)
{
    uint32_t idx;
    vx_status status = (vx_status)VX_FAILURE;

    ownCheckAndInitModule();

    if((publish != NULL) && (unpublish != NULL))
    {
        for(idx=0; idx<dimof(g_module_table); idx++)
        {
            if((g_module_table[idx].publish == NULL)
                &&
                (g_module_table[idx].unpublish == NULL)
              )
            {
                strncpy(g_module_table[idx].name, name, TIVX_MODULE_MAX_NAME-1U);
                g_module_table[idx].name[TIVX_MODULE_MAX_NAME-1U] = '\0';
                g_module_table[idx].publish = publish;
                g_module_table[idx].unpublish = unpublish;
                ownLogResourceAlloc("TIVX_MODULE_MAX", 1);
                status = (vx_status)VX_SUCCESS;
                break;
            }
        }
        if(idx>=dimof(g_module_table))
        {
            VX_PRINT(VX_ZONE_ERROR, "Module table is full\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_MODULE_MAX in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Publish and/or unpublish are NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxUnRegisterModule(const char *name)
{
    vx_status status = (vx_status)VX_FAILURE;
    uint32_t idx;

    ownCheckAndInitModule();

    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( (g_module_table[idx].publish != NULL)
            &&
            (g_module_table[idx].unpublish != NULL)
            &&
            (strncmp(g_module_table[idx].name, name, TIVX_MODULE_MAX_NAME) == 0)
          )
        {
            g_module_table[idx].publish = NULL;
            g_module_table[idx].unpublish = NULL;
            status = (vx_status)VX_SUCCESS;
            break;
        }
    }

    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to unregister module [%s]\n", name);
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxLoadKernels(vx_context context, const vx_char *module)
{
    uint32_t idx, kernels_loaded = 0;
    vx_status status = (vx_status)VX_SUCCESS;

    ownCheckAndInitModule();
    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( (g_module_table[idx].publish != NULL)
            &&
            (g_module_table[idx].unpublish != NULL)
            &&
            (strncmp(g_module_table[idx].name, module, TIVX_MODULE_MAX_NAME) == 0)
          )
        {
            status = g_module_table[idx].publish(context);

            if ((vx_status)VX_SUCCESS == status)
            {
                g_module_table[idx].is_loaded = (vx_bool)vx_true_e;
                kernels_loaded ++;
                break;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Publish function for module %s failed\n", module);
            }
        }
    }
    if((idx>=dimof(g_module_table)) && (0U == kernels_loaded))
    {
        if ((vx_status)VX_SUCCESS == status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernels can not be loaded since module %s has not yet been regstered.  Call tivxRegisterModule first.\n", module);
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxUnloadKernels(vx_context context, const vx_char *module)
{
    uint32_t idx;
    vx_status status = (vx_status)VX_FAILURE;

    ownCheckAndInitModule();
    for(idx=0; idx<dimof(g_module_table); idx++)
    {
        if( (g_module_table[idx].publish != NULL)
            &&
            (g_module_table[idx].unpublish != NULL)
            &&
            (strncmp(g_module_table[idx].name, module, TIVX_MODULE_MAX_NAME) == 0)
            &&
            (g_module_table[idx].is_loaded != (vx_bool)vx_false_e)
          )
        {
            status = g_module_table[idx].unpublish(context);

            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Unublish function for module %s failed\n", module);
            }
            g_module_table[idx].is_loaded = (vx_bool)vx_false_e;
            break;
        }
    }
    if(idx>=dimof(g_module_table))
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to unload kernels for module %s\n", module);
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}
