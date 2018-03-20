/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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

static void tivxObjDescQueueReset(tivx_obj_desc_queue_t *obj_desc)
{
    tivx_obj_desc_queue_blocked_nodes_t *blocked_nodes;

    blocked_nodes = &obj_desc->blocked_nodes;

    obj_desc->cur_rd = 0;
    obj_desc->cur_wr = 0;
    obj_desc->count = 0;

    blocked_nodes->num_nodes = 0;
}

vx_status tivxObjDescQueueCreate(uint16_t *obj_desc_id)
{
    tivx_obj_desc_queue_t *obj_desc;
    vx_status status = VX_FAILURE;

    *obj_desc_id = TIVX_OBJ_DESC_INVALID;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescAlloc(TIVX_OBJ_DESC_QUEUE, NULL);

    if(obj_desc != NULL)
    {
        *obj_desc_id = obj_desc->base.obj_desc_id;

        tivxObjDescQueueReset(obj_desc);

        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to alloc object descriptor \n");
    }
    return status;
}

vx_status tivxObjDescQueueRelease(uint16_t *obj_desc_id)
{
    tivx_obj_desc_queue_t *obj_desc = NULL;
    vx_status status = VX_FAILURE;

    if( (obj_desc_id!=NULL) && *obj_desc_id != TIVX_OBJ_DESC_INVALID)
    {
        obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(*obj_desc_id);

        if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
        {
            tivxObjDescFree((tivx_obj_desc_t**)&obj_desc);
            *obj_desc_id = TIVX_OBJ_DESC_INVALID;

            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid object descriptor\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }
    return status;
}

vx_status tivxObjDescQueueEnqueue(uint16_t obj_desc_q_id, uint16_t obj_desc_id)
{
    tivx_obj_desc_queue_t *obj_desc;
    vx_status status = VX_FAILURE;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(obj_desc_q_id);

    if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
    {
        uint16_t cur_wr, count;

        cur_wr = obj_desc->cur_wr;
        count = obj_desc->count;

        if(count < TIVX_OBJ_DESC_QUEUE_MAX_DEPTH)
        {
            obj_desc->queue_mem[cur_wr] = obj_desc_id;
            cur_wr = (cur_wr+1) % TIVX_OBJ_DESC_QUEUE_MAX_DEPTH;
            count++;

            obj_desc->count = count;
            obj_desc->cur_wr = cur_wr;

            status = VX_SUCCESS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }

    return status;
}

vx_status tivxObjDescQueueGetCount(uint16_t obj_desc_q_id, uint32_t *count)
{
    tivx_obj_desc_queue_t *obj_desc;
    vx_status status = VX_FAILURE;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(obj_desc_q_id);

    if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
    {
        *count = obj_desc->count;

        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }
    return status;
}

vx_status tivxObjDescQueueDequeue(uint16_t obj_desc_q_id, uint16_t *obj_desc_id)
{
    tivx_obj_desc_queue_t *obj_desc;
    vx_status status = VX_FAILURE;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(obj_desc_q_id);
    *obj_desc_id = TIVX_OBJ_DESC_INVALID;

    if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
    {
        uint16_t cur_rd, count;

        cur_rd = obj_desc->cur_rd;
        count = obj_desc->count;

        if(count > 0)
        {
            *obj_desc_id = obj_desc->queue_mem[cur_rd];
            cur_rd = (cur_rd+1) % TIVX_OBJ_DESC_QUEUE_MAX_DEPTH;
            count--;

            obj_desc->count = count;
            obj_desc->cur_rd = cur_rd;

            status = VX_SUCCESS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }
    return status;
}

vx_status tivxObjDescQueueAddBlockedNode(uint16_t obj_desc_q_id, uint16_t node_id)
{
    tivx_obj_desc_queue_t *obj_desc;
    tivx_obj_desc_queue_blocked_nodes_t *blocked_nodes;
    vx_status status = VX_FAILURE;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(obj_desc_q_id);

    if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
    {
        uint16_t num_nodes;

        blocked_nodes = &obj_desc->blocked_nodes;

        num_nodes = blocked_nodes->num_nodes;

        if(num_nodes < TIVX_OBJ_DESC_QUEUE_MAX_BLOCKED_NODES)
        {
            blocked_nodes->node_id[num_nodes] = node_id;
            num_nodes++;

            blocked_nodes->num_nodes = num_nodes;

            status = VX_SUCCESS;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to add node to blocked nodes\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }

    return status;
}

vx_status tivxObjDescQueueExtractBlockedNodes(uint16_t obj_desc_q_id,
            tivx_obj_desc_queue_blocked_nodes_t *out_blocked_nodes)
{
    tivx_obj_desc_queue_t *obj_desc;
    tivx_obj_desc_queue_blocked_nodes_t *blocked_nodes;
    vx_status status = VX_FAILURE;

    obj_desc = (tivx_obj_desc_queue_t *)tivxObjDescGet(obj_desc_q_id);

    if(obj_desc!=NULL && obj_desc->base.type == TIVX_OBJ_DESC_QUEUE)
    {
        uint16_t node_id;

        blocked_nodes = &obj_desc->blocked_nodes;

        for(node_id = 0; node_id<blocked_nodes->num_nodes; node_id++)
        {
            out_blocked_nodes->node_id[out_blocked_nodes->num_nodes+node_id] = blocked_nodes->node_id[node_id];
        }

        out_blocked_nodes->num_nodes += blocked_nodes->num_nodes;

        /* since nodes are extracted make num_nodes as zero */
        blocked_nodes->num_nodes = 0;

        status = VX_SUCCESS;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid parameters\n");
    }
    return status;
}


