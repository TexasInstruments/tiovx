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

#define CONSUMER_MAX_CONSECUTIVE_FAILURES (5u)

static int32_t get_buffer_id(vx_reference buffer_ref, vx_reference* reference_array, uint32_t reference_array_size)
{
    int32_t buffer_id;
    for (buffer_id = 0; (uint32_t)buffer_id < reference_array_size; buffer_id++)
    {
        if (reference_array[buffer_id] == buffer_ref)
        {
            VX_PRINT(VX_ZONE_REFERENCE, "CONSUMER: found a buffer ref %p at index %d \n", buffer_ref, buffer_id);
            break;
        }
    }

    if ((uint32_t)buffer_id == reference_array_size)
    {
        buffer_id = -1;
        VX_PRINT(
            VX_ZONE_ERROR,
            "CONSUMER: Dequeued reference cannot be found in the consumer registered references %s",
            "\n");
    }

    return buffer_id;
}

static int32_t send_buffer_release_message(vx_consumer consumer, void* message_buffer, uint8_t buffer_id, vx_gw_message_type message_type)
{
    int32_t status = 0U;
#ifdef IPPC_SHEM_ENABLED
    vx_cons_msg_content_t* msg   = (vx_cons_msg_content_t*)message_buffer;
    msg->consumer_id        = consumer->consumer_id;
    msg->msg_type           = message_type;
#elif SOCKET_ENABLED
    vx_gw_buff_id_msg* msg = (vx_gw_buff_id_msg*)message_buffer;
    msg->msg_type           = message_type;
    msg->consumer_id        = consumer->consumer_id;
#endif
    msg->buffer_id          = buffer_id;

    // check if this is the last buffer which has been received - either dequeued or dropped
    if ((consumer->last_buffer == 1) &&
        ((consumer->last_buffer_id == buffer_id) || (consumer->last_buffer_dropped == 1)))
    {
        consumer->state = VX_CONS_STATE_WAIT;
        VX_PRINT(
            VX_ZONE_INFO,
            "CONSUMER: last buffer has been processed, wait before putting pipeline in flush mode%s",
            "\n");
        msg->last_buffer = 1U;
    }

#ifdef IPPC_SHEM_ENABLED
    status = ippc_shem_send(&consumer->m_sender_ctx);
#elif SOCKET_ENABLED
    status = socket_write(consumer->socket_fd, message_buffer, NULL, 0);
#endif
    if (status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: buffer ID %d message could not be sent%s", buffer_id, "\n");
    }

    return status;
}

#ifdef IPPC_SHEM_ENABLED
static vx_status
import_ref_from_producer(vx_consumer consumer, vx_prod_msg_content_t* buff_desc_msg)
{
    vx_status  status = VX_SUCCESS;
    consumer->num_refs = buff_desc_msg->num_refs;

    if(0U == consumer->num_refs)
    {
        status = (vx_status)VX_FAILURE;
    }

    for(vx_uint32 idx = 0U; idx < buff_desc_msg->num_refs; idx++)
    {
        // number of items message field must be set if references sent are members of an object array
        if (buff_desc_msg->num_items > 0)
        {
            for(vx_uint32 jdx = 0U; jdx < (buff_desc_msg->num_items + 1U); jdx++)
            {
                tivx_utils_ref_ipc_msg_t* ref_export_handle          = &buff_desc_msg->ref_export_handle[idx][jdx];
                // determine if receiving object array metadata (last message after object array items)
                if (ref_export_handle->refDesc.meta.type == VX_TYPE_OBJECT_ARRAY)
                {
                    VX_PRINT(
                        VX_ZONE_INFO,
                        "CONSUMER: Importing object array with %d, items of type %d\n",
                        buff_desc_msg->num_items,
                        ref_export_handle->refDesc.meta.type);

                    // finish reception of object array
                    vx_reference objarray_ref = NULL;
                    status = rbvx_utils_import_ref_from_ipc_xfer_objarray(
                        consumer->context, ref_export_handle, (tivx_utils_ref_ipc_msg_t*)&consumer->ipcMessageArray, &objarray_ref);
                    if ((status != VX_SUCCESS) && (vxGetStatus(objarray_ref) != VX_SUCCESS))
                    {
                        VX_PRINT(
                            VX_ZONE_ERROR,
                            "CONSUMER: rbvx_utils_import_ref_from_ipc_xfer_objarray() failed for ref [%d]\n",
                            consumer->num_refs);
                    }
                    else
                    {
                        consumer->refs[idx] = objarray_ref;
                        consumer->ipcMessageCount = 0; // Wrap the intermediate reference counter
                    }
                }
                else
                {
                    // store object arra item metadata
                    VX_PRINT(
                        VX_ZONE_INFO,
                        "CONSUMER: Receiving object array item %d, of type %d\n",
                        jdx,
                        ref_export_handle->refDesc.meta.type);

                    for (uint32_t i = 0; i < ref_export_handle->numFd; i++)
                    {
                        consumer->ipcMessageArray[consumer->ipcMessageCount].fd[i] = ref_export_handle->fd[i];
                    }
                    consumer->ipcMessageArray[consumer->ipcMessageCount].refDesc = ref_export_handle->refDesc;
                    consumer->ipcMessageArray[consumer->ipcMessageCount].numFd = ref_export_handle->numFd;
                    consumer->ipcMessageCount++;
                }
            }
        }
        else if (buff_desc_msg->num_items == 0)
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: Importing single reference of type %d\n", buff_desc_msg->ref_export_handle[idx][0].refDesc.meta.type);

            // receiving non-object array single reference
            vx_reference single_ref = NULL;
            status = tivx_utils_import_ref_from_ipc_xfer(consumer->context, &buff_desc_msg->ref_export_handle[idx][0], &single_ref);
            if ((status == VX_SUCCESS) && (vxGetStatus(single_ref) == VX_SUCCESS))
            {
                consumer->refs[consumer->num_refs] = single_ref;
            }
            else
            {
                VX_PRINT(
                    VX_ZONE_ERROR,
                    "CONSUMER: tivx_utils_import_ref_from_ipc_xfer() failed for ref [%d]\n",
                    consumer->num_refs);
            }
        }
        else
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}

void *consumer_backchannel(void* arg)
{
    vx_consumer consumer = (vx_consumer) arg;
    vx_reference dequeued_refs[VX_GW_MAX_NUM_REFS] = {0};

    char threadname[280U];
    snprintf(threadname, 280U, "%s_bck_thread", consumer->name);
    pthread_setname_np(pthread_self(), threadname);

    VX_PRINT(VX_ZONE_INFO, "CONSUMER: Starting backchannel %s", "\n");

    while(1U)
    {
        vx_uint32 num_dequeued_refs = 0U;
        vx_status status = consumer->subscriber_cb.dequeueCallback(consumer->graph_obj, dequeued_refs, &num_dequeued_refs);
        if (status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Error while dequeuing buffer %s", "\n");
        }
        for (vx_uint32 current_ref_num = 0; current_ref_num < num_dequeued_refs; current_ref_num++)
        {
            // Process one reference at a time
            vx_reference ref_from_graph = dequeued_refs[current_ref_num];
            //  Get producer internal buffer id, and send it to the consumer
            int32_t buffer_id = get_buffer_id(ref_from_graph, consumer->refs, consumer->num_refs);
            VX_PRINT(
                    VX_ZONE_INFO,
                    "CONSUMER: dequeue successfull, send the buffer id: %d back to producer!\n",
                    buffer_id);
            if (buffer_id < 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: wrong buffer ID %d \n", buffer_id);
                consumer->state = VX_CONS_STATE_FAILED;
                break;
            }
            else
            {
                // we have something dequeued and the graph is finished processing
                EIppcStatus ippc_status;
                vx_cons_msg_content_t* l_send_msg;
                pthread_mutex_lock(&consumer->buffer_mutex);
                l_send_msg = ippc_shem_payload_pointer(&consumer->m_sender_ctx, sizeof(vx_cons_msg_content_t), &ippc_status);
                if (ippc_status == E_IPPC_OK)
                {
                    VX_PRINT(
                        VX_ZONE_INFO,
                        "CONSUMER: current buffer ID %d last buffer ID %d dequeued, last buffer flag %d \n",
                        buffer_id,
                        consumer->last_buffer_id,
                        consumer->last_buffer);
                    l_send_msg->last_buffer = consumer->last_buffer;
                    send_buffer_release_message(consumer, l_send_msg, buffer_id, VX_MSGTYPE_BUF_RELEASE);
                }
                pthread_mutex_unlock(&consumer->buffer_mutex);
            }
        }

        if (consumer->last_buffer || (VX_CONS_STATE_FLUSH == consumer->state))
        {
            break;
        }
    }
    return NULL;
}

void consumer_msg_handler(const void * consumer_p, const void * data_p, uint8_t last_buffer_from_series)
{
    int32_t  status = 0;
    EIppcStatus l_status = (EIppcStatus)E_IPPC_OK;
    vx_consumer consumer = (vx_consumer) consumer_p;
    vx_prod_msg_content_t* const l_received_message = (vx_prod_msg_content_t*)data_p;
    
    //if the consumer is ready to communicate, send the back channel port id to the producer
    if ((vx_bool)vx_false_e == consumer->init_done)
    {
        VX_PRINT(VX_ZONE_INFO, "CONSUMER %u (%s): attaching to backhannel \n", consumer->consumer_id, consumer->name);
        /* feed the data for the sender */
        consumer->m_sender_ctx.m_msg_size = sizeof(vx_cons_msg_content_t);
        const SIppcPortMap *l_port = ippc_get_port_by_recv_index(consumer->ippc_port, consumer->consumer_id);
        consumer->m_sender_ctx.m_port_map.m_port_id        = l_port->m_port_id;
        consumer->m_sender_ctx.m_port_map.m_port_type      = l_port->m_port_type;
        consumer->m_sender_ctx.m_port_map.m_receiver_index = l_port->m_receiver_index;

        l_status = ippc_registry_sender_attach(&consumer->m_registry, &consumer->m_sender_ctx.m_sender, 
                                                consumer->m_sender_ctx.m_port_map.m_port_id, consumer->m_sender_ctx.m_msg_size);

        if (E_IPPC_OK == l_status)
        {
            // sender is on 1->1 port, attach to a single sync; offset by number of syncs for broadcast
            l_status = ippc_registry_sync_attach(&consumer->m_registry, &consumer->m_sender_ctx.m_sync[0], 
                                                    consumer->m_sender_ctx.m_port_map.m_receiver_index + VX_GW_NUM_CLIENTS);
        }

        if (E_IPPC_OK != l_status)
        {
            status = VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Failed to attach to back channel port!%s", "\n");
        }
        else
        {
            consumer->init_done = (vx_bool)vx_true_e;
        }
    }

    // For multiple client scenario, the import of references doesnt happen in first receive
    // Hence, wait until the data is available, then proceed
    if (((vx_bool)vx_true_e == consumer->init_done) && ((vx_bool)vx_false_e == consumer->ref_import_done))
    {
        status = import_ref_from_producer(consumer, l_received_message);

        if (VX_SUCCESS == status)
        {
            status = consumer->subscriber_cb.createCallback(consumer->graph_obj, consumer->refs, consumer->num_refs);
            if (status != VX_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER (%s): application create graph failed \n", consumer->name);
            }
            else
            {
                status = VX_GW_STATUS_CONSUMER_GRAPH_READY; 
                VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): application create graph success", consumer->name);
                // open a new backchannel thread to dequeue stuff
                int thread_status = pthread_create(&consumer->backchannel_thread, NULL, consumer_backchannel, (void*)(consumer));
                if (thread_status != 0)
                {
                    VX_PRINT(VX_ZONE_ERROR, "CONSUMER (%s): failed to create consumer_backchannel client thread", consumer->name);
                }
                else
                {
                    consumer->ref_import_done = (vx_bool)vx_true_e;
                }
            }
        }
    }

    if (-1 != l_received_message->buffer_id)
    {
        if (1U == l_received_message->last_buffer)
        {
            // if last buffer was sent, producer needs to be notified by setting last buffer flag, response needs to be sent regardless of whether receiver was addressed via mask or not
            consumer->last_buffer = 1U;
            consumer->last_buffer_transmitted = 1;
            consumer->last_buffer_id = l_received_message->buffer_id;
            VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): Received last buffer id %d \n", consumer->name, consumer->last_buffer_id);
        }            

        if (l_received_message->mask & (1U << consumer->consumer_id)) // mask applies to consumer
        {
            if((1U == last_buffer_from_series)) // pass reference to graph
            {
                VX_PRINT(
                    VX_ZONE_INFO,
                    "CONSUMER (%s): Received buffer ID (%d), last buffer %d mask %d\n",
                    consumer->name,
                    l_received_message->buffer_id,
                    l_received_message->last_buffer, l_received_message->mask);
                // check if metadata from IPC was received and apply it to consumer reference
                if ((1U == l_received_message->metadata_valid) && (NULL != consumer->subscriber_cb.storeMetadataCallback))
                {
                    status = consumer->subscriber_cb.storeMetadataCallback(
                        consumer->graph_obj, consumer->refs[l_received_message->buffer_id], &l_received_message->metadata_buffer, l_received_message->metadata_size);
                }

                VX_PRINT(
                    VX_ZONE_INFO,
                    "CONSUMER (%s): enqueue the incoming buffer id: %d with ref: %p into the pipeline as input buffer, last "
                    "buffer %d mask %d \n",
                    consumer->name,
                    l_received_message->buffer_id,
                    consumer->refs[l_received_message->buffer_id],
                    l_received_message->last_buffer, 
                    l_received_message->mask);

                status = consumer->subscriber_cb.enqueueCallback(consumer->graph_obj, (vx_reference)consumer->refs[l_received_message->buffer_id]);            
            }
            else // drop reference   
            {
                VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): Received buffer ID (%d) mask %d set for this consumer, but not latest message, drop ref \n", 
                    consumer->name, l_received_message->buffer_id, l_received_message->mask);
                status = VX_GW_STATUS_CONSUMER_REF_DROP;
            }  
        }
        else // mask not set, respond to producer only if it was last buffer sent by producer
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): Received buffer ID (%d) mask %d not set for this consumer, return success without notifying Producer\n", consumer->name, l_received_message->buffer_id, l_received_message->mask);
            if(1U == l_received_message->last_buffer) // only in case of last buffer, notify producer
            {
                status = VX_GW_STATUS_CONSUMER_REF_DROP;
            }
        } 
    }

    if (0 > status)
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER (%s): MSG RECEIVE STATUS: FAILED.", consumer->name);
    }
    else if (consumer->state == VX_CONS_STATE_FLUSH)
    {
        // buffer was not enqueued, transmit the buffer back to the producer immediately and shutdown
        consumer->last_buffer_dropped = 1U;

        vx_cons_msg_content_t* l_send_msg;
        pthread_mutex_lock(&consumer->buffer_mutex);
        l_send_msg = ippc_shem_payload_pointer(&consumer->m_sender_ctx, sizeof(vx_cons_msg_content_t), &l_status);
        l_send_msg->last_buffer = 1U;
        if(E_IPPC_OK == l_status)
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): CONSUMER DROPS FRAME and sets last buffer flag, BUFFER ID %d, %s.", consumer->name, l_received_message->buffer_id, "\n");
            send_buffer_release_message(consumer, l_send_msg, l_received_message->buffer_id, VX_MSGTYPE_BUF_RELEASE);
        }
        pthread_mutex_unlock(&consumer->buffer_mutex);
    }
    else if (VX_GW_STATUS_CONSUMER_REF_DROP == status)
    {
        // buffer was not enqueued, transmit the buffer back to the producer immediately
        consumer->last_buffer_dropped = 1U;

        vx_cons_msg_content_t* l_send_msg;
        pthread_mutex_lock(&consumer->buffer_mutex);
        l_send_msg = ippc_shem_payload_pointer(&consumer->m_sender_ctx, sizeof(vx_cons_msg_content_t), &l_status);
        l_send_msg->last_buffer = 0U;
        if(E_IPPC_OK == l_status)
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): CONSUMER DROPS FRAME, BUFFER ID %d, %s.", consumer->name, l_received_message->buffer_id, "\n");
            send_buffer_release_message(consumer, l_send_msg, l_received_message->buffer_id, VX_MSGTYPE_BUF_RELEASE);
        }
        pthread_mutex_unlock(&consumer->buffer_mutex);
    }
    else if (VX_GW_STATUS_CONSUMER_GRAPH_READY == status)
    {
        // notify producer that consumer is ready to consume 
        vx_cons_msg_content_t* l_send_msg;
        pthread_mutex_lock(&consumer->buffer_mutex);
        l_send_msg = ippc_shem_payload_pointer(&consumer->m_sender_ctx, sizeof(vx_cons_msg_content_t), &l_status);
        l_send_msg->last_buffer = 0U;
        if(E_IPPC_OK == l_status)
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER (%s): send graph ready to producer, BUFFER ID %d, %s.", consumer->name, l_received_message->buffer_id, "\n");
            send_buffer_release_message(consumer, l_send_msg, l_received_message->buffer_id, VX_MSGTYPE_CONSUMER_CREATE_DONE);
        }
        pthread_mutex_unlock(&consumer->buffer_mutex);
    }
    else
    {
        consumer->last_buffer_dropped = 0;
        status                        = 0;
    }
    
}

static void* consumer_receiver_thread(void* arg)
{
    vx_consumer consumer = (vx_consumer)arg;
    vx_bool shutdown = (vx_bool)vx_false_e;
    EIppcStatus status = (EIppcStatus)E_IPPC_OK;

    char threadname[280U];
    snprintf(threadname, 280U, "%s_receiver_thread", consumer->name);
    pthread_setname_np(pthread_self(), threadname);

    while((vx_bool)vx_false_e == shutdown)
    {
        switch(consumer->state)
        {
            case VX_CONS_STATE_DISCONNECTED:
            {
                status = ippc_shm_exists(consumer->access_point_name);
                if (E_IPPC_OK == status)
                {
                    status = ippc_shm_attach_registry(&consumer->m_registry, consumer->access_point_name);

                }
                
                if (E_IPPC_OK == status)
                {
                    VX_PRINT(VX_ZONE_PERF, "CONSUMER: connection made with producer on SHM %s\n", consumer->access_point_name);
                    consumer->state = VX_CONS_STATE_INIT;
                }
                else
                {
                    VX_PRINT(VX_ZONE_INFO, "CONSUMER: Waiting for connection with producer...%s", "\n");
                    tivxTaskWaitMsecs(consumer->connect_polling_time);
                }
            }
            break;

            case VX_CONS_STATE_INIT:
            {
                // attaching reciever now
                consumer->m_receiver_ctx.m_application_ctx = consumer;
                consumer->m_receiver_ctx.m_client_handler = consumer_msg_handler;

                /* feed the same information again into the receiver member, cleanup necessary */
                consumer->m_receiver_ctx.m_port_map.m_receiver_index = consumer->consumer_id;
                consumer->m_receiver_ctx.m_port_map.m_port_id        = consumer->ippc_port[0].m_port_id;
                consumer->m_receiver_ctx.m_port_map.m_port_type      = consumer->ippc_port[0].m_port_type;
                /* we should avoid this kind of thing: consumer->m_receiver_ctx.m_receiver_ctx*/
                consumer->m_receiver_ctx.m_msg_size = sizeof(vx_prod_msg_content_t);
                status  = ippc_registry_receiver_attach(&consumer->m_registry,
                                                        &consumer->m_receiver_ctx.m_receiver,
                                                        consumer->m_receiver_ctx.m_port_map.m_port_id,
                                                        consumer->m_receiver_ctx.m_port_map.m_receiver_index,
                                                        consumer->m_receiver_ctx.m_msg_size,
                                                        E_IPPC_RECEIVER_DISCARD_PAST);
                if (E_IPPC_OK == status)
                {
                    /* init the receiver */
                    status = ippc_registry_sync_attach(&consumer->m_registry, 
                                                        &consumer->m_receiver_ctx.m_sync, 
                                                        consumer->m_receiver_ctx.m_port_map.m_receiver_index);
                }

                if (E_IPPC_OK == status)
                {
                    VX_PRINT(VX_ZONE_PERF, " [UPT] First Time Connected to producer!%s", "\n");
                    VX_PRINT(VX_ZONE_INFO, "CONSUMER: attached to producer with SHM %s\n", consumer->access_point_name);
                    consumer->state = VX_CONS_STATE_RUN;
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Could not attach with producer!%s", "\n");
                    consumer->state = VX_CONS_STATE_FAILED;
                }                                                                                                
            }
            break;

            case VX_CONS_STATE_RUN:
            {
                if (consumer->last_buffer)
                {
                    consumer->state = VX_CONS_STATE_WAIT;
                }
                else
                {
                    ippc_receive(&consumer->m_receiver_ctx);
                }
            }
            break;

            case VX_CONS_STATE_WAIT:
            {
                tivxTaskWaitMsecs(100);
                VX_PRINT(VX_ZONE_INFO, "CONSUMER: going to flush state%s", "\n");
                consumer->state = VX_CONS_STATE_FLUSH;
            }
            break;

            case VX_CONS_STATE_FAILED:
            {
                // consumer failed too many times OR a recovery cannot be done
                if (consumer->num_failures >= CONSUMER_MAX_CONSECUTIVE_FAILURES || consumer->subscriber_cb.recoveryCallback == NULL ||
                    consumer->last_buffer == 1U)
                {
                    VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Disconnected too many times, shutting down %s", "\n");
                    consumer->state = VX_CONS_STATE_FLUSH;
                    break;
                }

                consumer->num_failures++;

                if (consumer->subscriber_cb.recoveryCallback != NULL)
                {
                    consumer->num_refs        = 0;
                    consumer->ipcMessageCount = 0;

                    // reset state machine
                    consumer->state = VX_CONS_STATE_DISCONNECTED;

                    VX_PRINT(VX_ZONE_INFO, "CONSUMER: calling recovery callback %s", "\n");
                    consumer->subscriber_cb.recoveryCallback(consumer->graph_obj);
                    // consumer deinit can be called from recovery callback, which will cause this thread to shut down
                }
            }
            break;

            case VX_CONS_STATE_FLUSH:
            {
                VX_PRINT(VX_ZONE_INFO, "CONSUMER: pipeline is flushed, reached normal shutdown%s", "\n");
                shutdown = (vx_bool)vx_true_e;
            }
            break;
        }
    }
    return NULL;
}

#elif SOCKET_ENABLED
static void* buffer_id_thread(void* ctxPtr);

static int32_t
import_ref_from_producer(vx_consumer consumer, vx_gw_buff_desc_msg* buff_desc_msg, int32_t* fd, uint32_t numFd)
{
    int32_t                   status           = VX_GW_STATUS_FAILURE;
    vx_status                 framework_status = VX_FAILURE;
    tivx_utils_ref_ipc_msg_t* ref_export_handle          = &buff_desc_msg->ref_export_handle;

    // for linux platforms switch to FDs received over CMSG
    ref_export_handle->numFd = numFd;
    ref_export_handle->fd[0] = fd[0];

    // number of items message field must be set if references sent are members of an object array
    if (buff_desc_msg->num_items > 0)
    {
        // determine if receiving object array metadata (last message after object array items)
        if (ref_export_handle->refDesc.meta.type == VX_TYPE_OBJECT_ARRAY)
        {
            VX_PRINT(
                VX_ZONE_INFO,
                "CONSUMER: Importing object array with %d, items of type %d\n",
                buff_desc_msg->num_items,
                ref_export_handle->refDesc.meta.type);

            // finish reception of object array
            vx_reference objarray_ref = NULL;
            framework_status          = rbvx_utils_import_ref_from_ipc_xfer_objarray(
                consumer->context, ref_export_handle, (tivx_utils_ref_ipc_msg_t*)&consumer->ipcMessageArray, &objarray_ref);
            if ((framework_status != VX_SUCCESS) && (vxGetStatus(objarray_ref) != VX_SUCCESS))
            {
                VX_PRINT(
                    VX_ZONE_ERROR,
                    "CONSUMER: rbvx_utils_import_ref_from_ipc_xfer_objarray() failed for ref [%d]\n",
                    consumer->num_refs);
            }
            else
            {
                consumer->refs[consumer->num_refs] = objarray_ref;
                consumer->num_refs++;
                consumer->ipcMessageCount = 0; // Wrap the intermediate reference counter
                status                    = VX_GW_STATUS_SUCCESS;
            }
        }
        else
        {
            // store object arra item metadata
            VX_PRINT(
                VX_ZONE_INFO,
                "CONSUMER: Receiving object array item %d, of type %d\n",
                buff_desc_msg->item_index,
                ref_export_handle->refDesc.meta.type);

            for (uint32_t i = 0; i < ref_export_handle->numFd; i++)
            {
                consumer->ipcMessageArray[consumer->ipcMessageCount].fd[i] = ref_export_handle->fd[i];
            }
            memcpy(
                (void*)&consumer->ipcMessageArray[consumer->ipcMessageCount].refDesc,
                &ref_export_handle->refDesc,
                sizeof(tivx_utils_ref_desc_t));
            consumer->ipcMessageArray[consumer->ipcMessageCount].numFd = ref_export_handle->numFd;
            consumer->ipcMessageCount++;
            status = VX_GW_STATUS_SUCCESS;
        }
    }
    else if (buff_desc_msg->num_items == 0)
    {
        VX_PRINT(VX_ZONE_INFO, "CONSUMER: Importing single reference of type %d\n", ref_export_handle->refDesc.meta.type);

        // receiving non-object array single reference
        vx_reference single_ref = NULL;
        framework_status        = tivx_utils_import_ref_from_ipc_xfer(consumer->context, ref_export_handle, &single_ref);
        if ((framework_status == VX_SUCCESS) && (vxGetStatus(single_ref) == VX_SUCCESS))
        {
            consumer->refs[consumer->num_refs] = single_ref;
            consumer->num_refs++;
            status = VX_GW_STATUS_SUCCESS;
        }
        else
        {
            VX_PRINT(
                VX_ZONE_ERROR,
                "CONSUMER: tivx_utils_import_ref_from_ipc_xfer() failed for ref [%d]\n",
                consumer->num_refs);
        }
    }
    else
    {
        // error
    }

    return status;
}

static vx_gw_status_t start_sync_with_producer(vx_consumer consumer, char* access_point_name, vx_gw_hello_msg* msg)
{
    int32_t status    = 0;
    int32_t socket_fd = 0;

    // Wait for a socket connection with the producer
    while (1)
    {
        status = socket_client_connect(access_point_name, &socket_fd);
        if ((socket_fd < 0) && (status == SOCKET_STATUS_CONNECT_ERROR))
        {
            tivxTaskWaitMsecs(consumer->connect_polling_time);
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: Waiting for connection with producer...%s", "\n");
        }
        else if ((socket_fd > 0) && (status == SOCKET_STATUS_OK))
        {
            VX_PRINT(VX_ZONE_PERF, "CONSUMER: connection made with producer on socket %s\n", access_point_name);
            break;
        }
        else if (consumer->state == VX_CONS_STATE_FAILED)
        {
            return VX_GW_STATUS_FAILURE;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Fatal error when creating client socket!%s", "\n");
            return VX_GW_STATUS_FAILURE;
        }

        if (consumer->state == VX_CONS_STATE_FLUSH)
        {
            return VX_GW_STATUS_CONSUMER_FLUSHED;
        }
    }

    msg->msg_type = VX_MSGTYPE_HELLO;
    msg->consumer_id = consumer->consumer_id;

    status = socket_write(socket_fd, (uint8_t*)msg, NULL, 0);
    if ((status < SOCKET_STATUS_OK) || (status == SOCKET_STATUS_PEER_CLOSED))
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Failed while sending HELLO msg%s", "\n");
        socket_fd = 0;
    }
    else
    {
        VX_PRINT(
            VX_ZONE_INFO,
            "CONSUMER: HELLO message sent, starting sync with producer on socket %s.%s",
            access_point_name,
            "\n");
    }

    consumer->socket_fd = socket_fd;

    return VX_GW_STATUS_SUCCESS;
}

static int32_t handle_receive_message(vx_consumer consumer, int32_t socket_fd, uint8_t* message_buffer)
{
    int32_t  fd[VX_IPC_MAX_VX_PLANES];
    uint32_t num_fd = 0;
    int32_t  status = 0;

    vx_gw_hello_msg* message_header = (vx_gw_hello_msg*)message_buffer;

    if (consumer->last_buffer == 1U)
    {
        VX_PRINT(VX_ZONE_INFO, "CONSUMER: reconfiguring the socket timeouts for release %s", "\n");
        socket_reconfigure_timeout(socket_fd, SOCKET_TIMEOUT_USECS_RELEASE);
    }

    //  this is called from different threads based on state - blocks while reading on the socket
    status = socket_read(socket_fd, message_buffer, fd, &num_fd);

    if (status < 0)
    {
        // there was an error, return error immediately
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: socket_read() failed%s", "\n");
        status = SOCKET_STATUS_FAILURE;
        return status;
    }
    else if (status == SOCKET_STATUS_PEER_CLOSED)
    {
        // socket has been closed
        VX_PRINT(VX_ZONE_INFO, "CONSUMER: Connection was closed by peer for socket %d %s", socket_fd, "\n");
        return SOCKET_STATUS_PEER_CLOSED;
    }
    else
    {
        // message was received fine (status = 0), process the message
    }

    switch (message_header->msg_type)
    {
    case VX_MSGTYPE_REF_BUF:
    {
        vx_gw_buff_desc_msg* msg = (vx_gw_buff_desc_msg*)message_buffer;
        VX_PRINT(VX_ZONE_INFO, "CONSUMER: Received [VX_MSGTYPE_REF_BUF], creating tiovx reference%s", "\n");

        status = import_ref_from_producer(consumer, msg, fd, num_fd);
        if (status != VX_GW_STATUS_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "CONSUMER: import_ref_from_producer() failed.%s", "\n");
            break;
        }
        if (msg->last_reference == 1)
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: all buffers have been transmitted, start the buffer ID thread, create graph%s", "\n");

            // create the buffer ID client thread; from there switch to reading buffer ID messages and enqueueing
            // into the consumer graph input
            status = pthread_create(&consumer->backchannel_thread, NULL, buffer_id_thread, (void*)consumer);
            if (status != VX_GW_STATUS_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: failed to create buffer ID client thread%s", "\n");
                break;
            }

            // Create the consumer graph by jumping into application defined callback
            status = consumer->subscriber_cb.createCallback(consumer->graph_obj, consumer->refs, consumer->num_refs);
            if (status != VX_GW_STATUS_SUCCESS)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: application create graph failed%s", "\n");
                break;
            }
            else
            {
                VX_PRINT(VX_ZONE_INFO, "CONSUMER: application create graph success%s", "\n");  
                status = VX_MSGTYPE_CONSUMER_CREATE_DONE;
            }
            consumer->state = VX_CONS_STATE_RUN;
            consumer->ref_import_done = (vx_bool)vx_true_e;
        }
    }
    break;

    case VX_MSGTYPE_BUFID_CMD:
    {
        status                  = VX_GW_STATUS_SUCCESS;
        vx_gw_buff_id_msg* msg = (vx_gw_buff_id_msg*)message_buffer;

        VX_PRINT(
            VX_ZONE_INFO,
            "CONSUMER: Received [VX_MSGTYPE_BUFID_CMD], buffer ID (%d), last buffer %d\n",
            msg->buffer_id,
            msg->last_buffer);

        // check if metadata from IPC was received and apply it to consumer reference
        if (msg->metadata_valid == 1 && NULL != consumer->subscriber_cb.storeMetadataCallback)
        {
            status = consumer->subscriber_cb.storeMetadataCallback(
                consumer->graph_obj, consumer->refs[msg->buffer_id], &msg[1], msg->metadata_size);
        }

        VX_PRINT(
            VX_ZONE_INFO,
            "CONSUMER: enqueue the incoming buffer id: %d with ref: %p into the pipeline as input buffer, last "
            "buffer %d, last frame dropped: %d \n",
            msg->buffer_id,
            consumer->refs[msg->buffer_id],
            msg->last_buffer,
            msg->last_frame_dropped);

        status = consumer->subscriber_cb.enqueueCallback(consumer->graph_obj, (vx_reference)consumer->refs[msg->buffer_id]);

        if (msg->last_buffer == 1)
        {
            consumer->last_buffer    = 1;
            consumer->last_buffer_id = msg->buffer_id;
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: Received last buffer id %d \n", consumer->last_buffer_id);
        }
    }
    break;

    default:
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Received [UNKNOWN MESSAGE]: %d\n", message_header->msg_type);
        status = -1;
        break;
    }

    if (status < 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: MSG RECEIVE STATUS: FAILED.%s", "\n");
    }
    else if (status == VX_MSGTYPE_CONSUMER_CREATE_DONE)
    {
        // notify producer that create graph is now done and consumer is ready 
        vx_gw_buff_id_msg* msg       = (vx_gw_buff_id_msg*)message_buffer;

        VX_PRINT(VX_ZONE_INFO, "CONSUMER: Graph newly created, notify producer %s.", "\n");
        msg->last_buffer = 0U;
        status = send_buffer_release_message(consumer, message_buffer, msg->buffer_id, VX_MSGTYPE_CONSUMER_CREATE_DONE);
        if (status < 0)
        {
            consumer->state = VX_CONS_STATE_FAILED;
        }
    }
    else if (status == VX_GW_STATUS_CONSUMER_REF_DROP)
    {
        // buffer was not enqueued, transmit the buffer back to the producer immediately
        consumer->last_buffer_dropped = 1;
        vx_gw_buff_id_msg* msg       = (vx_gw_buff_id_msg*)message_buffer;
        msg->last_buffer = 0U;
        VX_PRINT(VX_ZONE_INFO, "CONSUMER: CONSUMER DROPS FRAME, BUFFER ID %d, %s.", msg->buffer_id, "\n");
        status = send_buffer_release_message(consumer, message_buffer, msg->buffer_id, VX_MSGTYPE_BUF_RELEASE);
        if (status < 0)
        {
            consumer->state = VX_CONS_STATE_FAILED;
        }
    }
    else
    {
        consumer->last_buffer_dropped = 0;
        status                        = 0;
    }

    return status;
}

static void* buffer_id_thread(void* arg)
{
    int32_t         status   = 0;
    vx_consumer consumer = (vx_consumer)arg;
    uint8_t         message_buffer[SOCKET_MAX_MSG_SIZE];

    VX_PRINT(VX_ZONE_INFO, "CONSUMER: buffer id channel thread started%s", "\n");

    if (NULL == consumer)
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Bad argument, shutting down thread%s", "\n");
        return NULL;
    }

    while (consumer->state != VX_CONS_STATE_FLUSH)
    {
        // switch to reading socket messages with buffer IDs
        status = handle_receive_message(consumer, consumer->socket_fd, message_buffer);
        if (status < SOCKET_STATUS_OK)
        {
            VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Error while receiving message from producer %s", "\n");
            // state changes here are used to signal the master thread
            consumer->state = VX_CONS_STATE_FAILED;
            break;
        }
        else if (status == SOCKET_STATUS_PEER_CLOSED)
        {
            if (consumer->state < VX_CONS_STATE_WAIT)
            {
                // connection closed during streaming, this is an error
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Socket closed unexpectedly %s", "\n");
                consumer->state = VX_CONS_STATE_FAILED;
            }
            break;
        }
    }

    VX_PRINT(VX_ZONE_INFO, "CONSUMER: buffer ID thread shutdown%s", "\n");
    return NULL;
}

static void* consumer_receiver_thread(void* arg)
{
    uint8_t           message_buffer[SOCKET_MAX_MSG_SIZE];
    vx_consumer   consumer              = (vx_consumer)arg;
    vx_gw_hello_msg* msg                   = (vx_gw_hello_msg*)message_buffer;
    uint32_t          done                  = 0;
    int32_t           status                = 0;
    int32_t           first_buffer_released = 0;

    while (!done)
    {
        switch (consumer->state)
        {
        case VX_CONS_STATE_DISCONNECTED:
        {
            vx_gw_status_t gw_status = VX_GW_STATUS_SUCCESS;
            // Wait for a socket connection with the producer
            gw_status = start_sync_with_producer(consumer, consumer->access_point_name, msg);
            if (VX_GW_STATUS_FAILURE == gw_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: cannot start sync with producer on master channel%s", "\n");
                consumer->state = VX_CONS_STATE_FAILED;
            }
            else if (VX_GW_STATUS_SUCCESS == gw_status)
            {
                VX_PRINT(VX_ZONE_PERF, " [UPT] First Time Connected to producer!%s", "\n");
                VX_PRINT(VX_ZONE_INFO, "CONSUMER: connected to producer with socket %d\n", consumer->socket_fd);
                consumer->state = VX_CONS_STATE_INIT;
            }
            else if (VX_GW_STATUS_CONSUMER_FLUSHED == gw_status)
            {
                consumer->state = VX_CONS_STATE_FLUSH;
            }
            else
            {
                // do nothing
            }
        }
        break;

        case VX_CONS_STATE_INIT:
        {
            // receive buffer ref messages in the first (master) thread
            status = handle_receive_message(consumer, consumer->socket_fd, message_buffer);
            if (status < SOCKET_STATUS_OK || status == SOCKET_STATUS_PEER_CLOSED)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Failed while handling message receive in master thread %s", "\n");
                consumer->state = VX_CONS_STATE_FAILED;
                break;
            }
        }
        break;

        case VX_CONS_STATE_RUN:
        {
            int32_t      buffer_id                         = 0;
            vx_reference dequeued_refs[VX_GW_MAX_NUM_REFS] = {0};
            vx_uint32    current_ref_num                   = 0;
            vx_uint32    num_dequeued_refs                 = 0;

            // graph is running, check if input buffer can be dequeued
            status = consumer->subscriber_cb.dequeueCallback(consumer->graph_obj, dequeued_refs, &num_dequeued_refs);
            if (status != VX_SUCCESS)
            {
                // if the consumer cannot dequeue, it will be automatically disconnected using the socket timeout
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Error while dequeuing buffer %s", "\n");
                break;
            }
            // dequeue timeout makes sure that we always return from the otherwise blocking dequeue callback.
            for (current_ref_num = 0; current_ref_num < num_dequeued_refs; current_ref_num++)
            {
                buffer_id = get_buffer_id(dequeued_refs[current_ref_num], consumer->refs, consumer->num_refs);
                VX_PRINT(
                    VX_ZONE_INFO,
                    " CONSUMER: dequeue successfull, send the buffer id: %d back via master socket: %d\n",
                    buffer_id,
                    consumer->socket_fd);
                if (buffer_id >= 0)
                {
                    // send back the buffer to producer
                    VX_PRINT(
                        VX_ZONE_INFO,
                        "CONSUMER: current buffer ID %d buffer ID %d dequeued, last buffer flag %d \n",
                        buffer_id,
                        consumer->last_buffer_id,
                        consumer->last_buffer);
                    status = send_buffer_release_message(consumer, &message_buffer, buffer_id, VX_MSGTYPE_BUF_RELEASE);
                    if (status < 0)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Error while sending back buffer %s", "\n");
                        consumer->state = VX_CONS_STATE_FAILED;
                        break;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "CONSUMER: wrong buffer ID %d \n", buffer_id);
                }
                if (first_buffer_released == 0)
                {
                    // due to long delays in setting up the producer/consumer communication, there are different timeout
                    // values for init/streaming phases
                    VX_PRINT(VX_ZONE_INFO, "CONSUMER: reconfiguring the socket timeouts for streaming values %s", "\n");
                    socket_reconfigure_timeout(consumer->socket_fd, SOCKET_TIMEOUT_USECS_STREAMING);
                    first_buffer_released = 1;
                }
            }
        }
        break;

        case VX_CONS_STATE_WAIT:
        {
            tivxTaskWaitMsecs(50000);
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: wait finished%s", "\n");
            consumer->last_buffer_transmitted = 1;
            consumer->state                   = VX_CONS_STATE_FLUSH;
        }
        break;

        case VX_CONS_STATE_FLUSH:
        {
            VX_PRINT(VX_ZONE_INFO, "CONSUMER: pipeline is flushed, reached normal shutdown%s", "\n");
            done = 1;
        }
        break;

        case VX_CONS_STATE_FAILED:
        {
            // one of the consumer socket connections has failed or there has been an unknown error
            VX_PRINT(
                VX_ZONE_INFO,
                "CONSUMER: reached failure state (%u time(s)), trying to clean up connections %s",
                consumer->num_failures + 1,
                "\n");
            if (consumer->socket_fd > 0)
            {
                close(consumer->socket_fd);
                consumer->socket_fd = 0;
            }

            // consumer failed too many times OR a recovery cannot be done
            if (consumer->num_failures >= CONSUMER_MAX_CONSECUTIVE_FAILURES || consumer->subscriber_cb.recoveryCallback == NULL ||
                consumer->last_buffer == 1U)
            {
                VX_PRINT(VX_ZONE_ERROR, "CONSUMER: Disconnected too many times, shutting down %s", "\n");
                consumer->state = VX_CONS_STATE_WAIT;
                break;
            }

            consumer->num_failures++;

            if (consumer->subscriber_cb.recoveryCallback != NULL)
            {
                consumer->num_refs        = 0;
                consumer->ipcMessageCount = 0;

                // reset state machine
                consumer->state = VX_CONS_STATE_DISCONNECTED;

                VX_PRINT(VX_ZONE_INFO, "CONSUMER: calling recovery callback %s", "\n");
                consumer->subscriber_cb.recoveryCallback(consumer->graph_obj);
                // consumer deinit can be called from recovery callback, which will cause this thread to shut down
            }
        }
        break;

        default:
            break;
        }
    }

    VX_PRINT(VX_ZONE_INFO, "CONSUMER: master_thread shutdown%s", "\n");
    return NULL;
}
#endif

static vx_status ownInitConsumerObject(vx_consumer consumer, const vx_consumer_params_t* params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    (void)snprintf(consumer->name, VX_MAX_CONSUMER_NAME, params->name);
    (void)snprintf(consumer->access_point_name , VX_MAX_ACCESS_POINT_NAME, params->access_point_name);
    consumer->last_buffer               = 0;
    consumer->last_buffer_dropped       = 0;
    consumer->last_buffer_transmitted   = 0;
    consumer->init_done                 = vx_false_e;
    consumer->ref_import_done           = vx_false_e;
    consumer->num_failures              = 0;
    consumer->graph_obj                 = params->graph_obj;
    consumer->subscriber_cb             = params->subscriber_cb;
    consumer->consumer_id               = params->consumer_id;
    consumer->connect_polling_time      = params->connect_polling_time;
    consumer->state                     = VX_CONS_STATE_DISCONNECTED;    

    pthread_mutexattr_t buffInfoMutexAttr;
    pthread_mutexattr_init(&buffInfoMutexAttr);

    status = pthread_mutex_init(&consumer->buffer_mutex, &buffInfoMutexAttr);
    if (status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "CONSUMER: pthread_mutex_init() failed for buffer info mutex\n");
        return (vx_status)VX_FAILURE;
    }

#ifdef IPPC_SHEM_ENABLED
    for(vx_uint32 idx = 0U; idx < IPPC_PORT_COUNT; idx++)
    {
        consumer->ippc_port[idx] = params->ippc_port[idx];
    }
#endif

    return status;
}

static vx_status ownDestructConsumer(vx_reference ref)
{
    return ((vx_status)VX_SUCCESS);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConsumer(vx_consumer* consumer)
{
    vx_consumer this_consumer = consumer[0];
    this_consumer->state = VX_CONS_STATE_FLUSH;
    pthread_join(this_consumer->receiver_thread, NULL);
    pthread_join(this_consumer->backchannel_thread, NULL);
    return (ownReleaseReferenceInt(
        vxCastRefFromConsumerP(consumer), VX_TYPE_CONSUMER, (vx_enum)VX_EXTERNAL, NULL));
}

static vx_status ownAllocConsumerBuffer(vx_reference ref)
{
    return ((vx_status)VX_SUCCESS);
}

VX_API_ENTRY vx_consumer VX_API_CALL vxCreateConsumer(vx_context context, const vx_consumer_params_t* params)
{
    vx_consumer consumer = NULL;
    vx_reference ref = NULL;
    vx_status status = (vx_status)VX_SUCCESS;
    /* create a consumer object */
    ref = ownCreateReference(context, (vx_enum)VX_TYPE_CONSUMER, (vx_enum)VX_EXTERNAL, &context->base);
    if ((vxGetStatus(ref) == (vx_status)VX_SUCCESS) &&
        (ref->type == (vx_enum)VX_TYPE_CONSUMER))
    {
        /* status set to NULL due to preceding type check */
        consumer = vxCastRefAsConsumer(ref,NULL); 
        consumer->base.destructor_callback = &ownDestructConsumer; /* specific destructor because of no tiovx_obj*/
        consumer->base.mem_alloc_callback  = &ownAllocConsumerBuffer;
        consumer->base.release_callback    = &ownReleaseReferenceBufferGeneric;
        consumer->context = context;

        status = ownInitConsumerObject(consumer, params);
        if(status!=(vx_status)VX_SUCCESS)
        {
            status = vxReleaseConsumer(&consumer);
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to release reference to a consumer \n");
            }

            VX_PRINT(VX_ZONE_ERROR, "Could not create consumer\n");
            ref = ownGetErrorObject(context, (vx_status)VX_ERROR_NO_RESOURCES);
            /* status set to NULL due to preceding type check */
            consumer = vxCastRefAsConsumer(ref, NULL);
        }
    }

    /* return the consumer object */
    return(consumer);
}

VX_API_ENTRY vx_status VX_API_CALL vxConsumerStart(vx_consumer consumer)
{
    /* start the ippc broadcasting thread */
    int thread_status = pthread_create(&consumer->receiver_thread, NULL, consumer_receiver_thread, (void*)consumer);
    return ((vx_status)thread_status);
}

VX_API_ENTRY vx_uint32 VX_API_CALL vxConsumerShutdownStatus(vx_consumer consumer)
{
    return consumer->last_buffer_transmitted;
}
