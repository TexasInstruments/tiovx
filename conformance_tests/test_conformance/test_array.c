/*

 * Copyright (c) 2015-2017 The Khronos Group Inc.
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
#include <VX/vx.h>
#include <VX/vxu.h>

#include "test_engine/test.h"


#define FLOAT_EPSILON 0.000001f

TESTCASE(Array, CT_VXContext, ct_setup_vx_context, 0)

typedef struct _own_struct
{
    vx_uint32  some_uint;
    vx_float64 some_double;
} own_struct;


typedef struct
{
    const char* testName;
    const char* p;
    vx_enum item_type;
} Array_Arg;


static
void* own_alloc_init_data_items(vx_enum item_type, vx_size num_items)
{
    vx_size i;
    vx_size item_size;
    void* p = 0;

    switch (item_type)
    {
    case VX_TYPE_CHAR:              item_size = sizeof(vx_char); break;
    case VX_TYPE_INT8:              item_size = sizeof(vx_int8); break;
    case VX_TYPE_UINT8:             item_size = sizeof(vx_uint8); break;
    case VX_TYPE_INT16:             item_size = sizeof(vx_int16); break;
    case VX_TYPE_UINT16:            item_size = sizeof(vx_uint16); break;
    case VX_TYPE_INT32:             item_size = sizeof(vx_int32); break;
    case VX_TYPE_UINT32:            item_size = sizeof(vx_uint32); break;
    case VX_TYPE_INT64:             item_size = sizeof(vx_int64); break;
    case VX_TYPE_UINT64:            item_size = sizeof(vx_uint64); break;
    case VX_TYPE_FLOAT32:           item_size = sizeof(vx_float32); break;
    case VX_TYPE_FLOAT64:           item_size = sizeof(vx_float64); break;
    case VX_TYPE_ENUM:              item_size = sizeof(vx_enum); break;
    case VX_TYPE_SIZE:              item_size = sizeof(vx_size); break;
    case VX_TYPE_DF_IMAGE:          item_size = sizeof(vx_df_image); break;
    case VX_TYPE_BOOL:              item_size = sizeof(vx_bool); break;
    case VX_TYPE_RECTANGLE:         item_size = sizeof(vx_rectangle_t); break;
    case VX_TYPE_KEYPOINT:          item_size = sizeof(vx_keypoint_t); break;
    case VX_TYPE_COORDINATES2D:     item_size = sizeof(vx_coordinates2d_t); break;
    case VX_TYPE_COORDINATES3D:     item_size = sizeof(vx_coordinates3d_t); break;

    default:
        if (item_type >= VX_TYPE_USER_STRUCT_START && item_type <= VX_TYPE_USER_STRUCT_END)
            item_size = sizeof(own_struct);
        else
            item_size = 0;
        break;
    }

    p = ct_alloc_mem(num_items * item_size);
    if (NULL == p)
        return p;

    for (i = 0; i < num_items; i++)
    {
        switch (item_type)
        {
        case VX_TYPE_CHAR:
            ((vx_char*)p)[i] = ((i & 1) ? (vx_char)i : -(vx_char)i);
            break;

        case VX_TYPE_INT8:
            ((vx_int8*)p)[i] = ((i & 1) ? (vx_int8)i : -(vx_int8)i);
            break;

        case VX_TYPE_UINT8:
            ((vx_uint8*)p)[i] = (vx_uint8)i;
            break;

        case VX_TYPE_INT16:
            ((vx_int16*)p)[i] = ((i & 1) ? (vx_int16)i : -(vx_int16)i);
            break;

        case VX_TYPE_UINT16:
            ((vx_uint16*)p)[i] = (vx_uint16)i;
            break;

        case VX_TYPE_INT32:
            ((vx_int32*)p)[i] = ((i & 1) ? (vx_int32)i : -(vx_int32)i);
            break;

        case VX_TYPE_UINT32:
            ((vx_uint32*)p)[i] = (vx_uint32)i;
            break;

        case VX_TYPE_INT64:
            ((vx_int64*)p)[i] = ((i & 1) ? (vx_int64)i : -(vx_int64)i);
            break;

        case VX_TYPE_UINT64:
            ((vx_uint64*)p)[i] = (vx_uint64)i;
            break;

        case VX_TYPE_FLOAT32:
            ((vx_float32*)p)[i] = (vx_float32)((i & 1) ? (100.0f * i) : -(1.0f / i));
            break;

        case VX_TYPE_FLOAT64:
            ((vx_float64*)p)[i] = (vx_float64)((i & 1) ? (100.0 * i) : -(1.0 / i));
            break;

        case VX_TYPE_ENUM:
            {
                vx_enum data[3];
                data[0] = VX_READ_ONLY;
                data[1] = VX_WRITE_ONLY;
                data[2] = VX_READ_AND_WRITE;
                ((vx_enum*)p)[i] = data[i % (sizeof(data) / sizeof(vx_enum))];
            }
            break;

        case VX_TYPE_SIZE:
            ((vx_size*)p)[i] = (vx_size)i;
            break;

        case VX_TYPE_DF_IMAGE:
            {
                vx_df_image data[] = { VX_DF_IMAGE_VIRT, VX_DF_IMAGE_RGB, VX_DF_IMAGE_RGBX };
                ((vx_df_image*)p)[i] = data[i % (sizeof(data) / sizeof(vx_df_image))];
            }
            break;

        case VX_TYPE_BOOL:
            ((vx_bool*)p)[i] = (vx_bool)(i & 1 ? vx_true_e : vx_false_e);
            break;

        case VX_TYPE_RECTANGLE:
            {
                vx_rectangle_t r = { (vx_uint32)i, (vx_uint32)(num_items - i), (vx_uint32)(i + i), 0 };
                ((vx_rectangle_t*)p)[i] = r;
            }
            break;

        case VX_TYPE_KEYPOINT:
            {
                vx_keypoint_t kp = { (vx_int32)i, (vx_int32)(num_items - i), (1.0f / i), (1.0f / i), (1.0f / i), (vx_int32)i, (1.0f / i) };
                ((vx_keypoint_t*)p)[i] = kp;
            }
            break;

        case VX_TYPE_COORDINATES2D:
            {
                vx_coordinates2d_t c2d = { (vx_uint32)i, (vx_uint32)(num_items - i) };
                ((vx_coordinates2d_t*)p)[i] = c2d;
            }
            break;

        case VX_TYPE_COORDINATES3D:
            {
                vx_coordinates3d_t c3d = { (vx_uint32)i, (vx_uint32)(num_items - i), (vx_uint32)(i + i) };
                ((vx_coordinates3d_t*)p)[i] = c3d;
            }
            break;

        default:
            if (item_type >= VX_TYPE_USER_STRUCT_START && item_type <= VX_TYPE_USER_STRUCT_END)
            {
                own_struct ms = { (vx_uint32)i, 1.0 / i };
                ((own_struct*)p)[i] = ms;
            }
            else
            {
                ct_free_mem(p);
                p = NULL;
            }
            break;
        }
    }

    return p;
}

static
int own_verify_data_items(void* ptr1, void* ptr2, vx_size stride, vx_enum item_type, vx_size num_items)
{
    vx_size i;
    vx_uint8* curr_ptr1;
    vx_uint8* curr_ptr2;

    for (i = 0; i < num_items; i++)
    {
        curr_ptr1 = (vx_uint8*)ptr1 + i*stride;
        curr_ptr2 = (vx_uint8*)ptr2 + i*stride;

        switch (item_type)
        {
        case VX_TYPE_CHAR:
            if( ((vx_char*)curr_ptr1)[0] != ((vx_char*)curr_ptr2)[0] )
                return 1;
            break;

        case VX_TYPE_INT8:
            if (((vx_int8*)curr_ptr1)[0] != ((vx_int8*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_UINT8:
            if (((vx_uint8*)curr_ptr1)[0] != ((vx_uint8*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_INT16:
            if (((vx_int16*)curr_ptr1)[0] != ((vx_int16*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_UINT16:
            if (((vx_uint16*)curr_ptr1)[0] != ((vx_uint16*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_INT32:
            if (((vx_int32*)curr_ptr1)[0] != ((vx_int32*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_UINT32:
            if (((vx_uint32*)curr_ptr1)[0] != ((vx_uint32*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_INT64:
            if (((vx_int64*)curr_ptr1)[0] != ((vx_int64*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_UINT64:
            if (((vx_uint64*)curr_ptr1)[0] != ((vx_uint64*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_FLOAT32:
            if ( fabs(((vx_float32*)curr_ptr1)[0] - ((vx_float32*)curr_ptr2)[0]) > FLOAT_EPSILON )
                return 1;
            break;

        case VX_TYPE_FLOAT64:
            if (fabs(((vx_float64*)curr_ptr1)[0] - ((vx_float64*)curr_ptr2)[0]) > FLOAT_EPSILON)
                return 1;
            break;

        case VX_TYPE_ENUM:
            if (((vx_enum*)curr_ptr1)[0] != ((vx_enum*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_SIZE:
            if (((vx_size*)curr_ptr1)[0] != ((vx_size*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_DF_IMAGE:
            if (((vx_df_image*)curr_ptr1)[0] != ((vx_df_image*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_BOOL:
            if (((vx_bool*)curr_ptr1)[0] != ((vx_bool*)curr_ptr2)[0])
                return 1;
            break;

        case VX_TYPE_RECTANGLE:
            if ( ((vx_rectangle_t*)curr_ptr1)[0].start_x != ((vx_rectangle_t*)curr_ptr2)[0].start_x ||
                 ((vx_rectangle_t*)curr_ptr1)[0].start_y != ((vx_rectangle_t*)curr_ptr2)[0].start_y ||
                 ((vx_rectangle_t*)curr_ptr1)[0].end_x != ((vx_rectangle_t*)curr_ptr2)[0].end_x ||
                 ((vx_rectangle_t*)curr_ptr1)[0].end_y != ((vx_rectangle_t*)curr_ptr2)[0].end_y)
                return 1;
        break;

        case VX_TYPE_KEYPOINT:
            if ( ((vx_keypoint_t*)curr_ptr1)[0].x != ((vx_keypoint_t*)curr_ptr2)[0].x ||
                 ((vx_keypoint_t*)curr_ptr1)[0].y != ((vx_keypoint_t*)curr_ptr2)[0].y ||
                 fabs( ((vx_keypoint_t*)curr_ptr1)[0].strength - ((vx_keypoint_t*)curr_ptr2)[0].strength ) > FLOAT_EPSILON ||
                 fabs( ((vx_keypoint_t*)curr_ptr1)[0].scale - ((vx_keypoint_t*)curr_ptr2)[0].scale ) > FLOAT_EPSILON ||
                 fabs( ((vx_keypoint_t*)curr_ptr1)[0].orientation - ((vx_keypoint_t*)curr_ptr2)[0].orientation) > FLOAT_EPSILON ||
                 ((vx_keypoint_t*)curr_ptr1)[0].tracking_status != ((vx_keypoint_t*)curr_ptr2)[0].tracking_status ||
                 fabs(((vx_keypoint_t*)curr_ptr1)[0].error - ((vx_keypoint_t*)curr_ptr2)[0].error) > FLOAT_EPSILON)
                return 1;
            break;

        case VX_TYPE_COORDINATES2D:
            if ( ((vx_coordinates2d_t*)curr_ptr1)[0].x != ((vx_coordinates2d_t*)curr_ptr2)[0].x ||
                 ((vx_coordinates2d_t*)curr_ptr1)[0].y != ((vx_coordinates2d_t*)curr_ptr2)[0].y )
                return 1;
            break;

        case VX_TYPE_COORDINATES3D:
            if ( ((vx_coordinates3d_t*)curr_ptr1)[0].x != ((vx_coordinates3d_t*)curr_ptr2)[0].x ||
                 ((vx_coordinates3d_t*)curr_ptr1)[0].y != ((vx_coordinates3d_t*)curr_ptr2)[0].y ||
                 ((vx_coordinates3d_t*)curr_ptr1)[0].z != ((vx_coordinates3d_t*)curr_ptr2)[0].z)
                return 1;
            break;

        default:
            if (item_type >= VX_TYPE_USER_STRUCT_START && item_type <= VX_TYPE_USER_STRUCT_END)
            {
                if ( ((own_struct*)curr_ptr1)[0].some_uint != ((own_struct*)curr_ptr2)[0].some_uint ||
                    fabs(((own_struct*)curr_ptr1)[0].some_double - ((own_struct*)curr_ptr2)[0].some_double) > FLOAT_EPSILON )
                    return 1;
            }
            else
                return 1;
        }
    }

    return 0;
}

#define ARRAY_CAPACITY  10
#define ARRAY_NUM_ITEMS 10

#define ADD_VX_ARRAY_TYPES(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_CHAR", __VA_ARGS__, VX_TYPE_CHAR)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_INT8", __VA_ARGS__, VX_TYPE_INT8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_UINT8", __VA_ARGS__, VX_TYPE_UINT8)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_INT16", __VA_ARGS__, VX_TYPE_INT16)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_UINT16", __VA_ARGS__, VX_TYPE_UINT16)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_INT32", __VA_ARGS__, VX_TYPE_INT32)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_UINT32", __VA_ARGS__, VX_TYPE_UINT32)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_INT64", __VA_ARGS__, VX_TYPE_INT64)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_UINT64", __VA_ARGS__, VX_TYPE_UINT64)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_FLOAT32", __VA_ARGS__, VX_TYPE_FLOAT32)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_FLOAT64", __VA_ARGS__, VX_TYPE_FLOAT64)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_ENUM", __VA_ARGS__, VX_TYPE_ENUM)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_SIZE", __VA_ARGS__, VX_TYPE_SIZE)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_DF_IMAGE", __VA_ARGS__, VX_TYPE_DF_IMAGE)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_BOOL", __VA_ARGS__, VX_TYPE_BOOL)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_RECTANGLE", __VA_ARGS__, VX_TYPE_RECTANGLE)), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_KEYPOINT", __VA_ARGS__, VX_TYPE_KEYPOINT )), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_COORDINATES2D", __VA_ARGS__, VX_TYPE_COORDINATES2D )), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_COORDINATES3D", __VA_ARGS__, VX_TYPE_COORDINATES3D )), \
    CT_EXPAND(nextmacro(testArgName "/VX_TYPE_USER_STRUCT_START", __VA_ARGS__, VX_TYPE_USER_STRUCT_START ))

#define PARAMETERS \
    CT_GENERATE_PARAMETERS("array", ADD_VX_ARRAY_TYPES, ARG, NULL)

TEST_WITH_ARG(Array, test_vxCreateArray, Array_Arg, PARAMETERS)
{
    vx_context context = context_->vx_context_;
    vx_enum item_type = VX_TYPE_INVALID;
    vx_enum actual_type = VX_TYPE_INVALID;
    vx_size capacity = ARRAY_CAPACITY;
    vx_size actual_capacity = 0;
    vx_size num_items = ARRAY_NUM_ITEMS;
    vx_size actual_num_items = 0;
    vx_size item_size = 0;
    void* array_items = 0;
    vx_array array = 0;

    if (VX_TYPE_USER_STRUCT_START == arg_->item_type)
    {
        item_type = vxRegisterUserStruct(context, sizeof(own_struct));
        ASSERT(item_type >= VX_TYPE_USER_STRUCT_START && item_type <= VX_TYPE_USER_STRUCT_END);
    }
    else
    {
        item_type = arg_->item_type;
    }

    /* 1. check if array can be created with allowed types*/
    ASSERT_VX_OBJECT(array = vxCreateArray(context, item_type, capacity), VX_TYPE_ARRAY);

    /* 2. check if array's actual item_type corresponds to requested item_type */
    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMTYPE, &actual_type, sizeof(actual_type)));
    ASSERT_EQ_INT(item_type, actual_type);

    /* 3. check if array's actual item_size corresponds to requested item_type size */
    VX_CALL(vxQueryArray(array, VX_ARRAY_ITEMSIZE, &item_size, sizeof(item_size)));

    switch (actual_type)
    {
    case VX_TYPE_CHAR:              ASSERT_EQ_INT(sizeof(vx_char), item_size); break;
    case VX_TYPE_INT8:              ASSERT_EQ_INT(sizeof(vx_int8), item_size); break;
    case VX_TYPE_UINT8:             ASSERT_EQ_INT(sizeof(vx_uint8), item_size); break;
    case VX_TYPE_INT16:             ASSERT_EQ_INT(sizeof(vx_int16), item_size); break;
    case VX_TYPE_UINT16:            ASSERT_EQ_INT(sizeof(vx_uint16), item_size); break;
    case VX_TYPE_INT32:             ASSERT_EQ_INT(sizeof(vx_int32), item_size); break;
    case VX_TYPE_UINT32:            ASSERT_EQ_INT(sizeof(vx_uint32), item_size); break;
    case VX_TYPE_INT64:             ASSERT_EQ_INT(sizeof(vx_int64), item_size); break;
    case VX_TYPE_UINT64:            ASSERT_EQ_INT(sizeof(vx_uint64), item_size); break;
    case VX_TYPE_FLOAT32:           ASSERT_EQ_INT(sizeof(vx_float32), item_size); break;
    case VX_TYPE_FLOAT64:           ASSERT_EQ_INT(sizeof(vx_float64), item_size); break;
    case VX_TYPE_ENUM:              ASSERT_EQ_INT(sizeof(vx_enum), item_size); break;
    case VX_TYPE_SIZE:              ASSERT_EQ_INT(sizeof(vx_size), item_size); break;
    case VX_TYPE_DF_IMAGE:          ASSERT_EQ_INT(sizeof(vx_df_image), item_size); break;
    case VX_TYPE_BOOL:              ASSERT_EQ_INT(sizeof(vx_bool), item_size); break;
    case VX_TYPE_RECTANGLE:         ASSERT_EQ_INT(sizeof(vx_rectangle_t), item_size); break;
    case VX_TYPE_KEYPOINT:          ASSERT_EQ_INT(sizeof(vx_keypoint_t), item_size); break;
    case VX_TYPE_COORDINATES2D:     ASSERT_EQ_INT(sizeof(vx_coordinates2d_t), item_size); break;
    case VX_TYPE_COORDINATES3D:     ASSERT_EQ_INT(sizeof(vx_coordinates3d_t), item_size); break;

    default:
        if (actual_type >= VX_TYPE_USER_STRUCT_START && actual_type <= VX_TYPE_USER_STRUCT_END)
            ASSERT_EQ_INT(sizeof(own_struct), item_size);
        else
            ASSERT(0);
        break;
    }

    /* 4. check if array's actual capacity corresponds to requested capacity */
    VX_CALL(vxQueryArray(array, VX_ARRAY_CAPACITY, &actual_capacity, sizeof(actual_capacity)));
    ASSERT_EQ_INT(capacity, actual_capacity);

    /* 5. check if array is empty after creation */
    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &actual_num_items, sizeof(actual_num_items)));
    ASSERT_EQ_INT(0, actual_num_items);

    array_items = own_alloc_init_data_items(item_type, num_items);
    ASSERT(NULL != array_items);

    VX_CALL(vxAddArrayItems(array, num_items, array_items, item_size));

    /* 6. check if num_items added to the array */
    VX_CALL(vxQueryArray(array, VX_ARRAY_NUMITEMS, &actual_num_items, sizeof(actual_num_items)));
    ASSERT_EQ_INT(num_items, actual_num_items);

    /* 7. check data in array */
    {
        int res;
        vx_size stride = 0;
        void* ptr = 0;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, 0, num_items, &map_id, &stride, &ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0));
        res = own_verify_data_items(array_items, ptr, stride, item_type, num_items);
        VX_CALL(vxUnmapArrayRange(array, map_id));
        ASSERT_EQ_INT(0, res);
    }

    if (NULL != array_items)
        ct_free_mem(array_items);

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

#define N 100
TEST(Array, DISABLED_testAccessCopyWrite)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    int i;
    vx_map_id map_id;

    /* Initialization */
    for (i = 0; i < N; i++) {
        localArrayInit[i].x = 0;
        localArrayInit[i].y = 0;

        localArray[i].x = i;
        localArray[i].y = i;

        localArray2[3*i].x = 2*i;
        localArray2[3*i].y = 2*i;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT( array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL( vxAddArrayItems(array, N, &localArrayInit[0], sizeof(vx_coordinates2d_t)) );

    /* Write, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL( vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT(p == &localArray[N/2]);
    }
    /* Check (MAP) */
    {
        vx_uint8 *p = NULL;
        vx_size stride = 0;
        VX_CALL( vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride >=  sizeof(vx_coordinates2d_t));
        ASSERT(p != NULL);

        for (i = N/2; i<N; i++) {
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->x == i);
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->y == i);
        }
        VX_CALL( vxUnmapArrayRange (array, map_id));
    }

    /* Write, COPY, Spacing */
    {
        vx_size stride = 3*sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray2[3*(N/2)];
        VX_CALL( vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT(p == &localArray2[3*(N/2)]);
    }
    /* Check (MAP) */
    {
        vx_uint8 *p = NULL;
        vx_size stride = 0;
        VX_CALL( vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride >=  sizeof(vx_coordinates2d_t));
        ASSERT(p != NULL);

        for (i = N/2; i<N; i++) {
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->x == 2*i);
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->y == 2*i);
        }
        VX_CALL( vxUnmapArrayRange (array, map_id));
    }

    VX_CALL( vxReleaseArray(&array) );
    ASSERT( array == 0);
}

TEST(Array, DISABLED_testAccessCopyRead)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++) {
        localArrayInit[i].x = i;
        localArrayInit[i].y = i;

        localArray[i].x = 0;
        localArray[i].y = 0;

        localArray2[3*i].x = 0;
        localArray2[3*i].y = 0;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT( array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL( vxAddArrayItems(array, N, &localArrayInit[0], sizeof(vx_coordinates2d_t)) );

    /* READ, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL( vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT(p == &localArray[N/2]);
    }
    /* Check */
    for (i = 0; i < N/2; i++) {
        ASSERT(localArray[i].x == 0);
        ASSERT(localArray[i].y == 0);
    }
    for (i = N/2; i < N; i++) {
        ASSERT(localArray[i].x == i);
        ASSERT(localArray[i].y == i);
    }

    /* READ, COPY, Spacing */
    {
        vx_size stride = 3*sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray2[3*(N/2)];
        VX_CALL( vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
        ASSERT(p == &localArray2[3*(N/2)]);
    }
    /* Check */
    for (i = 0; i < N/2; i++) {
        ASSERT(localArray2[3*i].x == 0);
        ASSERT(localArray2[3*i].y == 0);
        ASSERT(localArray2[3*i+1].x == 0);
        ASSERT(localArray2[3*i+1].y == 0);
        ASSERT(localArray2[3*i+2].x == 0);
        ASSERT(localArray2[3*i+2].y == 0);
    }
    for (i = N/2; i < N; i++) {
        ASSERT(localArray2[3*i].x == i);
        ASSERT(localArray2[3*i].y == i);

        /* Unchanged in between */
        ASSERT(localArray2[3*i+1].x == 0);
        ASSERT(localArray2[3*i+1].y == 0);
        ASSERT(localArray2[3*i+2].x == 0);
        ASSERT(localArray2[3*i+2].y == 0);
    }

    VX_CALL( vxReleaseArray(&array) );
    ASSERT( array == 0);
}

TEST(Array, test_vxCopyArrayRangeWrite)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = 0;
        localArrayInit[i].y = 0;

        localArray[i].x = i;
        localArray[i].y = i;

        localArray2[3*i].x = 2*i;
        localArray2[3*i].y = 2*i;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)) );

    /* Write, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    }

    /* Check (MAP) */
    {
        vx_uint8 *p = NULL;
        vx_size stride = 0;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));

        ASSERT(stride >=  sizeof(vx_coordinates2d_t));
        ASSERT(p != NULL);
        for (i = N/2; i<N; i++)
        {
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->x == i);
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->y == i);
        }

        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    /* Write, COPY, Spacing */
    {
        vx_size stride = 3*sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray2[3*(N/2)];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));
    }
    /* Check (MAP) */
    {
        vx_uint8 *p = NULL;
        vx_size stride = 0;
        vx_map_id map_id;
        vx_uint32 flags = VX_NOGAP_X;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, flags));

        ASSERT(stride >=  sizeof(vx_coordinates2d_t));
        ASSERT(p != NULL);
        for (i = N/2; i<N; i++) {
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->x == 2*i);
            ASSERT(((vx_coordinates2d_t *)(p+stride*(i-N/2)))->y == 2*i);
        }

        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TEST(Array, test_vxCopyArrayRangeRead)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_coordinates2d_t localArray[N];
    vx_coordinates2d_t localArray2[N*3];
    vx_array array;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = i;
        localArrayInit[i].y = i;

        localArray[i].x = 0;
        localArray[i].y = 0;

        localArray2[3*i].x = 0;
        localArray2[3*i].y = 0;
        localArray2[3*i+1].x = 0;
        localArray2[3*i+1].y = 0;
        localArray2[3*i+2].x = 0;
        localArray2[3*i+2].y = 0;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)));

    /* READ, COPY, No spacing */
    {
        vx_size stride = sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray[N/2];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    }
    /* Check */
    for (i = 0; i < N/2; i++)
    {
        ASSERT(localArray[i].x == 0);
        ASSERT(localArray[i].y == 0);
    }
    for (i = N/2; i < N; i++)
    {
        ASSERT(localArray[i].x == i);
        ASSERT(localArray[i].y == i);
    }

    /* READ, COPY, Spacing */
    {
        vx_size stride = 3*sizeof(vx_coordinates2d_t);
        vx_coordinates2d_t *p = &localArray2[3*(N/2)];
        VX_CALL(vxCopyArrayRange(array, N/2, N, stride, (void *)p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));
    }
    /* Check */
    for (i = 0; i < N/2; i++)
    {
        ASSERT(localArray2[3*i].x == 0);
        ASSERT(localArray2[3*i].y == 0);
        ASSERT(localArray2[3*i+1].x == 0);
        ASSERT(localArray2[3*i+1].y == 0);
        ASSERT(localArray2[3*i+2].x == 0);
        ASSERT(localArray2[3*i+2].y == 0);
    }
    for (i = N/2; i < N; i++)
    {
        ASSERT(localArray2[3*i].x == i);
        ASSERT(localArray2[3*i].y == i);

        /* Unchanged in between */
        ASSERT(localArray2[3*i+1].x == 0);
        ASSERT(localArray2[3*i+1].y == 0);
        ASSERT(localArray2[3*i+2].x == 0);
        ASSERT(localArray2[3*i+2].y == 0);
    }

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TEST(Array, test_vxMapArrayRangeWrite)
{
    vx_context context = context_->vx_context_;
    vx_coordinates2d_t localArrayInit[N];
    vx_array array;
    int i;

    /* Initialization */
    for (i = 0; i < N; i++)
    {
        localArrayInit[i].x = i;
        localArrayInit[i].y = i;
    }

    ASSERT_VX_OBJECT(array = vxCreateArray(context, VX_TYPE_COORDINATES2D, N), VX_TYPE_ARRAY);
    VX_CALL(vxAddArrayItems(array, N, localArrayInit, sizeof(vx_coordinates2d_t)) );

    /* Map, READ_AND_WRITE mode*/
    {
        vx_size stride = 0;
        vx_coordinates2d_t *p = NULL;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride == sizeof(vx_coordinates2d_t));
        /* Check */
        for (i = 0; i < N/2; i++)
        {
            ASSERT(p[i].x == i + N/2);
            ASSERT(p[i].y == i + N/2);
        }
        /* Write into array */
        for (i = 0; i < N/2; i++)
        {
            p[i].x = 0;
            p[i].y = 0;
        }
        VX_CALL(vxUnmapArrayRange(array, map_id));
        /* Check */
        VX_CALL(vxMapArrayRange(array, 0, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        for (i = 0; i < N/2; i++)
        {
            ASSERT(p[i].x == i);
            ASSERT(p[i].y == i);
        }
        for (i = N/2; i < N; i++)
        {
            ASSERT(p[i].x == 0);
            ASSERT(p[i].y == 0);
        }
        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    /* Map, WRITE_ONLY mode */
    {
        vx_size stride = 0;
        vx_coordinates2d_t *p = NULL;
        vx_map_id map_id;
        VX_CALL(vxMapArrayRange(array, N/2, N, &map_id, &stride, (void **)&p, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        ASSERT(stride == sizeof(vx_coordinates2d_t));
        /* Write into array */
        for (i = 0; i < N/2; i++)
        {
            p[i].x = 0;
            p[i].y = 0;
        }
        VX_CALL(vxUnmapArrayRange(array, map_id));
        /* Check */
        VX_CALL(vxMapArrayRange(array, 0, N, &map_id, &stride, (void **)&p, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, VX_NOGAP_X));
        for (i = 0; i < N/2; i++)
        {
            ASSERT(p[i].x == i);
            ASSERT(p[i].y == i);
        }
        for (i = N/2; i < N; i++)
        {
            ASSERT(p[i].x == 0);
            ASSERT(p[i].y == 0);
        }
        VX_CALL(vxUnmapArrayRange(array, map_id));
    }

    VX_CALL(vxReleaseArray(&array));
    ASSERT(array == 0);
}

TESTCASE_TESTS(
    Array,
    test_vxCreateArray,
    DISABLED_testAccessCopyWrite,
    DISABLED_testAccessCopyRead,
    test_vxCopyArrayRangeRead,
    test_vxCopyArrayRangeWrite,
    test_vxMapArrayRangeWrite)
