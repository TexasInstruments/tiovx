/*

 * Copyright (c) 2016-2017 The Khronos Group Inc.
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

static void own_init_scalar_value(vx_enum type, scalar_val* val, vx_bool variant)
{
    switch (type)
    {
    case VX_TYPE_CHAR:     val->chr = variant == vx_true_e ? 1 : 2; break;
    case VX_TYPE_INT8:     val->s08 = variant == vx_true_e ? 2 : 3; break;
    case VX_TYPE_UINT8:    val->u08 = variant == vx_true_e ? 3 : 4; break;
    case VX_TYPE_INT16:    val->s16 = variant == vx_true_e ? 4 : 5; break;
    case VX_TYPE_UINT16:   val->u16 = variant == vx_true_e ? 5 : 6; break;
    case VX_TYPE_INT32:    val->s32 = variant == vx_true_e ? 6 : 7; break;
    case VX_TYPE_UINT32:   val->u32 = variant == vx_true_e ? 7 : 8; break;
    case VX_TYPE_INT64:    val->s64 = variant == vx_true_e ? 8 : 9; break;
    case VX_TYPE_UINT64:   val->u64 = variant == vx_true_e ? 9 : 10; break;
    case VX_TYPE_FLOAT32:  val->f32 = variant == vx_true_e ? 1.5f : 9.9f; break;
    case VX_TYPE_FLOAT64:  val->f64 = variant == vx_true_e ? 1.5 : 9.9; break;
    case VX_TYPE_ENUM:     val->enm = variant == vx_true_e ? VX_BORDER_CONSTANT : VX_BORDER_REPLICATE; break;
    case VX_TYPE_SIZE:     val->size = variant == vx_true_e ? 10 : 999; break;
    case VX_TYPE_DF_IMAGE: val->fcc = variant == vx_true_e ? VX_DF_IMAGE_RGB : VX_DF_IMAGE_U8; break;
    case VX_TYPE_BOOL:     val->boolean = variant == vx_true_e ? vx_true_e : vx_false_e; break;

    default:
        FAIL("Unsupported type: (%.4s)", &type);
    }
    return;
}

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

    own_init_scalar_value(ref_type, &ref, vx_true_e);

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

    own_init_scalar_value(ref_type, &ref, vx_true_e);

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

    own_init_scalar_value(type, &val1, vx_true_e);
    own_init_scalar_value(type, &val2, vx_false_e);

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
