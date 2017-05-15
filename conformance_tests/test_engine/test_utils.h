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

#ifndef __VX_CT_TEST_UTILS_H__
#define __VX_CT_TEST_UTILS_H__

#include <VX/vx.h>

#define MAXPATHLENGTH           (512u)

#define VX_CALL(fn_call) ASSERT_EQ_VX_STATUS(VX_SUCCESS, fn_call)
#define VX_CALL_(ret_code, fn_call) ASSERT_EQ_VX_STATUS_AT_(ret_code, VX_SUCCESS, fn_call, __FUNCTION__, __FILE__, __LINE__)
#define VX_CALL_RET(fn_call) ASSERT_EQ_VX_STATUS_AT_(return VX_FAILURE, VX_SUCCESS, fn_call, __FUNCTION__, __FILE__, __LINE__)

const char* ct_vx_status_to_str(vx_status s);
const char* ct_vx_type_to_str(enum vx_type_e type);

int ct_assert_reference_impl(vx_reference ref, enum vx_type_e type, vx_status expect_status, const char* str_ref, const char* func, const char* file, const int line);

#define EXPECT_VX_REFERENCE(ref) ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)
#define ASSERT_VX_REFERENCE(ref) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_REFERENCE_(ret_error, ref) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {ret_error;}}while(0)

#define EXPECT_VX_REFERENCE_AT(ref, func, file, line) ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, func, file, line)
#define ASSERT_VX_REFERENCE_AT(ref, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, func, file, line)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_REFERENCE_AT_(ret_error, ref, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, VX_SUCCESS, #ref, func, file, line)) {} else {ret_error;}}while(0)

#define EXPECT_VX_OBJECT(ref, type) ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)
#define ASSERT_VX_OBJECT(ref, type) do{ if (ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_OBJECT_(ret_error, ref, type) do{ if (ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {ret_error;}}while(0)

#define EXPECT_VX_OBJECT_AT(ref, type, func, file, line) ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, func, file, line)
#define ASSERT_VX_OBJECT_AT(ref, type, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, func, file, line)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_OBJECT_AT_(ret_error, ref, type, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), (type), VX_SUCCESS, #ref, func, file, line)) {} else {ret_error;}}while(0)

#define EXPECT_VX_ERROR(ref, error) ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, __FUNCTION__, __FILE__, __LINE__)
#define ASSERT_VX_ERROR(ref, error) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_ERROR_(ret_error, ref, error) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, __FUNCTION__, __FILE__, __LINE__)) {} else {ret_error;}}while(0)

#define EXPECT_VX_ERROR_AT(ref, error, func, file, line) ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, func, file, line)
#define ASSERT_VX_ERROR_AT(ref, error, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, func, file, line)) {} else {CT_DO_FAIL;}}while(0)
#define ASSERT_VX_ERROR_AT_(ret_error, ref, error, func, file, line) do{ if (ct_assert_reference_impl((vx_reference)(ref), VX_TYPE_KHRONOS_OBJECT_END, (error), #ref, func, file, line)) {} else {ret_error;}}while(0)

#define ASSERT_NE_VX_STATUS(val1, val2)                                 \
    do {                                                                \
        vx_status s0 = (vx_status)(val1);                               \
        vx_status s1 = (vx_status)(val2);                               \
        if (s0 != s1) {/*passed*/} else                                 \
        {                                                               \
            CT_RecordFailureAtFormat("Expected: %s != %s\n\t"           \
                "Actual: %s vs %s"                                      \
                , __FUNCTION__, __FILE__, __LINE__, #val1, #val2,       \
                ct_vx_status_to_str(s0), ct_vx_status_to_str(s1));      \
            {CT_DO_FAIL;}                                               \
        }                                                               \
    }while(0)

#define ASSERT_EQ_VX_STATUS(expected, actual)                           \
    do {                                                                \
        vx_status s0 = (vx_status)(expected);                           \
        vx_status s1 = (vx_status)(actual);                             \
        if (s0 == s1) {/*passed*/} else                                 \
        {                                                               \
            CT_RecordFailureAtFormat("Expected: %s == %s\n\t"           \
                "Actual: %s != %s"                                      \
                , __FUNCTION__, __FILE__, __LINE__, #expected, #actual, \
                ct_vx_status_to_str(s0), ct_vx_status_to_str(s1));      \
            {CT_DO_FAIL;}                                               \
        }                                                               \
    }while(0)

#define ASSERT_EQ_VX_STATUS_AT_(rexpr, expected, actual, func, file, line)  \
    do {                                                                    \
        vx_status s0 = (vx_status)(expected);                               \
        vx_status s1 = (vx_status)(actual);                                 \
        if (s0 == s1) {/*passed*/} else                                     \
        {                                                                   \
            CT_RecordFailureAtFormat("Expected: %s == %s\n\t"               \
                "Actual: %s != %s"                                          \
                , func, file, line, #expected, #actual,                     \
                ct_vx_status_to_str(s0), ct_vx_status_to_str(s1));          \
            {rexpr;}                                                        \
        }                                                                   \
    }while(0)


#define EXPECT_NE_VX_STATUS(val1, val2)                                 \
    do {                                                                \
        vx_status s0 = (vx_status)(val1);                               \
        vx_status s1 = (vx_status)(val2);                               \
        if (s0 != s1) {/*passed*/} else                                 \
        {                                                               \
            CT_RecordFailureAtFormat("Expected: %s != %s\n\t"           \
                "Actual: %s vs %s"                                      \
                , __FUNCTION__, __FILE__, __LINE__, #val1, #val2,       \
                ct_vx_status_to_str(s0), ct_vx_status_to_str(s1));      \
        }                                                               \
    }while(0)

#define EXPECT_EQ_VX_STATUS(expected, actual)                           \
    do {                                                                \
        vx_status s0 = (vx_status)(expected);                           \
        vx_status s1 = (vx_status)(actual);                             \
        if (s0 == s1) {/*passed*/} else                                 \
        {                                                               \
            CT_RecordFailureAtFormat("Expected: %s == %s\n\t"           \
                "Actual: %s != %s"                                      \
                , __FUNCTION__, __FILE__, __LINE__, #expected, #actual, \
                ct_vx_status_to_str(s0), ct_vx_status_to_str(s1));      \
        }                                                               \
    }while(0)


typedef struct CT_VXContext_ {
    vx_context vx_context_;
    vx_uint32  vx_context_base_references_;
    vx_uint32  vx_context_base_modules_;
    vx_uint32  vx_context_base_kernels_;
} CT_VXContext;

void* ct_setup_vx_context();
void ct_create_global_vx_context();
void ct_release_global_vx_context();

#define ADD_VX_BORDERS(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=0", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=1", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 1 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=127", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 127 }} })), \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_CONSTANT=255", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 255 }} }))

#define ADD_VX_BORDERS_REQUIRE_UNDEFINED_ONLY(testArgName, nextmacro, ...) \
    CT_EXPAND(nextmacro(testArgName "/VX_BORDER_UNDEFINED", __VA_ARGS__, { VX_BORDER_UNDEFINED, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/DISABLED_VX_BORDER_REPLICATE", __VA_ARGS__, { VX_BORDER_REPLICATE, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/DISABLED_VX_BORDER_CONSTANT=0", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 0 }} })), \
    CT_EXPAND(nextmacro(testArgName "/DISABLED_VX_BORDER_CONSTANT=1", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 1 }} })), \
    CT_EXPAND(nextmacro(testArgName "/DISABLED_VX_BORDER_CONSTANT=127", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 127 }} })), \
    CT_EXPAND(nextmacro(testArgName "/DISABLED_VX_BORDER_CONSTANT=255", __VA_ARGS__, { VX_BORDER_CONSTANT, {{ 255 }} }))

void ct_fill_image_random_impl(vx_image image, uint64_t* seed, const char* func, const char* file, const int line);
#define ct_fill_image_random(image, seed) ct_fill_image_random_impl(image, seed, __FUNCTION__, __FILE__, __LINE__)

vx_image ct_clone_image_impl(vx_image image, vx_graph graph, const char* func, const char* file, const int line);
#define ct_clone_image(image, graph) ct_clone_image_impl(image, graph, __FUNCTION__, __FILE__, __LINE__)

vx_image ct_create_similar_image_impl(vx_image image, const char* func, const char* file, const int line);
#define ct_create_similar_image(image) ct_create_similar_image_impl(image, __FUNCTION__, __FILE__, __LINE__)

vx_image ct_create_similar_image_with_format_impl(vx_image image, vx_df_image format, const char* func, const char* file, const int line);
#define ct_create_similar_image_with_format(image, format) ct_create_similar_image_with_format_impl(image, format, __FUNCTION__, __FILE__, __LINE__)

struct InputGenerator
{
    const char* fileName; // and testArgName;
    vx_image (*generator)(vx_context context, const char* fileName);
};


vx_status ct_dump_vx_image_info(vx_image image);

uint32_t ct_floor_u32_no_overflow(float v);

#define CT_RNG_INIT(rng, seed) ((rng) = (seed) ? (seed) : (uint64_t)(int64_t)(-1))
#define CT_RNG_NEXT(rng) ((rng) = ((uint64_t)(uint32_t)(rng)*4164903690U + ((rng) >> 32)))
#define CT_RNG_NEXT_INT(rng, a, b) (int)((uint32_t)CT_RNG_NEXT(rng) % ((b) - (a)) + (a))
#define CT_RNG_NEXT_BOOL(rng)      CT_RNG_NEXT_INT(rng, 0, 2)
#define CT_RNG_NEXT_REAL(rng, a, b) ((uint32_t)CT_RNG_NEXT(rng)*(2.3283064365386963e-10*((b) - (a))) + (a))

#define CT_CAST_U8(x)  (uint8_t)((x) < 0 ? 0 : (x) > 255 ? 255 : (x))
#define CT_CAST_U16(x) (uint16_t)((x) < 0 ? 0 : (x) > 65535 ? 65535 : (x))
#define CT_CAST_S16(x) (int16_t)((x) < -32768 ? -32768 : (x) > 32767 ? 32767 : (x))
#define CT_CAST_U32(x) (uint32_t)((x) < 0 ? 0 : (x) > ((1ll << 32) - 1) ? ((1ll << 32) - 1) : (x))
#define CT_CAST_S32(x) (int32_t)(x)

#define CT_SATURATE_U8(x) CT_CAST_U8(x)
#define CT_SATURATE_U16(x) CT_CAST_U16(x)
#define CT_SATURATE_S16(x) CT_CAST_S16(x)
#define CT_SATURATE_U32(x) CT_CAST_U32(x)
#define CT_SATURATE_S32(x) CT_CAST_S32(x)

#define CT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define CT_MAX(a, b) ((a) > (b) ? (a) : (b))

void ct_update_progress(int i, int niters);

// generates random number using logarithmic scale (convenient for image size)
float ct_log_rng(uint64_t* rng, float minlog2, float maxlog2);
int ct_roundf(float val);
int ct_scalar_as_int(vx_scalar val);
vx_scalar ct_scalar_from_int(vx_context ctx, vx_enum valtype, int val);

vx_enum ct_read_array(vx_array src, void** dst, vx_size* _capacity_bytes,
                      vx_size* _arrsize, vx_size* _arrcap);

uint8_t ct_clamp_8u(int32_t v);

int ct_check_any_size();
void ct_set_check_any_size(int flag);

void ct_destroy_vx_context(void **pContext);

char *ct_get_test_file_path();

void *ct_alloc_mem(size_t size);

void ct_free_mem(void *ptr);

void ct_memset(void *ptr, vx_uint8 c, size_t);
void *ct_calloc(size_t nmemb, size_t size);

#endif // __VX_CT_TEST_UTILS_H__
