/*
*
* Copyright (c) 2017 Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/



#include <tivx_alg_ivision_if.h>
#include <TI/tivx_mem.h>

typedef struct IM_Fxns
{
  IVISION_Fxns * fxns;
}IM_Fxns;

/* Local Functions */
vx_int32 tivxAlgiVisionDeleteAlg(void *algHandle);
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
            default:
                status = VX_FAILURE;
                break;
        }
    }

    return status;
}

vx_int32 tivxAlgiVisionDeleteAlg(void *algHandle)
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

void *tivxAlgiVisionCreate(const IVISION_Fxns *fxns, IALG_Params *pAlgPrms)
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
                    tivxAlgiVisionDeleteAlg(algHandle);
                    algHandle = NULL;
                }
            }
        }

        tivxMemFree(memRec, numMemRec * sizeof(IALG_MemRec), TIVX_MEM_EXTERNAL);
    }

    return algHandle;
}

vx_int32 tivxAlgiVisionDelete(void *algHandle)
{
    return tivxAlgiVisionDeleteAlg(algHandle);
}

vx_int32 tivxAlgiVisionProcess(void *algHandle,
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

vx_int32 tivxAlgiVisionControl(void *algHandle,
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

void tivxAlgiVisionActivate(void *algHandle)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;

    ivision->fxns->ialg.algActivate((IALG_Handle)ivision);
}

void tivxAlgiVisionDeActivate(void *algHandle)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;

    ivision->fxns->ialg.algDeactivate((IALG_Handle)ivision);
}
