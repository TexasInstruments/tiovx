/*
* Queue.h
*
* Interface to Linked List
* ------------------------
*
* This is an implementation of a type-independent linked list class for C.
* It's basically a template class, but uses macros instead so that it can
* be compiled with a C-only compiler.
*
* To define a linked list class:
* #include "Queue.h"
* TYPE_QUEUE_DEFINITION(object_type,Class_Identifier)
*
* In a separate C file:
* #include "Queue.h"
* TYPE_QUEUE_DEFINITION(object_type,Class_Identifier)
* TYPE_QUEUE_IMPLEMENTATION(object_type,Class_Identifier)
*
* Now, to create a list:
* Class_Identifier_Queue name;
* Get it initialized to zero everywhere somehow, maybe like this:
* Class_Identifier_initialize_queue(&name);
*
* To add to the list:
* Class_Identifier_enqueue(&name, object);
*
* To iterate over the list:
* Class_Identifier_Queue_Node *it = name.front;
* while(it) { do_something_to_(it->value); it = it->next; }
*
* To delete from the list:
*  If it's the first node:
*   Class_Identifier_dequeue(&name);
*  If it's not:
*   predecessor_node->next_ptr = deleted_node->next_ptr;
*   name.size--;
*
* Copyright (C) 2009-2014 Texas Instruments Incorporated - http://www.ti.com/
*
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the
* distribution.
*
* Neither the name of Texas Instruments Incorporated nor the names of
* its contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef QUEUE_H
#define QUEUE_H

#include <inttypes.h>
#include "dload_api.h"

/*****************************************************************************/
/* TYPE_QUEUE_DEFINITION() - Define structure specifications for a linked    */
/*       list of t_name objects.                                              */
/*****************************************************************************/
#define TYPE_QUEUE_DEFINITION(t, t_name)                                  \
struct t_name##_Queue_Node_                                               \
{                                                                         \
     t value;                                                             \
     struct t_name##_Queue_Node_* next_ptr;                               \
};                                                                        \
typedef struct t_name##_Queue_Node_ t_name##_Queue_Node;                  \
                                                                          \
typedef struct                                                            \
{                                                                         \
     t_name##_Queue_Node* front_ptr;                                      \
     t_name##_Queue_Node* back_ptr;                                       \
     int32_t size;                                                        \
} t_name##_Queue;                                                         \
                                                                          \
extern void t_name##_initialize_queue(t_name##_Queue* queue);             \
extern void t_name##_enqueue(t_name##_Queue* queue, t to_enqueue);        \
extern t    t_name##_dequeue(t_name##_Queue* queue);                      \
extern void t_name##_remove(t_name##_Queue* queue, t to_remove);

/*****************************************************************************/
/* TYPE_QUEUE_INITIALIZER() - Define the initializer to initialize Queues.   */
/*****************************************************************************/
#define TYPE_QUEUE_INITIALIZER   {NULL, NULL, 0}


/*****************************************************************************/
/* TYPE_QUEUE_IMPLEMENTATION() - Define member functions of new linked list  */
/*       "class" of t_name objects.                                          */
/*                                                                           */
/* <type>_initialize_queue() - clears the queue                              */
/* <type>_enqueue() - adds a <t> type object to the end of the queue         */
/* <type>_dequeue() - remove a <t> type object from the front of the queue   */
/*       and provide access to it to the caller                              */
/* <type>_remove() - find and remove a <t> type object from the queue        */
/*****************************************************************************/
#define TYPE_QUEUE_IMPLEMENTATION(t, t_name)                             \
void t_name##_initialize_queue (t_name##_Queue* queue)                   \
{                                                                        \
     queue->front_ptr = queue->back_ptr = NULL;                          \
     queue->size = 0;                                                    \
}                                                                        \
void t_name##_enqueue(t_name##_Queue* queue, t to_enqueue)               \
{                                                                        \
     queue->size++;                                                      \
                                                                         \
     if(!queue->back_ptr)                                                \
         queue->back_ptr = queue->front_ptr =                            \
              (t_name##_Queue_Node*)                                     \
                (DLIF_malloc(sizeof(t_name##_Queue_Node)));              \
     else                                                                \
     {                                                                   \
         queue->back_ptr->next_ptr =                                     \
              (t_name##_Queue_Node*)(DLIF_malloc(                        \
                                 sizeof(t_name##_Queue_Node)));          \
         queue->back_ptr = queue->back_ptr->next_ptr;                    \
     }                                                                   \
                                                                         \
     queue->back_ptr->value = to_enqueue;                                \
     queue->back_ptr->next_ptr = NULL;                                   \
}                                                                        \
                                                                         \
t t_name##_dequeue(t_name##_Queue* queue)                                \
{                                                                        \
     t to_ret;                                                           \
     t_name##_Queue_Node* next_ptr = NULL;                               \
                                                                         \
     if (!queue->size) return (t) NULL;                                  \
                                                                         \
     next_ptr = queue->front_ptr->next_ptr;                              \
     queue->size--;                                                      \
     to_ret = queue->front_ptr->value;                                   \
     DLIF_free((void*)(queue->front_ptr));                               \
                                                                         \
     if(!queue->size)                                                    \
         queue->front_ptr = queue->back_ptr = NULL;                      \
     else                                                                \
         queue->front_ptr = next_ptr;                                    \
                                                                         \
     return to_ret;                                                      \
}                                                                        \
                                                                         \
void t_name##_remove(t_name##_Queue* queue, t to_remove)                 \
{                                                                        \
     t_name##_Queue_Node* prev_ptr = NULL;                               \
     t_name##_Queue_Node* curr_ptr = queue->front_ptr;                   \
     t_name##_Queue_Node* next_ptr = NULL;                               \
                                                                         \
     for (; curr_ptr; curr_ptr = next_ptr)                               \
     {                                                                   \
	next_ptr = curr_ptr->next_ptr;                                   \
        if (curr_ptr->value == to_remove) break;                         \
	prev_ptr = curr_ptr;                                             \
     }                                                                   \
                                                                         \
     if (curr_ptr)                                                       \
     {                                                                   \
        if (prev_ptr) prev_ptr->next_ptr = next_ptr;                     \
	queue->size--;                                                   \
        DLIF_free((void*)(curr_ptr));                                    \
     }                                                                   \
                                                                         \
     if (!queue->size)                                                   \
        queue->front_ptr = queue->back_ptr = NULL;                       \
     else                                                                \
     {                                                                   \
        if (!prev_ptr) queue->front_ptr = next_ptr;                      \
        if (!next_ptr) queue->back_ptr = prev_ptr;                       \
     }                                                                   \
}


#endif
