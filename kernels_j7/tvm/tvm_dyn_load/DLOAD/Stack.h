/*
* Stack.h
*
* Interface to Stack
* ------------------
*
* This is an implementation of a type-independent stack implemented as
* a signly linked list class for C.  It's basically a template class, but
* uses macros instead, so that it can be compiled with a C-only compiler.
*
* To define a Stack class:
* #include "Stack.h"
* TYPE_STACK_DEFINITION(object_type,Class_Identifier)
*
* In a separate C file:
* #include "Stack.h"
* TYPE_STACK_DEFINITION(object_type,Class_Identifier)
* TYPE_STACK_IMPLEMENTATION(object_type,Class_Identifier)
*
* Now, to create a stack:
* struct Class_Identifier_Stack name;
* Get it initialized to zero everywhere somehow, maybe like this:
* initialize_stack_Class_Identifier(&name);
*
* To add to the stack:
* push_Class_Identifier(&name, object);
*
* To access the top of the stack:
* Class_Identifier_Stack_Node *tos = name.top_ptr;
* do_something_to_(tos->value);
*
* To delete from the stack:
*   if (name.size > 0) pop_Class_Identifier(&name);
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

#ifndef STACK_H
#define STACK_H

#include <inttypes.h>
#include "dload_api.h"

/*****************************************************************************/
/* TYPE_STACK_DEFINITION() - Define structure specifications for a last-in,  */
/*	first-out linked list of t_name objects.                             */
/*****************************************************************************/
#define TYPE_STACK_DEFINITION(t, t_name)                                      \
struct t_name##_Stack_Node_                                                   \
{                                                                             \
     t value;                                                                 \
     struct t_name##_Stack_Node_* next_ptr;                                   \
};                                                                            \
typedef struct t_name##_Stack_Node_ t_name##_Stack_Node;                      \
                                                                              \
typedef struct                                                                \
{                                                                             \
     t_name##_Stack_Node* top_ptr;                                            \
     t_name##_Stack_Node* bottom_ptr;                                         \
     int size;                                                                \
} t_name##_Stack;                                                             \
                                                                              \
extern void t_name##_initialize_stack(t_name##_Stack* stack);                 \
extern void t_name##_push(t_name##_Stack* stack, t to_push);                  \
extern t    t_name##_pop(t_name##_Stack* stack);

/*****************************************************************************/
/* TYPE_STACK_DEFINITION() - Define the initializer to initalize Stacks.     */
/*****************************************************************************/
#define TYPE_STACK_INITIALIZER  {NULL, NULL, 0 }

/*****************************************************************************/
/* TYPE_STACK_IMPLEMENTATION() - Define member functions of new LIFO linked  */
/*	list "class" of t_name objects.                                      */
/*                                                                           */
/* <type>_initialize_stack() - clears the stack                              */
/* <type>_push() - pushes a <t> type object to the top of the stack          */
/* <type>_pop() - pop a <t> type object from the top of the stack            */
/*	and provide access to it to the caller                               */
/*****************************************************************************/
#define TYPE_STACK_IMPLEMENTATION(t, t_name)                                  \
void t_name##_initialize_stack (t_name##_Stack* stack)                        \
{                                                                             \
     stack->top_ptr = stack->bottom_ptr = NULL;                               \
     stack->size = 0;                                                         \
}                                                                             \
void t_name##_push(t_name##_Stack* stack, t to_push)                          \
{                                                                             \
     stack->size++;                                                           \
                                                                              \
     if(!stack->top_ptr)                                                      \
     {                                                                        \
	  stack->bottom_ptr = stack->top_ptr =                                \
	    (t_name##_Stack_Node*)(DLIF_malloc(sizeof(t_name##_Stack_Node))); \
          stack->top_ptr->next_ptr = NULL;                                    \
     }                                                                        \
     else                                                                     \
     {                                                                        \
          t_name##_Stack_Node* next_ptr = stack->top_ptr;                     \
	  stack->top_ptr =                                                    \
	    (t_name##_Stack_Node*)(DLIF_malloc(sizeof(t_name##_Stack_Node))); \
	  stack->top_ptr->next_ptr = next_ptr;                                \
     }                                                                        \
                                                                              \
     stack->top_ptr->value = to_push;                                         \
}                                                                             \
                                                                              \
t t_name##_pop(t_name##_Stack* stack)                                         \
{                                                                             \
     t to_ret;                                                                \
     t_name##_Stack_Node* next_ptr = stack->top_ptr->next_ptr;                \
                                                                              \
     stack->size--;                                                           \
     to_ret = stack->top_ptr->value;                                          \
     DLIF_free((void*)(stack->top_ptr));                                      \
                                                                              \
     if(!stack->size)                                                         \
	  stack->top_ptr = stack->bottom_ptr = NULL;                          \
     else                                                                     \
	  stack->top_ptr = next_ptr;                                          \
                                                                              \
     return to_ret;                                                           \
}

#endif
