/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <vx_internal.h>

static tivx_target_t g_target_table[TIVX_TARGET_MAX_TARGETS_IN_CPU];

static tivx_target tivxTargetAllocHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[target_inst];

        if(tmp_target->target_id == target_id)
        {
            /* target id already allocated so return null */

            target = NULL;
        }
        else
        {
            /* target ID is not allocated, allocate it */
            tmp_target->target_id = target_id;

            target = tmp_target;
        }
    }

    return target;
}

static void tivxTargetFreeHandle(tivx_target *target_handle)
{
    if(target_handle && *target_handle!=NULL)
    {
        /* mark target handle as free */
        (*target_handle)->target_id = TIVX_TARGET_ID_INVALID;

        *target_handle = NULL;
    }
}

static vx_status tivxTargetDequeueObjDesc(tivx_target target, uint16_t *obj_desc_id, uint32_t timeout)
{
    vx_status status;
    uint32_t value = TIVX_OBJ_DESC_INVALID;

    *obj_desc_id = TIVX_OBJ_DESC_INVALID;

    status = tivxQueueGet(&target->job_queue_handle,
                &value, timeout);

    if(status == VX_SUCCESS)
    {
        *obj_desc_id = (uint16_t)value;
    }

    return status;
}

static tivx_target tivxTargetGetHandle(vx_enum target_id)
{
    uint16_t target_inst = TIVX_GET_TARGET_INST(target_id);
    tivx_target tmp_target = NULL, target = NULL;

    if(target_inst < TIVX_TARGET_MAX_TARGETS_IN_CPU)
    {
        tmp_target = &g_target_table[target_inst];

        if(tmp_target->target_id == target_id)
        {
            /* target id matches so return it */

            target = tmp_target;
        }
        else
        {
            /* target ID does not match so return NULL */
            target = NULL;
        }
    }

    return target;
}

static void tivxTargetNodeDescSendComplete(tivx_obj_desc_node_t *node_obj_desc)
{
    uint16_t cmd_obj_desc_id;

    if( tivxFlagIsBitSet(node_obj_desc->flags, TIVX_NODE_FLAG_IS_USER_CALLBACK)
            ||
        node_obj_desc->num_out_nodes == 0
        )
    {
        cmd_obj_desc_id = node_obj_desc->node_complete_cmd_obj_desc_id;

        if( cmd_obj_desc_id != TIVX_OBJ_DESC_INVALID)
        {
            tivx_obj_desc_cmd_t *cmd_obj_desc = (tivx_obj_desc_cmd_t *)tivxObjDescGet(cmd_obj_desc_id);

            if( tivxObjDescIsValidType( (tivx_obj_desc_t*)cmd_obj_desc, TIVX_OBJ_DESC_CMD) )
            {
                /* users wants a notification of node complete or this is leaf node
                 * so send node complete command to host
                 */
                tivxObjDescSend( cmd_obj_desc->dst_target_id, cmd_obj_desc_id);
            }
        }
    }
}

static vx_bool tivxTargetNodeDescCanNodeExecute(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_node_t *prev_node_obj_desc;
    uint16_t prev_node_obj_desc_id;
    uint16_t i;
    vx_bool can_execute = vx_true_e;

    for(i=0; i<node_obj_desc->num_in_nodes; i++)
    {
        prev_node_obj_desc_id = node_obj_desc->in_node_id[i];
        prev_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(prev_node_obj_desc_id);

        if( tivxObjDescIsValidType( (tivx_obj_desc_t*)prev_node_obj_desc, TIVX_OBJ_DESC_NODE) )
        {
            if( tivxFlagIsBitSet(prev_node_obj_desc->flags,
                        TIVX_NODE_FLAG_IS_EXECUTED) == vx_false_e)
            {
                can_execute = vx_false_e;
            }
        }
    }

    return can_execute;
}

static void tivxTargetNodeDescTriggerNextNodes(tivx_obj_desc_node_t *node_obj_desc)
{
    vx_bool can_execute;
    uint16_t i;
    tivx_obj_desc_node_t *next_node_obj_desc;
    uint16_t next_node_obj_desc_id;

    /* check and trigger next set of nodes */
    for(i=0; i<node_obj_desc->num_out_nodes; i++)
    {
        next_node_obj_desc_id = node_obj_desc->out_node_id[i];
        next_node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(next_node_obj_desc_id);

        if( tivxObjDescIsValidType( (tivx_obj_desc_t*)next_node_obj_desc, TIVX_OBJ_DESC_NODE) )
        {
            can_execute = tivxTargetNodeDescCanNodeExecute(next_node_obj_desc);

            if(can_execute == vx_true_e)
            {
                tivxObjDescSend( next_node_obj_desc->target_id, next_node_obj_desc_id);
            }
        }
    }
}

static void tivxTargetNodeDescNodeExecute(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];
    uint32_t cur_time;
    uint16_t i;
    tivx_target_kernel_instance target_kernel_instance;

    /* if node is already executed do nothing */
    if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_EXECUTED) == vx_false_e )
    {
        target_kernel_instance = tivxTargetKernelInstanceGet(node_obj_desc->target_kernel_index, node_obj_desc->kernel_id);

        cur_time = tivxPlatformGetTimeInUsecs();

        if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == vx_true_e )
        {
            /* TODO: Handle replicated node */

        }
        else
        {
            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
            }

            node_obj_desc->exe_status = tivxTargetKernelExecute(target_kernel_instance, params, node_obj_desc->num_params);
        }

        node_obj_desc->exe_time_usecs = tivxPlatformGetTimeInUsecs() - cur_time;

        tivxFlagBitSet(&node_obj_desc->flags, TIVX_NODE_FLAG_IS_EXECUTED);

        tivxTargetNodeDescSendComplete(node_obj_desc);
        tivxTargetNodeDescTriggerNextNodes(node_obj_desc);
    }
}

static vx_status tivxTargetNodeDescNodeCreate(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

    target_kernel_instance = tivxTargetKernelInstanceAlloc(node_obj_desc->kernel_id, node_obj_desc->target_id);
    if(target_kernel_instance == NULL)
    {
        status = VX_ERROR_NO_RESOURCES;
    }
    else
    {
        /* save index key for fast retrival of handle during run-time */
        node_obj_desc->target_kernel_index = tivxTargetKernelInstanceGetIndex(target_kernel_instance);

        if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == vx_true_e )
        {
            /* TODO: Handle replicated node */

        }
        else
        {
            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
            }

            /* copy border mode also in the target_kernel_instance */
            target_kernel_instance->border_mode = node_obj_desc->border_mode;

            status = tivxTargetKernelCreate(target_kernel_instance, params, node_obj_desc->num_params);
        }

        if(status!=VX_SUCCESS)
        {
            tivxTargetKernelInstanceFree(&target_kernel_instance);
        }
    }

    return status;
}

static vx_status tivxTargetNodeDescNodeDelete(tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i;
    tivx_obj_desc_t *params[TIVX_KERNEL_MAX_PARAMS];

    target_kernel_instance = tivxTargetKernelInstanceGet(node_obj_desc->target_kernel_index, node_obj_desc->kernel_id);

    if(target_kernel_instance == NULL)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        if( tivxFlagIsBitSet(node_obj_desc->flags,TIVX_NODE_FLAG_IS_REPLICATED) == vx_true_e )
        {
            /* TODO: Handle replicated node */

        }
        else
        {
            for(i=0; i<node_obj_desc->num_params ; i++)
            {
                params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
            }

            status = tivxTargetKernelDelete(target_kernel_instance, params, node_obj_desc->num_params);
        }

        tivxTargetKernelInstanceFree(&target_kernel_instance);
    }

    return status;
}

static vx_status tivxTargetNodeDescNodeControl(tivx_obj_desc_cmd_t *cmd_obj_desc, tivx_obj_desc_node_t *node_obj_desc)
{
    tivx_target_kernel_instance target_kernel_instance;
    vx_status status = VX_SUCCESS;
    uint16_t i;
    tivx_obj_desc_t *params[TIVX_CMD_MAX_OBJ_DESCS];

    target_kernel_instance = tivxTargetKernelInstanceGet(node_obj_desc->target_kernel_index, node_obj_desc->kernel_id);

    if(target_kernel_instance == NULL)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        for(i=0; i<cmd_obj_desc->num_obj_desc; i++)
        {
            params[i] = tivxObjDescGet(node_obj_desc->data_id[i]);
        }

        status = tivxTargetKernelControl(target_kernel_instance, params, cmd_obj_desc->num_obj_desc);
    }

    return status;
}

static void tivxTargetCmdDescHandleAck(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK))
    {
        tivxFlagBitClear( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        tivxEventPost((tivx_event)cmd_obj_desc->ack_event_handle);
    }
}

static void tivxTargetCmdDescHandleUserCallback(tivx_obj_desc_node_t *node_obj_desc)
{
    vx_node node = (vx_node)node_obj_desc->host_node_ref;

    /* return action is ignored */
    ownNodeExecuteUserCallback(node);

    /* if this is leaf node, send completion event */
    if(ownNodeGetNumOutNodes(node)==0)
    {
        /* post completeion event */
        ownNodeSendCompletionEvent(node);
    }
}

static void tivxTargetCmdDescSendAck(tivx_obj_desc_cmd_t *cmd_obj_desc, vx_status status)
{
    if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_SEND_ACK))
    {
        tivxFlagBitSet( &cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK);

        cmd_obj_desc->cmd_status = (uint32_t)status;

        tivxObjDescSend(cmd_obj_desc->src_target_id, cmd_obj_desc->base.obj_desc_id);
    }
}

static void tivxTargetCmdDescHandler(tivx_obj_desc_cmd_t *cmd_obj_desc)
{
    uint16_t node_obj_desc_id;
    tivx_obj_desc_node_t *node_obj_desc;
    vx_status status = VX_SUCCESS;

    switch(cmd_obj_desc->cmd_id)
    {
        case TIVX_CMD_NODE_CREATE:
        case TIVX_CMD_NODE_DELETE:
        case TIVX_CMD_NODE_CONTROL:
            if( tivxFlagIsBitSet( cmd_obj_desc->flags, TIVX_CMD_FLAG_IS_ACK) == vx_false_e )
            {
                node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
                node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc_id);

                if( tivxObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) )
                {
                    if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_CREATE)
                    {
                        status = tivxTargetNodeDescNodeCreate(node_obj_desc);
                    }
                    else
                    if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_DELETE)
                    {
                        status = tivxTargetNodeDescNodeDelete(node_obj_desc);
                    }
                    else
                    if(cmd_obj_desc->cmd_id == TIVX_CMD_NODE_CONTROL)
                    {
                        status = tivxTargetNodeDescNodeControl(cmd_obj_desc, node_obj_desc);
                    }
                }
                else
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                }
                tivxTargetCmdDescSendAck(cmd_obj_desc, status);
            }
            else
            {
                /* this is ACK for a previously sent command */
                tivxTargetCmdDescHandleAck(cmd_obj_desc);
            }
            break;

        case TIVX_CMD_NODE_USER_CALLBACK:
            node_obj_desc_id = cmd_obj_desc->obj_desc_id[0];
            node_obj_desc = (tivx_obj_desc_node_t*)tivxObjDescGet(node_obj_desc_id);

            if( tivxObjDescIsValidType( (tivx_obj_desc_t*)node_obj_desc, TIVX_OBJ_DESC_NODE) )
            {
                tivxTargetCmdDescHandleUserCallback(node_obj_desc);
            }
            /* No ack for user callback command */
            break;
        default:

            break;
    }

}

void tivxTargetTaskMain(void *app_var)
{
    tivx_target target = (tivx_target)app_var;
    tivx_obj_desc_t *obj_desc;
    uint16_t obj_desc_id;
    vx_status status = VX_SUCCESS;

    while(target->targetExitRequest == vx_false_e)
    {
        status = tivxTargetDequeueObjDesc(target,
                    &obj_desc_id, TIVX_EVENT_TIMEOUT_WAIT_FOREVER);

        if(    status != VX_SUCCESS
            || obj_desc_id == TIVX_OBJ_DESC_INVALID )
        {
            /* in case of error, do nothing,
             * if target exit was requested, while(...) condition check with
             * check and exit
             */
        }
        else
        {
            obj_desc = tivxObjDescGet(obj_desc_id);
            if(obj_desc == NULL)
            {
                /* in valid obj_desc_id received */
            }
            else
            {
                switch(obj_desc->type)
                {
                    case TIVX_OBJ_DESC_CMD:
                        if( tivxObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_CMD) )
                        {
                            tivxTargetCmdDescHandler((tivx_obj_desc_cmd_t*)obj_desc);
                        }
                        break;
                    case TIVX_OBJ_DESC_NODE:
                        if( tivxObjDescIsValidType( obj_desc, TIVX_OBJ_DESC_NODE) )
                        {
                            tivxTargetNodeDescNodeExecute((tivx_obj_desc_node_t*)obj_desc);
                        }
                        break;
                    default:
                        /* unsupported obj_desc received at target */
                        break;
                }
            }
        }
    }

    target->targetExitDone = vx_true_e;
}

vx_status tivxTargetCreate(vx_enum target_id, tivx_target_create_params_t *params)
{
    vx_status status = VX_SUCCESS;
    tivx_target target;

    target = tivxTargetAllocHandle(target_id);

    if(target == NULL)
    {
        status = VX_ERROR_NO_RESOURCES;
    }
    else
    {
        target->target_id = target_id;

        tivxTaskSetDefaultCreateParams(&target->task_params);
        target->task_params.stack_ptr = params->task_stack_ptr;
        target->task_params.stack_size = params->task_stack_size;
        target->task_params.core_affinity = params->task_core_affinity;
        target->task_params.priority = params->task_priority;
        target->task_params.task_main = tivxTargetTaskMain;
        target->task_params.app_var = target;

        target->targetExitRequest = vx_false_e;
        target->targetExitDone = vx_false_e;


        /* create job queue */
        status = tivxQueueCreate(&target->job_queue_handle,
                        TIVX_TARGET_MAX_JOB_QUEUE_DEPTH,
                        target->job_queue_memory,
                        TIVX_QUEUE_FLAG_BLOCK_ON_GET
                );
        if(status == VX_SUCCESS)
        {
            /* create and start target task */
            status = tivxTaskCreate(&target->task_handle, &target->task_params);
            if(status != VX_SUCCESS)
            {
                /* delete job queue due to task create error */
                tivxQueueDelete(&target->job_queue_handle);
            }
        }
        if(status!=VX_SUCCESS)
        {
            tivxTargetFreeHandle(&target);
        }
    }
    return status;
}

vx_status tivxTargetDelete(vx_enum target_id)
{
    vx_status status = VX_SUCCESS;
    tivx_target target;

    target = tivxTargetGetHandle(target_id);

    /* delete task */

    /* set flag to break target from main loop */
    target->targetExitRequest = vx_true_e;

    /* queue a invalid object descriptor to unblock queue wait */
    tivxTargetQueueObjDesc(target_id, TIVX_OBJ_DESC_INVALID);

    /* wait until target exit is done */
    while(target->targetExitDone==vx_false_e)
    {
        tivxTaskWaitMsecs(1);
    }
    tivxTaskDelete(&target->task_handle);

    /* delete job queue */
    tivxQueueDelete(&target->job_queue_handle);

    return status;
}

vx_status tivxTargetQueueObjDesc(vx_enum target_id, uint16_t obj_desc_id)
{
    vx_status status = VX_ERROR_INVALID_PARAMETERS;
    tivx_target target = tivxTargetGetHandle(target_id);

    if(target!=NULL)
    {
        status = tivxQueuePut(&target->job_queue_handle,
                obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
    }

    return status;
}

void tivxTargetSetDefaultCreateParams(tivx_target_create_params_t *params)
{
    params->task_stack_ptr = NULL;
    params->task_stack_size = 0;
    params->task_core_affinity = 0;
    params->task_priority = TIVX_TASK_PRI_LOWEST;
}

vx_enum tivxTargetGetCpuId(vx_enum target_id)
{
    return TIVX_GET_CPU_ID(target_id);
}

void tivxTargetInit()
{
    uint16_t i;

    for(i=0; i<dimof(g_target_table); i++)
    {
        g_target_table[i].target_id = TIVX_TARGET_ID_INVALID;
    }

    tivxTargetKernelInit();
    tivxTargetKernelInstanceInit();
}

void tivxTargetDeInit()
{
    tivxTargetKernelInstanceDeInit();
    tivxTargetKernelDeInit();

}
