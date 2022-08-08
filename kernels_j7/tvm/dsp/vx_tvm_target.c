/*
*
* Copyright (c) 2021 Texas Instruments Incorporated
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
#include <TI/j7_tvm.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include "tivx_platform.h"
#include "tivx_kernels_target_utils.h"
#include "../tvm_dyn_load/dsp_load.h"


#ifndef x86_64
#include "c7x.h"
#include <ti/osal/HwiP.h>
/* #define DISABLE_INTERRUPTS_DURING_PROCESS */
#endif

/* #define TIVX_TVM_TARGET_DEBUG */

typedef int (*TVM_NOARG_FUNC)(void);
typedef int (*TVM_CREATE_FUNC)(void* rt_info);
typedef int (*TVM_PROCESS_FUNC)(int32_t num_inputs, int32_t num_outputs,
                                uint32_t* input_names_offset, uint8_t* input_names,
                                void *tensors[]);

/*! \brief App object, saved in the VX kernel instance context */
typedef struct
{
  tivxTVMJ7Params    tvmParams;
  void               *dspload_handle;
  void               *tvm_runtime_handle;
  void               *tvm_tensors[TIVX_KERNEL_TVM_MAX_TOTAL_TENSORS];
  TVM_CREATE_FUNC     tvm_main_create;
  TVM_PROCESS_FUNC    tvm_main_process;
  TVM_NOARG_FUNC      tvm_main_delete;
} tivxTVMObj;

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

#define TVM_MAX_TARGETS (sizeof(target_name)/sizeof(target_name[0]))

/*! \brief vx target kernel for tvm node */
static tivx_target_kernel vx_tvm_target_kernel[TVM_MAX_TARGETS] = {NULL};

/* OpenVX Node callbacks */
static vx_status VX_CALLBACK tivxKernelTVMCreate(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTVMProcess(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTVMDelete(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);

/* TVM App function callbacks */
int32_t TVM_lockInterrupts(void);
void    TVM_unlockInterrupts(int32_t oldIntState);


/*
 * Following lock/unlock functions are provided to TVM (TIDL subgraph and generated code) to internally
 * disable and enable interrupts around critical section.
 */
int32_t TVM_lockInterrupts(void)
{
  int32_t oldIntState = 0;
#ifndef x86_64
  oldIntState = HwiP_disable();
#endif
  return oldIntState;
}

void TVM_unlockInterrupts(int32_t oldIntState)
{
#ifndef x86_64
  HwiP_restore(oldIntState);
#endif
}

/*!
 * \brief TVM VX Kernel create function, called during vxVerifyGraph()
 * \param kernel vx target kernel instance
 * \param obj_desc array of vx kernel parameters
 * \param num_params number of vx kernel parameters
 * \param priv_arg not used
 */
static vx_status VX_CALLBACK
tivxKernelTVMCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  uint16_t num_params,
  void *priv_arg
)
{
  vx_status status = (vx_status)VX_SUCCESS;

  tivx_obj_desc_user_data_object_t *config;
  /* tvm c7x deployable module is a shared executable that needs dynamic loading */
  tivx_obj_desc_user_data_object_t *deploy_mod;
  tivx_obj_desc_user_data_object_t *trace;

  tivxTVMObj *tvmObj = NULL;

  void *config_target_ptr = NULL;
  void *deploy_mod_target_ptr = NULL;
  void *trace_target_ptr = NULL;

  uint32_t i;

  #ifdef TIVX_TVM_TARGET_DEBUG
  tivx_set_debug_zone(VX_ZONE_INFO);
  #endif
  VX_PRINT(VX_ZONE_INFO, "tivxKernelTVMCreate...\n");

  for (i = 0U; i < num_params; i ++)
  {
    /* The trace data buffer parameter at i == 2 is optional */
    if ((i != TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMCreate: kernel parameter %d should not be NULL\n", i);
      status = (vx_status)VX_FAILURE;
      break;
    }
  }
  if ((vx_status)VX_SUCCESS == status)
  {
    status = tivxMemResetScratchHeap((vx_enum)TIVX_MEM_EXTERNAL_SCRATCH);
    VX_PRINT(VX_ZONE_INFO, "ResetScratchHep: status = %d\n", status);
  }
  if ((vx_status)VX_SUCCESS == status)
  {
    /* IMPORTANT! Config data is assumed to be available at index 0 */
    config    = (tivx_obj_desc_user_data_object_t *)obj_desc[
                                        TIVX_KERNEL_TVM_IN_CONFIG_IDX];
    /* IMPORTANT! deploy_mod is assumed to be available at index 1 */
    deploy_mod = (tivx_obj_desc_user_data_object_t *)obj_desc[
                                        TIVX_KERNEL_TVM_IN_DEPLOY_MOD_IDX];
    /* IMPORTANT! trace is assumed to be available at index 2 */
    trace = (tivx_obj_desc_user_data_object_t *)obj_desc[
                                        TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX];
    VX_PRINT(VX_ZONE_INFO, "config->mem_size = %d\n", config->mem_size);
    VX_PRINT(VX_ZONE_INFO, "deploy_mod->mem_size = %d\n", deploy_mod->mem_size);
    if (trace != NULL)
    {
      VX_PRINT(VX_ZONE_INFO, "trace->mem_size = %d\n", trace->mem_size);
    }

    tvmObj = tivxMemAlloc(sizeof(tivxTVMObj), (vx_enum)TIVX_MEM_EXTERNAL);
    if (NULL != tvmObj)
    {
      memset(tvmObj, 0, sizeof(tivxTVMObj));
    }
    else
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMCreate: failed to allocate tivxTVMObj\n");
      status = (vx_status)VX_ERROR_NO_MEMORY;
    }

    /* Copy tvm configuration parameters from kernel parameters */
    if ((vx_status)VX_SUCCESS == status)
    {
      config_target_ptr = tivxMemShared2TargetPtr(&config->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      if ((vx_status)VX_SUCCESS == status)
      {
        memcpy(&tvmObj->tvmParams, config_target_ptr, sizeof(tivxTVMJ7Params));
        tivxCheckStatus(&status, tivxMemBufferUnmap(config_target_ptr, config->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      }
    }

    /* Load c7x tvm deployable module */
    if ((vx_status)VX_SUCCESS == status)
    {
      deploy_mod_target_ptr = tivxMemShared2TargetPtr(&deploy_mod->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(deploy_mod_target_ptr,
                                                deploy_mod->mem_size,
                     (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      VX_PRINT(VX_ZONE_INFO, "deploy_mod_target_ptr = %p, loading...\n", deploy_mod_target_ptr);
      if ((vx_status)VX_SUCCESS == status)
      {
        tvmObj->dspload_handle = dspload_load_program(deploy_mod_target_ptr,
                                                      deploy_mod->mem_size);
        tvmObj->tvm_main_create  = (TVM_CREATE_FUNC) dspload_query_symbol(
                                    tvmObj->dspload_handle, "tvm_main_create");
        tvmObj->tvm_main_process = (TVM_PROCESS_FUNC) dspload_query_symbol(
                                    tvmObj->dspload_handle, "tvm_main_process");
        tvmObj->tvm_main_delete  = (TVM_NOARG_FUNC) dspload_query_symbol(
                                    tvmObj->dspload_handle, "tvm_main_delete");
        VX_PRINT(VX_ZONE_INFO, "tvm_main funcs: %p %p %p\n",
                 tvmObj->tvm_main_create,
                 tvmObj->tvm_main_process,
                 tvmObj->tvm_main_delete);
        tivxCheckStatus(&status, tivxMemBufferUnmap(deploy_mod_target_ptr, deploy_mod->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      }

      if (tvmObj->dspload_handle == NULL)
      {
        VX_PRINT(VX_ZONE_ERROR, "ERROR: Dsp dynamic loader failed.\n");
        status = VX_FAILURE;
      }
    }

    /* Set up trace */
    if ((vx_status)VX_SUCCESS == status && trace != NULL)
    {
      trace_target_ptr = tivxMemShared2TargetPtr(&trace->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(trace_target_ptr,
                                                trace->mem_size,
                     (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      VX_PRINT(VX_ZONE_INFO, "trace_target_ptr = %p\n", trace_target_ptr);
      if ((vx_status)VX_SUCCESS == status)
      {
        tvmObj->tvmParams.rt_info.tvm_rt_trace_ptr = (uint64_t)trace_target_ptr;
        tvmObj->tvmParams.rt_info.tvm_rt_trace_size = trace->mem_size;
        tivxCheckStatus(&status, tivxMemBufferUnmap(trace_target_ptr, trace->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      }
    }

    /* Save tvmObj in target kernel */
    if ((vx_status)VX_SUCCESS == status)
    {
      tivxSetTargetKernelInstanceContext(kernel, tvmObj,  sizeof(tivxTVMObj));
    }
  }

  /* Call tvm_main create function */
  if ((vx_status)VX_SUCCESS == status)
  {
    status = tvmObj->tvm_main_create((void *) &tvmObj->tvmParams.rt_info);
  }

  /* Error handling: something wrong, clean up */
  if (((vx_status)VX_SUCCESS != status) && (tvmObj != NULL))
  {
    if (tvmObj->dspload_handle != NULL)
    {
      dspload_unload_program(tvmObj->dspload_handle);
    }
    tivxMemFree(tvmObj, sizeof(tivxTVMObj), (vx_enum)TIVX_MEM_EXTERNAL);
  }

  #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
  VX_PRINT(VX_ZONE_WARNING, "All Interrupts DISABLED during TVM+TIDL process\n");
  #endif

  return (status);
}

/*!
 * \brief TVM VX Kernel process function, called during vxProcessGraph()
 * \param kernel vx target kernel instance
 * \param obj_desc array of vx kernel parameters
 * \param num_params number of vx kernel parameters
 * \param priv_arg not used
 */
static vx_status VX_CALLBACK
tivxKernelTVMProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
  vx_status status = (vx_status)VX_SUCCESS;

  tivxTVMObj *tvmObj;
  uint32_t i, size;

  #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
  uint32_t oldIntState;
  #endif

  #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
  /* disabling interrupts when doing TVM+TIDL processing
   *
   * suspect some stability issue due to interrupt handling,
   * until stability issue is root caused disabling interrupts
   * */
  oldIntState = HwiP_disable();
  #endif

  VX_PRINT(VX_ZONE_INFO, "tivxKernelTVMProcess...\n");

  for (i = 0U; i < num_params; i ++)
  {
    /* The trace data buffer parameter at i == 2 is optional */
    if ((i != TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMProcess: kernel parameter %d should not be NULL\n", i);
      status = (vx_status)VX_FAILURE;
      break;
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tvmObj,&size);

    if (((vx_status)VX_SUCCESS != status) || (NULL == tvmObj) ||
         (sizeof(tivxTVMObj) != size))
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMProcess: tvmObj saved in context is invalid: ptr %p, size %d\n", tvmObj, size);
      status = (vx_status)VX_FAILURE;
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    tivx_obj_desc_tensor_t *inTensor;
    tivx_obj_desc_tensor_t *outTensor;
    tivx_obj_desc_user_data_object_t *trace;
    void *in_tensor_target_ptr;
    void *out_tensor_target_ptr;
    void *trace_target_ptr = NULL;

    /* trace init */
    trace = (tivx_obj_desc_user_data_object_t *)obj_desc[
                                        TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX];
    if (trace != NULL && tvmObj->tvmParams.rt_info.tvm_rt_debug_level > 1)
    {
      trace_target_ptr = tivxMemShared2TargetPtr(&trace->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(trace_target_ptr,
                                                trace->mem_size,
                     (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    /* Idx 0 - config data,
       Idx 1 - deployable module,
       Idx 2 - traceData,
       Idx 3 - input tensor */
    uint32_t in_tensor_idx = TIVX_KERNEL_TVM_IN_FIRST_TENSOR;
    uint32_t num_input_tensors  = tvmObj->tvmParams.num_input_tensors;
    uint32_t num_output_tensors = tvmObj->tvmParams.num_output_tensors;

    /* Idx N - output tensors, where N = Idx 3 + number of input tensors */
    uint32_t out_tensor_idx = in_tensor_idx + num_input_tensors;
    uint32_t id;

    for(id = 0; id < num_input_tensors; id++) {
      inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
      in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
      tvmObj->tvm_tensors[id] = in_tensor_target_ptr;
    }

    for(id = 0; id < num_output_tensors; id++) {
      outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
      out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferMap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
      tvmObj->tvm_tensors[num_input_tensors + id] = out_tensor_target_ptr;
    }

    tvmObj->tvm_main_process(num_input_tensors, num_output_tensors,
                             tvmObj->tvmParams.input_names_offset,
                             tvmObj->tvmParams.input_names,
                             tvmObj->tvm_tensors);

    for(id = 0; id < num_input_tensors; id++) {
      inTensor  = (tivx_obj_desc_tensor_t *)obj_desc[in_tensor_idx + id];
      in_tensor_target_ptr  = tivxMemShared2TargetPtr(&inTensor->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferUnmap(in_tensor_target_ptr, inTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    for(id = 0; id < num_output_tensors; id++) {
      outTensor = (tivx_obj_desc_tensor_t *)obj_desc[out_tensor_idx + id];
      out_tensor_target_ptr = tivxMemShared2TargetPtr(&outTensor->mem_ptr);
      tivxCheckStatus(&status, tivxMemBufferUnmap(out_tensor_target_ptr, outTensor->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }

    /* trace write */
    if (trace != NULL && tvmObj->tvmParams.rt_info.tvm_rt_debug_level > 1)
    {
      tivxCheckStatus(&status, tivxMemBufferUnmap(trace_target_ptr,
                                                  trace->mem_size,
                     (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }
  }

  #ifdef DISABLE_INTERRUPTS_DURING_PROCESS
  HwiP_restore(oldIntState);
  #endif

  return (status);
}

/*!
 * \brief TVM VX Kernel delete function, called during vxReleaseGraph()
 * \param kernel vx target kernel instance
 * \param obj_desc array of vx kernel parameters
 * \param num_params number of vx kernel parameters
 * \param priv_arg not used
 */
static vx_status VX_CALLBACK
tivxKernelTVMDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
  vx_status status = (vx_status)VX_SUCCESS;
  uint32_t i;
  uint32_t size;
  tivxTVMObj *tvmObj = NULL;

  VX_PRINT(VX_ZONE_INFO, "tivxKernelTVMDelete...\n");

  for (i = 0U; i < num_params; i ++)
  {
    if((i != TIVX_KERNEL_TVM_IN_TRACE_DATA_IDX) && (NULL == obj_desc[i]))
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMDelete: kernel parameter %d should not be NULL\n", i);
      status = (vx_status)VX_FAILURE;
      break;
    }
  }

  if ((vx_status)VX_SUCCESS == status)
  {
    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&tvmObj,&size);

    if (((vx_status)VX_SUCCESS == status) && (NULL != tvmObj) &&
        (sizeof(tivxTVMObj) == size))
    {
      tvmObj->tvm_main_delete();
      if (NULL != tvmObj->dspload_handle)
      {
        dspload_unload_program(tvmObj->dspload_handle);
      }
      tivxMemFree(tvmObj, sizeof(tivxTVMObj), (vx_enum)TIVX_MEM_EXTERNAL);
    }
    else
    {
      VX_PRINT(VX_ZONE_ERROR, "tivxKernelTVMDelete: tvmObj saved in context is invalid: ptr %p, size %d\n", tvmObj, size);
      status = (vx_status)VX_FAILURE;
    }
  }

  #ifdef TIVX_TVM_TARGET_DEBUG
  tivx_clr_debug_zone(VX_ZONE_INFO);
  #endif

  return (status);
}

/* Public Functions */

void tivxAddTargetKernelTVM(void)
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

    for (i = 0; i < TVM_MAX_TARGETS; i++)
    {
      vx_tvm_target_kernel[i] = tivxAddTargetKernelByName
                            (
                              TIVX_KERNEL_TVM_NAME,
                              target_name[i],
                              tivxKernelTVMProcess,
                              tivxKernelTVMCreate,
                              tivxKernelTVMDelete,
                              NULL,
                              NULL
                            );
    }
  }
}

void tivxRemoveTargetKernelTVM(void)
{
  uint32_t i;

  for (i = 0; i < TVM_MAX_TARGETS; i++)
  {
    tivxRemoveTargetKernel(vx_tvm_target_kernel[i]);
  }
}

