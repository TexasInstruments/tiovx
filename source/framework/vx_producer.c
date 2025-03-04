/*
 * Copyright (c) 2024 The Khronos Group Inc.
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

static vx_status set_buffer_status(vx_reference current_ref, producer_buffer_status status, vx_producer producer)
{
    uint8_t     buffer_id;
    vx_bool     found                = vx_false_e;
    vx_bool     forbidden_transition = vx_false_e;
    const char* state2string[]       = {"IN_GRAPH", "LOCKED", "FREE"};

    pthread_mutex_lock(&producer->buffer_mutex);

    for (buffer_id = 0; buffer_id < producer->numBuffers; buffer_id++)
    {
        if (current_ref == (producer->refs[buffer_id].ovx_ref))
        {
            producer_buffer_status old_status = producer->refs[buffer_id].buffer_status;
            found                             = vx_true_e;

            // only certain state transitions are allowed and are guarded for LOCKED with refcount
            if ((old_status == IN_GRAPH) && (status == LOCKED))
            {
                // IN_GRAPH -> LOCKED: after leaving producer graph
                producer->refs[buffer_id].refcount++;
                producer->refs[buffer_id].buffer_status = status;
            }
            else if ((old_status == LOCKED) && (status == LOCKED))
            {
                // LOCKED -> LOCKED: after being sent to more consumers
                producer->refs[buffer_id].refcount++;
                // base the locked count on the latest transmission therefore reset from here
                VX_PRINT(VX_ZONE_INFO, "reset locked count for reference with id %d \n", buffer_id); 
                producer->refs[buffer_id].locked_count = 0;
            }
            else if ((old_status == LOCKED) && (status == FREE))
            {
                // LOCKED -> FREE: after coming back from consumer or consumer timeout
                producer->refs[buffer_id].refcount--;
                if (producer->refs[buffer_id].refcount == 0)
                {
                    producer->refs[buffer_id].buffer_status = IN_GRAPH;
                    // enqueue reference into graph from here
                    producer->streaming_cb.enqueueCallback(producer->graph_obj, producer->refs[buffer_id].ovx_ref);
                    producer->nbEnqueueFrames++;

                    VX_PRINT(VX_ZONE_INFO, "enqueued back and reset locked count for reference with id %d \n", buffer_id); 
                    producer->refs[buffer_id].locked_count = 0;
                }
            }
            else if ((old_status == FREE) && (status == IN_GRAPH))
            {
                // FREE -> IN_GRAPH: enqueueing a fresh ref into producer
                producer->refs[buffer_id].buffer_status = status;
            }
            else if ((old_status == IN_GRAPH) && (status == FREE))
            {
                /*
                 * IN_GRAPH -> FREE: dequeue from graph in wait state OR
                 * it could happen that we have a double enqueue due to a miscommunication, where a buffer is freed by 
                 * a consumer that was not supposed to free it (buffer message overwrite while consumer whas processing it)
                 * to prevent that, query the number of enqueues for the reference, only enqueue if not done already                
                 * enqueue reference into graph from here, do not change its status 
                 *
                 * Example Usecase: 
                 * 1. consumer gets new message (e.g. bufID 0, mask 1)
                 * 2. consumer does time consuming copy of supplementary (during that time, producer overwrites with (e.g. bufID 1, mask 0)
                 * 3. consumer enqueues bufID 1 (although it shouldn't but it doesn't re evaluate mask flag)
                 * 4. consumer releases bufID 1 which producer does not expect to be released from that consumer, 
                 * while it DOESNT release bufID 0 although producer expects that.
                 */ 

                vx_uint32 num_enqueues = 0;
                vx_status query_status = vxQueryReference(producer->refs[buffer_id].ovx_ref, VX_REFERENCE_ENQUEUE_COUNT, &num_enqueues, sizeof(num_enqueues));
                if (query_status == VX_SUCCESS)
                {
                    if (num_enqueues > 0)
                    {
                        VX_PRINT(VX_ZONE_WARNING, "reference has been enqueued back to the graph already \n"); 
                    }
                    else
                    {
                        producer->streaming_cb.enqueueCallback(producer->graph_obj, producer->refs[buffer_id].ovx_ref);
                        producer->nbEnqueueFrames++;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to query the number of enqueues for reference with id %d\n", buffer_id);
                }
            }
            else if ((old_status == FREE) && (status == LOCKED))
            {
                // FREE -> LOCKED: possible if the locked buffer is freed before the transmission to second consumer is
                // shutdown, still we need to increase refcount to prevent buffer handling problems
                producer->refs[buffer_id].refcount++;
                producer->refs[buffer_id].buffer_status = status;
            }
            else
            {
                // fatal error; state transition not allowed; should never get here
                // state remains unchanged
                // FREE -> FREE
                // LOCKED -> IN_GRAPH
                // IN_GRAPH -> IN_GRAPH: this is handled implicitly in handle_producer_graph
                VX_PRINT(
                    VX_ZONE_ERROR,
                    "PRODUCER: Reference state transition (%s -> %s) not allowed for buffer %d!%s",
                    state2string[old_status],
                    state2string[status],
                    buffer_id,
                    "\n");
                forbidden_transition = vx_true_e;
            }

            if (producer->refs[buffer_id].buffer_status != old_status)
            {
                uint64_t currentTime = tivxPlatformGetTimeInUsecs();
                VX_PRINT(
                    VX_ZONE_REFERENCE,
                    "PRODUCER: buffer %d found, status changed from %s to %s, refcount is %d \n",
                    buffer_id,
                    state2string[old_status],
                    state2string[status],
                    producer->refs[buffer_id].refcount);
                VX_PRINT(
                    VX_ZONE_REFERENCE,
                    "PRODUCER: reference was in state %s for %llu usecs\n",
                    state2string[old_status],
                    currentTime - producer->refs[buffer_id].state_timestamp);
                producer->refs[buffer_id].state_timestamp = currentTime;
            }
            else
            {
                VX_PRINT(
                    VX_ZONE_REFERENCE,
                    "PRODUCER: buffer %d found, status unchanged (%s), refcount is %d\n",
                    buffer_id,
                    state2string[old_status],
                    producer->refs[buffer_id].refcount);
            }

            break;
        }
    }

    pthread_mutex_unlock(&producer->buffer_mutex);

    if ((found == vx_true_e) && (forbidden_transition == vx_false_e))
    {
        return VX_SUCCESS;
    }
    else
    {
        return VX_FAILURE;
    }
}

static vx_uint8 get_num_buffer_with_status(vx_producer producer, producer_buffer_status status)
{
    uint32_t     buffer_id;
    vx_uint8     num_buffers_found = 0;

    for (buffer_id = 0; buffer_id < producer->numBuffers; buffer_id++)
    {
        if (status == producer->refs[buffer_id].buffer_status)
        {
            VX_PRINT(VX_ZONE_REFERENCE, "PRODUCER found a buffer ref %p with status %d \n", producer->refs[buffer_id].ovx_ref, status);
            num_buffers_found++;
        }
    }
    if (0U == num_buffers_found)
    {
        VX_PRINT(VX_ZONE_INFO, "PRODUCER no buffer found with status %d \n", status);
    }

    return num_buffers_found;
}

static vx_int32 get_buffer_id(vx_reference current_ref, vx_producer producer)
{
    vx_int32 buffer_id;
    for (buffer_id = 0; (vx_uint32)buffer_id < producer->numBuffers; buffer_id++)
    {
        if (producer->refs[buffer_id].ovx_ref == current_ref)
        {
            VX_PRINT(VX_ZONE_REFERENCE, "PRODUCER found a buffer ref %p at index %d \n", current_ref, buffer_id);
            break;
        }
    }

    if ((vx_uint32)buffer_id == producer->numBuffers)
    {
        buffer_id = -1;
        VX_PRINT(
            VX_ZONE_ERROR,
            "PRODUCER Dequeued reference cannot be found in the consumer registered references %s",
            "\n");
    }

    return buffer_id;
}

static uint32_t getNumLockedFramesByClient(vx_producer producer, vx_uint32 client)
{
    uint32_t locked_cnt = 0;
    for (uint32_t i = 0; i < producer->numBuffers; i++)
    {
        if (producer->refs[i].attached_to_client[client] == 1)
            locked_cnt++;
    }
    return locked_cnt;
}

static vx_int32 send_id_message_consumers(
                                            vx_producer producer,
#ifdef IPPC_SHEM_ENABLED
                                            vx_prod_msg_content_t* msg,
#elif SOCKET_ENABLED
                                            vx_gw_buff_id_msg* msg,
#endif
                                            buffer_info_t* ref)
{
    int32_t status = 0;
    vx_int32 sent_to_consumer = 0;
    uint32_t locked_cnt = 0U;
    uint32_t mask = 0U;
#ifdef SOCKET_ENABLED
    uint8_t message_buffer[SOCKET_MAX_MSG_SIZE];

    // append buffer ID message and all metadata to the buffer
    memcpy(&message_buffer[0], msg, sizeof(vx_gw_buff_id_msg));
    memcpy(&message_buffer[0] + sizeof(vx_gw_buff_id_msg), &producer->metadata_buffer, msg->metadata_size);

    pthread_mutex_lock(&producer->client_mutex);

#endif
    for (vx_uint32 i = 0; i < VX_GW_NUM_CLIENTS; i++)
    {          
        if (
#ifdef SOCKET_ENABLED
            (producer->consumers_list[i].socket_fd > 0) &&
#endif
            (producer->consumers_list[i].state == PROD_STATE_CLI_GRAPH_VERIFIED)
        )
        {
            locked_cnt = getNumLockedFramesByClient(producer, i);
            if ((locked_cnt < producer->maxRefsLockedByClient) || (1U == producer->last_buffer)) // in case of last buffer, transmission of that buffer is still necessary
            {
                mask |= (1U << i);
                if (ref != NULL)
                {
                    // the position of this locking is critical, the refcount should be incremented based on the number of
                    // successfull sends
                    status = set_buffer_status(ref->ovx_ref, LOCKED, producer);
                    if (status != VX_SUCCESS)
                    {
                        VX_PRINT(
                            VX_ZONE_ERROR, "PRODUCER %s: Reference %u, could not be set to LOCKED \n", producer->name, i);
                        break;
                    }

                    ref->attached_to_client[i] = 1;
                }

#ifdef SOCKET_ENABLED
                status = socket_write(producer->consumers_list[i].socket_fd, message_buffer, NULL, 0);
                if (status == SOCKET_STATUS_OK)
                {
                    // copy reference sent to the consumers
                    sent_to_consumer++;
                    if (ref != NULL)
                    {
                        VX_PRINT(
                            VX_ZONE_INFO,
                            "PRODUCER %s: buffer ID sent to consumer %d with consumer_id %d\n",
                            producer->name,
                            i,
                            producer->consumers_list[i].consumer_id);
                    }
                }
                else
                {
                    VX_PRINT(
                        VX_ZONE_ERROR, "PRODUCER %s: buffer ID could not be sent to consumer %d \n", producer->name, i);
                    producer->consumers_list[i].state = PROD_STATE_CLI_FAILED;
                    status = VX_FAILURE;
                    // the failure here should result in a socket timeout/early close for the other thread
                    // this is why no special error handling is needed in the graph thread
                }
#endif                
            }
        }
    }

#ifdef IPPC_SHEM_ENABLED

    // in case mask is 0 (all consumers lock maximum amount of buffers allowed), 
    // free reference from here so producer can find it as an available ref
    if ((0U == mask) && (ref != NULL))
    {
        set_buffer_status(ref->ovx_ref, FREE, producer);
    }
    if (msg != NULL)
    {
        msg->mask = mask;
    }   

    status = ippc_shem_send(&producer->m_sender_ctx);
    if (status == E_IPPC_OK)
    {
        sent_to_consumer = VX_GW_NUM_CLIENTS;
        VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: buffer ID sent to consumers with mask %d\n", producer->name, mask);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "PRODUCER %s: buffer ID could not be sent to consumers with mask %d \n", producer->name, mask);
    }

#elif SOCKET_ENABLED
    pthread_mutex_unlock(&producer->client_mutex);
#endif

    return sent_to_consumer;
}

#ifdef IPPC_SHEM_ENABLED

void* producer_bck_thread(void* arg)
{
    producer_bckchannel_t* l_consumer = (producer_bckchannel_t*) arg;

    char threadname[280U];
    snprintf(threadname, 280U, "producer_bck_thread_%u", l_consumer->consumer_id);
    pthread_setname_np(pthread_self(), threadname);

    VX_PRINT(VX_ZONE_INFO, "PRODUCER : starting backchannel worker for consumer %u on port %u\n", l_consumer->consumer_id, 
                                                                    l_consumer->m_receiver_ctx.m_port_map.m_port_id);
    while(1)
    {
        // wait for message on backchannel
        ippc_receive(&l_consumer->m_receiver_ctx);
    }

    return NULL;
}

static void fill_reference_info(vx_producer producer, vx_prod_msg_content_t* buffid_message)
{
    buffid_message->num_refs = producer->numBufferRefsExport;
    uint32_t i = 0;
    uint32_t j = 0;
    for (i = 0; i < producer->numBufferRefsExport; i++)
    {
        vx_enum                  ref_type;
        vx_uint32                num_items          = 0;
        tivx_utils_ref_ipc_msg_t ipc_message_parent = {0};
        tivx_utils_ref_ipc_msg_t ipc_message_item[VX_GW_MAX_NUM_REFS];

        vx_status framework_status =
            vxQueryReference(producer->refs[i].ovx_ref, VX_REFERENCE_TYPE, (void*)&ref_type, (vx_size)sizeof(ref_type));
        if (framework_status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER: vxQueryReference() failed for object [%d]\n", i);
            break;
        }
        else if (ref_type == VX_TYPE_OBJECT_ARRAY)
        {
            framework_status = rbvx_utils_export_ref_for_ipc_xfer_objarray(
                producer->refs[i].ovx_ref,
                &num_items,
                &ipc_message_parent,
                (tivx_utils_ref_ipc_msg_t*)&ipc_message_item[0]);

            buffid_message->num_items = num_items;

            // send object array items data, if present
            for (j = 0; j < num_items; j++)
            {
                buffid_message->ref_export_handle[i][j] = ipc_message_item[j];

                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s: sending objarray element %d with fd count %d\n",
                    producer->name,
                    j,
                    buffid_message->ref_export_handle[i][j].numFd);
            }
        }
        else
        {
            framework_status = tivx_utils_export_ref_for_ipc_xfer(producer->refs[i].ovx_ref, &ipc_message_parent);
        }

        if (framework_status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER: export_ref_for_ipc_xfer() failed for buffer [%d]\n", i);
            break;
        }
        else
        {
            VX_PRINT(
                VX_ZONE_INFO,
                "PRODUCER %s: export of buffer successfull: %d of total: %d\n",
                producer->name,
                i + 1,
                producer->numBufferRefsExport);
        }

        // send reference data, for object array this is final metadata
        buffid_message->ref_export_handle[i][j] = ipc_message_parent;
    }

}

void producer_msg_handler(const void * producer_p, const void * data_p, uint8_t last_buffer_from_series)
{
    const vx_cons_msg_content_t* const received_msg = (const vx_cons_msg_content_t*)data_p;
    vx_producer producer = (vx_producer)producer_p;
    switch (received_msg->msg_type)
    {
        case VX_MSGTYPE_HELLO:
        case VX_MSGTYPE_REF_BUF:
        case VX_MSGTYPE_BUFID_CMD:
        case VX_MSGTYPE_COUNT:
        {
            // do nothing
        }
        break; 

        case VX_MSGTYPE_BUF_RELEASE:
        {
            if (received_msg->last_buffer == 1)
            {
                VX_PRINT(VX_ZONE_INFO, "received last_buffer release from consumer %d \n", received_msg->consumer_id); 
                // release all buffers in possession of this consumer
                for (vx_uint32 buffId = 0; buffId < producer->numBuffers; buffId++)
                {
                    if (1U == producer->refs[buffId].attached_to_client[received_msg->consumer_id])
                    {
                        producer->refs[buffId].attached_to_client[received_msg->consumer_id] = 0U;
                        set_buffer_status(producer->refs[buffId].ovx_ref, FREE, producer);
                    }
                } 
                producer->consumers_list[received_msg->consumer_id].state = PROD_STATE_CLI_FLUSHED;
            }
            else
            {
                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s:Received release id: %d from consumer %d \n",
                    producer->name,
                    received_msg->buffer_id,
                    received_msg->consumer_id);
                vx_reference next_out_ref = producer->refs[received_msg->buffer_id].ovx_ref;
                if (next_out_ref != NULL)
                {
                    // enqueue the new buffer in the handle producer thread, here the refcount is decreased
                    producer->refs[received_msg->buffer_id].attached_to_client[received_msg->consumer_id] = 0;
                    set_buffer_status(next_out_ref, FREE, producer);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "PRODUCER %s: buffer ID not valid.\n", producer->name);
                }
            }
        }
        break;

        case VX_MSGTYPE_CONSUMER_CREATE_DONE: // consumer notifys about graph creation being completed
        {
            VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: received VX_GW_STATUS_CONSUMER_CREATE_DONE state from consumer %d \n", producer->name, received_msg->consumer_id);
            producer->consumers_list[received_msg->consumer_id].state = PROD_STATE_CLI_GRAPH_VERIFIED; 
        }
        break;

        default:
            VX_PRINT(
                VX_ZONE_ERROR,
                "PRODUCER %s: Received [UNKNOWN MESSAGE] %d\n",
                producer->name,
                received_msg->msg_type);
        break;
    }
}

static vx_bool check_ippc_clients_connected(vx_producer producer)
{
    // if one of the receiver is ready, register it and the sender can start sending data
    vx_bool new_client_connected = vx_false_e;
    for (vx_uint32 i = 0U; i < VX_GW_NUM_CLIENTS; i++)
    {
        if (E_IPPC_OK == ippc_sender_receiver_ready(&producer->m_sender_ctx.m_sender, i) && 
            (producer->consumers_list[i].state == PROD_STATE_CLI_NOT_CONNECTED))
        {
            producer->consumers_list[i].state       = PROD_STATE_CLI_CONNECTED;
            producer->consumers_list[i].consumer_id = i;
            producer->nb_consumers++;
            new_client_connected = vx_true_e;
        }
    
        if (producer->consumers_list[i].state == PROD_STATE_CLI_CONNECTED)
        {
            EIppcStatus l_status;

            VX_PRINT(
                VX_ZONE_INFO,
                "PRODUCER %s: send buffer metadata for consumer %u \n", producer->name, i);

            // set up backchannel context
            producer->consumers_list[i].m_receiver_ctx.m_port_map = producer->ippc_port[i + 1U];
            producer->consumers_list[i].m_receiver_ctx.m_msg_size = sizeof(vx_cons_msg_content_t);
            producer->consumers_list[i].m_receiver_ctx.m_client_handler = producer_msg_handler;
            producer->consumers_list[i].m_receiver_ctx.m_application_ctx = producer;
            
            //create the backchannel connnector
            l_status  = ippc_registry_receiver_attach(&producer->m_shmem_ctx.m_registry,
                                                    &producer->consumers_list[i].m_receiver_ctx.m_receiver,
                                                    producer->consumers_list[i].m_receiver_ctx.m_port_map.m_port_id,
                                                    0,// always use receiver 0 for unicast ports;
                                                    producer->consumers_list[i].m_receiver_ctx.m_msg_size,
                                                    E_IPPC_RECEIVER_DISCARD_PAST);

            if(E_IPPC_OK == l_status)
            {
                l_status = ippc_registry_sync_attach(&producer->m_shmem_ctx.m_registry, &producer->consumers_list[i].m_receiver_ctx.m_sync, 
                    producer->consumers_list[i].m_receiver_ctx.m_port_map.m_receiver_index + 
                    VX_GW_NUM_CLIENTS);
            }

            if (E_IPPC_OK == l_status)
            {
                
                // launch backchannel thread, where we attach to the receiver of backchannel port
                int thread_status = pthread_create(&producer->consumers_list[i].bck_thread, NULL, producer_bck_thread, (void*)&producer->consumers_list[i]);
                if (thread_status == 0)
                {
                    producer->consumers_list[i].state = PROD_STATE_CLI_RUNNING;
                    VX_PRINT(
                        VX_ZONE_INFO,
                        "PRODUCER %s: consumer %u backchannel is ready, going to RUNNING state %u \n", producer->name, i);
                }
                EIppcStatus l_status;
                pthread_mutex_lock(&producer->client_mutex);
                vx_prod_msg_content_t* buffid_message = ippc_shem_payload_pointer(&producer->m_sender_ctx, sizeof(vx_prod_msg_content_t), &l_status);
                fill_reference_info(producer, buffid_message); 
                buffid_message->buffer_id        = -1;
                buffid_message->metadata_valid   = 0;
                buffid_message->last_buffer      = producer->last_buffer;
                buffid_message->metadata_size    = VX_GW_MAX_META_SIZE;
                send_id_message_consumers(producer, buffid_message, NULL);
                pthread_mutex_unlock(&producer->client_mutex);
            }
        }
    }
    return new_client_connected;
}

void* producer_connection_check_thread(void* arg)
{
    vx_producer producer = (vx_producer)arg;
    char threadname[280U];
    snprintf(threadname, 280U, "producer_conn_check_thread_%s", producer->name);
    pthread_setname_np(pthread_self(), threadname);

    VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: starting connection check thread \n", producer->name);
    while(vx_false_e == producer->connection_check_polling_exit) // assume that after first dequeue, frequent polling for clients is no longer necessary
    {
        pthread_mutex_lock(&producer->client_mutex);
        (void)check_ippc_clients_connected(producer);
        pthread_mutex_unlock(&producer->client_mutex);
        tivxTaskWaitMsecs(producer->connection_check_polling_time);
    }
    VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: exiting connection check thread \n", producer->name);

    return NULL;
}

#elif SOCKET_ENABLED

static int32_t send_reference_info(vx_producer producer, client_context* client)
{
    int32_t status = VX_GW_STATUS_FAILURE;

    if (sizeof(vx_gw_buff_desc_msg) >= SOCKET_MAX_MSG_SIZE)
    {
        VX_PRINT(
            VX_ZONE_ERROR, "PRODUCER: Cannot transmit TIVX object data, insufficient socket message size %s", "\n");
        return status;
    }

    for (uint32_t i = 0; i < producer->numBufferRefsExport; i++)
    {
        vx_enum                  ref_type;
        vx_uint32                num_items          = 0;
        tivx_utils_ref_ipc_msg_t ipc_message_parent = {0};
        tivx_utils_ref_ipc_msg_t ipc_message_item[VX_GW_MAX_NUM_REFS];

        uint8_t               message_buffer[SOCKET_MAX_MSG_SIZE];
        vx_gw_buff_desc_msg* buffer_desc_msg = (vx_gw_buff_desc_msg*)&message_buffer;
        buffer_desc_msg->msg_type             = VX_MSGTYPE_REF_BUF;
        buffer_desc_msg->last_reference       = 0;
        buffer_desc_msg->num_items            = 0;

        vx_status framework_status =
            vxQueryReference(producer->refs[i].ovx_ref, VX_REFERENCE_TYPE, (void*)&ref_type, (vx_size)sizeof(ref_type));
        if (framework_status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER: vxQueryReference() failed for object [%d]\n", i);
            break;
        }
        else if (ref_type == VX_TYPE_OBJECT_ARRAY)
        {
            framework_status = rbvx_utils_export_ref_for_ipc_xfer_objarray(
                producer->refs[i].ovx_ref,
                &num_items,
                &ipc_message_parent,
                (tivx_utils_ref_ipc_msg_t*)&ipc_message_item[0]);

            buffer_desc_msg->num_items = num_items;

            // send object array items data, if present
            for (uint32_t j = 0; j < num_items; j++)
            {
                buffer_desc_msg->item_index = j;
                memcpy(&buffer_desc_msg->ref_export_handle, (void*)&ipc_message_item[j], sizeof(tivx_utils_ref_ipc_msg_t));

                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s: [VX_MSGTYPE_REF_BUF] sending objarray element %d with fd count %d\n",
                    producer->name,
                    j,
                    buffer_desc_msg->ref_export_handle.numFd);

                status = socket_write(
                    client->socket_fd,
                    (uint8_t*)buffer_desc_msg,
                    (int32_t*)buffer_desc_msg->ref_export_handle.fd,
                    buffer_desc_msg->ref_export_handle.numFd);
                if (status != SOCKET_STATUS_OK)
                {
                    VX_PRINT(
                        VX_ZONE_ERROR,
                        "PRODUCER %s: send_reference_info() failed while sending socket message\n",
                        producer->name);
                    return status;
                }
            }
        }
        else
        {
            framework_status = tivx_utils_export_ref_for_ipc_xfer(producer->refs[i].ovx_ref, &ipc_message_parent);
        }

        if (framework_status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER: export_ref_for_ipc_xfer() failed for buffer [%d]\n", i);
            break;
        }
        else
        {
            VX_PRINT(
                VX_ZONE_INFO,
                "PRODUCER %s: export of buffer successfull: %d of total: %d\n",
                producer->name,
                i + 1,
                producer->numBufferRefsExport);
        }

        VX_PRINT(
            VX_ZONE_INFO,
            "PRODUCER %s: Sending [VX_MSGTYPE_REF_BUF] for buffer %d of type %d\n",
            producer->name,
            i,
            ref_type);

        if (i == (producer->numBufferRefsExport - 1))
        {
            VX_PRINT(
                VX_ZONE_INFO, "PRODUCER %s: number of objects to exchange reached, set last object\n", producer->name);
            buffer_desc_msg->last_reference = 1;
        }

        // send reference data, for object array this is final metadata
        buffer_desc_msg->item_index = 0; // used only for object array items
        memcpy(&buffer_desc_msg->ref_export_handle, &ipc_message_parent, sizeof(tivx_utils_ref_ipc_msg_t));

        status = socket_write(
            client->socket_fd,
            (uint8_t*)buffer_desc_msg,
            (int32_t*)buffer_desc_msg->ref_export_handle.fd,
            buffer_desc_msg->ref_export_handle.numFd);

        if (status != SOCKET_STATUS_OK)
        {
            VX_PRINT(
                VX_ZONE_ERROR,
                "PRODUCER %s: send_reference_info() failed while sending socket message\n",
                producer->name);
            break;
        }
    }

    return status;
}

static int32_t add_client(vx_producer producer, client_context* connection, uint64_t consumer_id)
{
    int32_t      client_num = -1;
    producer_bckchannel_t* client     = NULL;

    pthread_mutex_lock(&producer->client_mutex);

    for (uint32_t i = 0; i < VX_GW_NUM_CLIENTS; i++)
    {
        if (producer->consumers_list[i].state == PROD_STATE_CLI_NOT_CONNECTED)
        {
            client = &producer->consumers_list[i];

            client->state       = PROD_STATE_CLI_CONNECTED;
            client->consumer_id = consumer_id;
            client->socket_fd   = connection->socket_fd;
            client_num          = i;

            producer->nb_consumers++;
            break;
        }
    }

    if ((NULL == client) || (client_num == -1))
    {
        VX_PRINT(
            VX_ZONE_ERROR, "PRODUCER %s: Maximum number of clients reached or error in client state\n", producer->name);
    }

    pthread_mutex_unlock(&producer->client_mutex);
    return client_num;
}

static void drop_client(vx_producer producer, producer_bckchannel_t* client, int32_t client_num)
{
    pthread_mutex_lock(&producer->client_mutex);

    VX_PRINT(VX_ZONE_INFO, "PRODUCER: Cleaning up client with socket %d and PID %d\n", client->socket_fd, client->consumer_id);

    if (client->state != PROD_STATE_CLI_NOT_CONNECTED)
    {
        // zero out client info
        client->state                 = PROD_STATE_CLI_NOT_CONNECTED;
        client->first_buffer_released = 0;
        client->consumer_id                   = 0;
        client->socket_fd             = 0;
        producer->nb_consumers--;

        for (uint32_t i = 0; i < producer->numBuffers; i++)
        {
            // the ref we want to unlock will always be locked here, since the consumer is disconnected
            // (at least one refcount is > 0 for locked)
            if ((producer->refs[i].buffer_status == LOCKED) && (producer->refs[i].attached_to_client[client_num] == 1))
            {
                VX_PRINT(
                    VX_ZONE_WARNING,
                    "PRODUCER %s: Reference %u, was in LOCKED state, trying to FREE \n",
                    producer->name,
                    i,
                    producer->refs[i].buffer_status);
                producer->refs[i].attached_to_client[client_num] = 0;
                int32_t status = set_buffer_status(producer->refs[i].ovx_ref, FREE, producer);
                if (status != VX_GW_STATUS_SUCCESS)
                {
                    VX_PRINT(
                        VX_ZONE_ERROR, "PRODUCER %s: Reference %u, could not be set to FREE \n", producer->name, i);
                }
            }
        }
    }

    pthread_mutex_unlock(&producer->client_mutex);

}

static void handle_clients(void* clientPtr, void* data)
{
    // this function must be MT-safe
    client_context* client   = (client_context*)clientPtr;
    vx_producer producer = (vx_producer)data;

    uint8_t           message_buffer[SOCKET_MAX_MSG_SIZE];
    vx_gw_hello_msg* consumer_message;

    int32_t client_num = -1;
    int32_t status     = VX_GW_STATUS_SUCCESS;

    while (1)
    {
        if (producer->last_buffer == 1U)
        {
            VX_PRINT(
                VX_ZONE_INFO, "PRODUCER %s: reconfiguring the socket timeouts for release %s", producer->name, "\n");
            socket_reconfigure_timeout(client->socket_fd, SOCKET_TIMEOUT_USECS_RELEASE);
        }
        // block until data is ready
        status = socket_read(client->socket_fd, message_buffer, NULL, NULL);
        if ((status < SOCKET_STATUS_OK) || (status == SOCKET_STATUS_PEER_CLOSED))
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER %s: socket_read() timed out or error\n", producer->name);
            status = VX_GW_STATUS_FAILURE;
            break;
        }

        // handle message
        consumer_message = (vx_gw_hello_msg*)message_buffer;

        switch (consumer_message->msg_type)
        {
        case VX_MSGTYPE_HELLO:
            client_num = add_client(producer, client, consumer_message->consumer_id);
            if (client_num >= 0)
            {
                VX_PRINT(
                    VX_ZONE_INFO, "PRODUCER %s:Received [VX_MSGTYPE_HELLO] from client %d\n", producer->name, client_num);
                VX_PRINT(
                    VX_ZONE_PERF,
                    " [UPT] First Time Connected to Producer %s with ID %u \n ",
                    producer->name,
                    consumer_message->consumer_id);

                status = send_reference_info(producer, client);
                if (VX_GW_STATUS_SUCCESS == status)
                {
                    VX_PRINT(VX_ZONE_INFO, "PRODUCER: all buffers sent to client %d\n", client_num);
                    producer->consumers_list[client_num].state = PROD_STATE_CLI_RUNNING;
                }
                else
                {
                }
            }
            else
            {
                status = VX_GW_STATUS_FAILURE;
            }
            break;

        case VX_MSGTYPE_BUF_RELEASE:
        {
            if (producer->consumers_list[client_num].first_buffer_released == 0)
            {
                // due to long delays in setting up the producer/consumer communication, there are different timeout
                // values for init/streaming phases
                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s: reconfiguring the socket timeouts for streaming values %s",
                    producer->name,
                    "\n");

                socket_reconfigure_timeout(client->socket_fd, SOCKET_TIMEOUT_USECS_STREAMING);
                producer->consumers_list[client_num].first_buffer_released = 1;
            }

            vx_gw_buff_id_msg* bufferid_message = (vx_gw_buff_id_msg*)message_buffer;
            if (bufferid_message->last_buffer == 1)
            {
                // this client graph is flushed, shut the current client thread down
                producer->consumers_list[client_num].state = PROD_STATE_CLI_FLUSHED;
                producer->refs[bufferid_message->buffer_id].attached_to_client[client_num] = 0;
                status = VX_GW_STATUS_CONSUMER_FLUSHED;
            }
            else
            {
                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s:Received [VX_MSGTYPE_BUF_RELEASE] release id: %d from client %d \n",
                    producer->name,
                    bufferid_message->buffer_id,
                    client_num);
                vx_reference next_out_ref = producer->refs[bufferid_message->buffer_id].ovx_ref;
                if (next_out_ref != NULL)
                {
                    // enqueue the new buffer in the handle producer thread, here the refcount is decreased
                    producer->refs[bufferid_message->buffer_id].attached_to_client[client_num] = 0;
                    status = set_buffer_status(next_out_ref, FREE, producer);
                    if (status != VX_GW_STATUS_SUCCESS)
                    {
                        VX_PRINT(
                            VX_ZONE_ERROR,
                            "PRODUCER %s: cannot release buffer ID %d\n",
                            producer->name,
                            bufferid_message->buffer_id);
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "PRODUCER %s: buffer ID not valid.\n", producer->name);
                    status = VX_GW_STATUS_FAILURE;
                }
            }
        }
        break;

        case VX_MSGTYPE_CONSUMER_CREATE_DONE: // consumer notifys about graph creation being completed
        {
            VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: received VX_MSGTYPE_CONSUMER_CREATE_DONE state from consumer %d \n", producer->name, consumer_message->consumer_id);
            producer->consumers_list[consumer_message->consumer_id].state       = PROD_STATE_CLI_GRAPH_VERIFIED; 
            status = VX_GW_STATUS_SUCCESS;
        }
        break;

        default:
            VX_PRINT(
                VX_ZONE_ERROR,
                "PRODUCER %s: Received [UNKNOWN MESSAGE] %d\n",
                producer->name,
                consumer_message->msg_type);
            status = VX_GW_STATUS_FAILURE;
            break;
        }

        if (status != VX_GW_STATUS_SUCCESS)
        {
            break;
        }
    }

    // clean up client
    if (client_num >= 0)
    {
        drop_client(producer, &producer->consumers_list[client_num], client_num);
    }

    VX_PRINT(VX_ZONE_INFO, "PRODUCER: client %d thread shutting down %s", client_num, "\n");
    return;
}

#endif

/* 
 * every cycle (everytime a new buffer is dequeued) loop through 
 * all locked buffers and increase locked count for each buffer. if a 
 * buffer's locked count reaches a threshold, assume that consumer  
 * has a problem and release buffer back to the producer
 */
static void update_locked_state(vx_producer producer)
{
    uint8_t     buffer_id;

    pthread_mutex_lock(&producer->buffer_mutex);
    for (buffer_id = 0; buffer_id < producer->numBuffers; buffer_id++ )
    {
        if (LOCKED == producer->refs[buffer_id].buffer_status)
        {
            producer->refs[buffer_id].locked_count++;
        }

        if(VX_GW_MAX_LOCKED_CNT == producer->refs[buffer_id].locked_count)
        {
            for (uint32_t client_id = 0U; client_id < VX_GW_NUM_CLIENTS; client_id++)
            {
                VX_PRINT(VX_ZONE_WARNING, "detach a reference with id %d from client %d with current attach state %d \n", 
                                                buffer_id, client_id, producer->refs[buffer_id].attached_to_client[client_id]); 
                producer->refs[buffer_id].attached_to_client[client_id] = 0;
            }
            VX_PRINT(VX_ZONE_WARNING, "release a reference with id %d because it has been locked for too long \n", buffer_id);
            producer->refs[buffer_id].locked_count = 0;
            producer->refs[buffer_id].refcount = 0;            
            producer->refs[buffer_id].buffer_status = IN_GRAPH;
            producer->streaming_cb.enqueueCallback(producer->graph_obj, producer->refs[buffer_id].ovx_ref);
            producer->nbEnqueueFrames++;
        }
    }

    pthread_mutex_unlock(&producer->buffer_mutex);
}

static void* producer_broadcast_thread(void* arg)
{
    vx_producer producer = (vx_producer)arg;
    vx_reference dequeued_refs[VX_GW_MAX_NUM_REFS] = {0};
    vx_bool shutdown = (vx_bool)vx_false_e;
    vx_status status = (vx_status)VX_SUCCESS;

#ifdef IPPC_SHEM_ENABLED
    char threadname[280U];
    snprintf(threadname, 280U, "%s_gw_broadcast_thread", producer->name);
    pthread_setname_np(pthread_self(), threadname);
#endif

    while((vx_bool)vx_true_e != shutdown)
    {
        switch(producer->graph_state)
        {
            case VX_PROD_STATE_INIT:
            {
                // Enqueue all output buffer IDs so that graph can start processing
                for (vx_uint32 idx = 0; idx < producer->numBuffers; idx++)
                {
                    producer->streaming_cb.enqueueCallback(producer->graph_obj, producer->refs[idx].ovx_ref);
                    producer->nbEnqueueFrames++;
                    set_buffer_status(producer->refs[idx].ovx_ref, IN_GRAPH, producer);
                }
                producer->graph_state = VX_PROD_STATE_RUN;
                VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: starting graph from inside producer!\n", producer->name);
            }
            break;

            case VX_PROD_STATE_RUN:
            {
                vx_uint32 num_ready = 0;

                // go to cleanup if there is no consumer and last buffer flag is set
                if ((producer->nb_consumers == 0) && (producer->last_buffer == 1))
                {
                    producer->graph_state = VX_PROD_STATE_WAIT;
                    VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s: Consumer disconnected and last buffer signaled, shutting down! %s",
                    producer->name,
                    "\n");
                    break;
                }

                // Dequeue from the Graph
                status = producer->streaming_cb.dequeueCallback(producer->graph_obj, dequeued_refs, &num_ready);
                // update locked count for already locked refs
                update_locked_state(producer);
#ifdef IPPC_SHEM_ENABLED
                producer->connection_check_polling_exit = vx_true_e; 
#endif
                if (status != (vx_status)VX_SUCCESS)
                {
                    break;
                }

#ifdef IPPC_SHEM_ENABLED
                pthread_mutex_lock(&producer->client_mutex);
                (void)check_ippc_clients_connected(producer);
                pthread_mutex_unlock(&producer->client_mutex);
#endif                
                for (vx_uint32 current_ref_num = 0; current_ref_num < num_ready; current_ref_num++)
                {
                    // Process one reference at a time
                    vx_reference ref_from_graph = dequeued_refs[current_ref_num];
                    producer->nbDequeueFrames++;

                    if (producer->nb_consumers == 0)
                    {
                        //  No client connected-  eneuque the buffer directly
                        VX_PRINT(
                            VX_ZONE_INFO,
                            "PRODUCER %s: consumer is not ready, enqueue the buffer again \n",
                            producer->name);
                        producer->streaming_cb.enqueueCallback(producer->graph_obj, ref_from_graph);
                        producer->nbEnqueueFrames++;
                        if (producer->last_buffer)
                        {
                            // not connected and last buffer - exit
                            VX_PRINT(
                                VX_ZONE_INFO,
                                "PRODUCER %s: last buffer received and we are async, exiting.....\n",
                                producer->name);
                            producer->graph_state = VX_PROD_STATE_WAIT;
                            break;
                        }
                    }
                    else
                    {
                        VX_PRINT(
                            VX_ZONE_INFO,
                            "PRODUCER %s: dequeue output ref buffer %p\n",
                            producer->name,
                            (vx_reference)ref_from_graph);

                        //  Get producer internal buffer id, and send it to the consumer
                        vx_int32 buffer_id = get_buffer_id(ref_from_graph, producer);
                        if (buffer_id < 0)
                        {
                            VX_PRINT(
                                VX_ZONE_ERROR,
                                "PRODUCER %s: get_buffer_id buffer not found; FATAL ERROR \n",
                                producer->name);
                            shutdown = 1;
                            break;
                        }
                        else
                        {
#ifdef IPPC_SHEM_ENABLED
                            EIppcStatus l_status;
                            pthread_mutex_lock(&producer->client_mutex);
                            vx_prod_msg_content_t* buffid_message = ippc_shem_payload_pointer(&producer->m_sender_ctx, sizeof(vx_prod_msg_content_t), &l_status);
                            buffid_message->buffer_id        = buffer_id;
                            buffid_message->metadata_valid   = 0;
                            buffid_message->last_buffer      = producer->last_buffer;
                            buffid_message->metadata_size    = VX_GW_MAX_META_SIZE;
                            size_t metadata_size             = VX_GW_MAX_META_SIZE;

                            for (uint32_t i = 0U; i < VX_GW_NUM_CLIENTS; i++)
                            {
                                if (producer->consumers_list[i].state == PROD_STATE_CLI_NOT_CONNECTED)
                                {
                                    for (uint32_t j = 0; j < producer->numBuffers; j++)
                                    {
                                        // the ref we want to unlock will always be locked here, since the consumer is disconnected
                                        // (at least one refcount is > 0 for locked)
                                        if ((producer->refs[j].buffer_status == LOCKED) && (producer->refs[j].attached_to_client[i] == 1))
                                        {
                                            VX_PRINT(
                                                VX_ZONE_WARNING,
                                                "PRODUCER %s: Reference %u, was in LOCKED state, trying to FREE \n",
                                                producer->name,
                                                producer->refs[j].ovx_ref);
                                            producer->refs[j].attached_to_client[i] = 0;
                                            set_buffer_status(producer->refs[j].ovx_ref, FREE, producer);
                                        }
                                    }
                                }
                            }
#elif SOCKET_ENABLED
                            vx_gw_buff_id_msg buffid_message = {0};
                            buffid_message.msg_type           = VX_MSGTYPE_BUFID_CMD;
                            buffid_message.buffer_id          = buffer_id;
                            buffid_message.metadata_valid     = 0;
                            buffid_message.last_buffer        = producer->last_buffer;
                            buffid_message.metadata_size      = 0;

                            size_t metadata_size              = SOCKET_MAX_MSG_SIZE - sizeof(buffid_message);
#endif
                            if (producer->last_buffer)
                            {
                                // Last buffer (final frame) info was shared from the application
                                // The consumer needs this info to properly release the output buffer references.
                                VX_PRINT(
                                    VX_ZONE_INFO,
                                    "PRODUCER %s: send last frame signal to the consumer (%d) \n",
                                    producer->name,
                                    producer->nbDequeueFrames);
#ifdef IPPC_SHEM_ENABLED
                                vx_int32 num_messages = send_id_message_consumers(producer, buffid_message, NULL);
                                pthread_mutex_unlock(&producer->client_mutex);
#elif SOCKET_ENABLED
                                vx_int32 num_messages = send_id_message_consumers(producer, &buffid_message, NULL);
#endif
                                VX_PRINT(
                                    VX_ZONE_INFO,
                                    "PRODUCER %s: sent last buffer to %d consumers \n",
                                    producer->name,
                                    num_messages);
                                // put the producer in waiting, then flushing mode
                                producer->graph_state = VX_PROD_STATE_WAIT;
                            }
                            else
                            {
                                // if at least one buffer (excluding the recently dequeued one) are
                                // occupied by graph, we can safely distribute the buffer to consumers                               
                                if (1U < get_num_buffer_with_status(producer, IN_GRAPH))
                                {
                                    // fetch metadata from producer reference and store
                                    if (NULL != producer->streaming_cb.getMetadataCallback)
                                    {
                                        status        = producer->streaming_cb.getMetadataCallback(
                                            producer->graph_obj,
                                            ref_from_graph,
#ifdef IPPC_SHEM_ENABLED
                                            (void*)buffid_message->metadata_buffer,
                                            &metadata_size);
#elif SOCKET_ENABLED
                                            (void*)producer->metadata_buffer,
                                            &metadata_size);
#endif
                                        if (((vx_status)VX_SUCCESS != status) ||
#ifdef IPPC_SHEM_ENABLED
                                            (metadata_size > VX_GW_MAX_META_SIZE)
#elif SOCKET_ENABLED
                                            (metadata_size + sizeof(vx_gw_buff_id_msg) > SOCKET_MAX_MSG_SIZE)
#endif
                                        )
                                        {
                                            VX_PRINT(
                                                VX_ZONE_INFO,
                                                "PRODUCER %s: cannot get metadata OR metadata too large. \n",
                                                producer->name);
                                            // metadata payload suppressed
#ifdef IPPC_SHEM_ENABLED
                                            buffid_message->metadata_size = 0;
#elif SOCKET_ENABLED
                                            buffid_message.metadata_size = 0;
#endif
                                        }
                                        else
                                        {
                                            // curren value for metadata_size was set within getMetadataCallback
#ifdef IPPC_SHEM_ENABLED
                                            buffid_message->metadata_valid = 1;
                                            buffid_message->metadata_size  = metadata_size;
#elif SOCKET_ENABLED
                                            buffid_message.metadata_valid = 1;
                                            buffid_message.metadata_size  = metadata_size;
#endif
                                        }
                                    }
#ifdef IPPC_SHEM_ENABLED
                                    buffid_message->last_frame_dropped = producer->last_frame_dropped;
#elif SOCKET_ENABLED
                                    buffid_message.last_frame_dropped = producer->last_frame_dropped;
#endif
                                    producer->last_frame_dropped      = 0;

                                    // broadcast buffer to clients/consumers
#ifdef IPPC_SHEM_ENABLED
                                    vx_int32 sent_messages =
                                        send_id_message_consumers(producer, buffid_message, &producer->refs[buffer_id]);
                                    pthread_mutex_unlock(&producer->client_mutex);
#elif SOCKET_ENABLED
                                    vx_int32 sent_messages =
                                        send_id_message_consumers(producer, &buffid_message, &producer->refs[buffer_id]);
#endif
                                    if (sent_messages == 0)
                                    {
                                        producer->streaming_cb.enqueueCallback(producer->graph_obj, ref_from_graph);
                                        producer->nbEnqueueFrames++;
                                        // ref_from_graph remains IN_GRAPH since it is enqueued back in the producer
                                        VX_PRINT(
                                            VX_ZONE_INFO,
                                            "PRODUCER %s: buffer ID was not sent to any consumer, %d enqueue back to graph\n", producer->name,
                                            ref_from_graph);
                                    }
                                    else
                                    {
                                        // it was sent to at least one consumer
                                        VX_PRINT(
                                            VX_ZONE_INFO,
                                            "PRODUCER %s: objectbuffer ID %d sent to %d consumers\n",
                                            producer->name,
                                            buffer_id,
                                            sent_messages);
                                    }
                                }
                                else
                                {
                                    // Frame dropped, since we want the producer to keep running
                                    VX_PRINT(
                                        VX_ZONE_WARNING,
                                        "PRODUCER %s: graph is running, no free buffer found, enqueue current buffer "
                                        "directly, amount of last frames dropped %d\n",
                                        producer->name,
                                        producer->last_frame_dropped);
                                    producer->streaming_cb.enqueueCallback(producer->graph_obj, ref_from_graph);
                                    producer->nbEnqueueFrames++;
                                    producer->nbDroppedFrames++;
                                    producer->last_frame_dropped++;
                                }
                            }
                        }
                    }
                }
            }
            break;

            case VX_PROD_STATE_WAIT:
            {
                vx_uint32 waitCount;
                // wait a fixed ammount of times to dequeue each reference, enabling the graph to shut down properly
                for (waitCount = 0; waitCount < producer->numBuffers; waitCount++)
                {
                    vx_uint32 num_deque_refs = 0;

                    tivxTaskWaitMsecs(100);

                    // only dequeue if there is any reference in graph, to prevent forever blocking on dequeue
                    if (1U < get_num_buffer_with_status(producer, IN_GRAPH))
                    {
                        status = producer->streaming_cb.dequeueCallback(producer->graph_obj, dequeued_refs, &num_deque_refs);
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            // in case of error stop trying to dequeue
                            break;
                        }
                        set_buffer_status(dequeued_refs[0], FREE, producer);
                        producer->nbDequeueFrames += num_deque_refs;
                    }
                }

                // producer graph is flushed
                VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: graph flushed\n", producer->name);
                producer->graph_state = VX_PROD_STATE_FLUSH;

                // linger a while before shutting the producer thread - there might be consumers
                // still working with the buffers sent by the producer. we dont want to release memory too early
                if (producer->nb_consumers > 0)
                {
                    VX_PRINT(VX_ZONE_INFO, "PRODUCER %s: waiting for consumers before shutdown \n", producer->name);
                    tivxTaskWaitMsecs(200);
                }
            }
            break;

            case VX_PROD_STATE_FLUSH:
            {
                VX_PRINT(
                    VX_ZONE_INFO,
                    "PRODUCER %s: Output buffer stats:\nenqueued - %d \ndequeued - %d \ndropped - %d\n",
                    producer->name,
                    producer->nbEnqueueFrames,
                    producer->nbDequeueFrames,
                    producer->nbDroppedFrames);
                shutdown = (vx_bool)vx_true_e;
            }
            break;
        }
    }
    return NULL;
}

static vx_status ownInitProducerObject(vx_producer producer, const vx_producer_params_t* params)
{
    int32_t l_status = 0;
    vx_status status = (vx_status)VX_SUCCESS;

    (void)snprintf(producer->name, VX_MAX_PRODUCER_NAME, params->name);
    (void)snprintf(producer->access_point_name , VX_MAX_ACCESS_POINT_NAME, params->access_point_name);

    for (vx_uint32 i = 0U; i < VX_GW_NUM_CLIENTS; i++)
    {
        producer->consumers_list[i].state = PROD_STATE_CLI_NOT_CONNECTED;

#ifdef SOCKET_ENABLED
        producer->consumers_list[i].socket_fd = 0;
#endif
    }

    producer->graph_obj                     = params->graph_obj;
    producer->numBuffers                    = params->num_buffers;
    producer->numBufferRefsExport           = params->num_buffer_refs_export;
    producer->maxRefsLockedByClient         = params->max_refs_locked_by_client;
    producer->streaming_cb                  = params->streaming_cb;
#ifdef IPPC_SHEM_ENABLED        
    producer->connection_check_polling_time = params->connection_check_polling_time;
    producer->connection_check_polling_exit = vx_false_e; 

    for(vx_uint32 idx = 0U; idx < IPPC_PORT_COUNT; idx++)
    {
        producer->ippc_port[idx] = params->ippc_port[idx];
    }
#endif

    producer->nb_consumers = 0;

    producer->graph_state          = VX_PROD_STATE_INIT;

    if (producer->numBuffers < 2 || producer->numBuffers > VX_GW_MAX_NUM_REFS)
    {
        VX_PRINT(VX_ZONE_ERROR, "PRODUCER: Bad number of producer references!\n");
        return (vx_status)VX_FAILURE;
    }

    for (uint32_t buff_id = 0; buff_id < producer->numBufferRefsExport; buff_id++)
    {
        if ((params->ref_to_export[buff_id] != NULL) &&
            (VX_SUCCESS == vxGetStatus(params->ref_to_export[buff_id])))
        {
            //  Copy the graph OUTPUT references to the producer internal buffer
            producer->refs[buff_id].ovx_ref       = params->ref_to_export[buff_id];
            producer->refs[buff_id].buffer_status = FREE;
            producer->refs[buff_id].refcount      = 0;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "PRODUCER: NULL pipeline references detected, producer_init failed\n");
            return (vx_status)VX_FAILURE;
        }
    }

    pthread_mutexattr_t buffInfoMutexAttr;
    pthread_mutexattr_init(&buffInfoMutexAttr);

    status = pthread_mutex_init(&producer->buffer_mutex, &buffInfoMutexAttr);
    if (status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "PRODUCER: pthread_mutex_init() failed for buffer info mutex\n");
        return (vx_status)VX_FAILURE;
    }

#ifdef IPPC_SHEM_ENABLED
    l_status = ippc_shmem_init(producer->access_point_name, producer->numBuffers, producer->ippc_port,
                    IPPC_PORT_COUNT, sizeof(vx_prod_msg_content_t), sizeof(vx_cons_msg_content_t), &producer->m_shmem_ctx);

    if(E_IPPC_OK == l_status)
    {
        SIppcPortMap l_portMap = producer->ippc_port[0];
        l_status = ippc_sender_init(&producer->m_shmem_ctx, &l_portMap, &producer->m_sender_ctx);
    }

    if(E_IPPC_OK != l_status)
    {
        status = (vx_status)VX_FAILURE;
    }

#elif SOCKET_ENABLED
    producer->server.socket_name    = producer->access_point_name;
    producer->server.client_arg     = (void*)producer;
    producer->server.client_handler = handle_clients;
    l_status                        = socket_server_create(&producer->server);

    if (l_status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "PRODUCER: socket_server_create() failed for master channel.\n");
        status = (vx_status)VX_FAILURE;
    }
#endif
    pthread_mutexattr_t client_mutexAttr;
    pthread_mutexattr_init(&client_mutexAttr);

    status = pthread_mutex_init(&producer->client_mutex, &client_mutexAttr);
    if (status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "PRODUCER: pthread_mutex_init() failed for client handling mutex\n");
        return (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status ownDestructProducer(vx_reference ref)
{
    return ((vx_status)VX_SUCCESS);
    
}

static vx_status ownAllocProducerBuffer(vx_reference ref)
{
    return ((vx_status)VX_SUCCESS);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseProducer(vx_producer* producer)
{
    vx_producer this_producer = producer[0];

    pthread_join(this_producer->broadcast_thread, NULL);
    pthread_mutex_destroy(&(this_producer->buffer_mutex));

#ifdef IPPC_SHEM_ENABLED
    ippc_shmem_deinit(&this_producer->m_shmem_ctx, this_producer->access_point_name);
#elif SOCKET_ENABLED
    socket_server_close(&this_producer->server);
    pthread_mutex_destroy(&(this_producer->client_mutex));
#endif
    return (ownReleaseReferenceInt(
        vxCastRefFromProducerP(producer), VX_TYPE_PRODUCER, (vx_enum)VX_EXTERNAL, NULL));
}

VX_API_ENTRY vx_producer VX_API_CALL vxCreateProducer(vx_context context, const vx_producer_params_t* params)
{
    vx_producer producer = NULL;
    vx_reference ref = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    /* create a producer object */
    if(ownIsValidContext(context) == (vx_bool)vx_true_e)
    {
        ref = ownCreateReference(context, (vx_enum)VX_TYPE_PRODUCER, (vx_enum)VX_EXTERNAL, &context->base);

        if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
            (ref->type == (vx_enum)VX_TYPE_PRODUCER))
        {
            /* status set to NULL due to preceding type check */
            producer = vxCastRefAsProducer(ref,NULL);
            producer->base.destructor_callback = &ownDestructProducer; /* specific destructor because of no tiovx_obj*/
            producer->base.mem_alloc_callback  = &ownAllocProducerBuffer;
            producer->base.release_callback    = &ownReleaseReferenceBufferGeneric;

            status = ownInitProducerObject(producer, params);

            if((vx_status)VX_SUCCESS != status)
            {
                status = vxReleaseProducer(&producer);
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to release reference to a producer \n");
                }

                VX_PRINT(VX_ZONE_ERROR, "Could not create producer\n");
                ref = ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
                /* status set to NULL due to preceding type check */
                producer = vxCastRefAsProducer(ref, NULL);
            }
        }
    }
    /* return the producer object */
    return(producer);
}

VX_API_ENTRY vx_status VX_API_CALL vxProducerStart(vx_producer producer)
{
    /* start the ippc broadcasting thread */
    int thread_status = pthread_create(&producer->broadcast_thread, NULL, producer_broadcast_thread, (void*)producer);
    if (0U != thread_status)
    {
        VX_PRINT(VX_ZONE_ERROR, "error creating producer_broadcast_thread! \n");
    }
#ifdef IPPC_SHEM_ENABLED
    else
    {
        thread_status = pthread_create(&producer->connection_check_thread, NULL, producer_connection_check_thread, (void*)producer);
    }
#endif
    return ((vx_status)thread_status);
}

VX_API_ENTRY vx_status VX_API_CALL vxProducerShutdown(vx_producer producer)
{
    producer->last_buffer = 1;

    return ((vx_status)VX_SUCCESS);
}
