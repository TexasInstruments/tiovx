/*
 *******************************************************************************
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#include <tivx_alg_ivision_if.h>
#include <TI/tivx_mem.h>

typedef struct IM_Fxns
{
  IVISION_Fxns * fxns;
}IM_Fxns;

/* Local Functions */
Void *tivxAlgiVisionCreateUseLinkMem(
    const IVISION_Fxns *fxns, IALG_Params *pAlgPrms);
vx_int32 tivxAlgiVisionDeleteUseLinkMem(Void *algHandle);
vx_int32 tivxAlgiVisionAllocMem(vx_uint32 numMemRec, IALG_MemRec  *memRec);
vx_int32 tivxAlgiVisionFreeMem(vx_uint32 numMemRec, IALG_MemRec *memRec);

/**
 *******************************************************************************
 * \brief This function allocates memory for IVISION algorothm
 *
 * \param  numMemRec         [IN] Number of objects
 * \param  memRec            [IN] pointer to the memory records
 *
 * \return  VX_SUCCESS on success
 *
 *******************************************************************************
 */
vx_int32 tivxAlgiVisionAllocMem(vx_uint32 numMemRec, IALG_MemRec  *memRec)
{
    vx_uint32 memRecId;
    vx_status status = VX_SUCCESS;

    for (memRecId = 0u; memRecId < numMemRec; memRecId++)
    {
        switch(memRec[memRecId].space)
        {
            default:
                status = VX_FAILURE;
                break;
            case IALG_EPROG:
            case IALG_IPROG:
            case IALG_ESDATA:
            case IALG_EXTERNAL:
                memRec[memRecId].base = tivxMemAlloc(memRec[memRecId].size,
                    TIVX_MEM_EXTERNAL);
                break;
            case IALG_DARAM0:
            case IALG_DARAM1:
            case IALG_SARAM:
            case IALG_SARAM1:
            case IALG_DARAM2:
            case IALG_SARAM2:
                if(memRec[memRecId].attrs==IALG_SCRATCH)
                {
                    memRec[memRecId].base = tivxMemAlloc(
                        memRec[memRecId].size, TIVX_MEM_INTERNAL_L2);
                }
                else
                {
                    memRec[memRecId].base = tivxMemAlloc(
                        memRec[memRecId].size, TIVX_MEM_EXTERNAL);
                }
                break;
        }

        if (NULL == memRec[memRecId].base)
        {
            status = VX_FAILURE;
            break;
        }
    }

    return (status);
}


/**
 *******************************************************************************
 *
 * \brief This function frees memory for the IVISION algorithm
 *
 * \param  numMemRec         [IN] Number of objects
 * \param  memRec            [IN] pointer to the memory records
 *
 * \return  VX_SUCCESS on success
 *
 *******************************************************************************
 */
vx_int32 tivxAlgiVisionFreeMem(vx_uint32 numMemRec, IALG_MemRec *memRec)
{
    vx_uint32 memRecId;
    vx_status status = VX_SUCCESS;

    for (memRecId = 0; memRecId < numMemRec; memRecId++)
    {
        switch(memRec[memRecId].space)
        {
            case IALG_EPROG:
            case IALG_IPROG:
            case IALG_ESDATA:
            case IALG_EXTERNAL:
                if (NULL != memRec[memRecId].base)
                {
                    tivxMemFree(memRec[memRecId].base, memRec[memRecId].size,
                        TIVX_MEM_EXTERNAL);
                }
                break;
            case IALG_DARAM0:
            case IALG_DARAM1:
            case IALG_SARAM:
            case IALG_SARAM1:
            case IALG_DARAM2:
            case IALG_SARAM2:
                if (NULL != memRec[memRecId].base)
                {
                    if(memRec[memRecId].attrs==IALG_SCRATCH)
                    {
                        tivxMemFree(memRec[memRecId].base,
                            memRec[memRecId].size, TIVX_MEM_INTERNAL_L2);
                    }
                    else
                    {
                        tivxMemFree(memRec[memRecId].base,
                            memRec[memRecId].size, TIVX_MEM_EXTERNAL);
                    }
                }
                break;
        }
    }

    return status;
}

Void *tivxAlgiVisionCreateUseLinkMem(
    const IVISION_Fxns *fxns, IALG_Params *pAlgPrms)
{
    vx_uint32 numMemRec;
    IALG_MemRec *memRec;
    IM_Fxns *algHandle = NULL;
    vx_status status = VX_SUCCESS;

    numMemRec = fxns->ialg.algNumAlloc();

    /*
     * Allocate memory for the records. These are NOT the actual memory of
     * tha algorithm
     */
    memRec = tivxMemAlloc(numMemRec * sizeof(IALG_MemRec), TIVX_MEM_EXTERNAL);

    if(NULL != memRec)
    {
        status = fxns->ialg.algAlloc(pAlgPrms, NULL, memRec);

        if(status==IALG_EOK)
        {
            status = tivxAlgiVisionAllocMem(numMemRec, memRec);
            if(status==IALG_EOK)
            {
                algHandle = (IM_Fxns *)memRec[0].base;
                status = fxns->ialg.algInit(
                    (IALG_Handle)(algHandle),
                    memRec,
                    NULL,
                    pAlgPrms);

                if(status != IALG_EOK)
                {
                    tivxAlgiVisionDeleteUseLinkMem(algHandle);
                    algHandle = NULL;
                }
            }
        }

        tivxMemFree(memRec, numMemRec * sizeof(IALG_MemRec), TIVX_MEM_EXTERNAL);
    }

    return algHandle;
}

vx_int32 tivxAlgiVisionDeleteUseLinkMem(Void *algHandle)
{
    vx_uint32 numMemRec;
    IALG_MemRec   *memRec;
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = 0;

    numMemRec = ivision->fxns->ialg.algNumAlloc();

    /*
     * Allocate memory for the records. These are NOT the actual memory of
     * tha algorithm
     */
    memRec = tivxMemAlloc(numMemRec * sizeof(IALG_MemRec), TIVX_MEM_EXTERNAL);

    if(memRec != NULL)
    {
        status = ivision->fxns->ialg.algFree(algHandle, memRec);

        if(status==IALG_EOK)
        {
            status = tivxAlgiVisionFreeMem(numMemRec, memRec);
        }

        tivxMemFree(memRec, numMemRec * sizeof(IALG_MemRec), TIVX_MEM_EXTERNAL);
    }

    return status;
}

Void *tivxAlgiVisionCreate(const IVISION_Fxns *fxns, IALG_Params *pAlgPrms)
{
    return tivxAlgiVisionCreateUseLinkMem(fxns, pAlgPrms);
}

vx_int32 tivxAlgiVisionDelete(Void *algHandle)
{
    return tivxAlgiVisionDeleteUseLinkMem(algHandle);
}

vx_int32 tivxAlgiVisionProcess(Void *algHandle,
        IVISION_InBufs *inBufs,
        IVISION_OutBufs *outBufs,
        IVISION_InArgs *inArgs,
        IVISION_OutArgs *outArgs)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = VX_SUCCESS;

    ivision->fxns->ialg.algActivate((IALG_Handle)ivision);

    status = ivision->fxns->algProcess(
                    (IVISION_Handle)ivision,
                    inBufs,
                    outBufs,
                    inArgs,
                    outArgs);


    ivision->fxns->ialg.algDeactivate((IALG_Handle)ivision);

    return status;
}

vx_int32 tivxAlgiVisionControl(Void *algHandle,
                    IALG_Cmd cmd,
                    const IALG_Params *inParams,
                    IALG_Params *outParams)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = VX_SUCCESS;

    status = ivision->fxns->algControl(
                    (IVISION_Handle)ivision,
                    cmd,
                    inParams,
                    outParams);

    return status;
}

Void tivxAlgiVisionActivate(Void *algHandle)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;

    ivision->fxns->ialg.algActivate((IALG_Handle)ivision);
}

Void tivxAlgiVisionDeActivate(Void *algHandle)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;

    ivision->fxns->ialg.algDeactivate((IALG_Handle)ivision);
}
