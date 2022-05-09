/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
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

#include <math.h>
#include <string.h>
#include "test.h"

// As for OpenVX 1.0 both of following defines result in udefined behavior:
// #define OPENVX_PLANE_COPYING_VARIANT1
// #define OPENVX_PLANE_COPYING_VARIANT2

#define CT_ENUM_TO_STR(code) case code: return #code;

const char* ct_vx_status_to_str(vx_status s)
{
    switch(s)
    {
        CT_ENUM_TO_STR(VX_STATUS_MIN)
        CT_ENUM_TO_STR(VX_ERROR_REFERENCE_NONZERO)
        CT_ENUM_TO_STR(VX_ERROR_MULTIPLE_WRITERS)
        CT_ENUM_TO_STR(VX_ERROR_GRAPH_ABANDONED)
        CT_ENUM_TO_STR(VX_ERROR_GRAPH_SCHEDULED)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_SCOPE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_NODE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_GRAPH)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_TYPE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_VALUE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_DIMENSION)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_FORMAT)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_LINK)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_REFERENCE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_MODULE)
        CT_ENUM_TO_STR(VX_ERROR_INVALID_PARAMETERS)
        CT_ENUM_TO_STR(VX_ERROR_OPTIMIZED_AWAY)
        CT_ENUM_TO_STR(VX_ERROR_NO_MEMORY)
        CT_ENUM_TO_STR(VX_ERROR_NO_RESOURCES)
        CT_ENUM_TO_STR(VX_ERROR_NOT_COMPATIBLE)
        CT_ENUM_TO_STR(VX_ERROR_NOT_ALLOCATED)
        CT_ENUM_TO_STR(VX_ERROR_NOT_SUFFICIENT)
        CT_ENUM_TO_STR(VX_ERROR_NOT_SUPPORTED)
        CT_ENUM_TO_STR(VX_ERROR_NOT_IMPLEMENTED)
        CT_ENUM_TO_STR(VX_FAILURE)
        CT_ENUM_TO_STR(VX_SUCCESS)
        default:
            return "Non-standard VX_STATUS code"; //TODO: append numerical code
    };
}

const char* ct_vx_type_to_str(enum vx_type_e type)
{
    switch(type)
    {
        CT_ENUM_TO_STR(VX_TYPE_INVALID)
        CT_ENUM_TO_STR(VX_TYPE_CHAR)
        CT_ENUM_TO_STR(VX_TYPE_INT8)
        CT_ENUM_TO_STR(VX_TYPE_UINT8)
        CT_ENUM_TO_STR(VX_TYPE_INT16)
        CT_ENUM_TO_STR(VX_TYPE_UINT16)
        CT_ENUM_TO_STR(VX_TYPE_INT32)
        CT_ENUM_TO_STR(VX_TYPE_UINT32)
        CT_ENUM_TO_STR(VX_TYPE_INT64)
        CT_ENUM_TO_STR(VX_TYPE_UINT64)
        CT_ENUM_TO_STR(VX_TYPE_FLOAT32)
        CT_ENUM_TO_STR(VX_TYPE_FLOAT64)
        CT_ENUM_TO_STR(VX_TYPE_ENUM)
        CT_ENUM_TO_STR(VX_TYPE_SIZE)
        CT_ENUM_TO_STR(VX_TYPE_DF_IMAGE)
// #ifdef EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT
//         CT_ENUM_TO_STR(VX_TYPE_FLOAT16)
// #endif
        CT_ENUM_TO_STR(VX_TYPE_BOOL)
        CT_ENUM_TO_STR(VX_TYPE_SCALAR_MAX)
        CT_ENUM_TO_STR(VX_TYPE_RECTANGLE)
        CT_ENUM_TO_STR(VX_TYPE_KEYPOINT)
        CT_ENUM_TO_STR(VX_TYPE_COORDINATES2D)
        CT_ENUM_TO_STR(VX_TYPE_COORDINATES3D)
        CT_ENUM_TO_STR(VX_TYPE_KHRONOS_STRUCT_MAX)
        CT_ENUM_TO_STR(VX_TYPE_USER_STRUCT_START)
        CT_ENUM_TO_STR(VX_TYPE_REFERENCE)
        CT_ENUM_TO_STR(VX_TYPE_CONTEXT)
        CT_ENUM_TO_STR(VX_TYPE_GRAPH)
        CT_ENUM_TO_STR(VX_TYPE_NODE)
        CT_ENUM_TO_STR(VX_TYPE_KERNEL)
        CT_ENUM_TO_STR(VX_TYPE_PARAMETER)
        CT_ENUM_TO_STR(VX_TYPE_DELAY)
        CT_ENUM_TO_STR(VX_TYPE_LUT)
        CT_ENUM_TO_STR(VX_TYPE_DISTRIBUTION)
        CT_ENUM_TO_STR(VX_TYPE_PYRAMID)
        CT_ENUM_TO_STR(VX_TYPE_THRESHOLD)
        CT_ENUM_TO_STR(VX_TYPE_MATRIX)
        CT_ENUM_TO_STR(VX_TYPE_CONVOLUTION)
        CT_ENUM_TO_STR(VX_TYPE_SCALAR)
        CT_ENUM_TO_STR(VX_TYPE_ARRAY)
        CT_ENUM_TO_STR(VX_TYPE_IMAGE)
        CT_ENUM_TO_STR(VX_TYPE_REMAP)
        CT_ENUM_TO_STR(VX_TYPE_ERROR)
        CT_ENUM_TO_STR(VX_TYPE_META_FORMAT)
        CT_ENUM_TO_STR(VX_TYPE_KHRONOS_OBJECT_END)
        default:
            return "Non-standard vx_type_e code"; //TODO: append numerical code
    };
}

int ct_assert_reference_impl(vx_reference ref, enum vx_type_e expect_type, vx_status expect_status,
                             const char* str_ref, const char* func, const char* file, const int line)
{
    vx_status s;
    vx_enum ref_type;

    if (!ref)
    {
        if (expect_status != VX_STATUS_MIN)
        {
            if (expect_type != VX_TYPE_KHRONOS_OBJECT_END)
            {
                CT_RecordFailureAtFormat("Invalid OpenVX object \"%s\""
                    "\n\t\tExpected: %s object"
                    "\n\t\tActual:   NULL", func, file, line, str_ref, ct_vx_type_to_str(expect_type));
            }
            else
            {
                if (expect_status != VX_SUCCESS)
                {
                    return 1; // passed (assume NULL ref as any error)
                }
                CT_RecordFailureAtFormat("Invalid OpenVX reference \"%s\""
                    "\n\t\tExpected: valid object reference"
                    "\n\t\tActual:   NULL", func, file, line, str_ref);
            }
            return 0; //failed
        }
        else
        {
            // if expect_status == VX_STATUS_MIN then we are testing for non-reference
            return 1; //passed
        }
    }

    s = vxQueryReference(ref, VX_REFERENCE_TYPE, &ref_type, sizeof(ref_type));

    if (s == VX_ERROR_INVALID_REFERENCE)
    {
        if (expect_status != VX_STATUS_MIN)
        {
            if (expect_type != VX_TYPE_KHRONOS_OBJECT_END)
            {
                CT_RecordFailureAtFormat("Passed object \"%s\" is not a valid OpenVX reference"
                    "\n\t\tExpected: %s object"
                    "\n\t\tActual:   unknown object", func, file, line, str_ref, ct_vx_type_to_str(expect_type));
            }
            else
            {
                CT_RecordFailureAtFormat("Passed object \"%s\" is not a valid OpenVX object"
                    "\n\t\tExpected: OpenVX object"
                    "\n\t\tActual:   unknown object", func, file, line, str_ref);
            }
            return 0; //failed
        }
        else
        {
            // if expect_status == VX_STATUS_MIN then we are testing for non-reference
            return 1; //passed
        }
    }

    if (s != VX_SUCCESS)
    {
        CT_RecordFailureAtFormat("Error quering the type of \"%s\""
            "\n\t\tExpected: VX_SUCCESS"
            "\n\t\tActual:   %s", func, file, line, str_ref, ct_vx_status_to_str(s));
        return 0; //failed
    }

    if (ref_type == VX_TYPE_ERROR)
    {
        s = vxGetStatus(ref);

        if (s != expect_status)
        {
            CT_RecordFailureAtFormat("Error status of \"%s\" object"
                "\n\t\tExpected: %s"
                "\n\t\tActual:   %s", func, file, line, str_ref, ct_vx_status_to_str(expect_status), ct_vx_status_to_str(s));
            return 0; //failed
        }
    }

    if (expect_type != VX_TYPE_KHRONOS_OBJECT_END) // need to verify type
    {
        if (expect_type != (enum vx_type_e)ref_type)
        {
            CT_RecordFailureAtFormat("Error type of \"%s\" object"
                "\n\t\tExpected: %s"
                "\n\t\tActual:   %s", func, file, line, str_ref, ct_vx_type_to_str(expect_type), ct_vx_type_to_str((enum vx_type_e)ref_type));
            return 0; //failed
        }
    }

    if (expect_status != VX_SUCCESS && ref_type != VX_TYPE_ERROR)
    {
        // we have expected an error object, but got a valid reference
        CT_RecordFailureAtFormat("Error status of \"%s\""
            "\n\t\tExpected: %s"
            "\n\t\tActual:   valid %s object", func, file, line, str_ref, ct_vx_status_to_str(expect_status), ct_vx_type_to_str((enum vx_type_e)ref_type));
        return 0; // failed
    }

    return 1; //passed
}

void ct_fill_image_random_impl(vx_image image, uint64_t* seed, const char* func, const char* file, const int line)
{
    uint64_t rng;
    vx_status status;
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_size planes = 0;
    vx_df_image format = VX_DF_IMAGE_VIRT;
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr;
    uint32_t x, y, p;

    ASSERT_AT(seed != NULL, func, file, line);

    CT_RNG_INIT(rng, *seed);

    ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_WIDTH,   &width,   sizeof(width)),  func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_HEIGHT,  &height,  sizeof(height)), func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_PLANES,  &planes,  sizeof(planes)), func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_FORMAT,  &format,  sizeof(format)), func, file, line);

    ASSERT_AT(format != VX_DF_IMAGE_VIRT, func, file, line);

    rect.start_x = rect.start_y = 0;
    rect.end_x = width;
    rect.end_y = height;

    for (p = 0; p < planes; ++p)
    {
        vx_map_id map_id;
        void* base_ptr = 0;
        ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxMapImagePatch(image, &rect, p, &map_id, &addr, &base_ptr, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0), func, file, line);

#define SET_PIXELS(data_type, commands)                                     \
    for (y = 0; y < addr.dim_y; y += addr.step_y)                           \
    {                                                                       \
        for (x = 0; x < addr.dim_x; x += addr.step_x)                       \
        {                                                                   \
            data_type* data = (data_type*)vxFormatImagePatchAddress2d(base_ptr, x, y, &addr); \
            commands                                                        \
        }                                                                   \
    }

        status = VX_SUCCESS;
        if (format == VX_DF_IMAGE_U8) // 1 plane of 8-bit data
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_U16) // 1 plane of 16-bit data
            SET_PIXELS(vx_uint16,
            {
                data[0] = (vx_uint16)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_U32) // 1 plane of 32-bit data
            SET_PIXELS(vx_uint32,
            {
                data[0] = (vx_uint32)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_S16) // 1 plane of 16-bit data
            SET_PIXELS(vx_int16,
            {
                data[0] = (vx_int16)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_S32) // 1 plane of 32-bit data
            SET_PIXELS(vx_int32,
            {
                data[0] = (vx_int32)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_RGB) // 1 plane of 24-bit data
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                data[1] = (vx_uint8)CT_RNG_NEXT(rng);
                data[2] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_RGBX) // 1 plane of 32-bit data
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                data[1] = (vx_uint8)CT_RNG_NEXT(rng);
                data[2] = (vx_uint8)CT_RNG_NEXT(rng);
                data[3] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_YUV4) // 3 planes of 8-bit data
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_IYUV) // 3 planes of 8-bit data
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        else if (format == VX_DF_IMAGE_NV12) // 2 planes: one of 8-bit data and one of 16-bit data
        {
            // width must be multiple of 2
            if (p == 0)
                SET_PIXELS(vx_uint8,
                {
                    data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                })
            else if(p == 1)
                SET_PIXELS(vx_uint8,
                {
                    data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                    data[1] = (vx_uint8)CT_RNG_NEXT(rng);
                })
            else
                status = VX_FAILURE;
        }
        else if (format == VX_DF_IMAGE_NV21) // 2 planes: one of 8-bit data and one of 16-bit data
        {
            // width must be multiple of 2
            if (p == 0)
                SET_PIXELS(vx_uint8,
                {
                    data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                })
            else if(p == 1)
                SET_PIXELS(vx_uint8,
                {
                    data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                    data[1] = (vx_uint8)CT_RNG_NEXT(rng);
                })
            else
                status = VX_FAILURE;
        }
        else if (format == VX_DF_IMAGE_UYVY) // 1 plane of 16-bit data
        {
            // width must be multiple of 2
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                data[1] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        }
        else if (format == VX_DF_IMAGE_YUYV) // 1 plane of 16-bit data
        {
            // width must be multiple of 2
            SET_PIXELS(vx_uint8,
            {
                data[0] = (vx_uint8)CT_RNG_NEXT(rng);
                data[1] = (vx_uint8)CT_RNG_NEXT(rng);
            })
        }
        else
            status = VX_FAILURE;
#undef SET_PIXELS

        *seed = rng; // return new value of random number generator

        if (status != VX_SUCCESS)
        {
            vxUnmapImagePatch(image, map_id); // empty commit to abort transaction
            FAIL_AT("ENGINE: Unknown or broken vx_image format: %.4s", func, file, line, &format);
        }
        ASSERT_EQ_VX_STATUS_AT_(return, VX_SUCCESS, vxUnmapImagePatch(image, map_id), func, file, line);
    } /* for planes */
}

vx_image ct_clone_image_impl(vx_image image, vx_graph graph, const char* func, const char* file, const int line)
{
#define CLONE_FAILED() CT_RecordFailureAt("ENGINE: Unable to make a clone of vx_image", func, file, line)
    vx_status status;
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_size planes = 0;
    vx_df_image format = 0;
    vx_rectangle_t rect;
    void *base_ptr_src = 0;
    void *base_ptr_dst = 0;
    vx_uint32 p;
    vx_context context = 0;
    vx_image clone = 0;
    int is_virtual = 0;

    if (!image) return NULL; //OK
    if (vxGetStatus((vx_reference)image) != VX_SUCCESS) { CLONE_FAILED(); return NULL; }

    context = vxGetContext((vx_reference)image);
    if (vxGetStatus((vx_reference)context) != VX_SUCCESS) { CLONE_FAILED(); return NULL; }

    status = vxQueryImage(image, VX_IMAGE_WIDTH,  &width,  sizeof(width));
    if (status != VX_SUCCESS)  { CLONE_FAILED(); return NULL; }
    status = vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(height));
    if (status != VX_SUCCESS)  { CLONE_FAILED(); return NULL; }
    status = vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format));
    if (status != VX_SUCCESS)  { CLONE_FAILED(); return NULL; }

    if (format == VX_DF_IMAGE_VIRT || width == 0 || height == 0) is_virtual = 1;

    // test if still virtual
    if (!is_virtual)
    {
        vx_imagepatch_addressing_t addr_src = VX_IMAGEPATCH_ADDR_INIT;
        vx_map_id map_id;
        rect.start_x = rect.start_y = 0;
        rect.end_x = width; rect.end_y = height;
        status = vxMapImagePatch(image, &rect, 0, &map_id, &addr_src, &base_ptr_src, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if (status == VX_ERROR_OPTIMIZED_AWAY)
        {
            is_virtual = 1;
        }
        else
        {
            if (status != VX_SUCCESS) { CLONE_FAILED(); return NULL; }
            status = vxUnmapImagePatch(image, map_id);
            if (status != VX_SUCCESS) { CLONE_FAILED(); return NULL; }
        }
    }

    if (is_virtual)
    {
        if (graph == 0 || vxGetStatus((vx_reference)graph) != VX_SUCCESS)
        {
            CT_RecordFailureAt("ENGINE: Unable to make a clone of vx_image without valid graph object", func, file, line);
            return NULL;
        }

        clone = vxCreateVirtualImage(graph, width, height, format);
        if (vxGetStatus((vx_reference)clone) != VX_SUCCESS) { CLONE_FAILED(); return NULL; }
    }
    else
    {
        clone = vxCreateImage(context, width, height, format);
        if (vxGetStatus((vx_reference)clone) != VX_SUCCESS) { CLONE_FAILED(); return NULL; }

        status = vxQueryImage(image, VX_IMAGE_PLANES, &planes, sizeof(planes));
        if (status != VX_SUCCESS)  { CLONE_FAILED(); return NULL; }

        // TODO: use vxGetValidRegionImage
        rect.start_x = rect.start_y = 0;
        rect.end_x = width;
        rect.end_y = height;

#if defined OPENVX_PLANE_COPYING_VARIANT1
        for (p = 0; p < planes; ++p)
        {
            vx_imagepatch_addressing_t addr_src = VX_IMAGEPATCH_ADDR_INIT;
            vx_imagepatch_addressing_t addr_dst = VX_IMAGEPATCH_ADDR_INIT;
            vx_status status1;
            base_ptr_src = base_ptr_dst = 0;
            status = vxAccessImagePatch(clone, &rect, p, &addr_dst, &base_ptr_dst, VX_WRITE_ONLY);
            if (status != VX_SUCCESS) { CLONE_FAILED(); return NULL; }
            status = vxAccessImagePatch(image, &rect, p, &addr_src, &base_ptr_src, VX_READ_ONLY);
            if (status != VX_SUCCESS) { CLONE_FAILED(); vxReleaseImage(&clone); return NULL; }

            // UNDEFINED BEHAVIOR: pass pointer obtained for one image to commit on another image
            status  = vxCommitImagePatch(clone, &rect, p, &addr_src, base_ptr_src);
            status1 = vxCommitImagePatch(image, 0, p, &addr_dst, base_ptr_dst);
            if (status1 != VX_SUCCESS || status != VX_SUCCESS)
            {
                CLONE_FAILED(); vxReleaseImage(&clone); return NULL;
            }
        }
#elif defined OPENVX_PLANE_COPYING_VARIANT2
        for (p = 0; p < planes; ++p)
        {
            vx_imagepatch_addressing_t addr_src = VX_IMAGEPATCH_ADDR_INIT;
            vx_status status1;
            base_ptr_src = 0;
            status = vxAccessImagePatch(image, &rect, p, &addr_src, &base_ptr_src, VX_READ_ONLY);
            if (status != VX_SUCCESS) { CLONE_FAILED(); vxReleaseImage(&clone); return NULL; }

            // 1) passing non-zero address to vxAccessImagePatch means that we ask to use our buffer
            // 2) UNDEFINED BEHAVIOR: assuming that VX_WRITE_ONLY with external buffer does not modify the buffer data
            // 3) assuming that addressing structure is not changed or filled with the same values;
            //    actually existense of vxComputeImagePatchSize almost guarantee that this assumption is correct
            status = vxAccessImagePatch(clone, &rect, p, &addr_src, &base_ptr_src, VX_WRITE_ONLY);
            if (status != VX_SUCCESS) { CLONE_FAILED(); return NULL; }

            // commit using "externally" allocated buffer
            // implementation has to copy data internally
            // implementation must not release the buffer
            status  = vxCommitImagePatch(clone, &rect, p, &addr_src, base_ptr_src);

            // commit with pointer obtained from vxAccessImagePatch for the same image
            // implementation has to decrement image refcounter and recycle the buffer
            status1 = vxCommitImagePatch(image, 0, p, &addr_src, base_ptr_src);
            if (status1 != VX_SUCCESS || status != VX_SUCCESS)
            {
                CLONE_FAILED(); vxReleaseImage(&clone); return NULL;
            }
        }
#else
        for (p = 0; p < planes; ++p)
        {
            vx_imagepatch_addressing_t addr_src = VX_IMAGEPATCH_ADDR_INIT;
            vx_imagepatch_addressing_t addr_dst = VX_IMAGEPATCH_ADDR_INIT;
            vx_map_id map_id_src;
            vx_map_id map_id_dst;

            vx_uint32 w, h, x, y, k;
            vx_uint32 elem_sz = 0;
            switch(format)
            {
                case VX_DF_IMAGE_U8:
                case VX_DF_IMAGE_YUV4:
                case VX_DF_IMAGE_IYUV:
                    elem_sz = 1;
                    break;
                case VX_DF_IMAGE_S16:
                case VX_DF_IMAGE_U16:
                case VX_DF_IMAGE_UYVY:
                case VX_DF_IMAGE_YUYV:
                    elem_sz = 2;
                    break;
                case VX_DF_IMAGE_RGB:
                    elem_sz = 3;
                    break;
                case VX_DF_IMAGE_S32:
                case VX_DF_IMAGE_U32:
                case VX_DF_IMAGE_RGBX:
                    elem_sz = 4;
                    break;
                case VX_DF_IMAGE_NV12:
                case VX_DF_IMAGE_NV21:
                    if (p == 0)
                        elem_sz = 1;
                    else if (p == 1)
                        elem_sz = 2;
                    break;
                default:
                    /* unknown format*/
                    elem_sz = 0;
                    break;
            };
            if (elem_sz == 0)
            {
                CLONE_FAILED();
                vxReleaseImage(&clone);
                return NULL;
            }

            base_ptr_src = base_ptr_dst = 0;
            status = vxMapImagePatch(clone, &rect, p, &map_id_dst, &addr_dst, &base_ptr_dst, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);
            if (status != VX_SUCCESS) { CLONE_FAILED(); return NULL; }

            status = vxMapImagePatch(image, &rect, p, &map_id_src, &addr_src, &base_ptr_src, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
            if (status != VX_SUCCESS) { CLONE_FAILED(); vxReleaseImage(&clone); return NULL; }

            h = addr_src.dim_y / addr_src.step_y;
            w = addr_src.dim_x / addr_src.step_x;

            if (h != addr_dst.dim_y / addr_dst.step_y || w != addr_dst.dim_x / addr_dst.step_x)
            {
                // everything is bad
                CLONE_FAILED();
                vxUnmapImagePatch(clone, map_id_dst);
                vxUnmapImagePatch(image, map_id_src);
                vxReleaseImage(&clone);
                return NULL;
            }

            for (y = 0; y < h; y++)
            {
                int j_src = y * addr_src.stride_y * addr_src.step_y * addr_src.scale_y / VX_SCALE_UNITY;
                int j_dst = y * addr_dst.stride_y * addr_dst.step_y * addr_dst.scale_y / VX_SCALE_UNITY;

                for (x = 0; x < w; x++)
                {
                    int i_src = x * addr_src.stride_x * addr_src.step_x * addr_src.scale_x / VX_SCALE_UNITY;
                    int i_dst = x * addr_dst.stride_x * addr_dst.step_x * addr_dst.scale_x / VX_SCALE_UNITY;

                    vx_uint8 *psrc = (vx_uint8*)(((vx_uint8 *)base_ptr_src) + j_src + i_src);
                    vx_uint8 *pdst = (vx_uint8*)(((vx_uint8 *)base_ptr_dst) + j_dst + i_dst);

                    for(k = 0; k < elem_sz; ++k)
                        pdst[k] = psrc[k];
                }
            }

            status = vxUnmapImagePatch(image, map_id_src);
            if (status != VX_SUCCESS) { CLONE_FAILED(); vxReleaseImage(&clone); return NULL; }

            status = vxUnmapImagePatch(clone, map_id_dst);
            if (status != VX_SUCCESS) { CLONE_FAILED(); vxReleaseImage(&clone); return NULL; }
        }
#endif
    }

    // copy remaining attributes
    {
        vx_enum space;
        if (vxQueryImage(image, VX_IMAGE_SPACE, &space, sizeof(space)) != VX_SUCCESS)
        {
            CLONE_FAILED(); vxReleaseImage(&clone); return NULL;
        }
        if (vxSetImageAttribute(clone, VX_IMAGE_SPACE, &space, sizeof(space)) != VX_SUCCESS)
        {
            CLONE_FAILED(); vxReleaseImage(&clone); return NULL;
        }
    }

    return clone;
#undef CLONE_FAILED
}

static vx_context g_vx_context = 0;

static void VX_CALLBACK log_callback(vx_context context, vx_reference ref, vx_status status, const vx_char* str)
{
#if 0
    printf("LOG entry for object %p: %s\n", ref, str);
#endif
}

static vx_context ct_create_vx_context()
{
    if (g_vx_context)
    {
        return g_vx_context;
    }
    else
    {
        vx_context ctx = vxCreateContext();
        if (vxGetStatus((vx_reference)ctx) == VX_SUCCESS)
            vxRegisterLogCallback(ctx, log_callback, vx_true_e);

        return ctx;
    }
    return g_vx_context;
}

void ct_create_global_vx_context()
{
    g_vx_context = ct_create_vx_context();
}

void ct_release_global_vx_context()
{
    if (g_vx_context)
    {
        vxReleaseContext(&g_vx_context);
        ASSERT(g_vx_context == 0);
    }
}

static void ct_teardown_vx_context(void/*CT_VXContext*/ **context_)
{
    CT_VXContext* context = (CT_VXContext*)*context_;
    if (context == NULL)
        return;

    if (!g_vx_context && context->vx_context_)
    {
        if (!CT_HasFailure())
        {
            vx_uint32 dangling_refs_count, modules, kernels;
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_REFERENCES, &dangling_refs_count, sizeof(dangling_refs_count)));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_MODULES, &modules, sizeof(modules)));
            EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_UNIQUE_KERNELS, &kernels, sizeof(kernels)));

            // The specification needs to be clarified. Implementations may have a different interpretation of what
            // is an 'active reference'. The number of active references may or may not be 0 just after context creation.
            dangling_refs_count -= context->vx_context_base_references_;

            // The specification needs to be clarified. An implementation may or may not consider kernel objects
            // that are not referenced by the application (but that are still alive since a kernels exist until
            // explicitly removed) as 'active references'. In case the implementation does, an aditionnal subtraction
            // is needed.
            if ( ((modules != context->vx_context_base_modules_) ||
                  (kernels != context->vx_context_base_kernels_)) &&
                 (dangling_refs_count > 0))
            {
                dangling_refs_count -= kernels - context->vx_context_base_kernels_;
            }

            EXPECT_EQ_INT(0, dangling_refs_count);
        }
        vxReleaseContext(&context->vx_context_);
        ASSERT(context->vx_context_ == 0);
    }

    ct_free_mem(context);
}

void* ct_setup_vx_context()
{
    CT_VXContext* context = (CT_VXContext*)ct_alloc_mem(sizeof(CT_VXContext));
    context->vx_context_ = ct_create_vx_context();
    ASSERT_VX_OBJECT_({ct_free_mem(context); return 0;}, context->vx_context_, VX_TYPE_CONTEXT);

    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_REFERENCES,
        &context->vx_context_base_references_, sizeof(context->vx_context_base_references_)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_MODULES,
        &context->vx_context_base_modules_, sizeof(context->vx_context_base_modules_)));
    EXPECT_EQ_VX_STATUS(VX_SUCCESS, vxQueryContext(context->vx_context_, VX_CONTEXT_UNIQUE_KERNELS,
        &context->vx_context_base_kernels_, sizeof(context->vx_context_base_kernels_)));

    CT_RegisterForGarbageCollection(context, ct_teardown_vx_context, CT_GC_OBJECT);
    return context;
}

vx_image ct_create_similar_image_impl(vx_image image, const char* func, const char* file, const int line)
{
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_df_image format = VX_DF_IMAGE_VIRT;
    vx_image new_image = 0;

    vx_context context = vxGetContext((vx_reference)image);
    ASSERT_VX_OBJECT_AT_(return 0, context, VX_TYPE_CONTEXT, func, file, line);

    ASSERT_EQ_VX_STATUS_AT_(return 0, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_WIDTH,   &width,   sizeof(width)),  func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return 0, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_HEIGHT,  &height,  sizeof(height)), func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return 0, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_FORMAT,  &format,  sizeof(format)), func, file, line);

    new_image = vxCreateImage(context, width, height, format);
    ASSERT_VX_OBJECT_AT_(return 0, new_image, VX_TYPE_IMAGE, func, file, line);

    return new_image;
}

vx_image ct_create_similar_image_with_format_impl(vx_image image, vx_df_image format, const char* func, const char* file, const int line)
{
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_image new_image = 0;

    vx_context context = vxGetContext((vx_reference)image);
    ASSERT_VX_OBJECT_AT_(return 0, context, VX_TYPE_CONTEXT, func, file, line);

    ASSERT_EQ_VX_STATUS_AT_(return 0, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_WIDTH,   &width,   sizeof(width)),  func, file, line);
    ASSERT_EQ_VX_STATUS_AT_(return 0, VX_SUCCESS, vxQueryImage(image, VX_IMAGE_HEIGHT,  &height,  sizeof(height)), func, file, line);

    new_image = vxCreateImage(context, width, height, format);
    ASSERT_VX_OBJECT_AT_(return 0, new_image, VX_TYPE_IMAGE, func, file, line);

    return new_image;
}


vx_status ct_dump_vx_image_info(vx_image image)
{
    vx_uint32 width  = 0;
    vx_uint32 height = 0;
    vx_size planes = 0;
    vx_df_image format = 0;

    VX_CALL_RET(vxGetStatus((vx_reference)image));

    VX_CALL_RET(vxQueryImage(image, VX_IMAGE_WIDTH,  &width,  sizeof(width)));
    VX_CALL_RET(vxQueryImage(image, VX_IMAGE_HEIGHT, &height, sizeof(height)));
    VX_CALL_RET(vxQueryImage(image, VX_IMAGE_FORMAT, &format, sizeof(format)));
    VX_CALL_RET(vxQueryImage(image, VX_IMAGE_PLANES, &planes, sizeof(planes)));

    printf("Image %p DF_IMAGE(%.*s): %dx%d (%d planes)\n",
            image, (int)sizeof(format), (char*)&format,
            width, height, (int)planes);

    return VX_SUCCESS;
}

uint32_t ct_floor_u32_no_overflow(float v)
{
    return (uint32_t)(v);
}

uint8_t ct_clamp_8u(int32_t v)
{
    if (v >= 255)
        return v;
    if (v <= 0)
        return 0;
    return (uint8_t)v;
}

float ct_log_rng(uint64_t* rng, float minlog2, float maxlog2)
{
    float a = (float)CT_RNG_NEXT_REAL(*rng, minlog2, maxlog2);
    return expf(a*0.6931471805599453f);
}

int ct_roundf(float val)
{
    return (int)floor(val + 0.5);
}

typedef union ct_scalar_val
{
    vx_char   chr;
    vx_int8   s08;
    vx_uint8  u08;
    vx_int16  s16;
    vx_uint16 u16;
    vx_int32  s32;
    vx_uint32 u32;
    vx_int64  s64;
    vx_int64  u64;
    vx_float32 f32;
    vx_float64 f64;
    vx_df_image  fcc;
    vx_enum    enm;
    vx_size    size;
    vx_bool    boolean;
} ct_scalar_val;


int ct_scalar_as_int(vx_scalar val)
{
    vx_enum sctype = 0;
    ct_scalar_val scval;
    int ival = -1;

    vxQueryScalar(val, VX_SCALAR_TYPE, &sctype, sizeof(sctype));
    vxCopyScalar(val, &scval, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

    if( sctype == VX_TYPE_CHAR )
        ival = scval.chr;
    else if( sctype == VX_TYPE_INT8 )
        ival = scval.s08;
    else if( sctype == VX_TYPE_UINT8 )
        ival = scval.u08;
    else if( sctype == VX_TYPE_INT16 )
        ival = scval.s16;
    else if( sctype == VX_TYPE_UINT16 )
        ival = scval.u16;
    else if( sctype == VX_TYPE_INT32 )
        ival = scval.s32;
    else if( sctype == VX_TYPE_UINT32 )
        ival = (int)scval.u32;
    else if( sctype == VX_TYPE_INT64 )
        ival = (int)scval.s64;
    else if( sctype == VX_TYPE_UINT64 )
        ival = (int)scval.u64;
    else if( sctype == VX_TYPE_FLOAT32 )
        ival = ct_roundf(scval.f32);
    else if( sctype == VX_TYPE_FLOAT64 )
        ival = ct_roundf((float)scval.f64);
    else if( sctype == VX_TYPE_DF_IMAGE )
        ival = (int)scval.fcc;
    else if( sctype == VX_TYPE_ENUM )
        ival = ct_roundf((float)scval.enm);
    else if( sctype == VX_TYPE_SIZE )
        ival = ct_roundf((float)scval.size);
    else if( sctype == VX_TYPE_BOOL )
        ival = scval.boolean == vx_true_e;
    else
    {
        CT_ADD_FAILURE("Can not read int from scalar of type %d", (int)sctype);
    }

    return ival;
}

vx_scalar ct_scalar_from_int(vx_context ctx, vx_enum sctype, int ival)
{
    ct_scalar_val scval;

    if( sctype == VX_TYPE_CHAR )
        scval.chr = (char)ival;
    else if( sctype == VX_TYPE_INT8 )
        scval.s08 = (vx_int8)ival;
    else if( sctype == VX_TYPE_UINT8 )
        scval.u08 = (vx_uint8)ival;
    else if( sctype == VX_TYPE_INT16 )
        scval.s16 = (vx_int16)ival;
    else if( sctype == VX_TYPE_UINT16 )
        scval.u16 = (vx_uint16)ival;
    else if( sctype == VX_TYPE_INT32 )
        scval.s32 = (vx_int32)ival;
    else if( sctype == VX_TYPE_UINT32 )
        scval.u32 = (vx_uint32)ival;
    else if( sctype == VX_TYPE_INT64 )
        scval.s64 = (vx_int64)ival;
    else if( sctype == VX_TYPE_UINT64 )
        scval.u64 = (vx_uint64)ival;
    else if( sctype == VX_TYPE_FLOAT32 )
        scval.f32 = (vx_float32)ival;
    else if( sctype == VX_TYPE_FLOAT64 )
        scval.f64 = (vx_float64)ival;
    else if( sctype == VX_TYPE_DF_IMAGE )
        scval.fcc = (vx_df_image)ival;
    else if( sctype == VX_TYPE_ENUM )
        scval.enm = (vx_enum)ival;
    else if( sctype == VX_TYPE_SIZE )
        scval.size = (vx_size)ival;
    else if( sctype == VX_TYPE_BOOL )
        scval.boolean = ival != 0 ? vx_true_e : vx_false_e;
    else
    {
        CT_ADD_FAILURE("Can not read int from scalar of type %d", (int)sctype);
        return 0;
    }

    return vxCreateScalar(ctx, sctype, &scval);
}

vx_enum ct_read_array(vx_array src, void** dst, vx_size* _capacity_bytes,
                      vx_size* _arrsize, vx_size* _arrcap )
{
    vx_size i;
    vx_size arrsize_bytes;
    vx_size arrcap = 0;
    vx_size arrsize  = 0;
    vx_size itemsize = 0;
    vx_size stride   = 0;
    vx_enum itemtype = 0;
    vx_map_id map_id;
    char* ptr = 0;

    vxQueryArray(src, VX_ARRAY_ITEMTYPE, &itemtype, sizeof(itemtype));
    vxQueryArray(src, VX_ARRAY_CAPACITY, &arrcap, sizeof(arrcap));
    vxQueryArray(src, VX_ARRAY_NUMITEMS, &arrsize, sizeof(arrsize));
    vxQueryArray(src, VX_ARRAY_ITEMSIZE, &itemsize, sizeof(itemsize));

    if(_arrsize)
        *_arrsize = arrsize;
    if(_arrcap)
        *_arrcap = arrcap;

    if(arrsize == 0)
        return itemtype;

    arrsize_bytes = arrsize*itemsize;
    if( _capacity_bytes == 0 || arrsize_bytes >= *_capacity_bytes )
    {
        if(*dst)
            ct_free_mem(*dst);

        *dst = ct_alloc_mem(arrsize_bytes);

        if (_capacity_bytes)
            *_capacity_bytes = arrsize_bytes;
    }

    vxMapArrayRange(src, 0, arrsize, &map_id, &stride, (void**)&ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if( stride == itemsize )
        memcpy(*dst, ptr, arrsize_bytes);
    else
    {
        for( i = 0; i < arrsize; i++ )
            memcpy((char*)(*dst) + i*itemsize, ptr + i*stride, itemsize);
    }

    vxUnmapArrayRange(src, map_id);

    return itemtype;
}

void ct_update_progress(int i, int niters)
{
    if( i == 0 )
    {
        printf("[ ");
    }
    if( (i+1)%(niters/8) == 0 )
    {
        putchar('*');
        fflush(stdout);
    }
    if( i == niters-1 )
    {
        printf(" ]\n");
    }
}


static int check_any_size = 0;
int ct_check_any_size()
{
    return check_any_size;
}

void ct_set_check_any_size(int flag)
{
    check_any_size = flag;
}

void ct_destroy_vx_context(void **pContext)
{
    vxReleaseContext((vx_context *)pContext);
    *pContext = NULL;
}

char *ct_get_test_file_path()
{
    char *tivxPlatformGetEnv(char *env_var);
    
    #if defined(SYSBIOS) || defined(FREERTOS) || defined(SAFERTOS)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}
