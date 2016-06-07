/*
 * Copyright (c) 2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#include <math.h>
#include <float.h>
#include <VX/vx.h>

#include "test_engine/test.h"

TESTCASE(Scalar, CT_VXContext, ct_setup_vx_context, 0)

typedef union
{
    vx_char     chr;
    vx_int8     s08;
    vx_uint8    u08;
    vx_int16    s16;
    vx_uint16   u16;
    vx_int32    s32;
    vx_uint32   u32;
    vx_int64    s64;
    vx_uint64   u64;
    vx_float32  f32;
    vx_float64  f64;
    vx_enum     enm;
    vx_size     size;
    vx_df_image fcc;
    vx_bool     boolean;
    vx_uint8    data[8];

} scalar_val;

typedef struct
{
    const char* name;
    vx_enum data_type;
} format_arg;

TEST_WITH_ARG(Scalar, testCreateScalar, format_arg,
    ARG_ENUM(VX_TYPE_CHAR),
    ARG_ENUM(VX_TYPE_INT8),
    ARG_ENUM(VX_TYPE_UINT8),
    ARG_ENUM(VX_TYPE_INT16),
    ARG_ENUM(VX_TYPE_UINT16),
    ARG_ENUM(VX_TYPE_INT32),
    ARG_ENUM(VX_TYPE_UINT32),
    ARG_ENUM(VX_TYPE_INT64),
    ARG_ENUM(VX_TYPE_UINT64),
    ARG_ENUM(VX_TYPE_FLOAT32),
    ARG_ENUM(VX_TYPE_FLOAT64),
    ARG_ENUM(VX_TYPE_ENUM),
    ARG_ENUM(VX_TYPE_SIZE),
    ARG_ENUM(VX_TYPE_DF_IMAGE),
    ARG_ENUM(VX_TYPE_BOOL)
    )
{
    vx_context context = context_->vx_context_;
    vx_scalar  scalar = 0;
    vx_enum    ref_type = arg_->data_type;
    scalar_val ref;

    ref.data[0] = 0x01;
    ref.data[1] = 0x23;
    ref.data[2] = 0x45;
    ref.data[3] = 0x67;
    ref.data[4] = 0x89;
    ref.data[5] = 0xab;
    ref.data[6] = 0xcd;
    ref.data[7] = 0xef;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, ref_type, &ref), VX_TYPE_SCALAR);

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(scalar == 0);

    return;
} /* testCreateScalar() */

TEST_WITH_ARG(Scalar, testQueryScalar, format_arg,
    ARG_ENUM(VX_TYPE_CHAR),
    ARG_ENUM(VX_TYPE_INT8),
    ARG_ENUM(VX_TYPE_UINT8),
    ARG_ENUM(VX_TYPE_INT16),
    ARG_ENUM(VX_TYPE_UINT16),
    ARG_ENUM(VX_TYPE_INT32),
    ARG_ENUM(VX_TYPE_UINT32),
    ARG_ENUM(VX_TYPE_INT64),
    ARG_ENUM(VX_TYPE_UINT64),
    ARG_ENUM(VX_TYPE_FLOAT32),
    ARG_ENUM(VX_TYPE_FLOAT64),
    ARG_ENUM(VX_TYPE_ENUM),
    ARG_ENUM(VX_TYPE_SIZE),
    ARG_ENUM(VX_TYPE_DF_IMAGE),
    ARG_ENUM(VX_TYPE_BOOL)
    )
{
    vx_context context   = context_->vx_context_;
    vx_scalar  scalar    = 0;
    vx_enum    ref_type = arg_->data_type;
    vx_enum    tst_type = 0;
    scalar_val ref;

    ref.data[0] = 0x01;
    ref.data[1] = 0x23;
    ref.data[2] = 0x45;
    ref.data[3] = 0x67;
    ref.data[4] = 0x89;
    ref.data[5] = 0xab;
    ref.data[6] = 0xcd;
    ref.data[7] = 0xef;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, ref_type, &ref), VX_TYPE_SCALAR);

    VX_CALL(vxQueryScalar(scalar, VX_SCALAR_TYPE, &tst_type, sizeof(tst_type)));
    ASSERT_EQ_INT(ref_type, tst_type);

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(scalar == 0);

    return;
} /* testQueryScalar() */

TEST_WITH_ARG(Scalar, testCopyScalar, format_arg,
    ARG_ENUM(VX_TYPE_CHAR),
    ARG_ENUM(VX_TYPE_INT8),
    ARG_ENUM(VX_TYPE_UINT8),
    ARG_ENUM(VX_TYPE_INT16),
    ARG_ENUM(VX_TYPE_UINT16),
    ARG_ENUM(VX_TYPE_INT32),
    ARG_ENUM(VX_TYPE_UINT32),
    ARG_ENUM(VX_TYPE_INT64),
    ARG_ENUM(VX_TYPE_UINT64),
    ARG_ENUM(VX_TYPE_FLOAT32),
    ARG_ENUM(VX_TYPE_FLOAT64),
    ARG_ENUM(VX_TYPE_ENUM),
    ARG_ENUM(VX_TYPE_SIZE),
    ARG_ENUM(VX_TYPE_DF_IMAGE),
    ARG_ENUM(VX_TYPE_BOOL)
    )
{
    vx_context context   = context_->vx_context_;
    vx_scalar  scalar    = 0;
    vx_enum    type = arg_->data_type;
    scalar_val val1;
    scalar_val val2;
    scalar_val tst;

    val1.data[0] = 0x01;
    val1.data[1] = 0x23;
    val1.data[2] = 0x45;
    val1.data[3] = 0x67;
    val1.data[4] = 0x89;
    val1.data[5] = 0xab;
    val1.data[6] = 0xcd;
    val1.data[7] = 0xef;

    val2.data[0] = 0xef;
    val2.data[1] = 0xcd;
    val2.data[2] = 0xab;
    val2.data[3] = 0x89;
    val2.data[4] = 0x67;
    val2.data[5] = 0x45;
    val2.data[6] = 0x23;
    val2.data[7] = 0x01;

    ASSERT_VX_OBJECT(scalar = vxCreateScalar(context, type, &val1), VX_TYPE_SCALAR);

    VX_CALL(vxCopyScalar(scalar, &tst, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    switch (type)
    {
    case VX_TYPE_CHAR:
        ASSERT_EQ_INT(val1.chr, tst.chr);
        break;

    case VX_TYPE_INT8:
        ASSERT_EQ_INT(val1.s08, tst.s08);
        break;

    case VX_TYPE_UINT8:
        ASSERT_EQ_INT(val1.u08, tst.u08);
        break;

    case VX_TYPE_INT16:
        ASSERT_EQ_INT(val1.s16, tst.s16);
        break;

    case VX_TYPE_UINT16:
        ASSERT_EQ_INT(val1.u16, tst.u16);
        break;

    case VX_TYPE_INT32:
        ASSERT_EQ_INT(val1.s32, tst.s32);
        break;

    case VX_TYPE_UINT32:
        ASSERT_EQ_INT(val1.u32, tst.u32);
        break;

    case VX_TYPE_INT64:
        ASSERT_EQ_INT(val1.s64, tst.s64);
        break;

    case VX_TYPE_UINT64:
        ASSERT_EQ_INT(val1.u64, tst.u64);
        break;

    case VX_TYPE_FLOAT32:
        ASSERT(fabs(val1.f32 - tst.f32) < 0.000001f);
        break;

    case VX_TYPE_FLOAT64:
        ASSERT(fabs(val1.f64 - tst.f64) < 0.000001f);
        break;

    case VX_TYPE_DF_IMAGE:
        ASSERT_EQ_INT(val1.fcc, tst.fcc);
        break;

    case VX_TYPE_ENUM:
        ASSERT_EQ_INT(val1.enm, tst.enm);
        break;

    case VX_TYPE_SIZE:
        ASSERT_EQ_INT(val1.size, tst.size);
        break;

    case VX_TYPE_BOOL:
        ASSERT_EQ_INT(val1.boolean, tst.boolean);
        break;

    default:
        FAIL("Unsupported type: (%.4s)", &type);
        break;
    }

    /* change scalar value */
    VX_CALL(vxCopyScalar(scalar, &val2, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST));

    /* read value back */
    VX_CALL(vxCopyScalar(scalar, &tst, VX_READ_ONLY, VX_MEMORY_TYPE_HOST));

    switch (type)
    {
    case VX_TYPE_CHAR:
        ASSERT_EQ_INT(val2.chr, tst.chr);
        break;

    case VX_TYPE_INT8:
        ASSERT_EQ_INT(val2.s08, tst.s08);
        break;

    case VX_TYPE_UINT8:
        ASSERT_EQ_INT(val2.u08, tst.u08);
        break;

    case VX_TYPE_INT16:
        ASSERT_EQ_INT(val2.s16, tst.s16);
        break;

    case VX_TYPE_UINT16:
        ASSERT_EQ_INT(val2.u16, tst.u16);
        break;

    case VX_TYPE_INT32:
        ASSERT_EQ_INT(val2.s32, tst.s32);
        break;

    case VX_TYPE_UINT32:
        ASSERT_EQ_INT(val2.u32, tst.u32);
        break;

    case VX_TYPE_INT64:
        ASSERT_EQ_INT(val2.s64, tst.s64);
        break;

    case VX_TYPE_UINT64:
        ASSERT_EQ_INT(val2.u64, tst.u64);
        break;

    case VX_TYPE_FLOAT32:
        ASSERT(fabs(val2.f32 - tst.f32) < 0.000001f);
        break;

    case VX_TYPE_FLOAT64:
        ASSERT(fabs(val2.f64 - tst.f64) < 0.000001f);
        break;

    case VX_TYPE_DF_IMAGE:
        ASSERT_EQ_INT(val2.fcc, tst.fcc);
        break;

    case VX_TYPE_ENUM:
        ASSERT_EQ_INT(val2.enm, tst.enm);
        break;

    case VX_TYPE_SIZE:
        ASSERT_EQ_INT(val2.size, tst.size);
        break;

    case VX_TYPE_BOOL:
        ASSERT_EQ_INT(val2.boolean, tst.boolean);
        break;

    default:
        FAIL("Unsupported type: (%.4s)", &type);
        break;
    }

    VX_CALL(vxReleaseScalar(&scalar));

    ASSERT(scalar == 0);

    return;
} /* testCopyScalar() */

TESTCASE_TESTS(Scalar,
    testCreateScalar,
    testQueryScalar,
    testCopyScalar
    )
