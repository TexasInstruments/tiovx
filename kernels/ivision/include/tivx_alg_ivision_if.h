/*
 *******************************************************************************
 *
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_ALG_IVISION_IF_H_
#define TIVX_ALG_IVISION_IF_H_

#include <VX/vx.h>
#include <common/xdais_types.h> /* Adding to support vx_uint32/vx_int32 data types */
#include <ivision.h>
#include <ti/xdais/ires.h>

#ifdef __cplusplus
extern "C" {
#endif


Void *tivxAlgiVisionCreate(const IVISION_Fxns *fxns, IALG_Params *pAlgPrms);

vx_int32 tivxAlgiVisionDelete(Void *algHandle);

vx_int32 tivxAlgiVisionProcess(Void *algHandle,
    IVISION_InBufs *inBufs,
    IVISION_OutBufs *outBufs,
    IVISION_InArgs *inArgs,
    IVISION_OutArgs *outArgs);

vx_int32 tivxAlgiVisionControl(Void *algHandle,
    IALG_Cmd cmd,
    const IALG_Params *inParams,
    IALG_Params *outParams);

Void tivxAlgiVisionActivate(Void *algHandle);
Void tivxAlgiVisionDeActivate(Void *algHandle);

#ifdef __cplusplus
}
#endif

#endif

