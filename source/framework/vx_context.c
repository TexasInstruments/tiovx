/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */



#include <vx_internal.h>

static vx_bool ownIsValidBorderMode(vx_enum mode);
static vx_status ownContextGetUniqueKernels( vx_context context, vx_kernel_info_t *kernel_info, uint32_t max_kernels);
static vx_status ownContextCreateCmdObj(vx_context context);
static vx_status ownContextDeleteCmdObj(vx_context context);


static const vx_char g_context_implmentation_name[VX_MAX_IMPLEMENTATION_NAME] = "tiovx";

static const vx_char g_context_default_load_module[][TIVX_MODULE_MAX_NAME] = {TIVX_MODULE_NAME_OPENVX_CORE};

static const vx_char g_context_extensions[] = " ";

static vx_context g_context_handle = NULL;

static tivx_context_t g_context_obj;

static vx_bool ownIsValidBorderMode(vx_enum mode)
{
    vx_bool ret = (vx_bool)vx_true_e;
    switch (mode)
    {
        case (vx_enum)VX_BORDER_UNDEFINED:
        case (vx_enum)VX_BORDER_CONSTANT:
        case (vx_enum)VX_BORDER_REPLICATE:
            break;
        default:
            ret = (vx_bool)vx_false_e;
            break;
    }
    return ret;
}

/*
 * \brief Fill 'kernel_info' with valid unique kernels info from this context
 *
 *        If more than 'max_kernels' found, the return with error
 */
static vx_status ownContextGetUniqueKernels( vx_context context, vx_kernel_info_t *kernel_info, uint32_t max_kernels)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_kernel kernel;
    uint32_t num_kernel_info = 0, idx;

    if( ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        ownContextLock(context);

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            kernel = context->kerneltable[idx];
            if(ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
            {
                kernel_info[num_kernel_info].enumeration = kernel->enumeration;
                strncpy(kernel_info[num_kernel_info].name, kernel->name, VX_MAX_KERNEL_NAME);
                num_kernel_info++;
            }
            if(num_kernel_info > max_kernels)
            {
                VX_PRINT(VX_ZONE_ERROR,"num kernel info is greater than max kernels\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
                break;
            }
        }

        ownContextUnlock(context);
    }

    return status;
}

static vx_status ownContextCreateCmdObj(vx_context context)
{
    vx_status status = (vx_status)VX_SUCCESS;

    context->obj_desc_cmd = NULL;
    context->cmd_ack_event = NULL;

    context->obj_desc_cmd = (tivx_obj_desc_cmd_t*)tivxObjDescAlloc((vx_enum)TIVX_OBJ_DESC_CMD, NULL);
    if(context->obj_desc_cmd != NULL)
    {
        status = tivxEventCreate(&context->cmd_ack_event);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"context object descriptor allocation failed\n");
        status = (vx_status)VX_ERROR_NO_RESOURCES;
    }

    return status;
}

static vx_status ownContextDeleteCmdObj(vx_context context)
{
    vx_status status = (vx_status)VX_SUCCESS, status1 = (vx_status)VX_SUCCESS, status2 = (vx_status)VX_SUCCESS;

    if(context->obj_desc_cmd != NULL)
    {
        status1 = tivxObjDescFree((tivx_obj_desc_t**)&context->obj_desc_cmd);
    }
    if(context->cmd_ack_event != NULL)
    {
        status2 = tivxEventDelete(&context->cmd_ack_event);
    }
    if(status1 != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,"Context memory free-ing failed\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (status2 != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,"Context event deletion failed\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing */
    }

    return status;
}

vx_status ownContextLock(vx_context context)
{
    return tivxMutexLock(context->lock);
}

vx_status ownContextUnlock(vx_context context)
{
    return tivxMutexUnlock(context->lock);
}

vx_bool ownAddReferenceToContext(vx_context context, vx_reference ref)
{
    uint32_t ref_idx;
    vx_bool is_success = (vx_bool)vx_false_e;

    if (ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        ownContextLock(context);

        for(ref_idx=0; ref_idx < dimof(context->reftable); ref_idx++)
        {
            if(context->reftable[ref_idx]==NULL)
            {
                char name[VX_MAX_REFERENCE_NAME];

                context->reftable[ref_idx] = ref;
                context->num_references++;
                is_success = (vx_bool)vx_true_e;

                tivxLogResourceAlloc("TIVX_CONTEXT_MAX_REFERENCES", 1);

                switch(ref->type)
                {
                    case (vx_enum)VX_TYPE_DELAY:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "delay_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_LUT:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "lut_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_DISTRIBUTION:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "distribution_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_PYRAMID:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "pyramid_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_THRESHOLD:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "threshold_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_MATRIX:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "matrix_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_CONVOLUTION:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "convolution_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_SCALAR:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "scalar_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_ARRAY:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "array_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_IMAGE:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "image_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_REMAP:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "remap_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_OBJECT_ARRAY:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "object_array_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_NODE:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "node_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_GRAPH:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "graph_%d", ref_idx);
                        break;
                    case (vx_enum)TIVX_TYPE_DATA_REF_Q:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "data_ref_q_%d", ref_idx);
                        break;
                    case (vx_enum)VX_TYPE_TENSOR:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "tensor_%d", ref_idx);
                        break;
                    case VX_TYPE_USER_DATA_OBJECT:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "user_data_object_%d", ref_idx);
                        break;
                    case TIVX_TYPE_RAW_IMAGE:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "raw_image_%d", ref_idx);
                        break;
                    case TIVX_TYPE_SUPER_NODE:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "super_node_%d", ref_idx);
                        break;
                    default:
                        snprintf(name, VX_MAX_REFERENCE_NAME, "ref_%d", ref_idx);
                        break;
                }
                vxSetReferenceName(ref, name);

                break;
            }
        }

        ownContextUnlock(context);
    }
    return is_success;
}

vx_bool ownRemoveReferenceFromContext(vx_context context, vx_reference ref)
{
    uint32_t ref_idx;
    vx_bool is_success = (vx_bool)vx_false_e;

    if (ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        ownContextLock(context);

        for(ref_idx=0; ref_idx < dimof(context->reftable); ref_idx++)
        {
            if(context->reftable[ref_idx]==ref)
            {
                context->reftable[ref_idx] = NULL;
                context->num_references--;
                is_success = (vx_bool)vx_true_e;
                tivxLogResourceFree("TIVX_CONTEXT_MAX_REFERENCES", 1);
                break;
            }
        }

        ownContextUnlock(context);
    }
    return is_success;
}

vx_bool ownIsValidContext(vx_context context)
{
    vx_bool ret = (vx_bool)vx_false_e;
    if ((context != NULL) &&
        (context->base.magic == TIVX_MAGIC) &&
        (context->base.type == (vx_enum)VX_TYPE_CONTEXT) &&
        (context->base.context == NULL))
    {
        ret = (vx_bool)vx_true_e; /* this is the top level context */
    }
    return ret;
}

vx_status ownAddKernelToContext(vx_context context, vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;

    if(ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid context\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else if (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Kernel reference is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        ownContextLock(context);

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            if ((NULL == context->kerneltable[idx]) && (context->num_unique_kernels < dimof(context->kerneltable)))
            {
                /* found free entry */
                context->kerneltable[idx] = kernel;
                context->num_unique_kernels++;
                ownIncrementReference(&kernel->base, (vx_enum)VX_INTERNAL);
                tivxLogResourceAlloc("TIVX_CONTEXT_MAX_KERNELS", 1);
                break;
            }
        }
        if(idx>=dimof(context->kerneltable))
        {
            /* free entry not found */
            VX_PRINT(VX_ZONE_ERROR,"free entry not found\n");
            VX_PRINT(VX_ZONE_ERROR, "ownAddKernelToContext: May need to increase the value of TIVX_CONTEXT_MAX_KERNELS in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }

        ownContextUnlock(context);
    }

    return status;
}

vx_status ownRemoveKernelFromContext(vx_context context, vx_kernel kernel)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;

    if(ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid context\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else if (ownIsValidSpecificReference(&kernel->base, (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Kernel reference is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        ownContextLock(context);

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            if( (context->kerneltable[idx]==kernel) && (context->num_unique_kernels>0U) )
            {
                /* found free entry */
                context->kerneltable[idx] = NULL;
                context->num_unique_kernels--;
                ownDecrementReference(&kernel->base, (vx_enum)VX_INTERNAL);
                tivxLogResourceFree("TIVX_CONTEXT_MAX_KERNELS", 1);
                break;
            }
        }
        if(idx>=dimof(context->kerneltable))
        {
            /* kernel not found */
            VX_PRINT(VX_ZONE_ERROR,"kernel not found\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }

        ownContextUnlock(context);
    }

    return status;
}

vx_status ownIsKernelInContext(vx_context context, vx_enum enumeration, const vx_char string[VX_MAX_KERNEL_NAME], vx_bool *is_found)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;
    vx_kernel kernel;

    if( (ownIsValidContext(context) == (vx_bool)vx_false_e) || (is_found == NULL) )
    {
        VX_PRINT(VX_ZONE_ERROR,"invalid context\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        ownContextLock(context);

        *is_found = (vx_bool)vx_false_e;

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            kernel = context->kerneltable[idx];
            if((NULL != kernel) &&
                (ownIsValidSpecificReference( &kernel->base, (vx_enum)VX_TYPE_KERNEL) ==
                    (vx_bool)vx_true_e)
                &&
                ( (strncmp(kernel->name, string, VX_MAX_KERNEL_NAME) == 0)
                    ||
                    (kernel->enumeration == enumeration)
                )
                )
            {
                /* found match */
                *is_found = (vx_bool)vx_true_e;
                break;
            }
        }

        ownContextUnlock(context);
    }

    return status;
}


vx_status ownContextSendControlCmd(vx_context context, uint16_t node_obj_desc,
    uint32_t target_id, uint32_t replicated_node_idx, uint32_t node_cmd_id,
    const uint16_t obj_desc_id[], uint32_t num_obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    tivx_obj_desc_cmd_t *obj_desc_cmd;

    if((ownIsValidContext(context) == (vx_bool)vx_true_e) &&
       (num_obj_desc < TIVX_CMD_MAX_OBJ_DESCS))
    {
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

        ownContextLock(context);

        tivx_uint64_to_uint32(
            timestamp,
            &context->obj_desc_cmd->timestamp_h,
            &context->obj_desc_cmd->timestamp_l
        );

        /* alloc obj desc for kernel name */
        obj_desc_cmd = context->obj_desc_cmd;

        obj_desc_cmd->cmd_id = (vx_enum)TIVX_CMD_NODE_CONTROL;
        obj_desc_cmd->dst_target_id = target_id;
        obj_desc_cmd->src_target_id =
            (uint32_t)tivxPlatformGetTargetId(TIVX_TARGET_HOST);
        obj_desc_cmd->num_obj_desc = 1u;
        obj_desc_cmd->obj_desc_id[0u] = node_obj_desc;
        obj_desc_cmd->flags = TIVX_CMD_FLAG_SEND_ACK;
        obj_desc_cmd->ack_event_handle = (uint64_t)(uintptr_t)context->cmd_ack_event;

        obj_desc_cmd->replicated_node_idx = (int32_t)replicated_node_idx;
        obj_desc_cmd->node_cmd_id = node_cmd_id;
        obj_desc_cmd->num_cmd_params = num_obj_desc;
        for (i = 0; i < num_obj_desc; i ++)
        {
            obj_desc_cmd->cmd_params_desc_id[i] = obj_desc_id[i];
        }

        status = tivxObjDescSend(target_id, obj_desc_cmd->base.obj_desc_id);

        if(status == (vx_status)VX_SUCCESS)
        {
            status = tivxEventWait(context->cmd_ack_event,
                TIVX_EVENT_TIMEOUT_WAIT_FOREVER);
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            if ((vx_status)VX_SUCCESS != (vx_status)obj_desc_cmd->cmd_status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "ownContextSendControlCmd: Failed to send object desc\n");
                status = (vx_status)VX_FAILURE;
            }
        }

        ownContextUnlock(context);
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        if (num_obj_desc >= TIVX_CMD_MAX_OBJ_DESCS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "ownContextSendControlCmd: Invalid Number of object desc\n");
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "ownContextSendControlCmd: Invalid Context\n");
        }
    }

    return status;
}

vx_status ownContextSendCmd(vx_context context, uint32_t target_id, uint32_t cmd, uint32_t num_obj_desc, const uint16_t *obj_desc_id)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;

    if( (ownIsValidContext(context) == (vx_bool)vx_true_e) && (num_obj_desc < TIVX_CMD_MAX_OBJ_DESCS) )
    {
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

        ownContextLock(context);

        tivx_uint64_to_uint32(
            timestamp,
            &context->obj_desc_cmd->timestamp_h,
            &context->obj_desc_cmd->timestamp_l
        );

        context->obj_desc_cmd->cmd_id = cmd;
        context->obj_desc_cmd->dst_target_id = target_id;
        context->obj_desc_cmd->src_target_id = (uint32_t)tivxPlatformGetTargetId(TIVX_TARGET_HOST);
        context->obj_desc_cmd->num_obj_desc = num_obj_desc;
        context->obj_desc_cmd->flags = TIVX_CMD_FLAG_SEND_ACK;
        context->obj_desc_cmd->ack_event_handle = (uint64_t)(uintptr_t)context->cmd_ack_event;

        for(i=0; i<num_obj_desc; i++)
        {
            context->obj_desc_cmd->obj_desc_id[i] = obj_desc_id[i];
        }

        status = tivxObjDescSend(target_id, context->obj_desc_cmd->base.obj_desc_id);

        if(status == (vx_status)VX_SUCCESS)
        {
            status = tivxEventWait(context->cmd_ack_event, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

            if(status == (vx_status)VX_SUCCESS)
            {
                if ((vx_status)VX_SUCCESS != (vx_status)context->obj_desc_cmd->cmd_status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Command ack message returned failure cmd_status: %d\n", context->obj_desc_cmd->cmd_status);
                    status = (vx_status)VX_FAILURE;
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"sending object descriptor failed\n");
        }

        ownContextUnlock(context);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"invalid parameters\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext(void)
{
    vx_context context = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;

    tivxPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    {
        if (g_context_handle == NULL)
        {
            context = &g_context_obj;

            memset(context, 0, sizeof(tivx_context_t));

            context->imm_border.mode = (vx_enum)VX_BORDER_UNDEFINED;
            context->imm_border_policy = (vx_enum)VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED;
            context->next_dynamic_user_kernel_id = 0;
            context->next_dynamic_user_library_id = 1;
            context->perf_enabled = (vx_bool)vx_false_e;
            context->imm_target_enum = (vx_enum)VX_TARGET_ANY;
            memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
            context->num_references = 0;
            for(idx=0; idx<dimof(context->reftable); idx++)
            {
                context->reftable[idx] = NULL;
            }
            for(idx=0; idx<dimof(context->user_structs); idx++)
            {
                context->user_structs[idx].type = (vx_enum)VX_TYPE_INVALID;
            }
            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                context->kerneltable[idx] = NULL;
            }
            context->num_unique_kernels = 0;
            context->log_enabled = (vx_bool)vx_false_e;
            context->base.release_callback =
                (tivx_reference_release_callback_f)&vxReleaseContext;

            status = tivxMutexCreate(&context->lock);
            if(status==(vx_status)VX_SUCCESS)
            {
                status = tivxMutexCreate(&context->log_lock);
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                status = tivxEventQueueCreate(&context->event_queue);
            }
            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownInitReference(&context->base, NULL, (vx_enum)VX_TYPE_CONTEXT, NULL);
                if(status==(vx_status)VX_SUCCESS)
                {
                    status = ownContextCreateCmdObj(context);
                    if(status == (vx_status)VX_SUCCESS)
                    {
                        ownIncrementReference(&context->base, (vx_enum)VX_EXTERNAL);
                        ownCreateConstErrors(context);
                        g_context_handle = context;
                    }
                }
                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"context objection creation failed\n");
                    tivxMutexDelete(&context->lock);
                    tivxMutexDelete(&context->log_lock);
                }
            }
            if(status!=(vx_status)VX_SUCCESS)
            {
                /* some error context cannot be created */
                context = NULL;
            }

            if(status == (vx_status)VX_SUCCESS)
            {
                /* set flag to disallow removal of built kernels
                 * via remove kernel API
                 */
                ownContextSetKernelRemoveLock(context, (vx_bool)vx_true_e);

                for (idx = 0;
                     idx < (sizeof(g_context_default_load_module)/sizeof(g_context_default_load_module[0]));
                     idx ++)
                {
                    /* this loads default module kernels
                     * Any additional modules should be loaded by the user using
                     * vxLoadKernels()
                     * Error's are not checked here,
                     * User can check kernels that are added using vxQueryContext()
                     */
                    vxLoadKernels(context, g_context_default_load_module[idx]);
                }

                /* set flag to allow removal additional kernels
                 * installed by user via remove kernel API
                 */
                ownContextSetKernelRemoveLock(context, (vx_bool)vx_false_e);
            }
        }
        else
        {
            context = g_context_handle;
            ownIncrementReference(&context->base, (vx_enum)VX_EXTERNAL);
        }
    }
    tivxPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    return context;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *c)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_context context;
    vx_uint32 r;
    uint32_t idx;

    if(c != NULL)
    {
        context = *c;
    }
    else
    {
        context = 0;
    }

    if (c != NULL)
    {
        *c = 0;
    }
    tivxPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);
    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        if (ownDecrementReference(&context->base, (vx_enum)VX_EXTERNAL) == 0U)
        {
            ownContextSetKernelRemoveLock(context, (vx_bool)vx_true_e);

            for (idx = 0;
                 idx < (sizeof(g_context_default_load_module)/sizeof(g_context_default_load_module[0]));
                 idx ++)
            {
            /* Unload kernels */
            vxUnloadKernels(context, g_context_default_load_module[idx]);
            }

            ownContextSetKernelRemoveLock(context, (vx_bool)vx_false_e);

            /* Deregister any log callbacks if there is any registered */
            vxRegisterLogCallback(context, NULL, (vx_bool)vx_false_e);

            /*! \internal Garbage Collect All References */
            /* Details:
             *   1. This loop will warn of references which have not been released by the user.
             *   2. It will close all internally opened error references.
             *   3. It will close the external references, which in turn will internally
             *      close any internally dependent references that they reference, assuming the
             *      reference counting has been done properly in the framework.
             *   4. This garbage collection must be done before the targets are released since some of
             *      these external references may have internal references to target kernels.
             */
            for (r = 0; r < dimof(context->reftable); r++)
            {
                vx_reference ref = context->reftable[r];

                /* Warnings should only come when users have not released all external references */
                if ((NULL != ref) && (ref->external_count > 0U) ) {
                    VX_PRINT(VX_ZONE_WARNING,"Stale reference "VX_FMT_REF" of type %08x at external count %u, internal count %u\n",
                             ref, ref->type, ref->external_count, ref->internal_count);
                    if (NULL != ref->name)
                    {
                        VX_PRINT(VX_ZONE_WARNING,"Unreleased reference name = %s\n", ref->name);
                    }
                }

                /* These were internally opened during creation, so should internally close ERRORs */
                if((NULL != ref) && (ref->type == (vx_enum)VX_TYPE_ERROR) ) {
                    ownReleaseReferenceInt(&ref, ref->type, (vx_enum)VX_INTERNAL, NULL);
                }

                /* Warning above so user can fix release external objects, but close here anyway */
                while ((NULL != ref)&& (ref->external_count > 1U) ) {
                    ownDecrementReference(ref, (vx_enum)VX_EXTERNAL);
                }
                if ((NULL != ref) && (ref->external_count > 0U) ) {
                    ownReleaseReferenceInt(&ref, ref->type, (vx_enum)VX_EXTERNAL, NULL);
                }
            }

            /* By now, all external and internal references should be removed */
            for (r = 0; r < dimof(context->reftable); r++)
            {
                if(context->reftable[r] != NULL)
                {
                        VX_PRINT(VX_ZONE_ERROR,"Reference %d not removed\n", r);
                }
            }

            ownContextDeleteCmdObj(context);

            tivxEventQueueDelete(&context->event_queue);

            /*! \internal wipe away the context memory first */
            /* Normally destroy sem is part of release reference, but can't for context */
            tivxMutexDelete(&context->base.lock);

            tivxMutexDelete(&context->log_lock);
            tivxMutexDelete(&context->lock);

            memset(context, 0, sizeof(tivx_context_t));

            g_context_handle = NULL;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    tivxPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute)
        {
            case (vx_enum)VX_CONTEXT_VENDOR_ID:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1U))
                {
                    *(vx_uint16 *)ptr = (vx_enum)VX_ID_TI;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context vendor ID failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_VERSION:
                if (VX_CHECK_PARAM(ptr, size, vx_uint16, 0x1U))
                {
                    *(vx_uint16 *)ptr = (vx_uint16)VX_VERSION;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context version failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_MODULES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = ownGetModuleCount();
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context modules failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_REFERENCES:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = context->num_references;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context references failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_IMPLEMENTATION:
                if (((int32_t)size <= VX_MAX_IMPLEMENTATION_NAME) && (NULL != ptr))
                {
                    strncpy(ptr, g_context_implmentation_name, VX_MAX_IMPLEMENTATION_NAME);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context implementation failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_EXTENSIONS_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = sizeof(g_context_extensions);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context extensions size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_EXTENSIONS:
                if ( (size <= sizeof(g_context_extensions) ) && ptr)
                {
                    strncpy(ptr, g_context_extensions, sizeof(g_context_extensions));
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context extensions failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_CONVOLUTION_MAX_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_CONVOLUTION_DIM;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context max convolution dimensions failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_MAX_TENSOR_DIMS:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_TENSOR_DIMS;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context max tensor dimensions failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_NONLINEAR_MAX_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_NONLINEAR_DIM;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context max nonlinear dimensions failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = TIVX_CONTEXT_MAX_OPTICALFLOWPYRLK_DIM;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context max optical flow window dimensions failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_IMMEDIATE_BORDER:
                if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3U))
                {
                    *(vx_border_t *)ptr = context->imm_border;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context immediate border failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_IMMEDIATE_BORDER_POLICY:
                if (VX_CHECK_PARAM(ptr, size, vx_enum, 0x3U))
                {
                    *(vx_enum *)ptr = context->imm_border_policy;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context immediate border policy failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_UNIQUE_KERNELS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = context->num_unique_kernels;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context unique kernels failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_CONTEXT_UNIQUE_KERNEL_TABLE:
                if ((size == (context->num_unique_kernels * sizeof(vx_kernel_info_t))) &&
                    (ptr != NULL))
                {
                    status = ownContextGetUniqueKernels( context, (vx_kernel_info_t*)ptr, context->num_unique_kernels);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context unique kernel table failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;
    if (ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        switch (attribute) {
            case (vx_enum)VX_CONTEXT_IMMEDIATE_BORDER:
                if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3U))
                {
                    const vx_border_t *config = (const vx_border_t *)ptr;
                    if (ownIsValidBorderMode(config->mode) == (vx_bool)vx_false_e)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"invalid border mode\n");
                        status = (vx_status)VX_ERROR_INVALID_VALUE;
                    }
                    else
                    {
                        context->imm_border = *config;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"set context immediate border mode failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"unsupported attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxDirective(vx_reference reference, vx_enum directive) {
    vx_status status = (vx_status)VX_SUCCESS;
    vx_context context;
    vx_enum ref_type;

    status = vxQueryReference(reference, (vx_enum)VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type));
    if (status == (vx_status)VX_SUCCESS)
    {
        if (ref_type == (vx_enum)VX_TYPE_CONTEXT)
        {
            context = (vx_context)reference;
        }
        else
        {
            context = reference->context;
        }
        if (ownIsValidContext(context) == (vx_bool)vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
        else
        {
            switch (directive)
            {
                case (vx_enum)VX_DIRECTIVE_DISABLE_LOGGING:
                    context->log_enabled = (vx_bool)vx_false_e;
                    break;
                case (vx_enum)VX_DIRECTIVE_ENABLE_LOGGING:
                    context->log_enabled = (vx_bool)vx_true_e;
                    break;
                case (vx_enum)VX_DIRECTIVE_DISABLE_PERFORMANCE:
                    if (ref_type == (vx_enum)VX_TYPE_CONTEXT)
                    {
                        context->perf_enabled = (vx_bool)vx_false_e;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"unsupported reference type\n");
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    }
                    break;
                case (vx_enum)VX_DIRECTIVE_ENABLE_PERFORMANCE:
                    if (ref_type == (vx_enum)VX_TYPE_CONTEXT)
                    {
                        context->perf_enabled = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"unsupported reference type\n");
                        status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    }
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,"unsupported directive type\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    break;
            }
        }
    }
    return status;
}

VX_API_ENTRY vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    vx_enum type = (vx_enum)VX_TYPE_INVALID;
    vx_uint32 i = 0;

    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) &&
        (size != 0U))
    {
        (void)ownContextLock(context);

        for (i = 0; i < TIVX_CONTEXT_MAX_USER_STRUCTS; ++i)
        {
            if (context->user_structs[i].type == (vx_enum)VX_TYPE_INVALID)
            {
                context->user_structs[i].type = (vx_enum)VX_TYPE_USER_STRUCT_START + (int32_t)i;
                context->user_structs[i].size = size;
                type = context->user_structs[i].type;
                tivxLogSetResourceUsedValue("TIVX_CONTEXT_MAX_USER_STRUCTS", (uint16_t)i+1U);
                break;
            }
        }

        if (type == (vx_enum)VX_TYPE_INVALID)
        {
            VX_PRINT(VX_ZONE_WARNING, "vxRegisterUserStruct: May need to increase the value of TIVX_CONTEXT_MAX_USER_STRUCTS in tiovx/include/TI/tivx_config.h\n");
        }

        (void)ownContextUnlock(context);
    }
    return type;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) && (NULL != pKernelEnumId))
    {
        (void)ownContextLock(context);

        status = (vx_status)VX_ERROR_NO_RESOURCES;
        if((int32_t)context->next_dynamic_user_kernel_id <= VX_KERNEL_MASK)
        {
            *pKernelEnumId = VX_KERNEL_BASE(VX_ID_USER, 0U) + (vx_enum)context->next_dynamic_user_kernel_id++;
            status = (vx_status)VX_SUCCESS;
        }

        (void)ownContextUnlock(context);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) && (NULL != pLibraryId))
    {
        ownContextLock(context);

        status = (vx_status)VX_ERROR_NO_RESOURCES;
        if(context->next_dynamic_user_library_id <= VX_LIBRARY(VX_LIBRARY_MASK))
        {
            *pLibraryId = (int32_t)context->next_dynamic_user_library_id;
            context->next_dynamic_user_library_id =
                context->next_dynamic_user_library_id + 1u;
            status = (vx_status)VX_SUCCESS;
        }

        ownContextUnlock(context);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        ownContextLock(context);

        switch (target_enum)
        {
            case (vx_enum)VX_TARGET_ANY:
                context->imm_target_enum = (vx_enum)VX_TARGET_ANY;
                memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
                status = (vx_status)VX_SUCCESS;
                break;

            case (vx_enum)VX_TARGET_STRING:
                if (target_string != NULL)
                {
                    context->imm_target_enum = (vx_enum)VX_TARGET_STRING;
                    strncpy(context->imm_target_string, target_string, sizeof(context->imm_target_string));
                    context->imm_target_string[sizeof(context->imm_target_string) - 1U] = '\0';
                    status = (vx_status)VX_SUCCESS;
                }
                else /* target was not found */
                {
                    VX_PRINT(VX_ZONE_ERROR,"target was not found\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                }
                break;

            default:
                VX_PRINT(VX_ZONE_ERROR,"unsupported target_enum\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }

        ownContextUnlock(context);
    }
    return status;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, const vx_char string[VX_MAX_KERNEL_NAME])
{
    vx_kernel kernel = NULL;
    uint32_t idx;

    if( ownIsValidContext(context) == (vx_bool)vx_false_e )
    {
        kernel = NULL;
    }
    else
    {
        ownContextLock(context);

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            kernel = context->kerneltable[idx];
            if( (NULL != kernel) && (ownIsValidSpecificReference( &kernel->base, (vx_enum)VX_TYPE_KERNEL) != (vx_bool)vx_false_e)
                &&
                ( strncmp(kernel->name, string, VX_MAX_KERNEL_NAME) == 0 )
                )
            {
                /* found match */
                ownIncrementReference(&kernel->base, (vx_enum)VX_EXTERNAL);
                break;
            }
        }
        if(idx>=dimof(context->kerneltable))
        {
            /* not found */
            kernel = NULL;
        }

        ownContextUnlock(context);
    }

    return kernel;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByEnum(vx_context context, vx_enum kernelenum)
{
    vx_kernel kernel = NULL;
    uint32_t idx;

    if( ownIsValidContext(context) == (vx_bool)vx_false_e )
    {
        kernel = NULL;
    }
    else
    {
        ownContextLock(context);

        for(idx=0; idx<dimof(context->kerneltable); idx++)
        {
            kernel = context->kerneltable[idx];
            if((NULL != kernel) &&
               (ownIsValidSpecificReference( &kernel->base, (vx_enum)VX_TYPE_KERNEL) != (vx_bool)vx_false_e)
                &&
                ( kernel->enumeration == kernelenum )
                )
            {
                /* found match */
                ownIncrementReference(&kernel->base, (vx_enum)VX_EXTERNAL);
                break;
            }
        }
        if(idx>=dimof(context->kerneltable))
        {
            /* not found */
            kernel = NULL;
        }

        ownContextUnlock(context);
    }

    return kernel;
}

vx_bool ownContextGetKernelRemoveLock(vx_context context)
{
    return context->remove_kernel_lock;
}

void ownContextSetKernelRemoveLock(vx_context context, vx_bool do_lock)
{
    context->remove_kernel_lock = do_lock;
}

