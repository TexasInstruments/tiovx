/*
 * Copyright (c) 2012-2025 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM015 <justification end>*/
    if( ownIsValidContext(context) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM015	<justification end>*/
    else
/* LDRA_JUSTIFY_END */
    {
        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {
            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                kernel = context->kerneltable[idx];
                if(ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
                {
                    kernel_info[num_kernel_info].enumeration = kernel->enumeration;
                    (void)strncpy(kernel_info[num_kernel_info].name, kernel->name, VX_MAX_KERNEL_NAME-1);
                    kernel_info[num_kernel_info].name[VX_MAX_KERNEL_NAME-1] = '\0';
                    num_kernel_info++;
                }
                if(num_kernel_info > max_kernels)
                {
                    VX_PRINT(VX_ZONE_ERROR,"num kernel info is greater than max kernels\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    break;
                }
            }
            (void)ownContextUnlock(context);
        }
    }

    return status;
}

static vx_status ownContextCreateCmdObj(vx_context context)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t    i;
    vx_bool do_break = (vx_bool)vx_false_e;

    /* Create the free and pend queues. */
    status = tivxQueueCreate(&context->free_queue,
                             TIVX_MAX_CTRL_CMD_OBJECTS,
                             context->free_queue_memory,
                             TIVX_QUEUE_FLAG_BLOCK_ON_GET); /* blocking */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR001.
<justification end> */
    if (status == (vx_status)VX_SUCCESS)
    {
        status = tivxQueueCreate(&context->pend_queue,
                                 TIVX_MAX_CTRL_CMD_OBJECTS,
                                 context->pend_queue_memory,
                                 TIVX_QUEUE_FLAG_BLOCK_ON_GET); /* blocking */

        if (status!=(vx_status)VX_SUCCESS)
        {
            /* error status check is not done due to the
             * previous status check for tivxQueueCreate
             */
            (void)tivxQueueDelete(&context->free_queue);
        }
    }
/* LDRA_JUSTIFY_END */

    if (status == (vx_status)VX_SUCCESS)
    {
        /* Allocate {control object, event} pair. */
        for (i = 0; i < TIVX_MAX_CTRL_CMD_OBJECTS; i++)
        {
            context->obj_desc_cmd[i] = NULL;
            context->cmd_ack_event[i] = NULL;
        }

        for (i = 0; i < TIVX_MAX_CTRL_CMD_OBJECTS; i++)
        {
            context->obj_desc_cmd[i] = (tivx_obj_desc_cmd_t*)
                ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_CMD, NULL);

            if (context->obj_desc_cmd[i] != NULL)
            {
                status = tivxEventCreate(&context->cmd_ack_event[i]);

                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "context object event [%d] allocation failed\n", i);
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                    do_break = (vx_bool)vx_true_e;
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "context object descriptor [%d] allocation failed\n", i);
                status = (vx_status)VX_ERROR_NO_RESOURCES;
                VX_PRINT(VX_ZONE_ERROR, "context object descriptor [%d] allocation failed\n", i);
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available.\n");
                VX_PRINT_BOUND_ERROR("TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST");
                do_break = (vx_bool)vx_true_e;
            }
            if((vx_bool)vx_true_e == do_break)
            {
                break;
            }
        }

        if (status != (vx_status)VX_SUCCESS)
        {
            /* Release any allocated resources. */
            for (i = 0; i < TIVX_MAX_CTRL_CMD_OBJECTS; i++)
            {
                if (context->obj_desc_cmd[i] != NULL)
                {
                    status = ownObjDescFree((tivx_obj_desc_t**)&context->obj_desc_cmd[i]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TTIOVX_CODE_COVERAGE_CONTEXT_UM001.
<justification end> */
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to free object descriptor\n");
                    }
/* LDRA_JUSTIFY_END */
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT009.
<justification end> */
                if (context->cmd_ack_event[i] != NULL)
                {
                    /* Error status check is not done due to the previous check */
                    (void)tivxEventDelete(&context->cmd_ack_event[i]);
                }
/* LDRA_JUSTIFY_END */
            }

            /* Delete the queues. No error checks are being made since we know
             * that the queues have been created successfully above.
             */
            (void)tivxQueueDelete(&context->free_queue);
            (void)tivxQueueDelete(&context->pend_queue);
        }
    }

    if (status==(vx_status)VX_SUCCESS)
    {
        /* Enqueue all event element index's to free queue */
        for (i = 0; i < TIVX_MAX_CTRL_CMD_OBJECTS; i++)
        {
            /* This call wont fail since number of elements being inserted are equal to
             * queue depth, hence not doing any error checks
             */
            (void)tivxQueuePut(&context->free_queue,
                         i,
                         TIVX_EVENT_TIMEOUT_NO_WAIT);
        }
    }

    return status;
}

static vx_status ownContextDeleteCmdObj(vx_context context)
{
    vx_status status  = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t  i;

    /* Release any allocated resources. */
    for (i = 0; i < TIVX_MAX_CTRL_CMD_OBJECTS; i++)
    {
        if (context->obj_desc_cmd[i] != NULL)
        {
            status1 = ownObjDescFree((tivx_obj_desc_t**)&context->obj_desc_cmd[i]);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM016
<justification end> */
            if (status1 != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Context control comand memory [%d] free-ing failed\n",
                         i);
                status = status1;
            }
/* LDRA_JUSTIFY_END */
        }

        if (context->cmd_ack_event[i] != NULL)
        {
            status1 = tivxEventDelete(&context->cmd_ack_event[i]);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM017
<justification end> */
            if (status1 != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Context control comand event [%d] deletion failed\n",
                         i);

                if (status == (vx_status)VX_SUCCESS)
                {
                    status = status1;
                }
            }
/* LDRA_JUSTIFY_END */
        }
    }

    status1 = tivxQueueDelete(&context->free_queue);

    if (status1 != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Context control command free queue deletion failed\n");

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR025
<justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = status1;
        }
/* LDRA_JUSTIFY_END */
    }

    status1 = tivxQueueDelete(&context->pend_queue);

    if (status1 != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Context control command pend queue deletion failed\n");

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM019
<justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = status1;
        }
/* LDRA_JUSTIFY_END */
    }

    return status;
}

static vx_status ownDeallocateUserKernelId(vx_context context, vx_kernel kernel)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if ( (ownIsValidContext(context) == (vx_bool)vx_true_e) && /* TIOVX-1929- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR002 */
         (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e) ) /* TIOVX-1929- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR003 */
    {
        status = (vx_status)VX_SUCCESS;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR004
<justification end> */
        if ( (kernel->enumeration >= (int32_t)VX_KERNEL_BASE(VX_ID_USER, 0U)) &&
             (kernel->enumeration <  (int32_t)(VX_KERNEL_BASE(VX_ID_USER, 0U) + (int32_t)TIVX_MAX_KERNEL_ID)) )
        {
            int32_t idx = kernel->enumeration - VX_KERNEL_BASE(VX_ID_USER, 0U);
            uint32_t dynamic_user_kernel_idx = (uint32_t)idx;

            context->is_dynamic_user_kernel_id_used[dynamic_user_kernel_idx] = (vx_bool)vx_false_e;
            context->num_dynamic_user_kernel_id--;
        }
/* LDRA_JUSTIFY_END */
    }
    return status;
}

vx_status ownContextFlushCmdPendQueue(vx_context context)
{
    vx_status   status = (vx_status)VX_SUCCESS;
    vx_bool     isEmpty;

    isEmpty = tivxQueueIsEmpty(&context->pend_queue);

    if (isEmpty == (vx_bool)vx_false_e)
    {
        uintptr_t   obj_id;

        while (status == (vx_status)VX_SUCCESS)
        {
            status = tivxQueueGet(&context->pend_queue,
                                  &obj_id,
                                  TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR005
<justification end> */
            if (status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
            {
                vx_status   status1;

                status1 = tivxQueuePut(&context->free_queue,
                                       obj_id,
                                       TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR006
<justification end> */
                if (status1 != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                             "tivxQueuePut(free_queue) failed\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */
            }
        }
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
        if((vx_status)VX_SUCCESS != ownContextLock(context))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {
            for(ref_idx=0; ref_idx < dimof(context->reftable); ref_idx++)
            {
                if(context->reftable[ref_idx]==NULL)
                {
                    char name[VX_MAX_REFERENCE_NAME];

                    context->reftable[ref_idx] = ref;
                    context->num_references++;
                    is_success = (vx_bool)vx_true_e;

                    ownLogResourceAlloc("TIVX_CONTEXT_MAX_REFERENCES", 1);

                    switch(ref->type)
                    {
                        case (vx_enum)VX_TYPE_DELAY:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "delay_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_LUT:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "lut_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_DISTRIBUTION:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "distribution_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_PYRAMID:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "pyramid_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_THRESHOLD:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "threshold_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_MATRIX:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "matrix_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_CONVOLUTION:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "convolution_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_SCALAR:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "scalar_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_ARRAY:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "array_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_IMAGE:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "image_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_REMAP:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "remap_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_OBJECT_ARRAY:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "object_array_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_NODE:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "node_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_GRAPH:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "graph_%d", ref_idx);
                            break;
                        case (vx_enum)TIVX_TYPE_DATA_REF_Q:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "data_ref_q_%d", ref_idx);
                            break;
                        case (vx_enum)VX_TYPE_TENSOR:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "tensor_%d", ref_idx);
                            break;
                        case VX_TYPE_USER_DATA_OBJECT:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "user_data_object_%d", ref_idx);
                            break;
                        case TIVX_TYPE_RAW_IMAGE:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "raw_image_%d", ref_idx);
                            break;
#if defined(BUILD_BAM)
                        case TIVX_TYPE_SUPER_NODE:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "super_node_%d", ref_idx);
                            break;
#endif
                        default:
                            (void)snprintf(name, VX_MAX_REFERENCE_NAME, "ref_%d", ref_idx);
                            break;
                    }
                    if((vx_status)VX_SUCCESS != vxSetReferenceName(ref, name))
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to name reference\n");
                        is_success = (vx_bool)vx_false_e;
                        context->reftable[ref_idx] = NULL;
                        context->num_references--;
                    }

                    break;
                }
            }

            if ((vx_bool)vx_false_e == is_success)
            {
                VX_PRINT(VX_ZONE_ERROR, "Max context references exceeded or setting Reference name failed\n");
                VX_PRINT_BOUND_ERROR("TIVX_CONTEXT_MAX_REFERENCES");
            }

            (void)ownContextUnlock(context);
        }
    }
    return is_success;
}

vx_bool ownRemoveReferenceFromContext(vx_context context, vx_reference ref)
{
    uint32_t ref_idx;
    vx_bool is_success = (vx_bool)vx_false_e;

    if (ownIsValidContext(context)==(vx_bool)vx_true_e)
    {
        if((vx_status)VX_SUCCESS != ownContextLock(context))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {
            for(ref_idx=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR007
<justification end> */ 
            ref_idx < dimof(context->reftable);
/* LDRA_JUSTIFY_END */
            ref_idx++)
            {
                if(context->reftable[ref_idx]==ref)
                {
                    context->reftable[ref_idx] = NULL;
                    context->num_references--;
                    is_success = (vx_bool)vx_true_e;
                    ownLogResourceFree("TIVX_CONTEXT_MAX_REFERENCES", 1);
                    break;
                }
            }
            (void)ownContextUnlock(context);
        }
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
    else if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Kernel reference is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {
            for(idx=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR008
<justification end> */  
            idx<dimof(context->kerneltable);
/* LDRA_JUSTIFY_END */
            idx++)
            {
                if ((NULL == context->kerneltable[idx]) && (context->num_unique_kernels < dimof(context->kerneltable)))
                {
                    /* found free entry */
                    context->kerneltable[idx] = kernel;
                    context->num_unique_kernels++;
                    /* Setting it as void since return value 'count' is not used further */
                    (void)ownIncrementReference(&kernel->base, (vx_enum)VX_INTERNAL);
                    ownLogResourceAlloc("TIVX_CONTEXT_MAX_KERNELS", 1);
                    break;
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT001
<justification end> */
            if(idx>=dimof(context->kerneltable))
            {
                /* free entry not found */
                VX_PRINT(VX_ZONE_ERROR,"free entry not found\n");
                VX_PRINT_BOUND_ERROR("TIVX_CONTEXT_MAX_KERNELS");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
/* LDRA_JUSTIFY_END */

            (void)ownContextUnlock(context);
        }
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
    else if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_false_e)
    {
        VX_PRINT(VX_ZONE_ERROR,"Kernel reference is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                if( (context->kerneltable[idx]==kernel) && (context->num_unique_kernels>0U) )
                {
                    /* found kernel entry */

                    status = ownDeallocateUserKernelId(context, kernel);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR009
<justification end>*/
                    if ((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
                    {
                        context->kerneltable[idx] = NULL;
                        context->num_unique_kernels--;
                        ownLogResourceFree("TIVX_CONTEXT_MAX_KERNELS", 1);
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM020
<justification end>*/
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"deallocate user kernel id failed\n");
                    }
/* LDRA_JUSTIFY_END */
                    break;
                }
            }

            if(idx>=dimof(context->kerneltable))
            {
                /* kernel not found */
                VX_PRINT(VX_ZONE_ERROR,"kernel not found\n");
                status = (vx_status)VX_ERROR_INVALID_REFERENCE;
            }

            (void)ownContextUnlock(context);
        }
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
    else if (NULL == string)
    {
        VX_PRINT(VX_ZONE_ERROR,"provided kernel name was NULL, please provide non-NULL kernel name\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            *is_found = (vx_bool)vx_false_e;

            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                kernel = context->kerneltable[idx];
                if((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) ==(vx_bool)vx_true_e)
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
            (void)ownContextUnlock(context);
        }
    }

    return status;
}

vx_status ownContextSendControlCmd(vx_context context, uint16_t node_obj_desc,
    uint32_t target_id, uint32_t replicated_node_idx, uint32_t node_cmd_id,
    const uint16_t obj_desc_id[], uint32_t num_obj_desc, uint32_t timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t i;

    if((ownIsValidContext(context) == (vx_bool)vx_true_e) &&
       (num_obj_desc < TIVX_CMD_MAX_OBJ_DESCS))
    {
        tivx_obj_desc_cmd_t *obj_desc_cmd;
        tivx_event cmd_ack_event;
        uintptr_t   obj_id;
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

        status = tivxQueueGet(&context->free_queue, &obj_id, timeout);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR010
<justification end> */
        if (status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
        {
            if (obj_id < TIVX_MAX_CTRL_CMD_OBJECTS)
            {
                obj_desc_cmd  = context->obj_desc_cmd[obj_id];
                cmd_ack_event = context->cmd_ack_event[obj_id];

                tivx_uint64_to_uint32(
                    timestamp,
                    &obj_desc_cmd->timestamp_h,
                    &obj_desc_cmd->timestamp_l
                );

                obj_desc_cmd->cmd_id = (vx_enum)TIVX_CMD_NODE_CONTROL;
                obj_desc_cmd->dst_target_id = target_id;
                obj_desc_cmd->src_target_id =
                    (uint32_t)ownPlatformGetTargetId(TIVX_TARGET_HOST);
                obj_desc_cmd->num_obj_desc = 1u;
                obj_desc_cmd->obj_desc_id[0u] = node_obj_desc;
                obj_desc_cmd->flags = TIVX_CMD_FLAG_SEND_ACK;
                obj_desc_cmd->ack_event_handle = (uint64_t)(uintptr_t)cmd_ack_event;

                obj_desc_cmd->replicated_node_idx = (int32_t)replicated_node_idx;
                obj_desc_cmd->node_cmd_id = node_cmd_id;
                obj_desc_cmd->num_cmd_params = num_obj_desc;

                for (i = 0; i < num_obj_desc; i ++)
                {
                    obj_desc_cmd->cmd_params_desc_id[i] = obj_desc_id[i];
                }

                status = ownObjDescSend(target_id, obj_desc_cmd->base.obj_desc_id);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR011
<justification end> */
                if (status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
                {
                    status = tivxEventWait(cmd_ack_event, timeout);

                    if (status == (vx_status)VX_SUCCESS)
                    {

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT002
<justification end> */
                        if ((vx_status)VX_SUCCESS != (vx_status)obj_desc_cmd->cmd_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Command ack message returned failure cmd_status: %d\n",
                                    obj_desc_cmd->cmd_status);
                            status = (vx_status)VX_FAILURE;
                        }
/* LDRA_JUSTIFY_END */

                        /* Put the object back in the free queue. */
                        status1 = tivxQueuePut(&context->free_queue,
                                            obj_id,
                                            TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM021
<justification end> */
                        if (status1 != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Failed to release the object desc id.\n");
                            status = (vx_status)VX_FAILURE;
                        }
/* LDRA_JUSTIFY_END */
                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR012
<justification end> */
                    else if (status == (vx_status)VX_ERROR_TIMEOUT)
/* LDRA_JUSTIFY_END */
                    {
                        /* Queue the object into the pend queue for later
                        * action.
                        */
                        status1 = tivxQueuePut(&context->pend_queue,
                                            obj_id,
                                            TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM022
<justification end> */
                        if (status1 != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Failed to queue the object desc in pend queue.\n");
                            status = (vx_status)VX_FAILURE;
                        }
/* LDRA_JUSTIFY_END */
                    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM005
<justification end> */
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
                    }
/* LDRA_JUSTIFY_END */
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM006
<justification end> */
                else
                {
                    if ((vx_status)VX_SUCCESS != (vx_status)obj_desc_cmd->cmd_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                "Failed to send object desc\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
/* LDRA_JUSTIFY_END */
                ownLogSetResourceUsedValue("TIVX_MAX_CTRL_CMD_OBJECTS", (uint16_t)obj_id);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "Invalid object desc id (>= TIVX_MAX_CTRL_CMD_OBJECTS)\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        if (num_obj_desc >= TIVX_CMD_MAX_OBJ_DESCS)
        {
            VX_PRINT(VX_ZONE_ERROR,
                "Invalid Number of object desc\n");
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,
                "Invalid Context\n");
        }
    }

    return status;
}

vx_status ownContextSendCmd(vx_context context, uint32_t target_id, uint32_t cmd, uint32_t num_obj_desc, const uint16_t *obj_desc_id, uint32_t timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t i;

    if( (ownIsValidContext(context) == (vx_bool)vx_true_e) &&
    (num_obj_desc < TIVX_CMD_MAX_OBJ_DESCS) )
    {
        tivx_obj_desc_cmd_t *obj_desc_cmd;
        tivx_event cmd_ack_event;
        uintptr_t   obj_id;
        uint64_t timestamp = tivxPlatformGetTimeInUsecs()*1000U;

        status = tivxQueueGet(&context->free_queue, &obj_id, timeout);

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR010
<justification end> */
        if (status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
        {
            if (obj_id < TIVX_MAX_CTRL_CMD_OBJECTS)
            {
                obj_desc_cmd  = context->obj_desc_cmd[obj_id];
                cmd_ack_event = context->cmd_ack_event[obj_id];

                tivx_uint64_to_uint32(
                    timestamp,
                    &obj_desc_cmd->timestamp_h,
                    &obj_desc_cmd->timestamp_l
                );

                obj_desc_cmd->cmd_id = cmd;
                obj_desc_cmd->dst_target_id = target_id;
                obj_desc_cmd->src_target_id = (uint32_t)ownPlatformGetTargetId(TIVX_TARGET_HOST);
                obj_desc_cmd->num_obj_desc = num_obj_desc;
                obj_desc_cmd->flags = TIVX_CMD_FLAG_SEND_ACK;
                obj_desc_cmd->ack_event_handle = (uint64_t)(uintptr_t)cmd_ack_event;

                for(i=0; i<num_obj_desc; i++)
                {
                    obj_desc_cmd->obj_desc_id[i] = obj_desc_id[i];
                }

                status = ownObjDescSend(target_id, obj_desc_cmd->base.obj_desc_id);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR014
<justification end> */
                if (status == (vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
                {
                    status = tivxEventWait(cmd_ack_event, timeout);

                    if (status == (vx_status)VX_SUCCESS)
                    {
                        if ((vx_status)VX_SUCCESS != (vx_status)obj_desc_cmd->cmd_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Command ack message returned failure cmd_status: %d\n",
                                    obj_desc_cmd->cmd_status);
                            status = (vx_status)VX_FAILURE;
                        }

                        /* Put the object back in the free queue. */
                        status1 = tivxQueuePut(&context->free_queue,
                                            obj_id,
                                            TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM023
<justification end> */
                        if (status1 != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Failed to release the object desc id.\n");
                            status = (vx_status)VX_FAILURE;
                        }
/* LDRA_JUSTIFY_END */
                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR015
<justification end> */
                    else if (status == (vx_status)VX_ERROR_TIMEOUT)
/* LDRA_JUSTIFY_END */              
                    {
                        /* Queue the object into the pend queue for later
                        * action.
                        */
                        status1 = tivxQueuePut(&context->pend_queue,
                                            obj_id,
                                            TIVX_EVENT_TIMEOUT_NO_WAIT);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM024
<justification end> */
                        if (status1 != (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                    "Failed to queue the object desc in pend queue.\n");
                        }
/* LDRA_JUSTIFY_END */
                    }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM007
<justification end> */
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "tivxEventWait() failed.\n");
                    }
/* LDRA_JUSTIFY_END */
                }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM008
<justification end> */
                else
                {
                    if ((vx_status)VX_SUCCESS != (vx_status)obj_desc_cmd->cmd_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                "Failed to send object desc\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
/* LDRA_JUSTIFY_END */
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,
                        "Invalid object desc id (>= TIVX_MAX_CTRL_CMD_OBJECTS)\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"invalid parameters\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status ownReleaseReferenceBufferGeneric(vx_reference *ref)
{
    vx_enum value = (vx_enum)(*ref)->type;
    vx_status status;

    switch(value)
    {
        case (vx_enum)VX_TYPE_CONTEXT:
        {
            vx_context *temp_context;
            temp_context = (vx_context *)ref;
            status = vxReleaseContext(temp_context);
            break;
        }
        default:
            status = ownReleaseReferenceInt(
                ref, (vx_enum)(*ref)->type, (vx_enum)VX_EXTERNAL, NULL);
            break;
    }
    return status;
}

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext(void)
{
    vx_context context = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    {
        if (g_context_handle == NULL)
        {
            context = &g_context_obj;

            (void)memset(context, 0, sizeof(tivx_context_t));

            context->imm_border.mode = (vx_enum)VX_BORDER_UNDEFINED;
            context->imm_border_policy = (vx_enum)VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED;
            context->perf_enabled = (vx_bool)vx_false_e;
            context->imm_target_enum = (vx_enum)VX_TARGET_ANY;
            (void)memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
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
            for(idx=0; idx<TIVX_MAX_KERNEL_ID; idx++)
            {
                context->is_dynamic_user_kernel_id_used[idx] = (vx_bool)vx_false_e;
            }
            for(idx=0; idx<TIVX_MAX_LIBRARY_ID; idx++)
            {
                context->is_dynamic_user_library_id_used[idx] = (vx_bool)vx_false_e;
            }
            context->num_unique_kernels = 0;
            context->num_dynamic_user_kernel_id = 0;
            context->num_dynamic_user_library_id = 0;
            context->log_enabled = (vx_bool)vx_false_e;
            context->base.release_callback =
                &ownReleaseReferenceBufferGeneric;

            status = tivxMutexCreate(&context->lock);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR016
<justification end> */
            if(status==(vx_status)VX_SUCCESS)
            {
                status = tivxMutexCreate(&context->log_lock);
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR017
<justification end> */
            if(status==(vx_status)VX_SUCCESS)
            {
                status = ownEventQueueCreate(&context->event_queue);
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR018
<justification end> */
            if(status==(vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
            {
                status = ownInitReference(&context->base, NULL, (vx_enum)VX_TYPE_CONTEXT, NULL);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR019
<justification end> */
                if(status==(vx_status)VX_SUCCESS)
/* LDRA_JUSTIFY_END */
                {
                    status = ownContextCreateCmdObj(context);
                    if(status == (vx_status)VX_SUCCESS)
                    {
                        vx_bool ret;
                        /* Setting it as void since return value 'count' is not used further */
                        (void)ownIncrementReference(&context->base, (vx_enum)VX_EXTERNAL);
                        ret = ownCreateConstErrors(context);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM025
<justification end> */
                        if ((vx_bool)vx_false_e==ret)
                        {
                            status = (vx_status)VX_ERROR_NO_RESOURCES;
                            VX_PRINT(VX_ZONE_ERROR,"error object not created\n");
                        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM025
<justification end> */
                        else
/* LDRA_JUSTIFY_END */
                        {
                            g_context_handle = context;
                        }
                    }
                }

                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"context objection creation failed\n");

                    /* Added the delete to take care of the mutex created in previous ownInitReference() call */
                    status = ownDeleteReferenceLock(&context->base);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT010
<justification end> */
                    if((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Failed to deinitialize Reference\n");
                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT010
<justification end> */
                    else
/* LDRA_JUSTIFY_END */
                    {
                        status = ownEventQueueDelete(&context->event_queue);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT008
<justification end> */
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR,"Failed to delete Event Queue\n");
                        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT008
<justification end> */
                        else
/* LDRA_JUSTIFY_END */
                        {
                            status = tivxMutexDelete(&context->log_lock);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM002
<justification end> */
                            if((vx_status)VX_SUCCESS != status)
                            {
                                VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
                            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM002
<justification end> */
                            else
/* LDRA_JUSTIFY_END */
                            {
                                status = tivxMutexDelete(&context->lock);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM003
<justification end> */
                                if((vx_status)VX_SUCCESS != status)
                                {
                                    VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
                                }
/* LDRA_JUSTIFY_END */
                            }
                        }
                    }
                }
            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM009
<justification end> */
            if(status!=(vx_status)VX_SUCCESS)
            {
                /* some error context cannot be created */
                context = NULL;
            }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR020
<justification end> */
            if(status == (vx_status)VX_SUCCESS)
            {
                ownLogResourceAlloc("TIVX_CONTEXT_MAX_OBJECTS", 1);
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
                    (void)vxLoadKernels(context, g_context_default_load_module[idx]);
                }

                /* set flag to allow removal additional kernels
                 * installed by user via remove kernel API
                 */
                ownContextSetKernelRemoveLock(context, (vx_bool)vx_false_e);
            }
/* LDRA_JUSTIFY_END */
        }
        else
        {
            context = g_context_handle;
            /* Setting it as void since return value 'count' is not used further */
            (void)ownIncrementReference(&context->base, (vx_enum)VX_EXTERNAL);
        }
    }
    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    return context;
}

vx_context ownGetContext(void)
{
    vx_context context = NULL;

    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    context = g_context_handle;

    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

    return context;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *c)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    vx_bool do_break = (vx_bool)vx_false_e;
    vx_context context;
    vx_uint32 r;
    uint32_t idx;

    if(c != NULL)
    {
        context = *c;
    }
    else
    {
        context = NULL;
    }

    if (c != NULL)
    {
        *c = NULL;
    }
    ownPlatformSystemLock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);
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
                status = vxUnloadKernels(context, g_context_default_load_module[idx]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM004
<justification end> */
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to unload kernel\n");
                    break;
                }
/* LDRA_JUSTIFY_END */
            }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR021
<justification end> */
            if((vx_status)VX_SUCCESS == status)
/* LDRA_JUSTIFY_END */
            {

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
                        VX_PRINT(VX_ZONE_WARNING,"Found a reference "VX_FMT_REF" of type %08x at external count %u, internal count %u, releasing it\n",
                                ref, ref->type, ref->external_count, ref->internal_count);
                        VX_PRINT(VX_ZONE_WARNING,"Releasing reference (name=%s) now as a part of garbage collection\n", ref->name);
                    }

                    /* These were internally opened during creation, so should internally close ERRORs */
                    if((NULL != ref) && (ref->type == (vx_enum)VX_TYPE_ERROR) )
                    {
                        status1 = ownReleaseReferenceInt(&ref, ref->type, (vx_enum)VX_INTERNAL, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM010
<justification end> */
                        if((vx_status)VX_SUCCESS != status1)
                        {
                            VX_PRINT(VX_ZONE_ERROR,"Failed to destroy internal reference objects\n");
                            status = status1;
                            do_break = (vx_bool)vx_true_e;
                        }
/* LDRA_JUSTIFY_END */
                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_CONTEXT_UBR022
<justification end> */
                    if((vx_status)VX_SUCCESS == status1)
/* LDRA_JUSTIFY_END */
                    {
                        if((NULL != ref) && (ref->type == (vx_enum)VX_TYPE_KERNEL) ) {
                            VX_PRINT(VX_ZONE_WARNING,"A kernel with name %s has not been removed, possibly due to a kernel module not being unloaded.\n", ref->name);
                            VX_PRINT(VX_ZONE_WARNING,"Removing as a part of garbage collection\n");
                            /* status set to NULL due to preceding type check */
                            status = vxRemoveKernel(vxCastRefAsKernel(ref,NULL));
                        }

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT004
<justification end> */
                        /* Warning above so user can fix release external objects, but close here anyway */
                        while ((NULL != ref)&& (ref->external_count > 1U) ) {
                            /* Setting it as void since return value 'count' is not used further */
                            (void)ownDecrementReference(ref, (vx_enum)VX_EXTERNAL);
                        }
/* LDRA_JUSTIFY_END */

                        if ((NULL != ref) && (ref->external_count > 0U) )
                        {
                            status1 = ownReleaseReferenceInt(&ref, ref->type, (vx_enum)VX_EXTERNAL, NULL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM011
<justification end> */
                            if((vx_status)VX_SUCCESS != status1)
                            {
                                VX_PRINT(VX_ZONE_ERROR,"Failed to destroy external reference objects\n");
                                status = status1;
                                do_break = (vx_bool)vx_true_e;
                            }
/* LDRA_JUSTIFY_END */
                        }
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM012
<justification end> */
                    if((vx_bool)vx_true_e == do_break)
                    {
                        break;
                    }
/* LDRA_JUSTIFY_END */
                }

                /* By now, all external and internal references should be removed */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM026
<justification end> */
                for (r = 0; r < dimof(context->reftable); r++)
                {
                    if(context->reftable[r] != NULL)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Reference %d not removed\n", r);
                    }
                }
/* LDRA_JUSTIFY_END */

                status1 = ownContextDeleteCmdObj(context);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM013
<justification end> */
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"ownContextDeleteCmdObj() failed\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */

                status1 = ownEventQueueDelete(&context->event_queue);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM014
<justification end> */
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to delete event queue\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */

                ownLogResourceFree("TIVX_CONTEXT_MAX_OBJECTS", 1);

                /*! \internal wipe away the context memory first */
                /* Normally destroy sem is part of release reference, but can't for context */
                status1 = tivxMutexDelete(&context->base.lock);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT005
<justification end> */
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */

                status1 = tivxMutexDelete(&context->log_lock);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT006
<justification end> */
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */

                status1 = tivxMutexDelete(&context->lock);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UTJT007
<justification end> */
                if((vx_status)VX_SUCCESS != status1)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
                    status = status1;
                }
/* LDRA_JUSTIFY_END */

                (void)memset(context, 0, sizeof(tivx_context_t));

                g_context_handle = NULL;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    ownPlatformSystemUnlock((vx_enum)TIVX_PLATFORM_LOCK_CONTEXT);

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
                    (void)strncpy(ptr, g_context_implmentation_name, VX_MAX_IMPLEMENTATION_NAME);
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
                    uint32_t str_size = (uint32_t)sizeof(g_context_extensions);
                    (void)strncpy(ptr, g_context_extensions, str_size);
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
            case (vx_enum)VX_CONTEXT_EVENT_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    
                    *(vx_uint32*)ptr = context->timeout_events_val;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "query context VX_CONTEXT_EVENT_TIMEOUT failed\n");
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
            case (vx_enum)TIVX_CONTEXT_NUM_USER_KERNEL_ID:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = context->num_dynamic_user_kernel_id;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context number of user kernel ID's failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_CONTEXT_NUM_USER_LIBRARY_ID:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = context->num_dynamic_user_library_id;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"query context number of user kernel ID's failed\n");
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

            case (vx_enum)VX_CONTEXT_EVENT_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    const vx_uint32   timeout_val = *(const vx_uint32*)ptr;

                    /* Validate the timeout. It cannot be zero. */
                    if (timeout_val == 0U)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                                 "Invalid timeout value specified for events: %d\n",
                                 timeout_val);
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    else
                    {
                        context->timeout_events_val = timeout_val;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Set VX_CONTEXT_EVENT_TIMEOUT failed\n");
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM027
<justification end>*/
        if (ownIsValidContext(context) == (vx_bool)vx_false_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"context is invalid\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_CONTEXT_UM027
<justification end>*/
        else
/* LDRA_JUSTIFY_END */
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
        if((vx_status)VX_SUCCESS != ownContextLock(context))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            for (i = 0; i < TIVX_CONTEXT_MAX_USER_STRUCTS; ++i)
            {
                if (context->user_structs[i].type == (vx_enum)VX_TYPE_INVALID)
                {
                    context->user_structs[i].type = (vx_enum)VX_TYPE_USER_STRUCT_START + (int32_t)i;
                    context->user_structs[i].size = size;
                    type = context->user_structs[i].type;
                    ownLogSetResourceUsedValue("TIVX_CONTEXT_MAX_USER_STRUCTS", (uint16_t)i+1U);
                    break;
                }
            }

            if (type == (vx_enum)VX_TYPE_INVALID)
            {
                VX_PRINT_BOUND_ERROR("TIVX_CONTEXT_MAX_USER_STRUCTS");
            }
            (void)ownContextUnlock(context);
        }
    }
    return type;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) && (NULL != pKernelEnumId))
    {
        uint32_t idx;

        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            status = (vx_status)VX_ERROR_NO_RESOURCES;
            for(idx=0; idx<TIVX_MAX_KERNEL_ID; idx++)
            {
                if (context->is_dynamic_user_kernel_id_used[idx] == (vx_bool)vx_false_e)
                {
                    *pKernelEnumId = VX_KERNEL_BASE(VX_ID_USER, 0U) + (vx_enum)(idx);
                    status = (vx_status)VX_SUCCESS;
                    context->is_dynamic_user_kernel_id_used[idx] = (vx_bool)vx_true_e;
                    context->num_dynamic_user_kernel_id++;
                    break;
                }
            }
            (void)ownContextUnlock(context);
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if ((ownIsValidContext(context) == (vx_bool)vx_true_e) && (NULL != pLibraryId))
    {
        uint32_t idx;

        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            status = (vx_status)VX_ERROR_NO_RESOURCES;
            for(idx=1; idx<(TIVX_MAX_LIBRARY_ID); idx++)
            {
                if (context->is_dynamic_user_library_id_used[idx] == (vx_bool)vx_false_e)
                {
                    *pLibraryId = (int32_t)(idx);
                    status = (vx_status)VX_SUCCESS;
                    context->is_dynamic_user_library_id_used[idx] = (vx_bool)vx_true_e;
                    context->num_dynamic_user_library_id++;
                    break;
                }
            }
            (void)ownContextUnlock(context);
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        status = ownContextLock(context);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            switch (target_enum)
            {
                case (vx_enum)VX_TARGET_ANY:
                    context->imm_target_enum = (vx_enum)VX_TARGET_ANY;
                    (void)memset(context->imm_target_string, 0, sizeof(context->imm_target_string));
                    status = (vx_status)VX_SUCCESS;
                    break;

                case (vx_enum)VX_TARGET_STRING:
                    if (target_string != NULL)
                    {
                        context->imm_target_enum = (vx_enum)VX_TARGET_STRING;
                        (void)strncpy(context->imm_target_string, target_string, sizeof(context->imm_target_string)-1U);
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
            (void)ownContextUnlock(context);
        }
    }
    return status;
}

VX_API_ENTRY vx_kernel VX_API_CALL vxGetKernelByName(vx_context context, const vx_char *name)
{
    vx_kernel kernel = NULL;
    uint32_t idx;

    if( ownIsValidContext(context) == (vx_bool)vx_false_e )
    {
        kernel = NULL;
    }
    else
    {
        if((vx_status)VX_SUCCESS != ownContextLock(context))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {

            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                kernel = context->kerneltable[idx];
                if((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) != (vx_bool)vx_false_e)
                    &&
                    ( strncmp(kernel->name, name, VX_MAX_KERNEL_NAME) == 0 )
                    )
                {
                    /* found match and setting it as void since return value 'count' is not used further */
                    (void)ownIncrementReference(&kernel->base, (vx_enum)VX_EXTERNAL);
                    break;
                }
            }
            if(idx>=dimof(context->kerneltable))
            {
                /* not found */
                kernel = NULL;
            }
            (void)ownContextUnlock(context);
        }
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
        if((vx_status)VX_SUCCESS != ownContextLock(context))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to lock context\n");
        }
        else
        {
            for(idx=0; idx<dimof(context->kerneltable); idx++)
            {
                kernel = context->kerneltable[idx];
                if((ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) != (vx_bool)vx_false_e)
                    &&
                    ( kernel->enumeration == kernelenum )
                    )
                {
                    /* found match and setting it as void since return value 'count' is not used further */
                    (void)ownIncrementReference(&kernel->base, (vx_enum)VX_EXTERNAL);
                    break;
                }
            }
            if(idx>=dimof(context->kerneltable))
            {
                /* not found */
                kernel = NULL;
            }
            (void)ownContextUnlock(context);
        }
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

