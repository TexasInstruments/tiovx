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
#include <TI/tivx.h>
#include <TI/tivx_mem.h>
#include <TI/tivx_debug.h>
#include <tivx_kernels_common_utils.h>

typedef struct
{
    IVISION_Fxns * fxns;
}IM_Fxns;

/* Local Functions */
static vx_int32 tivxAlgiVisionDeleteAlg(void *algHandle);
static vx_int32 tivxAlgiVisionAllocMem(vx_uint32 numMemRec, IALG_MemRec  *memRec);
static vx_int32 tivxAlgiVisionFreeMem(vx_uint32 numMemRec, IALG_MemRec *memRec);
static vx_int32 tivxAlgiVisionGetHeapId(vx_uint32 space, vx_uint32 attrs, vx_uint32 *heap_id);
static void * activeHandle = NULL;

static vx_int32 tivxAlgiVisionGetHeapId(vx_uint32 space, vx_uint32 attrs, vx_uint32 *heap_id)
{
    vx_int32 status = (vx_status)VX_SUCCESS;

    *heap_id = (vx_enum)TIVX_MEM_EXTERNAL;
    switch(space)
    {
        default:
            status = (vx_status)VX_FAILURE;
            break;
        case (vx_uint32)IALG_EPROG:
        case (vx_uint32)IALG_IPROG:
        case (vx_uint32)IALG_ESDATA:
        case (vx_uint32)IALG_EXTERNAL:
            if((vx_enum)attrs==(vx_enum)IALG_SCRATCH)
            {
               *heap_id = (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH;
            }
            else
            {
               *heap_id = (vx_enum)TIVX_MEM_EXTERNAL;
            }
            break;
        case (vx_uint32)IALG_EXTERNAL_NON_CACHEABLE:
            if((vx_enum)attrs==(vx_enum)IALG_SCRATCH)
            {
               *heap_id = (vx_enum)TIVX_MEM_EXTERNAL_SCRATCH_NON_CACHEABLE;  
            }
            else
            {
               *heap_id = (vx_enum)TIVX_MEM_EXTERNAL_PERSISTENT_NON_CACHEABLE;
            }
            break;
        case (vx_uint32)IALG_DARAM0:
        case (vx_uint32)IALG_DARAM1:
        case (vx_uint32)IALG_SARAM0:
        case (vx_uint32)IALG_SARAM1:
        case (vx_uint32)IALG_DARAM2:
        case (vx_uint32)IALG_SARAM2:
            if((vx_enum)attrs==(vx_enum)IALG_SCRATCH)
            {
                if((vx_enum)space==(vx_enum)IALG_DARAM0)
                {
                  *heap_id = (vx_enum)TIVX_MEM_INTERNAL_L1;
                }
                else
                if((vx_enum)space==(vx_enum)IALG_DARAM1)
                {
#ifdef TDAX
                    tivx_cpu_id_e cpuId;
                    /* EVE does not have any L2 memory so DARAM1 space must be mapped to external memory instead */
                    cpuId= (tivx_cpu_id_e)tivxGetSelfCpuId();
                    if ( (cpuId== (vx_enum)TIVX_CPU_ID_EVE1) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE2) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE3) || (cpuId== (vx_enum)TIVX_CPU_ID_EVE4) )
                    {
                        *heap_id = (vx_enum)TIVX_MEM_EXTERNAL;
                    }
                    else
                    {
                        *heap_id = (vx_enum)TIVX_MEM_INTERNAL_L2;
                    }
#else
                    *heap_id = (vx_enum)TIVX_MEM_INTERNAL_L2;
#endif
                }
                else
                {
                  *heap_id = (vx_enum)TIVX_MEM_INTERNAL_L3;
                }
            }
            else
            {
                *heap_id = (vx_enum)TIVX_MEM_EXTERNAL;
            }
            break;
    }
    return status;
}

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
static vx_int32 tivxAlgiVisionAllocMem(vx_uint32 numMemRec, IALG_MemRec  *memRec)
{
    vx_uint32 memRecId, heap_id;
    vx_status status = (vx_status)VX_SUCCESS;

    for (memRecId = 0u; memRecId < numMemRec; memRecId++)
    {
        VX_PRINT(VX_ZONE_INFO, "Allocating memory record %d @ space = %d, size = %d, align = %d ... \n",
                 memRecId, memRec[memRecId].space, memRec[memRecId].size, memRec[memRecId].alignment);

        status = tivxAlgiVisionGetHeapId((vx_uint32)memRec[memRecId].space, (vx_uint32)memRec[memRecId].attrs, &heap_id);
        if(status==(vx_status)VX_SUCCESS)
        {
            memRec[memRecId].base = tivxMemAlloc(memRec[memRecId].size, (vx_int32)heap_id);
        }

        VX_PRINT(VX_ZONE_INFO, "Allocated memory record %d @ space = %d and size = %d, addr = %p ... \n",
                 memRecId, memRec[memRecId].space, memRec[memRecId].size, memRec[memRecId].base);

        if (NULL == memRec[memRecId].base)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Allocate memory record %d @ space = %d and size = %d !!! \n",
                     memRecId, memRec[memRecId].space, memRec[memRecId].size);
            status = (vx_status)VX_FAILURE;
            break;
        }
    }
/* Free the records that ahs SCRATCH attribute and that are in L2 so their space can be re-used for another iVision algorithm */
#ifndef HOST_EMULATION
    if(status==(vx_status)VX_SUCCESS)
    {
        for (memRecId = 0u; memRecId < numMemRec; memRecId++)
        {
            status = tivxAlgiVisionGetHeapId((vx_uint32)memRec[memRecId].space, (vx_uint32)memRec[memRecId].attrs, &heap_id);
            if(status==(vx_status)VX_SUCCESS)
            {
                if (((vx_enum)heap_id== (vx_enum)TIVX_MEM_INTERNAL_L2) && (memRec[memRecId].attrs== IALG_SCRATCH))
                {
                    tivxMemFree(memRec[memRecId].base, memRec[memRecId].size, (vx_int32)heap_id);
                }
            }
        }
    }
#endif

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
static vx_int32 tivxAlgiVisionFreeMem(vx_uint32 numMemRec, IALG_MemRec *memRec)
{
    vx_uint32 memRecId, heap_id;
    vx_status status = (vx_status)VX_SUCCESS;

    for (memRecId = 0; memRecId < numMemRec; memRecId++)
    {
        status = tivxAlgiVisionGetHeapId((vx_uint32)memRec[memRecId].space, (vx_uint32)memRec[memRecId].attrs, &heap_id);
        if(status==(vx_status)VX_SUCCESS)
        {
#ifndef HOST_EMULATION
            if (((vx_enum)heap_id != (vx_enum)TIVX_MEM_INTERNAL_L2) || (memRec[memRecId].attrs!= IALG_SCRATCH))
            {
                tivxMemFree(memRec[memRecId].base, memRec[memRecId].size, (vx_int32)heap_id);
            }
#else
            tivxMemFree(memRec[memRecId].base, memRec[memRecId].size, (vx_int32)heap_id);
#endif
        }
    }

    return status;
}

static vx_int32 tivxAlgiVisionDeleteAlg(void *algHandle)
{
    vx_uint32 numMemRec;
    IALG_MemRec   *memRec;
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = 0;

    numMemRec = (vx_uint32)ivision->fxns->ialg.algNumAlloc();

    /*
     * Allocate memory for the records. These are NOT the actual memory of
     * tha algorithm
     */
    memRec = tivxMemAlloc(numMemRec * sizeof(IALG_MemRec), (vx_enum)TIVX_MEM_EXTERNAL);

    if(memRec != NULL)
    {
        status = ivision->fxns->ialg.algFree(algHandle, memRec);

        if(status==IALG_EOK)
        {
            status = tivxAlgiVisionFreeMem(numMemRec, memRec);
        }

        tivxMemFree(memRec, numMemRec * sizeof(IALG_MemRec), (vx_enum)TIVX_MEM_EXTERNAL);
    }

    return status;
}

#undef PRINT_IVISION_MEMTAB_REQUESTS

void *tivxAlgiVisionCreate(const IVISION_Fxns *fxns, const IALG_Params *pAlgPrms)
{
    vx_uint32 numMemRec;
    IALG_MemRec *memRec;
    IM_Fxns *algHandle = NULL;
    vx_status status = (vx_status)VX_SUCCESS;

    #if defined(PRINT_IVISION_MEMTAB_REQUESTS)
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

    VX_PRINT(VX_ZONE_INFO, "Calling ialg.algNumAlloc ...\n");

    numMemRec = (vx_uint32)fxns->ialg.algNumAlloc();

    VX_PRINT(VX_ZONE_INFO, "Allocating %d memory records ...\n", numMemRec);

    /*
     * Allocate memory for the records. These are NOT the actual memory of
     * tha algorithm
     */
    memRec = tivxMemAlloc(numMemRec * sizeof(IALG_MemRec), (vx_enum)TIVX_MEM_EXTERNAL);

    if(NULL != memRec)
    {
        VX_PRINT(VX_ZONE_INFO, "Calling ialg.algAlloc ...\n");

        status = fxns->ialg.algAlloc(pAlgPrms, NULL, memRec);

        if(status==IALG_EOK)
        {
            status = tivxAlgiVisionAllocMem(numMemRec, memRec);
            if(status==IALG_EOK)
            {
                tivx_cpu_id_e cpuId= (tivx_cpu_id_e)tivxGetSelfCpuId();

                VX_PRINT(VX_ZONE_INFO, "Calling ialg.algInit ...\n");

                algHandle = (IM_Fxns *)memRec[0].base;
                status = fxns->ialg.algInit((IALG_Handle)(algHandle),
                                            memRec,
                                            NULL,
                                            pAlgPrms);

                if(status != IALG_EOK)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Calling ialg.algInit failed with status = %d\n", status);
                    tivxAlgiVisionDeleteAlg(algHandle);
                    algHandle = NULL;
                }
                else if ( (vx_enum)cpuId==(vx_enum)TIVX_CPU_ID_DSP1)
                {
                    /* Temporary workaround as this first record needs to be written back from cache before being dma-ed by alg activate implemented by the algorithm */
                    tivxCheckStatus(&status, tivxMemBufferUnmap(memRec[0].base, memRec[0].size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
                }
                #if defined (SOC_J721E)
                else if ( (vx_enum)cpuId==(vx_enum)TIVX_CPU_ID_DSP2)
                {
                    /* Temporary workaround as this first record needs to be written back from cache before being dma-ed by alg activate implemented by the algorithm */
                    tivxCheckStatus(&status, tivxMemBufferUnmap(memRec[0].base, memRec[0].size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
                }
                #endif
                else
                {
                    /* do nothing */
                }
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxAlgiVisionAllocMem Failed\n", numMemRec);
            }
        }
        else
        {
          VX_PRINT(VX_ZONE_ERROR, "Calling ialg.algAlloc failed with status = %d\n", status);
        }

        tivxMemFree(memRec, numMemRec * sizeof(IALG_MemRec), (vx_enum)TIVX_MEM_EXTERNAL);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate %d memory records !!!\n", numMemRec);
    }

    if(algHandle != NULL)
    {
      VX_PRINT(VX_ZONE_INFO, "Created AlgiVision handle.\n");
    }

    #if defined(PRINT_IVISION_MEMTAB_REQUESTS)
    tivx_clr_debug_zone(VX_ZONE_INFO);
    #endif

    return algHandle;
}

vx_int32 tivxAlgiVisionDelete(void *algHandle)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    ivision->fxns->ialg.algDeactivate((IALG_Handle)ivision);
    if(activeHandle == algHandle)
    {
        activeHandle = NULL;
    }
    return tivxAlgiVisionDeleteAlg(algHandle);
}

vx_int32 tivxAlgiVisionProcess(void *algHandle,
    IVISION_InBufs *inBufs,
    IVISION_OutBufs *outBufs,
    IVISION_InArgs *inArgs,
    IVISION_OutArgs *outArgs,
    vx_uint32 optAlgAct)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = (vx_status)VX_SUCCESS;

#define MANAGE_ACTIVATE_DEACTIVATE_IN_TIOVX 0
#if MANAGE_ACTIVATE_DEACTIVATE_IN_TIOVX
    if((activeHandle != algHandle) || (optAlgAct == 0))
    {
        if(activeHandle != NULL)
        {
            IM_Fxns *prevIvision = (IM_Fxns *)activeHandle;
            prevIvision->fxns->ialg.algDeactivate((IALG_Handle)prevIvision);
        }
        ivision->fxns->ialg.algActivate((IALG_Handle)ivision);
        if(optAlgAct == 1)
        {
            activeHandle = algHandle;
        }
        else
        {
            activeHandle = NULL;
        }
    }
#endif
    status = ivision->fxns->algProcess((IVISION_Handle)ivision,
                                       inBufs,
                                       outBufs,
                                       inArgs,
                                       outArgs);
#if MANAGE_ACTIVATE_DEACTIVATE_IN_TIOVX
    if(optAlgAct == 0)
    {
        ivision->fxns->ialg.algDeactivate((IALG_Handle)ivision);
    }
#endif
    return status;
}

vx_int32 tivxAlgiVisionControl(void *algHandle,
    IALG_Cmd cmd,
    const IALG_Params *inParams,
    IALG_Params *outParams)
{
    IM_Fxns *ivision = (IM_Fxns *)algHandle;
    vx_status status = (vx_status)VX_SUCCESS;

    status = ivision->fxns->algControl((IVISION_Handle)ivision,
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
