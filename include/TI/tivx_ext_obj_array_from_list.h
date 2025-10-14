/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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


#ifndef TIVX_OBJECT_ARRAY_FROM_LIST_H_
#define TIVX_OBJECT_ARRAY_FROM_LIST_H_

#define OBJECT_ARRAY_FROM_LIST   "object_array_from_list"
 
#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of an additional create function for the Object Array data object that takes a list of references instead of an exemplar.
 * This allows the object array to hold references with different metadata.
 * References still must be of the same type to maintain strong typeness. 
 */

/*!
 * \defgroup group_obj_array_from_list Object Array From List APIs
 * \brief APIs for creating an Object Array from List
 * \ingroup group_tivx_ext_host
 */

enum tivx_object_array_attribute_e
{
    /*! \brief A flag indicates whether or not the object array is created from a list. Read-only. Use a <tt>\ref vx_bool</tt> parameter.
     * \ingroup group_obj_array_from_list */
    TIVX_OBJECT_ARRAY_IS_FROM_LIST = VX_ATTRIBUTE_BASE(VX_ID_TI, VX_TYPE_OBJECT_ARRAY) + 0x0,
};


/*!
 * \brief Creates a reference to an ObjectArray of count objects which are uniform in type but can have non-uniform metadata. 
 *
 * This methods allows an object array to be instantiated from an array of references passed through list[] parameter.
 * It does not alter the references in the list nor keep or release this list.
 * Internally, it modifies the additional attribute added with the extension, <tt>\ref TIVX_OBJECT_ARRAY_IS_FROM_LIST</tt>.
 * 
 * \param [in] context              The reference to the overall Context.
 * \param [in] list                 An array of references that are to be stored in the object array. The type of the first
 *                                  reference in the list determines the type that will be enforced by the object array.
 * \param [in] count                Number of references from the list parameter designated for the object array.
 *                                  Acceptable numbers are greater than 0 and less than TIVX_OBJECT_ARRAY_MAX_ITEMS.
 *
 * \returns An ObjectArray reference <tt>\ref vx_object_array</tt>. Any possible errors preventing a 
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
 * 
 * \ingroup group_obj_array_from_list
 */
VX_API_ENTRY vx_object_array VX_API_CALL tivxCreateObjectArrayFromList(
    vx_context context, vx_reference list[], vx_size count
);

/*!
 * \brief Creates an opaque reference to a virtual ObjectArray instantiated by a list with no direct user access
 *
 * This function creates an ObjectArray of count objects with similar behavior as 
 * <tt>\ref tivxCreateObjectArrayFromList</tt>. The main difference is that the provided objects
 * in the list must also be virtual in the given graph.
 *
 * \param [in] graph      Reference to the graph where to create the virtual ObjectArray.  
 * \param [in] list       An array of references that are to be stored in the object array. The type of the first
 *                        reference in the list determines the type that will be enforced by the object array.
 *                        The references in the list must be virtual in the graph.
 * \param [in] count      Number of references from the list parameter designated for the object array.
 *                        Acceptable numbers are greater than 0 and less than TIVX_OBJECT_ARRAY_MAX_ITEMS.
 * 
 * \returns An ObjectArray reference <tt>\ref vx_object_array</tt>. Any possible errors preventing a 
 * successful creation should be checked using <tt>\ref vxGetStatus</tt>.
 
 * \ingroup group_obj_array_from_list
 */

VX_API_ENTRY vx_object_array VX_API_CALL tivxCreateVirtualObjectArrayFromList(
    vx_graph graph, vx_reference list[], vx_size count
);

#ifdef __cplusplus
}
#endif

#endif
