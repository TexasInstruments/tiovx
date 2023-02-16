/*
*
* Copyright (c) 2018 Texas Instruments Incorporated
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

#include <stdio.h>
#include <TI/tivx.h>
#include <TI/j7_tidl.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include "itidl_ti.h"
#include "tivx_tidl_trace.h"
#include "tivx_kernels_target_utils.h"
#include "tidl_custom.h"

#ifndef x86_64
#include "c7x.h"
#include <ti/osal/HwiP.h>
#if defined(SOC_AM62A)
#include <ti/kernel/freertos/portable/TI_CGT/c7x/Cache.h>
#endif

#if defined(SOC_AM62A)
/* #define DISABLE_PREEMPTION */
/* #define DISABLE_INTERRUPTS_DURING_PROCESS */
#else
/* #define DISABLE_INTERRUPTS_DURING_PROCESS */
#endif
#endif

/* #define TIVX_TIDL_TARGET_DEBUG */

#define TIDL_COPY_NETWORK_BUF

typedef struct
{
    IVISION_BufDesc     inBufDesc[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc     outBufDesc[TIDL_MAX_ALG_OUT_BUFS];

    IVISION_BufDesc    *inBufDescList[TIDL_MAX_ALG_IN_BUFS];
    IVISION_BufDesc    *outBufDescList[TIDL_MAX_ALG_OUT_BUFS];

    IVISION_InBufs      inBufs;
    IVISION_OutBufs     outBufs;

    TIDL_InArgs         *inArgs;
    TIDL_outArgs        *outArgs;

    TIDL_CreateParams   createParams;

    tivxTIDLJ7Params    tidlParams;

    void                *tidlNet;
    vx_uint32            netSize;

    void                *algHandle;

    tivxTIDLTraceDataManager mgr;

} tivxTIDLObj;

static char target_name[][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_DSP_C7_1_PRI_1,
    TIVX_TARGET_DSP_C7_1_PRI_2,
    TIVX_TARGET_DSP_C7_1_PRI_3,
    TIVX_TARGET_DSP_C7_1_PRI_4,
    TIVX_TARGET_DSP_C7_1_PRI_5,
    TIVX_TARGET_DSP_C7_1_PRI_6,
    TIVX_TARGET_DSP_C7_1_PRI_7,
    TIVX_TARGET_DSP_C7_1_PRI_8,
#if defined(SOC_J784S4)
    TIVX_TARGET_DSP_C7_2_PRI_1,
    TIVX_TARGET_DSP_C7_2_PRI_2,
    TIVX_TARGET_DSP_C7_2_PRI_3,
    TIVX_TARGET_DSP_C7_2_PRI_4,
    TIVX_TARGET_DSP_C7_2_PRI_5,
    TIVX_TARGET_DSP_C7_2_PRI_6,
    TIVX_TARGET_DSP_C7_2_PRI_7,
    TIVX_TARGET_DSP_C7_2_PRI_8,
    TIVX_TARGET_DSP_C7_3_PRI_1,
    TIVX_TARGET_DSP_C7_3_PRI_2,
    TIVX_TARGET_DSP_C7_3_PRI_3,
    TIVX_TARGET_DSP_C7_3_PRI_4,
    TIVX_TARGET_DSP_C7_3_PRI_5,
    TIVX_TARGET_DSP_C7_3_PRI_6,
    TIVX_TARGET_DSP_C7_3_PRI_7,
    TIVX_TARGET_DSP_C7_3_PRI_8,
    TIVX_TARGET_DSP_C7_4_PRI_1,
    TIVX_TARGET_DSP_C7_4_PRI_2,
    TIVX_TARGET_DSP_C7_4_PRI_3,
    TIVX_TARGET_DSP_C7_4_PRI_4,
    TIVX_TARGET_DSP_C7_4_PRI_5,
    TIVX_TARGET_DSP_C7_4_PRI_6,
    TIVX_TARGET_DSP_C7_4_PRI_7,
    TIVX_TARGET_DSP_C7_4_PRI_8,
#endif
};

#define TIDL_MAX_TARGETS (sizeof(target_name)/sizeof(target_name[0]))

static tivx_target_kernel vx_tidl_target_kernel[TIDL_MAX_TARGETS] = {NULL};

/* OpenVX Node callbacks */
static vx_status VX_CALLBACK tivxKernelTIDLCreate(tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLProcess(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLDelete(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

/* TIDL App function callbacks */
#if !defined(DISABLE_PREEMPTION)
static int32_t TIDL_lockInterrupts();
static void TIDL_unlockInterrupts(int32_t oldIntState);
#endif

static int32_t tivxKernelTIDLLog(const char * format, va_list va_args_ptr);
static int32_t tivxKernelTIDLDumpToFile(const char * fileName, void * addr, int32_t size, void * tracePtr);

/* File Private functions */
static void tivxTIDLFreeMem(tivxTIDLObj *tidlObj);
static vx_status testChecksum(void *dataPtr, uint8_t *refQC, vx_int32 data_size, uint32_t loc);
static void getQC(uint8_t *pIn, uint8_t *pOut, int32_t inSize);
static int32_t tidl_AllocNetInputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig);
static int32_t tidl_AllocNetOutputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig);

#if !defined(DISABLE_PREEMPTION)
/*
 * Following static lock/unlock functions passed as function pointers to TIDL to internally
 * disable and enable interrupts around critical section.
 */
static int32_t TIDL_lockInterrupts()
{
    int32_t oldIntState = 0;
#ifndef x86_64
    oldIntState = HwiP_disable();
#endif
    return oldIntState;
}

static void TIDL_unlockInterrupts(int32_t oldIntState)
{
#ifndef x86_64
    HwiP_restore(oldIntState);
#endif
}
#endif

static int32_t tidl_AllocNetInputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig)
{
  uint16_t numBuffs = 0;

  /* Currently only one input buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numInputBuf; numBuffs++)
  {
    BufDescList[numBuffs].numPlanes                          = 1;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
    BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

    /* This has to be width + horizontal padding */
    BufDescList[numBuffs].bufPlanes[0].width                 = pConfig->inWidth[numBuffs] + pConfig->inPadL[numBuffs] + pConfig->inPadR[numBuffs];
    /* This has to be numCh * (height + vertical padding) */
    BufDescList[numBuffs].bufPlanes[0].height                = pConfig->inNumChannels[numBuffs] * (pConfig->inHeight[numBuffs]  + pConfig->inPadT[numBuffs] + pConfig->inPadB[numBuffs]);
    /* This has to be just width */
    BufDescList[numBuffs].bufPlanes[0].frameROI.width        = pConfig->inWidth[numBuffs];
    /* This has to be just height */
    BufDescList[numBuffs].bufPlanes[0].frameROI.height       = pConfig->inHeight[numBuffs];

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].reserved[0]     = pConfig->inDataId[numBuffs];

    /* This comes from tidl_io_xxx.txt file (inDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
    BufDescList[numBuffs].bufferId = pConfig->inDataId[numBuffs];
  }
  return numBuffs;
}

static int32_t tidl_AllocNetOutputMem(IVISION_BufDesc *BufDescList, sTIDL_IOBufDesc_t *pConfig)
{
  uint16_t numBuffs = 0;

  /* Currently only one output buffer supported */
  for(numBuffs = 0; numBuffs < pConfig->numOutputBuf; numBuffs++)
  {
      BufDescList[numBuffs].numPlanes                          = 1;
      BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.x    = 0;
      BufDescList[numBuffs].bufPlanes[0].frameROI.topLeft.y    = 0;

      /* This requires output width + horizontal padding */
      BufDescList[numBuffs].bufPlanes[0].width                 = pConfig->outWidth[numBuffs] + pConfig->outPadL[numBuffs] + pConfig->outPadR[numBuffs];
      /* This requires numOutCh * (output height + vertial padding) */
      BufDescList[numBuffs].bufPlanes[0].height                = pConfig->outNumChannels[numBuffs] * (pConfig->outHeight[numBuffs] + pConfig->outPadT[numBuffs] + pConfig->outPadB[numBuffs]);
      /* This requires just output width */
      BufDescList[numBuffs].bufPlanes[0].frameROI.width        = pConfig->outWidth[numBuffs];
      /* This requires just output height */
      BufDescList[numBuffs].bufPlanes[0].frameROI.height       = pConfig->outHeight[numBuffs];

      /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
      BufDescList[numBuffs].reserved[0]      = pConfig->outDataId[numBuffs];

      /* This comes from tidl_io_xxx.txt file (outDataId), not sure how to pass it. Currently hardcoding for Jacinto Net */
      BufDescList[numBuffs].bufferId         = pConfig->outDataId[numBuffs];
  }
  return numBuffs;
}

static vx_status VX_CALLBACK tivxKernelTIDLProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxTIDLObj *tidlObj;
    uint32_t i, size;

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    uint32_t oldIntState;
    #endif

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    /* disabling interrupts when doing TIDL processing
     *
     * suspect some stability issue due to interrupt handling,
     * until stability issue is root caused disabling interrupts
     * */
    oldIntState = HwiP_disable();
    #endif

    for (i = 0U; i < num_params; i ++)
    {
        /* The parameter at i == 5 is optional and is used to provide a buffer for trace data */
        if ((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tidlObj, &size);

        if (((vx_status)VX_SUCCESS != status) || (NULL == tidlObj) ||  (sizeof(tivxTIDLObj) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivx_obj_desc_tensor_t *inTensor;
        tivx_obj_desc_tensor_t *outTensor;
        tivx_obj_desc_user_data_object_t *inArgs;
        tivx_obj_desc_user_data_object_t *outArgs;
        tivx_obj_desc_user_data_object_t *traceData;

        void *in_tensor_target_ptr;
        void *out_tensor_target_ptr;
        void *in_args_target_ptr;
        void *out_args_target_ptr;
        void *trace_data_target_ptr = NULL;

        /* IMPORTANT! inArgs is assumed to be available at index 3 */
        inArgs   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX];

        in_args_target_ptr = tivxMemShared2TargetPtr(&inArgs->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(in_args_target_ptr, inArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tidlObj->inArgs = in_args_target_ptr;

        /* IMPORTANT! outArgs is assumed to be available at index 4 */
        outArgs  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX];

        out_args_target_ptr = tivxMemShared2TargetPtr(&outArgs->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(out_args_target_ptr, outArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        tidlObj->outArgs = out_args_target_ptr;

        tivxTIDLTraceDataClear(&tidlObj->mgr);

        traceData  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX];
        if((tidlObj->createParams.traceWriteLevel > 0) && (traceData != NULL))
        {
          trace_data_target_ptr = tivxMemShared2TargetPtr(&traceData->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(trace_data_target_ptr, traceData->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY));

          tivxTIDLTraceDataInit(&tidlObj->mgr, trace_data_target_ptr, traceData->mem_size);
        }

        /* Idx 0 - config data,
           Idx 1 - network data,
           Idx 2 - create parameters,
           Idx 3 - inArgs,
           Idx 4 - outArgs,
           Idx 5 - traceData,
           Idx 6 - input tensor */
        uint32_t in_tensor_idx = TIVX_KERNEL_TIDL_IN_FIRST_TENSOR;

        /* Idx N - output tensors, where N = Idx 2 + number of input tensors */
        uint32_t out_tensor_idx = in_tensor_idx + tidlObj->inBufs.numBufs;
        uint32_t id;

        for(id = 0; id < tidlObj->inBufs.numBufs; id++) {
            inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
            in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            tidlObj->inBufDesc[id].bufPlanes[0].buf = in_tensor_target_ptr;
        }

        for(id = 0; id < tidlObj->outBufs.numBufs; id++) {
            outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
            out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
            tidlObj->outBufDesc[id].bufPlanes[0].buf = out_tensor_target_ptr;
        }

#if defined(SOC_AM62A)
#ifndef x86_64
        Cache_wbInvL1dAll();
#endif
#endif

        status = tivxAlgiVisionProcess
                 (
                    tidlObj->algHandle,
                    &tidlObj->inBufs,
                    &tidlObj->outBufs,
                    (IVISION_InArgs  *)tidlObj->inArgs,
                    (IVISION_OutArgs *)tidlObj->outArgs,
                    tidlObj->tidlParams.optimize_ivision_activation
                 );

#if defined(SOC_AM62A)
#ifndef x86_64
        Cache_wbInvL1dAll();
#endif
#endif

        tivxCheckStatus(&status, tivxMemBufferUnmap(in_args_target_ptr, inArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(out_args_target_ptr, outArgs->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        if((tidlObj->createParams.traceWriteLevel > 0) && (traceData != NULL))
        {
           tivxTIDLTraceWriteEOB(&tidlObj->mgr);

           tivxCheckStatus(&status, tivxMemBufferUnmap(trace_data_target_ptr, traceData->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY));
        }

        for(id = 0; id < tidlObj->inBufs.numBufs; id++) {
            inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
            in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        for(id = 0; id < tidlObj->outBufs.numBufs; id++) {
            outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
            out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
    }

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    HwiP_restore(oldIntState);
    #endif

    return (status);
}

static int32_t tivxKernelTIDLLog(const char * format, va_list va_args_ptr)
{
    static char buf[1024];

    vsnprintf(buf, 1024, format, va_args_ptr);

    printf(buf);

    return 0;
}

static int32_t tivxKernelTIDLDumpToFile(const char * fileName, void * addr, int32_t size, void * tracePtr)
{
    int32_t arg_errors = 0;
    tivxTIDLTraceDataManager *mgr = (tivxTIDLTraceDataManager *)tracePtr;

    if(mgr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "tracePtr set to NULL");
        arg_errors++;
    }
    if(fileName == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "fileName set to NULL");
        arg_errors++;
    }
    if(addr == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "addr set to NULL");
        arg_errors++;
    }
    if(size == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "size set to 0");
        arg_errors++;
    }

    if(arg_errors == 0)
    {
        if(mgr->current != NULL)
        {
            tivxTIDLTraceHeader header;

            strcpy(header.fileName, fileName);
            header.size   = size;
            header.offset = mgr->current_capacity + sizeof(tivxTIDLTraceHeader);

            tivxTIDLTraceSetData(mgr, (uint8_t *)&header, sizeof(tivxTIDLTraceHeader));
            tivxTIDLTraceSetData(mgr, addr, size);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "mgr->current is not initialized");
        }
    }

    return 0;
}
#ifdef x86_64
/* Udma_init for target flow is done part of App Common Init
   Udma is not used in in Host emulation mode of other module, but TIDL
   Has flows which uses UDMA. So intilizing here Specific to TIDL Init.
   This flow is controlled via flowCtrl in create Params
*/
#if defined(SOC_AM62A)
#include <ti/drv/udma/dmautils/udma_standalone/udma.h>
#else
#include <ti/drv/udma/udma.h>
#endif
static struct Udma_DrvObj  x86udmaDrvObj;
static uint64_t tidlVirtToPhyAddrConversion(const void *virtAddr, uint32_t chNum, void *appData);
static void tidlX86Printf(const char *str);
static void * tidlX86UdmaInit( void);

static uint64_t tidlVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
    return (uint64_t)virtAddr;
}
static void tidlX86Printf(const char *str)
{
}
static void * tidlX86UdmaInit( void)
{
    static uint8_t firstCall = 1;
    if(firstCall)
    {
        Udma_InitPrms initPrms;
        UdmaInitPrms_init(UDMA_INST_ID_MAIN_0, &initPrms);
        initPrms.printFxn = &tidlX86Printf;
        #if !defined(SOC_AM62A)
        initPrms.skipGlobalEventReg = 1;
        #endif
        initPrms.virtToPhyFxn = tidlVirtToPhyAddrConversion;
        Udma_init(&x86udmaDrvObj, &initPrms);
        firstCall = 0;
    }
    return &x86udmaDrvObj;
}
#endif

static vx_status VX_CALLBACK tivxKernelTIDLCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  uint16_t num_params,
  void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivx_obj_desc_user_data_object_t *config;
    tivx_obj_desc_user_data_object_t *network;
    tivx_obj_desc_user_data_object_t *createParams;

    tivxTIDLObj *tidlObj = NULL;

    void *config_target_ptr = NULL;
    void *network_target_ptr = NULL;
    void *create_params_target_ptr = NULL;

    uint32_t i;

    #ifdef TIVX_TIDL_TARGET_DEBUG
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

    for (i = 0U; i < num_params; i ++)
    {
        /* The parameter at i == 5 is optional and is used to provide a buffer for trace data */
        if ((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        /* IMPORTANT! Config data is assumed to be available at index 0 */
        config    = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_CONFIG_IDX];

        /* IMPORTANT! Network data is assumed to be available at index 1 */
        network   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_NETWORK_IDX];

        /* IMPORTANT! Create params is assumed to be available at index 2 */
        createParams   = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];

        tidlObj = tivxMemAlloc(sizeof(tivxTIDLObj), (vx_enum)TIVX_MEM_EXTERNAL);

        if (NULL != tidlObj)
        {
            memset(tidlObj, 0, sizeof(tivxTIDLObj));
        }
        else
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          config_target_ptr = tivxMemShared2TargetPtr(&config->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

          memcpy(&tidlObj->tidlParams, config_target_ptr, sizeof(tivxTIDLJ7Params));

          if(tidlObj->tidlParams.compute_config_checksum == 1)
          {
            status = testChecksum(&tidlObj->tidlParams.ioBufDesc, &tidlObj->tidlParams.config_checksum[0], sizeof(sTIDL_IOBufDesc_t), 0);
          }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            #ifdef TIDL_COPY_NETWORK_BUF
            tidlObj->tidlNet = tivxMemAlloc(network->mem_size, (vx_enum)TIVX_MEM_EXTERNAL);
            tidlObj->netSize = network->mem_size;
            if (NULL == tidlObj->tidlNet)
            {
                status = (vx_status)VX_ERROR_NO_MEMORY;
            }
            #else
            tidlObj->tidlNet = NULL;
            tidlObj->netSize = 0;
            #endif
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          network_target_ptr = tivxMemShared2TargetPtr(&network->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(network_target_ptr, network->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

          #ifdef TIDL_COPY_NETWORK_BUF
          memcpy(tidlObj->tidlNet, network_target_ptr, network->mem_size);
          #else
          tidlObj->tidlNet = network_target_ptr;
          tidlObj->netSize = network->mem_size;
          #endif

          if(tidlObj->tidlParams.compute_network_checksum == 1)
          {
            sTIDL_Network_t *pNet = (sTIDL_Network_t *)network_target_ptr;
            uint8_t *pPerfInfo = (uint8_t *)network_target_ptr + pNet->dataFlowInfo;

            VX_PRINT(VX_ZONE_INFO, "tidlObj->netSize = %d\n", tidlObj->netSize);
            VX_PRINT(VX_ZONE_INFO, "pNet->dataFlowInfo = %d \n", pNet->dataFlowInfo);

            status = testChecksum(pPerfInfo, &tidlObj->tidlParams.network_checksum[0], tidlObj->netSize - pNet->dataFlowInfo, 0);
          }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
          create_params_target_ptr = tivxMemShared2TargetPtr(&createParams->mem_ptr);
          tivxCheckStatus(&status, tivxMemBufferMap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            memcpy(&tidlObj->createParams, create_params_target_ptr, sizeof(TIDL_CreateParams));
            tidlObj->createParams.tracePtr = (void *)&tidlObj->mgr;

            #if defined(DISABLE_PREEMPTION)
            VX_PRINT(VX_ZONE_INFO, "Preemption is disabled\n");
            #else
            tidlObj->createParams.pFxnLock = TIDL_lockInterrupts;
            tidlObj->createParams.pFxnUnLock = TIDL_unlockInterrupts;
            status = tivxGetTargetKernelTargetId(kernel, &tidlObj->createParams.targetPriority);
            VX_PRINT(VX_ZONE_INFO, "Enabling preemption\n");
            #endif
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            tivx_mem_stats l1_stats;
            tivx_mem_stats l2_stats;
            tivx_mem_stats l3_stats;

            /* reset scratch heap offset to zero by doing a dummy free */
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L1);
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemFree(NULL, 0, (vx_enum)TIVX_MEM_INTERNAL_L3);

            tivxMemStats(&l1_stats, (vx_enum)TIVX_MEM_INTERNAL_L1);
            tivxMemStats(&l2_stats, (vx_enum)TIVX_MEM_INTERNAL_L2);
            tivxMemStats(&l3_stats, (vx_enum)TIVX_MEM_INTERNAL_L3);

            VX_PRINT(VX_ZONE_INFO, "L1 = %d KB, L2 = %d KB, L3 = %d KB\n",
                l1_stats.free_size/1024,
                l2_stats.free_size/1024,
                l3_stats.free_size/1024
                );
#ifdef x86_64
            tidlObj->createParams.udmaDrvObj = tidlX86UdmaInit();
#else
            tidlObj->createParams.udmaDrvObj = tivxPlatformGetDmaObj();
#endif

            tidlObj->createParams.net = (sTIDL_Network_t *)tidlObj->tidlNet;

            tidlObj->createParams.TIDLVprintf = tivxKernelTIDLLog;

            tidlObj->createParams.TIDLWriteBinToFile = tivxKernelTIDLDumpToFile;
            tidlObj->createParams.TIDL_CustomLayerProcess = TIDL_customLayerProcess;

            if(TIDL_NET_VERSION != tidlObj->createParams.net->netVersion)
            {
                VX_PRINT(VX_ZONE_ERROR, "Network version - 0x%08X, Expected version - 0x%08X\n",
                    tidlObj->createParams.net->netVersion,
                    TIDL_NET_VERSION
                );

                status = VX_FAILURE;
            }

            if ((vx_status)VX_SUCCESS == status)
            {
                tidlObj->algHandle = tivxAlgiVisionCreate
                                    (
                                        &TIDL_VISION_FXNS,
                                        (IALG_Params *)(&tidlObj->createParams)
                                    );

                if (NULL == tidlObj->algHandle)
                {
                    VX_PRINT(VX_ZONE_ERROR, "tivxAlgiVisionCreate returned NULL\n");
                    status = (vx_status)VX_FAILURE;
                }

                tidlObj->inBufs.size     = sizeof(tidlObj->inBufs);
                tidlObj->outBufs.size    = sizeof(tidlObj->outBufs);

                tidlObj->inBufs.bufDesc  = tidlObj->inBufDescList;
                tidlObj->outBufs.bufDesc = tidlObj->outBufDescList;

                tidlObj->inBufs.numBufs  = tidl_AllocNetInputMem(tidlObj->inBufDesc, &tidlObj->tidlParams.ioBufDesc);
                tidlObj->outBufs.numBufs = tidl_AllocNetOutputMem(tidlObj->outBufDesc, &tidlObj->tidlParams.ioBufDesc);

                for(i = 0; i < tidlObj->inBufs.numBufs; i++)
                {
                    tidlObj->inBufDescList[i]     = &tidlObj->inBufDesc[i];
                }
                for(i = 0; i < tidlObj->outBufs.numBufs; i++)
                {
                    tidlObj->outBufDescList[i]     = &tidlObj->outBufDesc[i];
                }
            }
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(network_target_ptr, network->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        tivxCheckStatus(&status, tivxMemBufferUnmap(create_params_target_ptr, createParams->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

        if ((vx_status)VX_SUCCESS == status)
        {
            tivxSetTargetKernelInstanceContext(kernel, tidlObj,  sizeof(tivxTIDLObj));
        }
        else
        {
            if (NULL != tidlObj)
            {
                #ifdef TIDL_COPY_NETWORK_BUF
                if (NULL != tidlObj->tidlNet)
                {
                    tivxMemFree(tidlObj->tidlNet, tidlObj->netSize, (vx_enum)TIVX_MEM_EXTERNAL);
                }
                #endif
                tivxTIDLFreeMem(tidlObj);
            }
        }
    }

    #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
    VX_PRINT(VX_ZONE_WARNING, "All Interrupts DISABLED during TIDL process\n");
    #endif

    return (status);
}

static vx_status VX_CALLBACK tivxKernelTIDLDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i;
    uint32_t size;
    tivxTIDLObj *tidlObj = NULL;

    for (i = 0U; i < num_params; i ++)
    {
        if((i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tidlObj, &size);

        if (((vx_status)VX_SUCCESS == status) && (NULL != tidlObj) && (sizeof(tivxTIDLObj) == size))
        {
            if (tidlObj->algHandle)
            {
                tivxAlgiVisionDelete(tidlObj->algHandle);
            }
            #ifdef TIDL_COPY_NETWORK_BUF
            if (NULL != tidlObj->tidlNet)
            {
                tivxMemFree(tidlObj->tidlNet, tidlObj->netSize, (vx_enum)TIVX_MEM_EXTERNAL);
            }
            #endif
            tivxTIDLFreeMem(tidlObj);
        }
    }

    #ifdef TIVX_TIDL_TARGET_DEBUG
    tivx_clr_debug_zone(VX_ZONE_INFO);
    #endif

    return (status);
}

static void tivxTIDLFreeMem(tivxTIDLObj *tidlObj)
{
    if (NULL != tidlObj)
    {
        tivxMemFree(tidlObj, sizeof(tivxTIDLObj), (vx_enum)TIVX_MEM_EXTERNAL);
    }
}

static void getQC(uint8_t *pIn, uint8_t *pOut, int32_t inSize)
{
  int32_t i, j;
  uint8_t vec[TIVX_TIDL_J7_CHECKSUM_SIZE];
  int32_t remSize;

  /* Initialize vector */
  for(j = 0; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
     vec[j] = 0;
  }

  /* Create QC */
  remSize = inSize;
  for(i = 0; i < inSize; i+=TIVX_TIDL_J7_CHECKSUM_SIZE)
  {
    int32_t elems;

    if (remSize < TIVX_TIDL_J7_CHECKSUM_SIZE)
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE - remSize;
      remSize += TIVX_TIDL_J7_CHECKSUM_SIZE;
    }
    else
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE;
      remSize -= TIVX_TIDL_J7_CHECKSUM_SIZE;
    }

    for(j = 0; j < elems; j++)
    {
      vec[j] ^= pIn[i + j];
    }
  }

  /* Return QC */
  for(j = 0; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
    pOut[j] = vec[j];
  }
}

static vx_status testChecksum(void *dataPtr, uint8_t *refQC, vx_int32 data_size, uint32_t loc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint8 qcData[TIVX_TIDL_J7_CHECKSUM_SIZE];
    int32_t match = 1;
    int32_t x;

    /* Get QC of config params passed from host to target */
    getQC(dataPtr, qcData, data_size);

    /* Print QC */
    for(x = 0; x < TIVX_TIDL_J7_CHECKSUM_SIZE; x++)
    {
      if(qcData[x] != refQC[x])
      {
        match = 0;
        break;
      }
    }
    if(match == 0)
    {
      VX_PRINT(VX_ZONE_ERROR, "Computing checksum at 0x%08X, size = %d\n", dataPtr,  data_size);
      VX_PRINT(VX_ZONE_ERROR, "QC code mismatch at %d \n", loc);
      status = (vx_status)VX_FAILURE;
    }
    else
    {
      VX_PRINT(VX_ZONE_INFO, "QC code match! \n");
    }

    return status;
}

/* Public Functions */

void tivxAddTargetKernelTIDL()
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    if ((self_cpu == TIVX_CPU_ID_DSP_C7_1)
#if defined(SOC_J784S4)
        || (self_cpu == TIVX_CPU_ID_DSP_C7_2)
        || (self_cpu == TIVX_CPU_ID_DSP_C7_3)
        || (self_cpu == TIVX_CPU_ID_DSP_C7_4)
#endif
        )
    {
        uint32_t i;

        for (i = 0; i < TIDL_MAX_TARGETS; i++)
        {
            vx_tidl_target_kernel[i] = tivxAddTargetKernelByName
                                    (
                                      TIVX_KERNEL_TIDL_NAME,
                                      target_name[i],
                                      tivxKernelTIDLProcess,
                                      tivxKernelTIDLCreate,
                                      tivxKernelTIDLDelete,
                                      NULL,
                                      NULL
                                    );
        }
    }
}

void tivxRemoveTargetKernelTIDL()
{
    uint32_t i;

    for (i = 0; i < TIDL_MAX_TARGETS; i++)
    {
        tivxRemoveTargetKernel(vx_tidl_target_kernel[i]);
    }
}
