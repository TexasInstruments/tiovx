/*
* ArrayList.h
*
* This implementation of ArrayList is a replacement for the C++
* vector class in C.
*
* This class emulates a resizable array along the lines of a C++ 
* vector or Java ArrayList class in C, and uses the convention
* of passing a pointer to the current "object" as the first
* argument.
*
* Usage is defined as follows:
*
* Array_List obj;
* AL_initialize(&obj, sizeof(type_name));
*
* ...
*
* type_name *ptr = (type_name*)(obj.buf);
* for(i = 0; i < AL_size(&obj); i++)
*     do_something_to(ptr[i]);
* type_name to_append = ...;
* AL_append(&obj, &to_append);
*
* ...
*
* AL_destroy(&obj);
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

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <inttypes.h>

/**********************************************************************/
/* Array_List - structure type specification.                         */
/**********************************************************************/
typedef struct 
{
   void   *buf;
   int32_t type_size;
   int32_t size;
   int32_t buffer_size;
} Array_List;

/*--------------------------------------------------------------------*/
/* Array_List Member Functions:                                       */
/*                                                                    */
/* AL_initialize() - Initialize a newly created Array_List object.    */
/* AL_append() - Append an element to the end of an Array_List.       */
/* AL_size() - Get number of elements in an Array_List.               */
/* AL_destroy() - Free memory associated with an Array_List that is   */
/*                no longer in use.                                   */
/*--------------------------------------------------------------------*/
void     AL_initialize(Array_List* obj, int32_t type_size, int32_t num_elem);
void     AL_append(Array_List* obj, void* to_append);
int32_t  AL_size(Array_List* obj);
void     AL_destroy(Array_List* obj);

#endif
