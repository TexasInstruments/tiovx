/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#include <TI/tivx.h>

typedef vx_status (*tivxHostKernel_Fxn) (vx_context context);

typedef struct {
    tivxHostKernel_Fxn    add_kernel;
    tivxHostKernel_Fxn    remove_kernel;
} Tivx_Host_Kernel_List;

vx_status tivxAddKernelAbsDiff(vx_context context);
vx_status tivxAddKernelLut(vx_context context);
vx_status tivxAddKernelBitwise(vx_context context);
vx_status tivxAddKernelAdd(vx_context context);
vx_status tivxAddKernelSub(vx_context context);
vx_status tivxAddKernelThreshold(vx_context context);
vx_status tivxAddKernelErode3x3(vx_context context);
vx_status tivxAddKernelMultiply(vx_context context);
vx_status tivxAddKernelConvolve(vx_context context);
vx_status tivxAddKernelWarpAffine(vx_context context);
vx_status tivxAddKernelWarpPerspective(vx_context context);
vx_status tivxAddKernelScale(vx_context context);
vx_status tivxAddKernelAccumulate(vx_context context);
vx_status tivxAddKernelAccumulateSquare(vx_context context);
vx_status tivxAddKernelAccumulateWeighted(vx_context context);
vx_status tivxAddKernelRemap(vx_context context);
vx_status tivxAddKernelIntegralImage(vx_context context);
vx_status tivxAddKernelSobel3x3(vx_context context);
vx_status tivxAddKernelPhase(vx_context context);
vx_status tivxAddKernelMagnitude(vx_context context);
vx_status tivxAddKernelConvertDepth(vx_context context);
vx_status tivxAddKernelHistogram(vx_context context);
vx_status tivxAddKernelEqualizeHistogram(vx_context context);
vx_status tivxAddKernelMagnitude(vx_context context);
vx_status tivxAddKernelMinMaxLoc(vx_context context);
vx_status tivxAddKernelMeanStdDev(vx_context context);
vx_status tivxAddKernelChannelExtract(vx_context context);
vx_status tivxAddKernelChannelCombine(vx_context context);
vx_status tivxAddKernelColorConvert(vx_context context);
vx_status tivxAddKernelGaussianPyramid(vx_context context);

vx_status tivxRemoveKernelAbsDiff(vx_context context);
vx_status tivxRemoveKernelLut(vx_context context);
vx_status tivxRemoveKernelBitwise(vx_context context);
vx_status tivxRemoveKernelAdd(vx_context context);
vx_status tivxRemoveKernelSub(vx_context context);
vx_status tivxRemoveKernelThreshold(vx_context context);
vx_status tivxRemoveKernelErode3x3(vx_context context);
vx_status tivxRemoveKernelMultiply(vx_context context);
vx_status tivxRemoveKernelConvolve(vx_context context);
vx_status tivxRemoveKernelWarpAffine(vx_context context);
vx_status tivxRemoveKernelWarpPerspective(vx_context context);
vx_status tivxRemoveKernelScale(vx_context context);
vx_status tivxRemoveKernelAccumulate(vx_context context);
vx_status tivxRemoveKernelAccumulateSquare(vx_context context);
vx_status tivxRemoveKernelAccumulateWeighted(vx_context context);
vx_status tivxRemoveKernelRemap(vx_context context);
vx_status tivxRemoveKernelIntegralImage(vx_context context);
vx_status tivxRemoveKernelSobel3x3(vx_context context);
vx_status tivxRemoveKernelPhase(vx_context context);
vx_status tivxRemoveKernelMagnitude(vx_context context);
vx_status tivxRemoveKernelConvertDepth(vx_context context);
vx_status tivxRemoveKernelHistogram(vx_context context);
vx_status tivxRemoveKernelEqualizeHistogram(vx_context context);
vx_status tivxRemoveKernelMagnitude(vx_context context);
vx_status tivxRemoveKernelMinMaxLoc(vx_context context);
vx_status tivxRemoveKernelMeanStdDev(vx_context context);
vx_status tivxRemoveKernelChannelExtract(vx_context context);
vx_status tivxRemoveKernelChannelCombine(vx_context context);
vx_status tivxRemoveKernelColorConvert(vx_context context);
vx_status tivxRemoveKernelGaussianPyramid(vx_context context);

Tivx_Host_Kernel_List  gTivx_host_kernel_list[] = {
    {tivxAddKernelAbsDiff, tivxRemoveKernelAbsDiff},
    {tivxAddKernelLut, tivxRemoveKernelLut},
    {tivxAddKernelBitwise, tivxRemoveKernelBitwise},
    {tivxAddKernelAdd, tivxRemoveKernelAdd},
    {tivxAddKernelSub, tivxRemoveKernelSub},
    {tivxAddKernelThreshold, tivxRemoveKernelThreshold},
    {tivxAddKernelErode3x3, tivxRemoveKernelErode3x3},
    {tivxAddKernelMultiply, tivxRemoveKernelMultiply},
    {tivxAddKernelConvolve, tivxRemoveKernelConvolve},
    {tivxAddKernelWarpAffine, tivxRemoveKernelWarpAffine},
    {tivxAddKernelWarpPerspective, tivxRemoveKernelWarpPerspective},
    {tivxAddKernelScale, tivxRemoveKernelScale},
    {tivxAddKernelAccumulate, tivxRemoveKernelAccumulate},
    {tivxAddKernelAccumulateSquare, tivxRemoveKernelAccumulateSquare},
    {tivxAddKernelAccumulateWeighted, tivxRemoveKernelAccumulateWeighted},
    {tivxAddKernelRemap, tivxRemoveKernelRemap},
    {tivxAddKernelIntegralImage, tivxRemoveKernelIntegralImage},
    {tivxAddKernelSobel3x3, tivxRemoveKernelSobel3x3},
    {tivxAddKernelPhase, tivxRemoveKernelPhase},
    {tivxAddKernelMagnitude, tivxRemoveKernelMagnitude},
    {tivxAddKernelConvertDepth, tivxRemoveKernelConvertDepth},
    {tivxAddKernelHistogram, tivxRemoveKernelHistogram},
    {tivxAddKernelEqualizeHistogram, tivxRemoveKernelEqualizeHistogram},
    {tivxAddKernelMinMaxLoc, tivxRemoveKernelMinMaxLoc},
    {tivxAddKernelMeanStdDev, tivxRemoveKernelMeanStdDev},
    {tivxAddKernelChannelExtract, tivxRemoveKernelChannelExtract},
    {tivxAddKernelChannelCombine, tivxRemoveKernelChannelCombine},
    {tivxAddKernelColorConvert, tivxRemoveKernelColorConvert},
    {tivxAddKernelGaussianPyramid, tivxRemoveKernelGaussianPyramid}
};

static vx_status tivxPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List); i ++)
    {
        if (gTivx_host_kernel_list[i].add_kernel)
        {
            status = gTivx_host_kernel_list[i].add_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

static vx_status tivxUnPublishKernels(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;

    for (i = 0; i <
        sizeof(gTivx_host_kernel_list)/sizeof(Tivx_Host_Kernel_List); i ++)
    {
        if (gTivx_host_kernel_list[i].remove_kernel)
        {
            status = gTivx_host_kernel_list[i].remove_kernel(context);
        }

        if (VX_SUCCESS != status)
        {
            break;
        }
    }

    return status;
}

void tivxRegisterOpenVXCoreKernels()
{
    tivxRegisterModule(TIVX_MODULE_NAME, tivxPublishKernels, tivxUnPublishKernels);
}

void tivxUnRegisterOpenVXCoreKernels()
{
    tivxUnRegisterModule(TIVX_MODULE_NAME);
}
