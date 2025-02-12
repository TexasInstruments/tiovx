/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
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

static vx_status ownDestructNode(vx_reference ref);
static vx_status ownInitNodeObjDesc(vx_node node, vx_kernel kernel, uint32_t pipeline_id);
static vx_status ownRemoveNodeInt(const vx_node *n);
static void ownNodeUserKernelUpdateParamsFlags(vx_kernel kernel, vx_reference params[], vx_uint32 num_params, vx_bool is_accessible);
static uint16_t ownNodeGetObjDescId(vx_node node, uint32_t pipeline_id);
static void ownNodeSetAccessibility(vx_node node, uint32_t num_params, vx_bool is_accessible);

static vx_status ownDestructNode(vx_reference ref)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_status status1 = (vx_status)VX_SUCCESS;
    uint32_t p, pipe_id;
    vx_bool put_break = (vx_bool)vx_false_e;

    if(ref->type == (vx_enum)VX_TYPE_NODE) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR001 */
    {
        /*status set to NULL due to preceding type check*/
        vx_node node = vxCastRefAsNode(ref, NULL);
        if(node->kernel!=NULL)
        {
            status = ownNodeKernelDeinit(node);

            if (status != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "ownNodeKernelDeinit() failed.\n");
            }

            /* remove, don't delete, all references from the node itself */
            for (p = 0; p < node->kernel->signature.num_parameters; p++)
            {
                vx_reference node_ref = node->parameters[p];
                if (NULL != node_ref)
                {
                    /* Remove the potential delay association */
                    if (node_ref->delay!=NULL) {
                        vx_bool res = ownRemoveAssociationToDelay(node_ref, node, p);
                        if (res == (vx_bool)vx_false_e)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Internal error removing delay association\n");
                        }
                    }
                    status1 = ownReleaseReferenceInt(&node_ref, node_ref->type, (vx_enum)VX_INTERNAL, NULL);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM001
<justification end> */
                    if (status1 != (vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "ownReleaseReferenceInt() failed.\n");

                        if (status == (vx_status)VX_SUCCESS)
                        {
                            status = status1;
                        }
                    }
/* LDRA_JUSTIFY_END */

                    node->parameters[p] = NULL;
                }
            }

            status1 = ownReleaseReferenceInt(vxCastRefFromKernelP(&node->kernel), (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_INTERNAL, NULL);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM002
<justification end> */
            if (status1 != (vx_status)VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "ownReleaseReferenceInt() failed.\n");

                if (status == (vx_status)VX_SUCCESS)
                {
                    status = status1;
                }
            }
/* LDRA_JUSTIFY_END */
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
            {
                if(node->obj_desc[pipe_id]!=NULL)
                {
                    status = ownObjDescFree((tivx_obj_desc_t**)&node->obj_desc[pipe_id]);
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Node object descriptor free failed!\n");
                        put_break = (vx_bool)vx_true_e;
                    }
                }
                if(node->obj_desc_cmd[pipe_id]!=NULL)
                {
                    status = ownObjDescFree((tivx_obj_desc_t**)&node->obj_desc_cmd[pipe_id]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM003
<justification end> */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Node command object descriptor free failed!\n");
                        put_break = (vx_bool)vx_true_e;
                    }
/* LDRA_JUSTIFY_END */
                }
                if ((vx_bool)vx_true_e == put_break)
                {
                    break;
                }
            }
        }
    }
    return (vx_status)status;
}

static vx_status ownInitNodeObjDesc(vx_node node, vx_kernel kernel, uint32_t pipeline_id)
{
    tivx_obj_desc_node_t *obj_desc = node->obj_desc[0];
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t idx;

    /* set default flags */
    obj_desc->pipeline_id = (uint16_t)pipeline_id;

    obj_desc->flags = 0;

    obj_desc->kernel_id = (uint32_t)kernel->enumeration;

    obj_desc->node_complete_cmd_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    obj_desc->border_mode.mode = (vx_enum)VX_BORDER_UNDEFINED;
    tivx_obj_desc_memset(&obj_desc->border_mode.constant_value, 0, (uint32_t)sizeof(vx_pixel_value_t));

    tivx_obj_desc_memset(obj_desc->target_kernel_index, 0,
        (TIVX_NODE_MAX_REPLICATE)*sizeof(uint32_t));

    obj_desc->target_kernel_index[0] = 0;
    obj_desc->exe_status = 0;
    obj_desc->num_params = kernel->signature.num_parameters;

    for(idx=0; idx<kernel->signature.num_parameters; idx++)
    {
        obj_desc->data_id[idx] = (vx_enum)TIVX_OBJ_DESC_INVALID;
        obj_desc->data_ref_q_id[idx] = (vx_enum)TIVX_OBJ_DESC_INVALID;
    }

    obj_desc->state = TIVX_NODE_OBJ_DESC_STATE_IDLE;
    obj_desc->num_out_nodes = 0;
    obj_desc->num_in_nodes = 0;
    obj_desc->is_prm_replicated = 0;
    obj_desc->is_prm_input = 0;
    obj_desc->is_prm_bidi = 0;
    obj_desc->is_prm_data_ref_q = 0;
    obj_desc->is_prm_array_element = 0;
    obj_desc->prev_pipe_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
    obj_desc->blocked_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
    obj_desc->prev_pipe_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID;


    if((vx_bool)vx_true_e == kernel->is_target_kernel)
    {
        tivxFlagBitSet(&obj_desc->flags, TIVX_NODE_FLAG_IS_TARGET_KERNEL);
    }

    obj_desc->num_pipeup_bufs = kernel->num_pipeup_bufs;
    obj_desc->pipeup_buf_idx  = kernel->pipeup_buf_idx;
    obj_desc->source_state    = (uint32_t)kernel->state;

    obj_desc->block_width = TIVX_DEFAULT_TILE_WIDTH;
    obj_desc->block_height = TIVX_DEFAULT_TILE_HEIGHT;

    obj_desc->target_id = (uint32_t)ownKernelGetDefaultTarget(kernel);
    if(obj_desc->target_id == (uint32_t)TIVX_TARGET_ID_INVALID)
    {
        /* invalid target or no target, associated with kernel,
         */
        VX_PRINT(VX_ZONE_ERROR, "invalid target or no target, associated with kernel\n");
        status = (vx_status)VX_ERROR_INVALID_VALUE;
    }

    return status;
}

static vx_status ownRemoveNodeInt(const vx_node *n)
{
    vx_node node;
    /* NULL check for node is skipped since the check is already done in vx_RemoveNode. */
    node = *n;
    vx_status status =  (vx_status)VX_ERROR_INVALID_REFERENCE;

    if ((NULL != node) && /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR002 */
              (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e))/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR003 */
    {
        if (node->graph != NULL)
        {
            status = ownReferenceLock(&node->graph->base);
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to lock the reference\n");
            }
            else
            {
                status = ownGraphRemoveNode(node->graph, node);

                (void)ownReferenceUnlock(&node->graph->base);
            }
        }
    }
    return status;
}

static void ownNodeUserKernelUpdateParamsFlags(vx_kernel kernel, vx_reference params[], vx_uint32 num_params, vx_bool kernel_entry)
{
    vx_uint32 i;
    for(i=0; i<num_params ; i++)
    {
        if (NULL != params[i])
        {
            if ((vx_bool)vx_true_e == kernel_entry)
            {
                /* Set flag to say we are in the kernel function */
                tivxFlagBitSet(&params[i]->obj_desc->flags, TIVX_REF_FLAG_IS_IN_KERNEL);
                /* And just in case it hasn't been set elsewhere */
                tivxFlagBitSet(&params[i]->obj_desc->flags, TIVX_REF_FLAG_IS_IN_GRAPH);
            }
            else
            {
                /* Clear flag that says we are in the kernel function */
                tivxFlagBitClear(&params[i]->obj_desc->flags, TIVX_REF_FLAG_IS_IN_KERNEL);
            }
            if ((vx_enum)VX_INPUT ==  kernel->signature.directions[i])
            {
                if ((vx_bool)vx_true_e == kernel_entry)
                {
                    tivxFlagBitSet(&params[i]->obj_desc->flags, TIVX_REF_FLAG_IS_INPUT);
                }
                else
                {
                    tivxFlagBitClear(&params[i]->obj_desc->flags, TIVX_REF_FLAG_IS_INPUT);
                }
            }
            if((params[i]->is_virtual != (vx_bool)vx_false_e))
            {
                params[i]->is_accessible = kernel_entry;
            }
        }
    }
}

static void ownNodeSetAccessibility(vx_node node, uint32_t num_params, vx_bool is_accessible)
{
    uint32_t i;

    for (i = 0; i < num_params; i ++)
    {
        if ((NULL != node->parameters[i]) &&
            ((vx_bool)vx_true_e == node->parameters[i]->is_virtual))
        {
            node->parameters[i]->is_accessible = is_accessible;
        }
    }
}

vx_status ownNodeKernelValidate(vx_node node, vx_meta_format meta[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t num_params;

    if((NULL != node) && (NULL != node->kernel))
    {
        if(node->kernel->validate != NULL)
        {
            /* the type of the parameter is known by the system, so let the
               system set it by default. */
            num_params = node->kernel->signature.num_parameters;
            for (i = 0; i < num_params; i ++)
            {
                meta[i]->type = node->kernel->signature.types[i];
            }

            ownNodeSetAccessibility(node, num_params, (vx_bool)vx_true_e);

            VX_PRINT(VX_ZONE_INFO, "Validating kernel %s\n", node->kernel->name);

            status = node->kernel->validate(node, node->parameters,
                num_params, meta);

            ownNodeSetAccessibility(node, num_params, (vx_bool)vx_false_e);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Node and/or kernel are NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status tivxNodeSendCommandTimed(vx_node node, uint32_t replicated_node_idx,
    uint32_t node_cmd_id, vx_reference ref[], uint32_t num_refs, uint32_t timeout)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    uint16_t obj_desc_id[TIVX_CMD_MAX_OBJ_DESCS];

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e)
    {
        if(NULL == node->kernel)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Node Kernel Not Valid\n");
        }

        if( (node->is_kernel_created == (vx_bool)vx_false_e) ||
            (node->graph->verified == (vx_bool)vx_false_e) )
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR,
                    "vxVerifyGraph should be called before tivxNodeSendCommand can be called on a node in the same graph\n");
        }

        if((vx_status)VX_SUCCESS == status)
        {
            if (num_refs > TIVX_CMD_MAX_OBJ_DESCS)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR,
                    "num_refs exceeds TIVX_CMD_MAX_OBJ_DESCS\n");
                VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_CMD_MAX_OBJ_DESCS in tiovx/source/include/tivx_obj_desc_priv.h\n");
            }
            else
            {
                for (cnt = 0u; cnt < num_refs; cnt ++)
                {
                    if (ownIsValidReference(ref[cnt]) == (vx_bool)vx_true_e)
                    {
                        obj_desc_id[cnt] = ref[cnt]->obj_desc->obj_desc_id;

                        /* Since it is possible that the reference sent across this API
                         * to the node via a control command is not connected to a graph
                         * and thus may not be allocated already, explicitly calling the
                         * reference allocation API. */
                        status = ownReferenceAllocMem(ref[cnt]);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT001
<justification end> */
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Allocating reference %d failed\n", cnt);
                            break;
                        }
/* LDRA_JUSTIFY_END */
                    }
                    else
                    {
                        /* Invalid Desc ID */
                        obj_desc_id[cnt] = (uint16_t)-1;
                        VX_PRINT(VX_ZONE_INFO, "ref[%d] is not valid\n", cnt);
                    }
                }

                if((vx_status)VX_SUCCESS == status)/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR004 */
                {
                    status = ownContextSendControlCmd(node->base.context,
                        node->obj_desc[0]->base.obj_desc_id,
                        node->obj_desc[0]->target_id,
                        replicated_node_idx, node_cmd_id,
                        obj_desc_id, num_refs, timeout);

                    if ((vx_status)TIVX_ERROR_EVENT_TIMEOUT==status)
                    {
                        node->is_timed_out = (vx_bool)vx_true_e;
                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR005
<justification end> */
                    else if ((vx_status)VX_SUCCESS==status)
/* LDRA_JUSTIFY_END */
                    {
                        node->is_timed_out = (vx_bool)vx_false_e;
                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT002
<justification end> */
                    else
                    {
                        /* do nothing */
                    }
/* LDRA_JUSTIFY_END */
                }
            }
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_NODE;
        VX_PRINT(VX_ZONE_ERROR, "Node Not Valid\n");
    }

    return (status);
}

vx_status tivxNodeSendCommand(vx_node node, uint32_t replicated_node_idx,
    uint32_t node_cmd_id, vx_reference ref[], uint32_t num_refs)
{
    vx_status status;

    status = tivxNodeSendCommandTimed(node, replicated_node_idx,
                                      node_cmd_id, ref, num_refs,
                                      TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

    return status;
}

vx_status ownNodeKernelInitKernelName(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_kernel_name_t *kernel_name_obj_desc;

    /* alloc obj desc for kernel name */
    kernel_name_obj_desc = (tivx_obj_desc_kernel_name_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_KERNEL_NAME, NULL);

    if(kernel_name_obj_desc!=NULL)
    {
        /* set kernel name */
        tivx_obj_desc_strncpy(kernel_name_obj_desc->kernel_name, node->kernel->name, VX_MAX_KERNEL_NAME);

        /* associated kernel name object descriptor with node object */
        node->obj_desc[0]->kernel_name_obj_desc_id = kernel_name_obj_desc->base.obj_desc_id;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Target kernel, TIVX_CMD_NODE_CREATE failed, unable to alloc obj desc for kernel_name\n");
        status=(vx_status)VX_FAILURE;
        VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
        VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
    }
    return status;
}

vx_status ownNodeKernelDeinitKernelName(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_obj_desc_t *obj_desc = ownObjDescGet((uint16_t)node->obj_desc[0]->kernel_name_obj_desc_id);

    /* dis associate kernel name obj desc, since it not required anymore,
     * free object desc
     */
    node->obj_desc[0]->kernel_name_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;
    status = ownObjDescFree((tivx_obj_desc_t**)&obj_desc);

    return status;
}

vx_status ownNodeKernelInit(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((vx_bool)vx_false_e == node->kernel->is_target_kernel)
    {
        if(node->kernel->local_data_size != 0U)
        {
            /* allocate memory for user kernel */
            node->local_data_size = node->kernel->local_data_size;
            node->local_data_ptr = tivxMemAlloc((uint32_t)node->local_data_size,
                (vx_enum)TIVX_MEM_EXTERNAL);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT003
<justification end>*/
            if(node->local_data_ptr==NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,"User kernel, local data memory alloc failed\n");
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT003
<justification end>*/
            else
/* LDRA_JUSTIFY_END */
            {
                node->local_data_ptr_is_alloc = (vx_bool)vx_true_e;
            }
        }
        else
        {
            node->local_data_size = 0;
            node->local_data_ptr = NULL;
        }

        if ((vx_bool)vx_false_e==node->is_initialized)
        {
            if((NULL != node->kernel->initialize) &&
                (status == (vx_status)VX_SUCCESS))
            {
                node->local_data_set_allow = (vx_bool)vx_true_e;

                ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_true_e);

                /* user has given initialize function so call it */
                status = node->kernel->initialize(node, node->parameters,
                    node->kernel->signature.num_parameters);

                ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_false_e);

                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"User kernel, kernel init callback failed\n");
                }
                else
                {
                    node->local_data_set_allow = (vx_bool)vx_false_e;
                    node->is_initialized = (vx_bool)vx_true_e;
                }
            }
        }
    }
    else
    {
        uint16_t obj_desc_id[1];

        if ((vx_bool)vx_false_e==node->is_initialized)
        {
            if(NULL != node->kernel->initialize) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR006 */
            {
                node->local_data_set_allow = (vx_bool)vx_true_e;
                tivx_obj_desc_node_t *node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
                uint32_t num_params = node->kernel->signature.num_parameters;

                if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == (vx_bool)vx_true_e )
                {
                    vx_reference params[TIVX_KERNEL_MAX_PARAMS];
                    vx_reference parent_ref[TIVX_KERNEL_MAX_PARAMS];
                    uint32_t i, n;

                    for(i=0; i<num_params ; i++)
                    {
                        parent_ref[i] = NULL;

                        if((0 != node->replicated_flags[i]) &&
                           (NULL != node->parameters[i]))
                        {
                            parent_ref[i] = node->parameters[i]->scope;
                        }
                    }

                    for(n=0; n<node_obj_desc->num_of_replicas; n++)
                    {
                        for(i=0; i<num_params ; i++)
                        {
                            params[i] = NULL;
                            if(node->replicated_flags[i] != 0)
                            {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR007
<justification end> */
                                if(parent_ref[i] != NULL)
                                {
                                    if(parent_ref[i]->type==(vx_enum)VX_TYPE_OBJECT_ARRAY)
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = (vxCastRefAsObjectArray(parent_ref[i], NULL))->ref[n];
                                    }
                                    else if(parent_ref[i]->type==(vx_enum)VX_TYPE_PYRAMID)
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = vxCastRefFromImage((vxCastRefAsPyramid(parent_ref[i], NULL))->img[n]);
                                    }
                                    else
                                    {
                                        params[i] = NULL;
                                    }
                                }
/* LDRA_JUSTIFY_END */
                            }
                            else
                            {
                                params[i] = node->parameters[i];
                            }
                        }

                        ownNodeSetAccessibility(node, num_params, (vx_bool)vx_true_e);

                        tivxCheckStatus(&status, node->kernel->initialize(node, params, num_params));

                        ownNodeSetAccessibility(node, num_params, (vx_bool)vx_false_e);
                    }
                }
                else
                {
                    ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_true_e);

                    /* user has given initialize function so call it */
                    status = node->kernel->initialize(node, node->parameters,
                        node->kernel->signature.num_parameters);

                    ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_false_e);
                }

                if(status!=(vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Target kernel, kernel init callback failed\n");
                }
                else
                {
                    node->is_initialized = (vx_bool)vx_true_e;
                }
            }
        }

        if ((vx_bool)vx_false_e==node->is_kernel_created)/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR008 */
        {
            if((vx_status)VX_SUCCESS==status)
            {
                obj_desc_id[0] = node->obj_desc[0]->base.obj_desc_id;

                if(((vx_bool)vx_true_e == node->is_super_node) || /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR009 */
                   (NULL == node->super_node)) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR010 */
                {
                    vx_reference ref;

                    ref = vxCastRefFromNode(node);

                    VX_PRINT(VX_ZONE_INFO,"Calling create callback for node %s\n", ref->name);

                    status = ownContextSendCmd(node->base.context,
                        node->obj_desc[0]->target_id, (vx_enum)TIVX_CMD_NODE_CREATE,
                        1, obj_desc_id, node->timeout_val);

                    if ((vx_status)TIVX_ERROR_EVENT_TIMEOUT==status)
                    {
                        node->is_timed_out = (vx_bool)vx_true_e;
                        VX_PRINT(VX_ZONE_ERROR,"Message for TIVX_CMD_NODE_CREATE command has TIMED OUT\n");
                    }
                    else if ((vx_status)VX_SUCCESS==status)
                    {
                        node->is_timed_out = (vx_bool)vx_false_e;
                    }
                    else
                    {
                        /* do nothing */
                    }

                    VX_PRINT(VX_ZONE_INFO,"Create callback for node %s completed\n", ref->name);
                }

                if(status!=(vx_status)VX_SUCCESS)
                {
                    vx_reference ref;

                    ref = vxCastRefFromNode(node);

                    VX_PRINT(VX_ZONE_ERROR,"Target kernel, TIVX_CMD_NODE_CREATE failed for node %s\n", ref->name);
                    VX_PRINT(VX_ZONE_ERROR,"Please be sure the target callbacks have been registered for this core\n");
                    VX_PRINT(VX_ZONE_ERROR,"If the target callbacks have been registered, please ensure no errors are occurring within the create callback of this kernel\n");
                }
                else /* copy the target_kernel_index[] from 0th object descriptor to other object descriptors */
                {
                    uint32_t i;

                    for(i=1; i<node->pipeline_depth; i++)
                    {
                        tivx_obj_desc_memcpy(node->obj_desc[i]->target_kernel_index,
                            node->obj_desc[0]->target_kernel_index,
                            (uint32_t)sizeof(node->obj_desc[i]->target_kernel_index)
                            );
                    }
                }
            }
        }
    }

    if((vx_status)VX_SUCCESS==status)
    {
        node->is_kernel_created = (vx_bool)vx_true_e;
    }

    return status;
}

vx_status ownNodeKernelDeinit(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((vx_bool)vx_false_e == node->kernel->is_target_kernel)
    {
        if ((vx_bool)vx_true_e == node->is_initialized)
        {
            if(node->kernel->deinitialize != NULL)
            {
                node->local_data_set_allow = (vx_bool)vx_true_e;

                ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_true_e);

                /* user has given deinitialize function so call it */
                status = node->kernel->deinitialize(node, node->parameters, node->kernel->signature.num_parameters);

                ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_false_e);

                if ((vx_status)VX_SUCCESS==status)
                {
                    node->is_initialized = (vx_bool)vx_false_e;
                }

                node->local_data_set_allow = (vx_bool)vx_false_e;

                if((vx_status)VX_SUCCESS!=status)
                {
                    VX_PRINT(VX_ZONE_ERROR,"Target kernel, deinitialize callback failed\n");
                }
            }
        }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR012
<justification end> */
        if(((vx_bool)vx_true_e == node->local_data_ptr_is_alloc)
            &&
            (NULL != node->local_data_ptr)
            &&
            (0U != node->local_data_size)
            )
        {
            status = tivxMemFree(node->local_data_ptr, (uint32_t)node->local_data_size,
                (vx_enum)TIVX_MEM_EXTERNAL);
            node->local_data_ptr = NULL;
            node->local_data_size = 0;
            node->local_data_ptr_is_alloc = (vx_bool)vx_false_e;
        }
/* LDRA_JUSTIFY_END */
    }
    else /* Target kernel condition */
    {
        uint16_t obj_desc_id[1];

        obj_desc_id[0] = node->obj_desc[0]->base.obj_desc_id;

        if ((vx_bool)vx_true_e == node->is_initialized)
        {
            if(node->kernel->deinitialize != NULL)
            {
                node->local_data_set_allow = (vx_bool)vx_true_e;
                tivx_obj_desc_node_t *node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
                uint32_t num_params = node->kernel->signature.num_parameters;

                if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == (vx_bool)vx_true_e )
                {
                    vx_reference params[TIVX_KERNEL_MAX_PARAMS];
                    vx_reference parent_ref[TIVX_KERNEL_MAX_PARAMS];
                    uint32_t i, n;

                    for(i=0; i<num_params ; i++)
                    {
                        parent_ref[i] = NULL;

                        if((0 != node->replicated_flags[i]) && /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR013 */
                           (NULL != node->parameters[i])) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR014 */
                        {
                            parent_ref[i] = node->parameters[i]->scope;
                        }
                    }

                    for(n=0; n<node_obj_desc->num_of_replicas; n++)
                    {
                        for(i=0; i<num_params ; i++)
                        {
                            params[i] = NULL;
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR015
<justification end>*/
                            if(node->replicated_flags[i] != 0)
/* LDRA_JUSTIFY_END */
                            {
                                if(parent_ref[i] != NULL) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR016 */
                                {
                                    if(parent_ref[i]->type==(vx_enum)VX_TYPE_OBJECT_ARRAY)
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = (vxCastRefAsObjectArray(parent_ref[i], NULL))->ref[n];
                                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR017
<justification end>*/
                                    else if(parent_ref[i]->type==(vx_enum)VX_TYPE_PYRAMID)
/* LDRA_JUSTIFY_END */
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = vxCastRefFromImage((vxCastRefAsPyramid(parent_ref[i], NULL))->img[n]);
                                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT004
<justification end>*/
                                    else
                                    {
                                        params[i] = NULL;
                                    }
/* LDRA_JUSTIFY_END */
                                }
                            }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT005
<justification end>*/
                            else
                            {
                                params[i] = node->parameters[i];
                            }
/* LDRA_JUSTIFY_END */
                        }

                        ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_true_e);

                        tivxCheckStatus(&status, node->kernel->deinitialize(node, params, num_params));

                        ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_false_e);
                    }
                }
                else
                {
                    ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_true_e);

                    /* user has given deinitialize function so call it */
                    status = node->kernel->deinitialize(node, node->parameters,
                        node->kernel->signature.num_parameters);

                    ownNodeSetAccessibility(node, node->kernel->signature.num_parameters, (vx_bool)vx_false_e);

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT006
<justification end>*/
                    if(status!=(vx_status)VX_SUCCESS)
                    {
                        VX_PRINT(VX_ZONE_ERROR,"deinitialize callback failed\n");
                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT006
<justification end>*/
                    else
/* LDRA_JUSTIFY_END */
                    {
                        node->is_initialized = (vx_bool)vx_false_e;
                    }
                }
            }
        }
        if( ((vx_bool)vx_true_e==node->is_kernel_created) ||
            ((vx_bool)vx_true_e==node->is_timed_out) )
        {
            if(((vx_bool)vx_true_e == node->is_super_node) ||/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR018 */
               (NULL == node->super_node)) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR019 */
            {
                vx_reference ref;

                ref = vxCastRefFromNode(node);

                VX_PRINT(VX_ZONE_INFO,"Calling delete callback for node %s\n", ref->name);

                vx_status tmp_status = ownContextSendCmd(node->base.context,
                        node->obj_desc[0]->target_id, (vx_enum)TIVX_CMD_NODE_DELETE,
                        1, obj_desc_id, node->timeout_val);
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR020
<justification end>*/
                if(status == (vx_status)VX_SUCCESS)
                {
                    if(tmp_status == (vx_status)VX_SUCCESS)
                    {
                        status = (vx_status)VX_SUCCESS;
                    }
                    else
                    {
                        status = tmp_status;
                    }
                }
/* LDRA_JUSTIFY_END */
                VX_PRINT(VX_ZONE_INFO,"Delete callback for node %s completed\n", ref->name);
                /* Note: not providing an option to reset "is_timed_out"
                 * here given that if the create timeout failed, then this
                 * would override that status, even if it was only successful
                 * in the deletion of the node */
                if ((vx_status)TIVX_ERROR_EVENT_TIMEOUT==status)
                {
                    node->is_timed_out = (vx_bool)vx_true_e;
                }
            }
        }
    }

    if( ((vx_status)VX_SUCCESS==status) ||
        ((vx_bool)vx_true_e==node->is_timed_out) )
    {
        node->is_kernel_created = (vx_bool)vx_false_e;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Target kernel, TIVX_CMD_NODE_DELETE failed\n");
    }

    return status;
}

vx_status ownNodeKernelSchedule(vx_node node, uint32_t pipeline_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    VX_PRINT(VX_ZONE_INFO,"Scheduling Node (node=%d, pipe=%d)\n",
                    node->obj_desc[pipeline_id]->base.obj_desc_id,
                    node->obj_desc[pipeline_id]->pipeline_id
                    );

    status = ownObjDescSend(node->obj_desc[pipeline_id]->target_id, node->obj_desc[pipeline_id]->base.obj_desc_id);

    return status;
}

vx_status ownNodeUserKernelExecute(vx_node node, vx_reference prm_ref[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ((NULL != node) && (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e))
    {
        if((NULL != node->kernel)
        && (node->is_kernel_created == (vx_bool)vx_true_e))
        {
            if((NULL != node->kernel->function) &&
                ((vx_bool)vx_false_e == node->kernel->is_target_kernel))
            {
                tivx_obj_desc_node_t *node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
                uint32_t num_params = node->kernel->signature.num_parameters;

                if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == (vx_bool)vx_true_e )
                {
                    vx_reference params[TIVX_KERNEL_MAX_PARAMS];
                    vx_reference parent_ref[TIVX_KERNEL_MAX_PARAMS];
                    uint32_t i, n;

                    for(i=0; i<num_params ; i++)
                    {
                        parent_ref[i] = NULL;

                        if((0 != node->replicated_flags[i]) &&
                           (NULL != prm_ref[i]))
                        {
                            if(prm_ref[i]->scope->obj_desc != NULL)
                            {
                                parent_ref[i] = prm_ref[i]->scope;
                            }
                            else
                            {
                                parent_ref[i] = prm_ref[i];
                            }
                        }
                    }

                    for(n=0; n<node_obj_desc->num_of_replicas; n++)
                    {
                        for(i=0; i<num_params ; i++)
                        {
                            params[i] = NULL;
                            if(node->replicated_flags[i] != 0)
                            {
                                if(parent_ref[i] != NULL)/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR022 */
                                {
                                    if(parent_ref[i]->type==(vx_enum)VX_TYPE_OBJECT_ARRAY)
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = (vxCastRefAsObjectArray(parent_ref[i], NULL))->ref[n];
                                    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR023
<justification end>*/
                                    else if(parent_ref[i]->type==(vx_enum)VX_TYPE_PYRAMID)
/* LDRA_JUSTIFY_END */
                                    {
                                        /*status set to NULL due to preceding type check*/
                                        params[i] = vxCastRefFromImage((vxCastRefAsPyramid(parent_ref[i], NULL))->img[n]);
                                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT007
<justification end>*/
                                    else
                                    {
                                        params[i] = NULL;
                                    }
/* LDRA_JUSTIFY_END */
                                }
                            }
                            else
                            {
                                params[i] = prm_ref[i];
                            }
                        }

                        ownNodeUserKernelUpdateParamsFlags(node->kernel, params, num_params, (vx_bool)vx_true_e);

                        tivxCheckStatus(&status, node->kernel->function(node, params, num_params));

                        ownNodeUserKernelUpdateParamsFlags(node->kernel, params, num_params, (vx_bool)vx_false_e);
                    }
                }
                else
                {
                    uint32_t i;

                    for(i=0; i<num_params; i++ )
                    {
                        if ( (NULL != node->parameters[i]) && (NULL != prm_ref[i]) )
                        {
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT010
<justification end> */
                            if((node->parameters[i]->type) != (prm_ref[i]->type) )
                            {
                                VX_PRINT(VX_ZONE_WARNING, "node %s param index %d not match, expect type %d, get %d \n", node->base.name, i, node->parameters[i]->type, prm_ref[i]->type );
                                if(prm_ref[i]->scope != NULL)
                                {
                                    if( (prm_ref[i]->scope->obj_desc != NULL) &&
                                        (prm_ref[i]->scope->type == node->parameters[i]->type) )
                                    {
                                        prm_ref[i] = prm_ref[i]->scope;
                                    }
                                }
                            }
/* LDRA_JUSTIFY_END */
                        }
                    }

                    ownNodeUserKernelUpdateParamsFlags(node->kernel, prm_ref, num_params, (vx_bool)vx_true_e);

                    /* user has given user kernel function so call it */
                    status = node->kernel->function(node, prm_ref, num_params);

                    ownNodeUserKernelUpdateParamsFlags(node->kernel, prm_ref, num_params, (vx_bool)vx_false_e);
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "kernel function is NULL and/or not target kernel\n");
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "invalid kernel reference\n");
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

vx_status ownResetNodePerf(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        node->perf.tmp = 0;
        node->perf.beg = 0;
        node->perf.end = 0;
        node->perf.sum = 0;
        node->perf.avg = 0;
        node->perf.min = 0xFFFFFFFFFFFFFFFFUL;
        node->perf.num = 0;
        node->perf.max = 0;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status ownUpdateNodePerf(vx_node node, uint32_t pipeline_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        tivx_uint32_to_uint64(
                &node->perf.beg,
                node->obj_desc[pipeline_id]->exe_time_beg_h,
                node->obj_desc[pipeline_id]->exe_time_beg_l
            );

        tivx_uint32_to_uint64(
                &node->perf.end,
                node->obj_desc[pipeline_id]->exe_time_end_h,
                node->obj_desc[pipeline_id]->exe_time_end_l
            );

        node->perf.tmp = (node->perf.end - node->perf.beg)*1000U; /* convert to nano secs */
        node->perf.sum += node->perf.tmp;
        node->perf.num++;
        if(node->perf.tmp < node->perf.min)
        {
            node->perf.min = node->perf.tmp;
        }
        if(node->perf.tmp > node->perf.max)
        {
            node->perf.max = node->perf.tmp;
        }
        node->perf.avg = node->perf.sum/node->perf.num;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status ownSetNodeImmTarget(vx_node node)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        vx_context context = node->base.context;

        status = vxSetNodeTarget(node,
                    context->imm_target_enum,
                    context->imm_target_string
                );
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

vx_status ownSetNodeAttributeValidRectReset(vx_node node, vx_bool is_reset)
{
   vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        node->valid_rect_reset = is_reset;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    return status;
}

uint32_t ownNodeGetNumParameters(vx_node node)
{
    /* references and structure fields values are checked outside this API,
      so simply return the required parameter */
    return node->kernel->signature.num_parameters;
}

vx_enum ownNodeGetParameterDir(vx_node node, uint32_t prm_index)
{
    /* references and structure fields values are checked outside this API,
      so simply return the required parameter */
    return node->kernel->signature.directions[prm_index];
}

vx_reference ownNodeGetParameterRef(vx_node node, uint32_t prm_index)
{
    /* references and structure fields values are checked outside this API,
      so simply return the required parameter */
    return node->parameters[prm_index];
}

vx_status ownNodeAddOutNode(vx_node node, vx_node out_node)
{
    vx_bool is_present = (vx_bool)vx_false_e;
    uint32_t num_out_nodes = node->obj_desc[0]->num_out_nodes;
    uint16_t out_node_id = out_node->obj_desc[0]->base.obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;

    /* check if out_node is already part of output node list associated with this node */
    for(i=0; i<num_out_nodes; i++)
    {
        if(out_node_id == node->obj_desc[0]->out_node_id[i])
        {
            is_present = (vx_bool)vx_true_e;
            break;
        }
    }
    if(is_present == (vx_bool)vx_false_e)
    {
        if(num_out_nodes < TIVX_NODE_MAX_OUT_NODES)
        {
            node->obj_desc[0]->out_node_id[num_out_nodes] = out_node_id;
            num_out_nodes++;
            node->obj_desc[0]->num_out_nodes = num_out_nodes;
            ownLogSetResourceUsedValue("TIVX_NODE_MAX_OUT_NODES", (uint16_t)num_out_nodes);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "number of out nodes greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_NODE_MAX_OUT_NODES in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    return status;
}

vx_status ownNodeReplaceOutNode(vx_node node, vx_node old_out_node, vx_node new_out_node)
{
    vx_bool is_present = (vx_bool)vx_false_e;
    uint32_t num_out_nodes = node->obj_desc[0]->num_out_nodes;
    uint16_t old_out_node_id = old_out_node->obj_desc[0]->base.obj_desc_id;
    uint16_t new_out_node_id = new_out_node->obj_desc[0]->base.obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i, j;

    /* check if new_out_node is already part of output node list associated with this node */
    for(i=0; i<num_out_nodes; i++)
    {
        if(new_out_node_id == node->obj_desc[0]->out_node_id[i])
        {
            is_present = (vx_bool)vx_true_e;
            break;
        }
    }

    for(i=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR024
<justification end> */
    i<num_out_nodes;
/* LDRA_JUSTIFY_END */
    i++)
    {
        if(old_out_node_id == node->obj_desc[0]->out_node_id[i])
        {
            if(is_present == (vx_bool)vx_false_e)
            {
                /* Simply replace existing one */
                node->obj_desc[0]->out_node_id[i] = new_out_node_id;
            }
            else
            {
                /* Remove old node from the list */
                for(j=i; j<(num_out_nodes-1U); j++)
                {
                    node->obj_desc[0]->out_node_id[j] = node->obj_desc[0]->out_node_id[j+1U];
                }
                node->obj_desc[0]->num_out_nodes = node->obj_desc[0]->num_out_nodes - 1U;
            }
            break;
        }
    }
    return status;
}

vx_status ownNodeAddInNode(vx_node node, vx_node in_node)
{
    vx_bool is_present = (vx_bool)vx_false_e;
    uint32_t num_in_nodes = node->obj_desc[0]->num_in_nodes;
    uint16_t in_node_id = in_node->obj_desc[0]->base.obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;

    /* check if in_node is already part of input node list associated with this node */
    for(i=0; i<num_in_nodes; i++)
    {
        if(in_node_id == node->obj_desc[0]->in_node_id[i])
        {
            is_present = (vx_bool)vx_true_e;
            break;
        }
    }
    if(is_present == (vx_bool)vx_false_e)
    {
        if(num_in_nodes < TIVX_NODE_MAX_IN_NODES)
        {
            node->obj_desc[0]->in_node_id[num_in_nodes] = in_node_id;
            num_in_nodes++;
            node->obj_desc[0]->num_in_nodes = num_in_nodes;
            ownLogSetResourceUsedValue("TIVX_NODE_MAX_IN_NODES", (uint16_t)num_in_nodes);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "number of in nodes greater than maximum allowed\n");
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_NODE_MAX_IN_NODES in tiovx/include/TI/tivx_config.h\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
    }

    return status;
}

vx_status ownNodeReplaceInNode(vx_node node, vx_node old_in_node, vx_node new_in_node)
{
    vx_bool is_present = (vx_bool)vx_false_e;
    uint32_t num_in_nodes = node->obj_desc[0]->num_in_nodes;
    uint16_t old_in_node_id = old_in_node->obj_desc[0]->base.obj_desc_id;
    uint16_t new_in_node_id = new_in_node->obj_desc[0]->base.obj_desc_id;
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i, j;

    /* check if new_in_node is already part of input node list associated with this node */
    for(i=0; i<num_in_nodes; i++)
    {
        if(new_in_node_id == node->obj_desc[0]->in_node_id[i])
        {
            is_present = (vx_bool)vx_true_e;
            break;
        }
    }

    for(i=0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR025
<justification end> */
    i<num_in_nodes;
/* LDRA_JUSTIFY_END */
    i++)
    {
        if(old_in_node_id == node->obj_desc[0]->in_node_id[i])
        {
            if(is_present == (vx_bool)vx_false_e)
            {
                /* Simply replace existing one */
                node->obj_desc[0]->in_node_id[i] = new_in_node_id;
            }
            else
            {
                /* Remove old node from the list */
                for(j=i; j<(num_in_nodes-1U); j++)
                {
                    node->obj_desc[0]->in_node_id[j] = node->obj_desc[0]->in_node_id[j+1U];
                }
                node->obj_desc[0]->num_in_nodes = node->obj_desc[0]->num_in_nodes - 1U;
            }
            break;
        }
    }

    return status;
}

uint32_t ownNodeGetNumInNodes(vx_node node)
{
    /* references and structure fields values are checked outside this API,
      so simply return the required parameter */
    return node->obj_desc[0]->num_in_nodes;
}

uint32_t ownNodeGetNumOutNodes(vx_node node)
{
    /* references and structure fields values are checked outside this API,
      so simply return the required parameter */
    return node->obj_desc[0]->num_out_nodes;
}

vx_status ownNodeCreateUserCallbackCommand(vx_node node, uint32_t pipeline_id)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if((NULL != node) && (pipeline_id < TIVX_GRAPH_MAX_PIPELINE_DEPTH))
    {
        tivx_obj_desc_node_t *obj_desc = node->obj_desc[pipeline_id];
        tivx_obj_desc_cmd_t *obj_desc_cmd = node->obj_desc_cmd[pipeline_id];

        if(obj_desc_cmd==NULL)
        {
            obj_desc_cmd = (tivx_obj_desc_cmd_t *)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_CMD, NULL);

            node->obj_desc_cmd[pipeline_id] = obj_desc_cmd;

            if(obj_desc_cmd != NULL)
            {
                obj_desc->node_complete_cmd_obj_desc_id
                    = obj_desc_cmd->base.obj_desc_id;

                obj_desc_cmd->cmd_id = (vx_enum)TIVX_CMD_NODE_USER_CALLBACK;

                /* No ACK needed */
                obj_desc_cmd->flags = 0;

                /* this command is sent by the target node to HOST hence dst_target_id is HOST */
                obj_desc_cmd->dst_target_id = (uint32_t)ownPlatformGetTargetId(TIVX_TARGET_HOST);

                /* source is node target which is not known at this moment, however
                 * since ACK is not required for this command, this can be set to INVALID
                 */
                obj_desc_cmd->src_target_id = TIVX_TARGET_ID_INVALID;

                /* parameter is node object descriptor ID */
                obj_desc_cmd->num_obj_desc = 1;
                obj_desc_cmd->obj_desc_id[0] = obj_desc->base.obj_desc_id;
            }
            else
            {
                status = (vx_status)VX_ERROR_NO_RESOURCES;
                VX_PRINT(VX_ZONE_ERROR, "Could not allocate object descriptor for user callback\n");
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
            }
        }
    }
    return status;
}

vx_action ownNodeExecuteUserCallback(vx_node node)
{
    vx_nodecomplete_f callback;
    vx_action action = (vx_enum)VX_ACTION_CONTINUE;

    callback = vxRetrieveNodeCallback(node);

    if(NULL != callback)
    {
        /* action is ignored */
        action = callback(node);
    }

    return action;
}

vx_bool ownNodeIsPrmReplicated(vx_node node, uint32_t prm_idx)
{
    vx_bool is_replicated = (vx_bool)vx_false_e;

    if((node) && (node->obj_desc[0]))
    {
        if( (tivxFlagIsBitSet( node->obj_desc[0]->flags, TIVX_NODE_FLAG_IS_REPLICATED) == (vx_bool)vx_true_e)
            &&
            (node->replicated_flags[prm_idx] != (vx_bool)vx_false_e))
        {
            is_replicated = (vx_bool)vx_true_e;
        }
    }
    return is_replicated;
}

static uint16_t ownNodeGetObjDescId(vx_node node, uint32_t pipeline_id)
{
    uint16_t obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID;

    if((node!=NULL) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR026 */
    && (pipeline_id < node->pipeline_depth) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR027 */
    && (node->obj_desc[pipeline_id]!=NULL) ) /* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR028 */
    {
        obj_desc_id = node->obj_desc[pipeline_id]->base.obj_desc_id;
    }

    return obj_desc_id;
}

void ownNodeSetObjDescParamDirection(vx_node node)
{
    uint32_t prm_idx, prm_dir, is_prm_input, is_prm_bidi;

    is_prm_input = 0;
    is_prm_bidi = 0;

    for(prm_idx=0; prm_idx < ownNodeGetNumParameters(node); prm_idx++)
    {
        prm_dir = (uint32_t)ownNodeGetParameterDir(node, prm_idx);

        if(prm_dir==(uint32_t)VX_INPUT)
        {
            tivxFlagBitSet(&is_prm_input, ((uint32_t)1U<<prm_idx));
        }
        if(prm_dir==(uint32_t)VX_BIDIRECTIONAL)
        {
            tivxFlagBitSet(&is_prm_bidi, ((uint32_t)1U<<prm_idx));
        }
    }

    node->obj_desc[0]->is_prm_input = is_prm_input;
    node->obj_desc[0]->is_prm_bidi  = is_prm_bidi;
}

void ownNodeCheckAndSendCompletionEvent(const tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp)
{
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;
    if((node!=NULL) && (node->base.context!=NULL))
    {
        if(node->is_enable_send_complete_event != (vx_bool)vx_false_e)
        {
            if ((vx_bool)vx_true_e == node->is_context_event)
            {
                if((vx_status)VX_SUCCESS != ownEventQueueAddEvent(&node->base.context->event_queue,
                            (vx_enum)VX_EVENT_NODE_COMPLETED, timestamp, node->node_completed_app_value,
                            (uintptr_t)node->graph, (uintptr_t)node, (uintptr_t)0))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to add event to event queue\n");
                }
            }

            if ((vx_bool)vx_true_e == node->is_graph_streaming_event)
            {
                if((vx_status)VX_SUCCESS != ownEventQueueAddEvent(&node->graph->streaming_event_queue,
                            (vx_enum)VX_EVENT_NODE_COMPLETED, timestamp, node->node_completed_app_value,
                            (uintptr_t)node->graph, (uintptr_t)node, (uintptr_t)0))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to add event to event queue\n");
                }
            }
        }
    }
}

void ownNodeCheckAndSendErrorEvent(const tivx_obj_desc_node_t *node_obj_desc, uint64_t timestamp, vx_status status)
{
    vx_node node = (vx_node)(uintptr_t)node_obj_desc->base.host_ref;

    if((node!=NULL) && (node->base.context!=NULL))
    {
        if(node->is_enable_send_error_event != (vx_bool)vx_false_e)
        {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR029
<justification end> */
            if ((vx_bool)vx_true_e == node->is_context_event)
            {
                if((vx_status)VX_SUCCESS != ownEventQueueAddEvent(&node->base.context->event_queue,
                            (vx_enum)VX_EVENT_NODE_ERROR, timestamp, node->node_error_app_value,
                            (uintptr_t)node->graph, (uintptr_t)node, (uintptr_t)status))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to add event to event queue \n");
                }
            }
/* LDRA_JUSTIFY_END */
            if ((vx_bool)vx_true_e == node->is_graph_streaming_event)
            {
                if((vx_status)VX_SUCCESS != ownEventQueueAddEvent(&node->graph->streaming_event_queue,
                            (vx_enum)VX_EVENT_NODE_ERROR, timestamp, node->node_error_app_value,
                            (uintptr_t)node->graph, (uintptr_t)node, (uintptr_t)status))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to add event to event queue \n");
                }
            }
        }
    }
}

VX_API_ENTRY vx_node VX_API_CALL vxCreateGenericNode(vx_graph graph, vx_kernel kernel)
{
    vx_node node = NULL;

    if (ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) == (vx_bool)vx_true_e)
    {
        if (ownIsValidSpecificReference(vxCastRefFromKernel(kernel), (vx_enum)VX_TYPE_KERNEL) == (vx_bool)vx_true_e)
        {
            int32_t n;
            uint32_t idx;
            vx_status status;
            vx_reference ref;

            status = ownReferenceLock(&graph->base);
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to lock reference\n");
            }
            else
            {
                n = ownGraphGetFreeNodeIndex(graph);
                if(n>=0)
                {
                    ref = ownCreateReference(graph->base.context, (vx_enum)VX_TYPE_NODE, (vx_enum)VX_EXTERNAL, &graph->base);
                    if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) && (ref->type == (vx_enum)VX_TYPE_NODE))
                    {
                        /*status set to NULL due to preceding type check*/
                        node = vxCastRefAsNode(ref, NULL);
                        /* set kernel, params, graph to NULL */
                        node->kernel = NULL;
                        node->graph = NULL;
                        node->is_kernel_created = (vx_bool)vx_false_e;

                        (void)ownResetNodePerf(node);
                        for(idx=0; idx<kernel->signature.num_parameters; idx++)
                        {
                            node->parameters[idx] = NULL;
                            node->replicated_flags[idx] = (vx_bool)vx_false_e;
                            node->parameter_index_num_buf[idx] = 0;
                        }
                        node->valid_rect_reset = (vx_bool)vx_false_e;
                        for(idx=0; idx<TIVX_GRAPH_MAX_PIPELINE_DEPTH; idx++)
                        {
                            node->obj_desc_cmd[idx] = NULL;
                            node->obj_desc[idx] = NULL;
                        }
                        node->user_callback = NULL;
                        node->local_data_ptr = NULL;
                        node->local_data_size = 0;
                        node->local_data_ptr_is_alloc = (vx_bool)vx_false_e;
                        node->local_data_set_allow = (vx_bool)vx_false_e;
                        node->pipeline_depth = 1;
                        node->is_context_event = (vx_bool)vx_false_e;
                        node->is_graph_streaming_event = (vx_bool)vx_false_e;
                        node->node_completed_app_value = 0;
                        node->node_error_app_value = 0;
                        node->is_enable_send_complete_event = (vx_bool)vx_false_e;
                        node->is_enable_send_error_event    = (vx_bool)vx_false_e;
                        node->is_super_node = (vx_bool)vx_false_e;
                        node->super_node = NULL;
                        node->timeout_val = kernel->timeout_val;
                        node->node_depth = 1;
                        node->is_timed_out = (vx_bool)vx_false_e;
                        node->is_initialized = (vx_bool)vx_false_e;

                        /* assign refernce type specific callback's */
                        node->base.destructor_callback = &ownDestructNode;
                        node->base.mem_alloc_callback = NULL;
                        node->base.release_callback = &ownReleaseReferenceBufferGeneric;

                        node->obj_desc[0] = (tivx_obj_desc_node_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_NODE, vxCastRefFromNode(node));

                        if(node->obj_desc[0] == NULL)
                        {
                            (void)vxReleaseNode(&node);

                            vxAddLogEntry(&graph->base, (vx_status)VX_ERROR_NO_RESOURCES, "Could not allocate node object descriptor\n");
                            node = (vx_node)ownGetErrorObject(graph->base.context, (vx_status)VX_ERROR_NO_RESOURCES);
                            VX_PRINT(VX_ZONE_ERROR, "Could not allocate node object descriptor\n");
                            VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                            VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                        }
                        else
                        {
                            status = ownInitNodeObjDesc(node, kernel, 0);

                            if(status!=(vx_status)VX_SUCCESS)
                            {
                                (void)vxReleaseNode(&node);

                                /* no valid target associated with this node, return error */
                                vxAddLogEntry(&graph->base, status, "No target associated with kernel\n");
                                node = (vx_node)ownGetErrorObject(graph->base.context, status);
                            }
                            else
                            {
                                /* all condition successful for node creation, now set kernel, graph references */
                                node->kernel = kernel;
                                node->graph = graph;
                                node->obj_desc[0]->debug_zonemask = node->graph->debug_zonemask;

                                /* show that there are potentially multiple nodes using this kernel. */
                                (void)ownIncrementReference(&kernel->base, (vx_enum)VX_INTERNAL);

                                status = ownGraphAddNode(graph, node, n);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM006
<justification end> */
                                if((vx_status)VX_SUCCESS != status)
                                {
                                    VX_PRINT(VX_ZONE_ERROR,"Failed to add graph node\n");
                                }
/* LDRA_JUSTIFY_END */
                            }
                        }
                    }
                }
                (void)ownReferenceUnlock(&graph->base);
            }
        }
        else
        {
            vxAddLogEntry(vxCastRefFromGraph(graph), (vx_status)VX_ERROR_INVALID_REFERENCE, "Kernel %p was invalid!\n", kernel);
            node = (vx_node)ownGetErrorObject(graph->base.context, (vx_status)VX_ERROR_INVALID_REFERENCE);
        }
    }
    else
    {
        vxAddLogEntry(vxCastRefFromGraph(graph), (vx_status)VX_ERROR_INVALID_REFERENCE, "Graph %p is invalid!\n", graph);
    }

    return node;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseNode(vx_node *n)
{
    return ownReleaseReferenceInt(vxCastRefFromNodeP(n), (vx_enum)VX_TYPE_NODE, (vx_enum)VX_EXTERNAL, NULL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryNode(vx_node node, vx_enum attribute, void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        switch (attribute)
        {
            case (vx_enum)VX_NODE_PERFORMANCE:
                #if defined(BUILD_BAM)
                if(node->super_node != NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR,"'node' is part of super node so VX_NODE_PERFORMANCE query is not available. Try to query the supernode instead.\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    break;
                }
                #endif
                if (VX_CHECK_PARAM(ptr, size, vx_perf_t, 0x3U))
                {
                    (void)memcpy(ptr, (void *)&node->perf, size);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node performance failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_STATUS:
                #if defined(BUILD_BAM)
                if(node->super_node != NULL)
                {
                    VX_PRINT(VX_ZONE_ERROR,"'node' is part of super node so VX_NODE_STATUS query is not available. Try to query the supernode instead.\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    break;
                }
                #endif
                if (VX_CHECK_PARAM(ptr, size, vx_status, 0x3U))
                {
                    /* returns  status for pipeline index 0,
                     * may not reflect correct status
                     * in pipelined mode
                     */
                    *(vx_status *)ptr = (vx_status)node->obj_desc[0]->exe_status;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node status failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_LOCAL_DATA_SIZE:
                if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U))
                {
                    *(vx_size *)ptr = node->local_data_size;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node local data size failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_LOCAL_DATA_PTR:
                if (VX_CHECK_PARAM(ptr, size, uintptr_t, 0x3U))
                {
                    *(uintptr_t *)ptr = (uintptr_t)node->local_data_ptr;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node local data pointer failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_BORDER:
                if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3U))
                {
                    tivx_obj_desc_memcpy((vx_border_t *)ptr, &node->obj_desc[0]->border_mode, (uint32_t)sizeof(vx_border_t));
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node border failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_PARAMETERS:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    vx_uint32 numParams = node->kernel->signature.num_parameters;

                    (void)memcpy((vx_uint32*)ptr, &numParams, sizeof(numParams));
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node parameters failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_IS_REPLICATED:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    uint32_t is_replicated_flag = node->obj_desc[0]->flags & TIVX_NODE_FLAG_IS_REPLICATED;

                    vx_bool is_replicated;

                    if (is_replicated_flag != 0U)
                    {
                        is_replicated = (vx_bool)vx_true_e;
                    }
                    else
                    {
                        is_replicated = (vx_bool)vx_false_e;
                    }
                    *(vx_bool*)ptr = is_replicated;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query 'is node replicated' failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_REPLICATE_FLAGS:
            {
                vx_size sz = sizeof(vx_bool)*node->kernel->signature.num_parameters;
                if ((size == sz) && (((vx_size)ptr & 0x3U) == 0U))
                {
                    vx_uint32 i = 0;
                    vx_uint32 numParams = node->kernel->signature.num_parameters;
                    for (i = 0; i < numParams; i++)
                    {
                        ((vx_bool*)ptr)[i] = node->replicated_flags[i];
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node replicate flage failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            }
            case (vx_enum)VX_NODE_VALID_RECT_RESET:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    vx_bool valid_rect_reset = node->valid_rect_reset;

                    *(vx_bool*)ptr = valid_rect_reset;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node valid rect reset failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_NODE_TARGET_STRING:
                if ((ptr != NULL) && (size >= TIVX_TARGET_MAX_NAME))
                {
                    ownPlatformGetTargetName((int32_t)node->obj_desc[0]->target_id, ptr);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query node target string failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)VX_NODE_STATE:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = node->obj_desc[0]->source_state;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query VX_NODE_STATE failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_NODE_TIMEOUT:
                if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U))
                {
                    *(vx_uint32 *)ptr = node->timeout_val;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query TIVX_NODE_TIMEOUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            case (vx_enum)TIVX_NODE_IS_TIMED_OUT:
                if (VX_CHECK_PARAM(ptr, size, vx_bool, 0x3U))
                {
                    *(vx_bool *)ptr = node->is_timed_out;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR,"Query TIVX_NODE_IS_TIMED_OUT failed\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR,"Invalid query node attribute\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeAttribute(vx_node node, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if (node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"Graph has been verified\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            switch (attribute)
            {
                case (vx_enum)VX_NODE_LOCAL_DATA_SIZE:
                    if (VX_CHECK_PARAM(ptr, size, vx_size, 0x3U) &&
                        (NULL != node->kernel))
                    {
                        if(((vx_bool)vx_true_e == node->local_data_ptr_is_alloc) ||
                            (node->local_data_set_allow == (vx_bool)vx_false_e))
                        {
                            /* local data ptr is allocated or local data size cannot be set
                             * by user
                             */
                            VX_PRINT(VX_ZONE_ERROR,"local data ptr is allocated or local data size cannot be set by user\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                        else
                        {
                            node->local_data_size = *(const vx_size*)ptr;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Set node local data size failed\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                case (vx_enum)VX_NODE_LOCAL_DATA_PTR:
                    if (VX_CHECK_PARAM(ptr, size, uintptr_t, 0x3U) && (node->kernel))
                    {
                        if(((vx_bool)vx_true_e == node->local_data_ptr_is_alloc) ||
                            (node->local_data_set_allow == (vx_bool)vx_false_e))
                        {
                            /* local data ptr is allocated or local data ptr cannot be set
                             * by user
                             */
                            VX_PRINT(VX_ZONE_ERROR,"local data ptr is allocated or local data size cannot be set by user\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                        else
                        {
                            node->local_data_ptr = (void*)(*(const uintptr_t*)ptr);
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Set node local data ptr failed\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                case (vx_enum)VX_NODE_BORDER:
                    if (VX_CHECK_PARAM(ptr, size, vx_border_t, 0x3U))
                    {
                        /* set for pipeline index 0, assumed to be called before graph verify */
                        tivx_obj_desc_memcpy(&node->obj_desc[0]->border_mode, (volatile vx_border_t *)ptr, (uint32_t)sizeof(vx_border_t));
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR,"Set node border failed\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                case (vx_enum)TIVX_NODE_TIMEOUT:
                    if (VX_CHECK_PARAM(ptr, size, vx_uint32, 0x3U) &&
                        (NULL != node->kernel))
                    {
                        const vx_uint32   timeout_val = *(const vx_uint32*)ptr;

                        /* Validate the timeout. It cannot be zero. */
                        if (timeout_val == 0U)
                        {
                            VX_PRINT(VX_ZONE_ERROR,
                                     "Invalid timeout value specified: %d\n",
                                     timeout_val);
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                        else
                        {
                            node->timeout_val = timeout_val;
                        }
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Set TIVX_NODE_TIMEOUT failed\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR,"Invalid attribute\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    break;
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,"Invalid node reference\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxRemoveNode(vx_node *n)
{
    vx_node node;

    if( n != NULL)
    {
        node = *n;
    }
    else
    {
        node = NULL;
    }

    vx_status status =  (vx_status)VX_ERROR_INVALID_REFERENCE;
    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_false_e)
    {
        status = ownRemoveNodeInt(n);
        if(status == (vx_status)VX_SUCCESS) {
            /* Interpretation of spec is to release all external
               references of Nodes when vxReleaseGraph() is called AND
               all graph references count == 0 (garbage collection).
               However, it may be possible that the user would have
               already released its external reference so we need to
               check. */
            if(node->base.external_count != 0U) {
                status = ownReleaseReferenceInt(vxCastRefFromNodeP(&node),
                    (vx_enum)VX_TYPE_NODE, (vx_enum)VX_EXTERNAL, NULL);
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR,"ownReleaseReferenceInt() failed.\n");
                }
            }
            if(status == (vx_status)VX_SUCCESS) {
                *n = NULL;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"ownRemoveNodeInt() failed.\n");
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeCallback(vx_node node, vx_nodecomplete_f callback)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if ((callback) && (node->user_callback))
        {
            VX_PRINT(VX_ZONE_ERROR, "callback and user callback failed\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            uint32_t i;

            node->user_callback = callback;

            for(i=0; i<node->pipeline_depth; i++)
            {
                if(callback != NULL)
                {
                    tivxFlagBitSet(&node->obj_desc[i]->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK);
                }
                else
                {
                    tivxFlagBitClear(&node->obj_desc[i]->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK);
                }
            }
            status = (vx_status)VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_nodecomplete_f VX_API_CALL vxRetrieveNodeCallback(vx_node node)
{
    vx_nodecomplete_f cb = NULL;
    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        cb = node->user_callback;
    }
    return cb;
}

VX_API_ENTRY vx_status VX_API_CALL vxReplicateNode(vx_graph graph, vx_node first_node, vx_bool replicate[], vx_uint32 number_of_parameters)
{
    vx_uint32 n;
    vx_uint32 p;
    vx_uint32 numParams = 0;
    vx_size   num_of_replicas = 0;
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromGraph(graph), (vx_enum)VX_TYPE_GRAPH) != (vx_bool)vx_true_e)
    {
        vxAddLogEntry(vxCastRefFromGraph(graph), (vx_status)VX_ERROR_INVALID_REFERENCE, "Graph %p is invalid!\n", graph);
        VX_PRINT(VX_ZONE_ERROR, "Graph %p is invalid!\n", graph);
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if (ownIsValidSpecificReference(vxCastRefFromNode(first_node), (vx_enum)VX_TYPE_NODE) != (vx_bool)vx_true_e)
        {
            vxAddLogEntry(vxCastRefFromNode(first_node), (vx_status)VX_ERROR_INVALID_REFERENCE, "Node %p is invalid!\n", first_node);
            VX_PRINT(VX_ZONE_ERROR, "Node %p is invalid!\n", first_node);
            status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if (first_node->graph != graph)
        {
            VX_PRINT(VX_ZONE_ERROR, "first node graph does node match given graph\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        if (replicate == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR, "replicate is NULL\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* validate replicated params */
        (void)vxQueryNode(first_node, (vx_enum)VX_NODE_PARAMETERS, &numParams, sizeof(numParams));

        if (numParams != number_of_parameters)
        {
            VX_PRINT(VX_ZONE_ERROR, "numParams does not equal number_of_parameters\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        for (p = 0; p < number_of_parameters; p++)
        {
            vx_parameter param = NULL;
            vx_reference ref = NULL;
            vx_enum type = 0;
            vx_enum state = 0;
            vx_enum dir = 0;
            vx_status temp_status = (vx_status)VX_FAILURE;

            param = vxGetParameterByIndex(first_node, p);

            if (NULL != param)
            {
                /* these condition checks are done
                 * in the rest of the code
                 */
                (void)vxQueryParameter(param, (vx_enum)VX_PARAMETER_TYPE, &type, sizeof(vx_enum));
                (void)vxQueryParameter(param, (vx_enum)VX_PARAMETER_REF, &ref, sizeof(vx_reference));
                (void)vxQueryParameter(param, (vx_enum)VX_PARAMETER_STATE, &state, sizeof(vx_enum));
                (void)vxQueryParameter(param, (vx_enum)VX_PARAMETER_DIRECTION, &dir, sizeof(vx_enum));

                if((state==(vx_enum)VX_PARAMETER_STATE_OPTIONAL) && (ownIsValidSpecificReference(ref, type) == (vx_bool)vx_false_e))
                {
                    /* parameter reference is invalid but since parameter is optional,
                     * this is not a error condition
                     */
                }
                else
                {
                    if ((replicate[p] == (vx_bool)vx_false_e) && ((dir == (vx_enum)VX_OUTPUT) || (dir == (vx_enum)VX_BIDIRECTIONAL)))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Parameter %d direction is incorrect\n", p);
                        status = (vx_status)VX_FAILURE;
                    }

                    if(status == (vx_status)VX_SUCCESS)
                    {
                        if (replicate[p] == (vx_bool)vx_true_e)
                        {
                            if (((NULL != ref) && ((vx_enum)VX_TYPE_REFERENCE == type)) ||/* TIOVX-1900- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR031 */
                                (ownIsValidSpecificReference(ref, type) == (vx_bool)vx_true_e))
                            {
                                vx_size items = 0;
                                if (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_PYRAMID) == (vx_bool)vx_true_e)
                                {
                                    /*status set to NULL due to preceding type check*/
                                    vx_pyramid pyramid = vxCastRefAsPyramid(ref->scope, NULL);
                                    (void)vxQueryPyramid(pyramid, (vx_enum)VX_PYRAMID_LEVELS, &items, sizeof(vx_size));
                                }
                                else if (ownIsValidSpecificReference(ref->scope, (vx_enum)VX_TYPE_OBJECT_ARRAY) == (vx_bool)vx_true_e)
                                {
                                    /*status set to NULL due to preceding type check*/
                                    vx_object_array object_array = vxCastRefAsObjectArray(ref->scope, NULL);
                                    (void)vxQueryObjectArray(object_array, (vx_enum)VX_OBJECT_ARRAY_NUMITEMS, &items, sizeof(vx_size));
                                }
                                else
                                {
                                    VX_PRINT(VX_ZONE_ERROR, "Invalid reference type\n");
                                    status = (vx_status)VX_FAILURE;
                                }
                                if(status == (vx_status)VX_SUCCESS)
                                {
                                    if (num_of_replicas == 0U)
                                    {
                                        num_of_replicas = items;
                                    }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT008
<justification end> */
                                    if ((num_of_replicas != 0U) && (items != num_of_replicas))
                                    {
                                        VX_PRINT(VX_ZONE_ERROR, "Number of replicas is not equal to zero and not equal to items\n");
                                        status = (vx_status)VX_FAILURE;
                                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT009
<justification end> */
                                    if (num_of_replicas > TIVX_NODE_MAX_REPLICATE)
                                    {
                                        VX_PRINT(VX_ZONE_ERROR, "Number of replicas is greater than maximum allowed\n");
                                        VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of TIVX_NODE_MAX_REPLICATE in tiovx/include/TI/tivx_config.h\n");
                                        status = (vx_status)VX_FAILURE;
                                    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UTJT009
<justification end> */
                                    else
/* LDRA_JUSTIFY_END */
                                    {
                                        ownLogSetResourceUsedValue("TIVX_NODE_MAX_REPLICATE", (uint16_t)num_of_replicas);
                                    }
                                }
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Invalid reference type\n");
                                status = (vx_status)VX_FAILURE;
                            }
                        }
                    }
                }
                if (NULL != ref)
                {
                    temp_status = vxReleaseReference(&ref);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM005
<justification end> */
                    if((vx_status)VX_SUCCESS != temp_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR,"Failed to release reference at index %d\n", p);
                    }
/* LDRA_JUSTIFY_END */
                }
                temp_status = vxReleaseParameter(&param);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_NODE_UM004
<justification end> */
                if((vx_status)VX_SUCCESS != temp_status)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,"Failed to release reference to parameter at index %d\n", p);
                }
/* LDRA_JUSTIFY_END */
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Parameter %d is NULL!\n", p);
                status = (vx_status)VX_FAILURE;
            }
            if(status != (vx_status)VX_SUCCESS)
            {
                break;
            }
        }
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        /* set replicate flag for node */
        first_node->obj_desc[0]->flags |= TIVX_NODE_FLAG_IS_REPLICATED;
        first_node->obj_desc[0]->is_prm_replicated = 0;
        first_node->obj_desc[0]->num_of_replicas = (uint32_t)num_of_replicas;

        for (n = 0u; n < number_of_parameters; n++)
        {
            if(replicate[n] != 0)
            {
                first_node->obj_desc[0]->is_prm_replicated |= (((uint32_t)1U)<<n);
            }

            first_node->replicated_flags[n] = replicate[n];
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL tivxSetNodeTileSize(vx_node node, vx_uint32 block_width, vx_uint32 block_height)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    tivx_obj_desc_node_t *node_obj_desc;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        /* In TI implementation, tivxSetNodeTileSize() cannot be called after a graph is verified
         *
         * This is because it will set the tile size at the graph verify stage
         */
        if (node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Graph has been verified\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            node_obj_desc = (tivx_obj_desc_node_t *)node->obj_desc[0];
            node_obj_desc->block_width = block_width;
            node_obj_desc->block_height = block_height;
            status = (vx_status)VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetNodeTarget(vx_node node, vx_enum target_enum, const char* target_string)
{
    vx_status status = (vx_status)VX_ERROR_INVALID_REFERENCE;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        /* In TI implementation, vxSetNodeTarget() cannot be called after a graph is verified
         *
         * This is because during verify kernel on target side is initailized
         * If this API call changes the target then we need to delete the
         * kernel from previous target and create again in new target during
         * graph verify.
         * This is possible but complex to do, a simpler condition is
         * to not allow this API after graph verify is called.
         */
        if (node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Graph has been verified\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            switch (target_enum)
            {
                case (vx_enum)VX_TARGET_ANY:
                    /* nothing to do, use the default target set during node create */
                    status = (vx_status)VX_SUCCESS;
                    break;

                case (vx_enum)VX_TARGET_STRING:
                    {
                        vx_enum target_id;

                        if ((0 == strncmp(target_string, "any",
                                TIVX_TARGET_MAX_NAME)) ||
                            (0 == strncmp(target_string, "aNy",
                                TIVX_TARGET_MAX_NAME)) ||
                            (0 == strncmp(target_string, "ANY",
                                TIVX_TARGET_MAX_NAME)))
                        {
                            status = (vx_status)VX_SUCCESS;
                        }
                        else
                        {
                            target_id = ownKernelGetTarget(node->kernel,
                                target_string);

                            if(target_id == (vx_enum)TIVX_TARGET_ID_INVALID)
                            {
                                vx_reference ref;

                                ref = vxCastRefFromNode(node);

                                /* use default and return error */
                                VX_PRINT(VX_ZONE_ERROR, "Target ID is invalid for node %s\n", ref->name);
                                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                            }
                            else
                            {
                                node->obj_desc[0]->target_id = (uint32_t)target_id;
                                status = (vx_status)VX_SUCCESS;
                            }
                        }
                    }
                    break;

                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid target enum\n");
                    status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                    break;
            }
        }
    }
    return status;
}

void ownNodeClearExecuteState(vx_node node, uint32_t pipeline_id)
{
    if ((NULL != node) && (pipeline_id<TIVX_GRAPH_MAX_PIPELINE_DEPTH) )
    {
        tivx_obj_desc_node_t *obj_desc = node->obj_desc[pipeline_id];

        if (NULL != obj_desc)
        {
            tivxFlagBitClear(&obj_desc->flags, TIVX_NODE_FLAG_IS_EXECUTED);
        }
    }
}

vx_status ownNodeSetParameter(vx_node node, vx_uint32 idx, vx_reference value)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != node->parameters[idx]) {
        status = ownReleaseReferenceInt(&node->parameters[idx],
            node->parameters[idx]->type, (vx_enum)VX_INTERNAL, NULL);
        if((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to destroy reference\n");
        }
    }
    if(status == (vx_status)VX_SUCCESS)
    {
        (void)ownIncrementReference(value, (vx_enum)VX_INTERNAL);
        node->parameters[idx] = (vx_reference)value;

        /* Assign parameter descriptor id in the node */
        node->obj_desc[0]->data_id[idx] = ownReferenceGetObjDescId(value);
    }
    return status;
}

vx_node ownNodeGetNextNode(vx_node node, vx_uint32 idx)
{
    vx_node next_node = NULL;

    if((node) && (node->obj_desc[0]) && (idx < node->obj_desc[0]->num_out_nodes))
    {
        tivx_obj_desc_node_t *next_node_obj_desc;

        next_node_obj_desc =
            (tivx_obj_desc_node_t *)
                ownObjDescGet( node->obj_desc[0]->out_node_id[idx] );

        if(next_node_obj_desc != NULL)
        {
            next_node = (vx_node)(uintptr_t)next_node_obj_desc->base.host_ref;
        }
    }
    return next_node;
}

vx_node ownNodeGetNextInNode(vx_node node, vx_uint32 idx)
{
    vx_node next_node = NULL;

    if((node) && (node->obj_desc[0]) && (idx < node->obj_desc[0]->num_in_nodes))
    {
        tivx_obj_desc_node_t *next_node_obj_desc;

        next_node_obj_desc =
            (tivx_obj_desc_node_t *)
                ownObjDescGet( node->obj_desc[0]->in_node_id[idx] );

        if(next_node_obj_desc != NULL)
        {
            next_node = (vx_node)(uintptr_t)next_node_obj_desc->base.host_ref;
        }
    }
    return next_node;
}

vx_status ownNodeRegisterEvent(vx_node node, vx_enum event_type, vx_uint32 app_value)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if (node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR, "Cannot register event on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        else
        {
            uint32_t i;

            for(i=0; i<node->pipeline_depth; i++)
            {
                tivxFlagBitSet(&node->obj_desc[i]->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK);
            }

            if ((vx_enum)VX_EVENT_NODE_COMPLETED == event_type)
            {
                node->is_enable_send_complete_event = (vx_bool)vx_true_e;
                node->node_completed_app_value = app_value;
            }
            else if ((vx_enum)VX_EVENT_NODE_ERROR == event_type)
            {
                node->is_enable_send_error_event = (vx_bool)vx_true_e;
                node->node_error_app_value = app_value;
            }
            else
            {
                /* do nothing */
            }

            VX_PRINT(VX_ZONE_INFO, "Enabling event at node [%s]\n", node->base.name);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid node\n");
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    return status;
}

void ownNodeLinkObjDescForPipeline(vx_node node)
{
    uint32_t pipe_id;
    tivx_obj_desc_node_t *obj_desc;
    tivx_obj_desc_node_t *obj_desc_0; /* 0th object descriptor */
    uint32_t linked_node_idx;
    vx_node linked_node;

    obj_desc_0 = node->obj_desc[0];

    for(pipe_id=1; pipe_id<node->pipeline_depth; pipe_id++)
    {
        /* current object descriptor */
        obj_desc = node->obj_desc[pipe_id];

        /* for input nodes */
        for(linked_node_idx=0; linked_node_idx<obj_desc_0->num_in_nodes; linked_node_idx++)
        {
            /* get object descriptor of input to current node */
            linked_node = vxCastRefAsNode(ownReferenceGetHandleFromObjDescId(obj_desc_0->in_node_id[linked_node_idx]), NULL);
            obj_desc->in_node_id[linked_node_idx] = ownNodeGetObjDescId(linked_node, pipe_id);
        }
        /* for output nodes */
        for(linked_node_idx=0; linked_node_idx<obj_desc_0->num_out_nodes; linked_node_idx++)
        {
            /* get object descriptor of output from current node */
            linked_node = vxCastRefAsNode(ownReferenceGetHandleFromObjDescId(obj_desc_0->out_node_id[linked_node_idx]), NULL);
            obj_desc->out_node_id[linked_node_idx] = ownNodeGetObjDescId(linked_node, pipe_id);
        }
    }

}

vx_status ownNodeAllocObjDescForPipeline(vx_node node, uint32_t pipeline_depth)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;

    if((node->pipeline_depth == 1U) && (pipeline_depth > 1U))
    {
        tivx_obj_desc_node_t *obj_desc;
        tivx_obj_desc_node_t *obj_desc_0; /* 0th object descriptor */
        uint32_t pipe_id;

        /* i.e node needs to get pipelined and object descriptor alloc not done previously */

        node->pipeline_depth = pipeline_depth;

        obj_desc_0 = node->obj_desc[0];

        /* alloc object descriptor, 0th one is alloacted earlier so allocate other ones */
        for(pipe_id=1; pipe_id<node->pipeline_depth; pipe_id++)
        {
            node->obj_desc[pipe_id] = (tivx_obj_desc_node_t*)ownObjDescAlloc((vx_enum)TIVX_OBJ_DESC_NODE, vxCastRefFromNode(node));
            if(node->obj_desc[pipe_id] != NULL)
            {
                obj_desc = node->obj_desc[pipe_id];

                /* make obj descriptor identical to 0th obj descriptor
                 */
                obj_desc->flags = obj_desc_0->flags;
                obj_desc->kernel_id = obj_desc_0->kernel_id;
                obj_desc->kernel_name_obj_desc_id = obj_desc_0->kernel_name_obj_desc_id;
                obj_desc->target_id = obj_desc_0->target_id;
                obj_desc->node_complete_cmd_obj_desc_id = (vx_enum)TIVX_OBJ_DESC_INVALID; /* obj_desc->node_complete_cmd_obj_desc_id, allocated later during graph verify */
                /* obj_desc->target_kernel_index[], updated after kernel init */
                obj_desc->exe_status = 0;
                obj_desc->exe_time_beg_h = 0;
                obj_desc->exe_time_beg_l = 0;
                obj_desc->exe_time_end_h = 0;
                obj_desc->exe_time_end_l = 0;
                obj_desc->num_params = obj_desc_0->num_params;
                obj_desc->num_out_nodes = obj_desc_0->num_out_nodes;
                obj_desc->num_in_nodes = obj_desc_0->num_in_nodes;
                tivx_obj_desc_memcpy(&obj_desc->border_mode, &obj_desc_0->border_mode, (uint32_t)sizeof(vx_border_t));
                obj_desc->is_prm_replicated = obj_desc_0->is_prm_replicated;
                obj_desc->num_of_replicas = obj_desc_0->num_of_replicas;
                obj_desc->debug_zonemask = obj_desc_0->debug_zonemask;

                /* copying data_id[], out_node_id[], in_node_id[]
                 * from 0th obj desc but these are overriden later
                 */
                for(i=0; i<TIVX_KERNEL_MAX_PARAMS; i++)
                {
                    obj_desc->data_id[i] = obj_desc_0->data_id[i];
                    obj_desc->data_ref_q_id[i] = (vx_enum)TIVX_OBJ_DESC_INVALID;
                }
                for(i=0; i<TIVX_NODE_MAX_OUT_NODES; i++)
                {
                    obj_desc->out_node_id[i] = obj_desc_0->out_node_id[i];
                }
                for(i=0; i<TIVX_NODE_MAX_IN_NODES; i++)
                {
                    obj_desc->in_node_id[i] = obj_desc_0->in_node_id[i];
                }

                obj_desc->state = obj_desc_0->state;
                obj_desc->blocked_node_id = obj_desc_0->blocked_node_id;
                /* pipeline ID is simply the index of object descriptor within node->obj_desc[] */
                obj_desc->pipeline_id = (uint16_t)pipe_id;
                obj_desc->prev_pipe_node_id = (vx_enum)TIVX_OBJ_DESC_INVALID; /* updated later */
                obj_desc->is_prm_input = obj_desc_0->is_prm_input;
                obj_desc->is_prm_bidi  = obj_desc_0->is_prm_bidi;
                obj_desc->is_prm_data_ref_q = 0; /* this field is updated later */
                obj_desc->is_prm_array_element = 0; /* this field is updated later */
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Unable to allocate obj desc for node\n");
                VX_PRINT(VX_ZONE_ERROR, "Exceeded max object descriptors available. Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value\n");
                VX_PRINT(VX_ZONE_ERROR, "Increase TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST value in include/TI/soc/tivx_config_<soc>.h\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
        }

        /* setup prev_pipe_node_id */
        if(status==(vx_status)VX_SUCCESS)
        {
            tivx_obj_desc_node_t *prev_obj_desc;
            uint32_t prev_pipe_id;

            for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
            {
                prev_pipe_id = (pipe_id+node->pipeline_depth-1U)%node->pipeline_depth;

                prev_obj_desc = node->obj_desc[prev_pipe_id];
                obj_desc = node->obj_desc[pipe_id];

                obj_desc->prev_pipe_node_id = prev_obj_desc->base.obj_desc_id;
            }
        }
    }


    return status;
}

void ownNodeLinkDataRefQueue(vx_node node, uint32_t prm_id, tivx_data_ref_queue data_ref_q)
{
    uint32_t pipe_id;

    for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
    {
        tivxFlagBitSet(&node->obj_desc[pipe_id]->is_prm_data_ref_q, ((uint32_t)1<<prm_id));
        node->obj_desc[pipe_id]->data_ref_q_id[prm_id] = ownDataRefQueueGetObjDescId(data_ref_q, pipe_id);
    }
}

void ownNodeLinkArrayElement(vx_node node, uint32_t prm_id)
{
    uint16_t pipe_id;
    for(pipe_id=0; pipe_id<node->pipeline_depth; pipe_id++)
    {
        tivxFlagBitSet(&node->obj_desc[pipe_id]->is_prm_array_element, ((uint32_t)1<<prm_id));
    }
}

uint32_t ownNodeGetParameterNumBuf(vx_node node, vx_uint32 idx)
{
    vx_uint32 num_buf = 0, num_pipeup_bufs = 0, sink_bufs = 0;

    if((node != NULL)
      && (idx < ownNodeGetNumParameters(node))
      && ((vx_enum)VX_INPUT != ownNodeGetParameterDir(node, idx)))
    {
        num_buf = node->parameter_index_num_buf[idx];

        sink_bufs = node->kernel->connected_sink_bufs - 1U;

        num_pipeup_bufs = node->kernel->num_pipeup_bufs + sink_bufs;

        if (num_pipeup_bufs > num_buf)
        {
            num_buf = num_pipeup_bufs;
        }
    }
    return num_buf;
}

vx_status VX_API_CALL tivxSetNodeParameterNumBufByIndex(vx_node node, vx_uint32 idx, vx_uint32 num_buf)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if (node->graph->verified == (vx_bool)vx_true_e)
        {
            VX_PRINT(VX_ZONE_ERROR,"Not supported on verified graph\n");
            status = (vx_status)VX_ERROR_NOT_SUPPORTED;
        }
        if(status==(vx_status)VX_SUCCESS)
        {
            if( (idx < ownNodeGetNumParameters(node))
                && (num_buf < TIVX_OBJ_DESC_QUEUE_MAX_DEPTH)
                && ((vx_enum)VX_INPUT != ownNodeGetParameterDir(node, idx))
                )
            {
                node->parameter_index_num_buf[idx] = num_buf;
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR,"Invalid parameters\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,"Invalid node\n");
    }
    return status;
}

vx_status VX_API_CALL tivxGetNodeParameterNumBufByIndex(vx_node node, vx_uint32 idx, vx_uint32 *num_buf)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if( (idx < ownNodeGetNumParameters(node))
            && (ownNodeGetParameterDir(node, idx) == (vx_enum)VX_OUTPUT)
            )
        {
            *(vx_uint32 *)num_buf = node->parameter_index_num_buf[idx];
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR,"Invalid parameters\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,"Invalid node\n");
    }
    return status;
}

vx_status tivxSetNodeDebugZone(vx_node node, vx_uint32 debug_zone, vx_bool enable)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        if ((vx_enum)debug_zone >= (vx_enum)VX_ZONE_MAX)
        {
            VX_PRINT(VX_ZONE_ERROR,
                        "Invalid debug level specified (value greater than VX_ZONE_MAX): %d\n",
                        debug_zone);
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            uint32_t i;

            for (i = 0;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_BRANCH_COVERAGE_TIVX_NODE_UBR034
<justification end> */
            i < TIVX_GRAPH_MAX_PIPELINE_DEPTH;
/* LDRA_JUSTIFY_END */
            i++)
            {
                if (node->obj_desc[i] != NULL)
                {
                    if (enable == (vx_bool)vx_true_e)
                    {
                        node->obj_desc[i]->debug_zonemask |= ZONE_BIT(debug_zone);
                        VX_PRINT_NODE(VX_ZONE_INFO, node, "Enabled %s for Node\n", tivx_find_zone_name((vx_enum)debug_zone));
                    }
                    else
                    {
                        node->obj_desc[i]->debug_zonemask &= ~(ZONE_BIT(debug_zone));
                        VX_PRINT_NODE(VX_ZONE_INFO, node, "Disabled %s for Node\n", tivx_find_zone_name((vx_enum)debug_zone));
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
        status = (vx_status)VX_ERROR_INVALID_REFERENCE;
        VX_PRINT(VX_ZONE_ERROR, "Invalid node reference\n");
    }

    return status;
}

vx_uint32 tivxGetNodeDebugZonemask(vx_node node)
{
    vx_uint32 debug_zonemask = 0U;

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        debug_zonemask = node->obj_desc[0]->debug_zonemask;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid node reference\n");
    }

    return debug_zonemask;
}

const char * tivxGetNodeName(vx_node node)
{
    const char * output = "";

    if (ownIsValidSpecificReference(vxCastRefFromNode(node), (vx_enum)VX_TYPE_NODE) == (vx_bool)vx_true_e)
    {
        output = node->base.name;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid node reference\n");
    }

    return output;
}
