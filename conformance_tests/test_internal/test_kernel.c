#include "test_engine/test.h"
#include <TI/tivx_debug.h>
#include <VX/vx.h>
#include <VX/vxu.h>
#include "TI/tivx.h"
#include <VX/vx_types.h>
#include <vx_internal.h>

#include <vx_context.h>

TESTCASE(tivxInternalKernel, CT_VXContext, ct_setup_vx_context, 0)

TEST(tivxInternalKernel, negativeTestOwnKernelGetTarget)
{
    vx_context context = context_->vx_context_;
    const char *target_string = NULL;
    vx_image kernel = NULL;
    vx_kernel kernel_temp = NULL;
    vx_reference ref = NULL;

    ASSERT_VX_OBJECT(ref = ownCreateReference(context, (vx_enum)VX_TYPE_KERNEL, (vx_enum)VX_EXTERNAL, &context->base), VX_TYPE_KERNEL);
    kernel_temp = vxCastRefAsKernel(ref, NULL);
    ASSERT((vx_enum)TIVX_TARGET_ID_INVALID == ownKernelGetTarget((vx_kernel)kernel_temp, target_string));

    ASSERT_VX_OBJECT(kernel = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);
    ASSERT((vx_enum)TIVX_TARGET_ID_INVALID == ownKernelGetTarget((vx_kernel)kernel, target_string));

    VX_CALL(vxReleaseImage(&kernel));
    VX_CALL(vxReleaseKernel(&kernel_temp));
}

TEST(tivxInternalKernel, negativeTestownKernelGetDefaultTarget)
{
    vx_context context = context_->vx_context_;
    vx_image kernel = NULL;

    ASSERT_VX_OBJECT(kernel = vxCreateImage(context, 16, 16, VX_DF_IMAGE_U8), VX_TYPE_IMAGE);

    ASSERT((vx_enum)TIVX_TARGET_ID_INVALID == ownKernelGetDefaultTarget((vx_kernel)kernel));

    VX_CALL(vxReleaseImage(&kernel));
}

TESTCASE_TESTS(
    tivxInternalKernel,
    negativeTestOwnKernelGetTarget,
    negativeTestownKernelGetDefaultTarget
)