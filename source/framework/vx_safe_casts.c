/*
 * Copyright (c) 2023 The Khronos Group Inc.
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

/*
Example Implementation of "safe casts"

This example implementation is compatible with the Khronos sample implementation.

The purpose of these functions is to provide conversion between the various OpenVX
objects and their common class vx_reference. The assertion is that all OpenVX objects
may be down-cast to a vx_reference since the first element in the structure of all
OpenVX Objects is a _vx_reference object. Therefore, the (vx_reference) casts in this
module are valid, in other words, a pointer to an object of for example type
struct _vx_image is in any case always a pointer to an object of type struct _vx_reference.

The opposite is not always true. However, it may be true, and here we make some checks,
so that if the developer calls vxCastRefAsImage, the vx_reference passed is checked to make
sure that its type is a VX_TYPE_IMAGE and consequently the vx_reference most probably
is also a vx_image, i.e., pointing to a _vx_image structure.

In the case that this is not the case, or if the vx_reference was found not to be a pointer
to a valid _vx_reference structure, we set a status variable with an error code and return
an error object. It is up to the developer to check the status variable frequently to
ensure that errors have not been made. If there was an attempt to cast the type 
incorrectly, a log entry is made.

Thus we argue that all 126 of the pointer casts in this file are "safe"
*/

static vx_reference getRefAs(vx_reference ref, vx_enum type, vx_status *status);
static vx_reference castRefAs(vx_reference ref, vx_enum type, vx_status *status);

#define DEFINE_SAFE_DOWNCASTS(typename, Name, TYPE)\
/*! \brief safely get a new vx_reference for the given vx_##typename variable*/\
VX_API_ENTRY vx_reference vxGetRefFrom##Name(const vx_##typename *typename)\
{\
    /* casting to the "base class" is inherently safe */\
    vx_reference lref = ((vx_reference)*typename);\
    (void)ownIncrementReference(lref, (vx_enum)VX_EXTERNAL);\
    return lref;\
}\
\
/*! \brief safe cast a vx##typename to a generic vx_reference*/\
VX_API_ENTRY vx_reference vxCastRefFrom##Name(vx_##typename typename)\
{\
    /* casting to the "base class" is inherently safe */\
    return (vx_reference)typename;\
}\
\
/*! \brief safe cast a pointer to vx##typename to a pointer to vx_reference */\
VX_API_ENTRY vx_reference *vxCastRefFrom##Name##P(vx_##typename *p_##typename)\
{\
    /* casting to the "base class" is inherently safe */\
    return (vx_reference *)p_##typename;\
}\
\
/*! \brief safe cast a const pointer to vx##typename to a const pointer to vx_reference */\
VX_API_ENTRY const vx_reference *vxCastRefFrom##Name##ConstP(const vx_##typename *p_##typename)\
{\
    /* casting to the "base class" is inherently safe */\
    return (const vx_reference *)p_##typename;\
}

#define DEFINE_SAFE_CASTS(typename, Name, TYPE) \
/*! \brief safely get a new vx_##typename or an error object from a vx_reference*/\
VX_API_ENTRY vx_##typename vxGetRefAs##Name(const vx_reference *ref, vx_status *status)\
{\
    /* Dynamic cast from base class upwards. \
       Not inherently safe, we need to check RTTI\
       and report an error if incorrect.\
    */\
    return (vx_##typename)getRefAs(*ref, (vx_enum)VX_TYPE_##TYPE, status);\
}\
\
/*! \brief safely upcast a vx_reference to a vx_##typename or an error object */\
VX_API_ENTRY vx_##typename vxCastRefAs##Name(vx_reference ref, vx_status *status)\
{\
    /* Dynamic cast from base class upwards. \
       Not inherently safe, we need to check RTTI\
       and report an error if incorrect.\
    */\
    return (vx_##typename)castRefAs(ref, (vx_enum)VX_TYPE_##TYPE, status);\
}\
\
DEFINE_SAFE_DOWNCASTS(typename, Name, TYPE)

DEFINE_SAFE_CASTS(array, Array, ARRAY)
DEFINE_SAFE_CASTS(convolution, Convolution, CONVOLUTION)
DEFINE_SAFE_DOWNCASTS(context, Context, CONTEXT)
DEFINE_SAFE_CASTS(delay, Delay, DELAY)
DEFINE_SAFE_CASTS(distribution, Distribution, DISTRIBUTION)
DEFINE_SAFE_CASTS(graph, Graph, GRAPH)
DEFINE_SAFE_CASTS(image, Image, IMAGE)
#ifdef VX_TYPE_IMPORT
DEFINE_SAFE_DOWNCASTS(import, Import, IMPORT)
#endif
DEFINE_SAFE_CASTS(kernel, Kernel, KERNEL)
DEFINE_SAFE_CASTS(lut, LUT, LUT)
DEFINE_SAFE_CASTS(matrix, Matrix, MATRIX)
DEFINE_SAFE_DOWNCASTS(meta_format, MetaFormat, META_FORMAT)
DEFINE_SAFE_CASTS(node, Node, NODE)
DEFINE_SAFE_CASTS(object_array, ObjectArray, OBJECT_ARRAY)
DEFINE_SAFE_CASTS(parameter, Parameter, PARAMETER)
DEFINE_SAFE_CASTS(pyramid, Pyramid, PYRAMID)
DEFINE_SAFE_CASTS(remap, Remap, REMAP)
DEFINE_SAFE_CASTS(scalar, Scalar, SCALAR)
DEFINE_SAFE_CASTS(tensor, Tensor, TENSOR)
DEFINE_SAFE_CASTS(threshold, Threshold, THRESHOLD)
#ifdef VX_TYPE_USER_DATA_OBJECT
DEFINE_SAFE_CASTS(user_data_object, UserDataObject, USER_DATA_OBJECT)
#endif

static vx_reference getRefAs(vx_reference ref, vx_enum type, vx_status *status)
{
    vx_status local_status = (vx_status)VX_SUCCESS;
    vx_reference retref = ref;
    if ((NULL != ref) &&
       (ref->magic == TIVX_MAGIC) && /* TIOVX-1928- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_SAFE_CASTS_UBR001 */
       (ownIsValidContext(ref->context) == (vx_bool)vx_true_e))
    {
        if (ref->type == type)
        {
            (void)ownIncrementReference(ref, (vx_enum)VX_EXTERNAL);
        }
        else
        {
            local_status = (vx_status)VX_ERROR_INVALID_TYPE;
            VX_PRINT(VX_ZONE_ERROR, "vxGetRefAs: Attempt to get reference %p as type %d when actual type is %d\n", ref, type, ref->type);
            vxAddLogEntry(ref, local_status, "vxGetRefAs: Attempt to get reference %p as type %d when actual type is %d\n", ref, type, ref->type);
            retref = ownGetErrorObject(ref->context, local_status);
        }
    }
    else
    {
        local_status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    if (status && /* TIOVX-1928- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_SAFE_CASTS_UBR002 */
        ((vx_status)VX_SUCCESS == *status)) /* TIOVX-1928- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_SAFE_CASTS_UBR003 */
    {
        *status = local_status;
    }
    return retref;
}

static vx_reference castRefAs(vx_reference ref, vx_enum type, vx_status *status)
{
    vx_status local_status = (vx_status)VX_SUCCESS;
    vx_reference retref = ref;
    if ((NULL != ref) &&
        (ref->magic == TIVX_MAGIC) && /* TIOVX-1928- LDRA Uncovered Branch Id: TIOVX_BRANCH_COVERAGE_TIVX_SAFE_CASTS_UBR004 */
        (ownIsValidContext(ref->context) == (vx_bool)vx_true_e))
    {
        if (ref->type != type)
        {
            local_status = (vx_status)VX_ERROR_INVALID_TYPE;
            VX_PRINT(VX_ZONE_ERROR, "vxCastRefAs: Attempt to cast reference %p as type %d when actual type is %d\n", ref, type, ref->type);
            vxAddLogEntry(ref, local_status, "vxCastRefAs: Attempt to cast reference %p as type %d when actual type is %d\n", ref, type, ref->type);
            retref = ownGetErrorObject(ref->context, local_status);
        }
    }
    else
    {
        local_status = (vx_status)VX_ERROR_INVALID_REFERENCE;
    }
    if (status &&
        ((vx_status)VX_SUCCESS == *status))
    {
        *status = local_status;
    }
    return retref;
};
