/*
*
* Copyright (c) 2023 Texas Instruments Incorporated
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



#ifndef TIVX_CONFIG_J6_H_
#define TIVX_CONFIG_J6_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Max parameters in a kernel
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_KERNEL_MAX_PARAMS      (64u)

/*! \brief Max nodes taking output form a given node
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_NODE_MAX_OUT_NODES      (8u)

/*! \brief Max nodes feeding input to a given node
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_NODE_MAX_IN_NODES      (8u)

/*! \brief Maximum number of objects supported in pyramid
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_PYRAMID_MAX_LEVEL_OBJECTS       (64u)

/*! \brief Maximum number of objects supported in object array
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_OBJECT_ARRAY_MAX_ITEMS           (32u)

/*! \brief Max possible nodes in graph
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_GRAPH_MAX_NODES               (64u)

/*! \brief Max possible super nodes in graph
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_GRAPH_MAX_SUPER_NODES         (8u)

/*! \brief Max possible pipeline depth of a graph
 *         The max value to be set by application is
 *         (TIVX_GRAPH_MAX_PIPELINE_DEPTH-1)
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_GRAPH_MAX_PIPELINE_DEPTH      (16u)

/*! \brief Max number of replicated nodes
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_NODE_MAX_REPLICATE            (64u)

/*!
 * \brief Max number meta format objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_META_FORMAT_MAX_OBJECTS        (512u)

/*!
 * \brief Max number context objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_CONTEXT_MAX_OBJECTS                    (1u)

/*!
 * \brief Max number graph objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_GRAPH_MAX_OBJECTS                      (256u)

/*!
 * \brief Max number super node objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_SUPER_NODE_MAX_OBJECTS                 (16u)

/*!
 * \brief Max number node objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_NODE_MAX_OBJECTS                       (64u)

/*!
 * \brief Max number kernel objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_KERNEL_MAX_OBJECTS                     (128u)

/*!
 * \brief Max number array objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_ARRAY_MAX_OBJECTS                      (96u)

/*!
 * \brief Max number user data objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_USER_DATA_OBJECT_MAX_OBJECTS           (96u)

/*!
 * \brief Max number raw image objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_RAW_IMAGE_MAX_OBJECTS                  (96u)

/*!
 * \brief Max number convolution objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_CONVOLUTION_MAX_OBJECTS                (48u)

/*!
 * \brief Max number delay objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_DELAY_MAX_OBJECTS                      (48u)

/*!
 * \brief Max number distribution objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_DISTRIBUTION_MAX_OBJECTS               (48u)

/*!
 * \brief Max number image objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_IMAGE_MAX_OBJECTS                      (512u + 1u)

/*!
 * \brief Max number tensor objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_TENSOR_MAX_OBJECTS                     (256u)

/*!
 * \brief Max number lut objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_LUT_MAX_OBJECTS                        (48u)

/*!
 * \brief Max number matrix objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_MATRIX_MAX_OBJECTS                     (48u)

/*!
 * \brief Max number pyramid objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_PYRAMID_MAX_OBJECTS                    (64u)

/*!
 * \brief Max number remap objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_REMAP_MAX_OBJECTS                      (48u)

/*!
 * \brief Max number scalar objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_SCALAR_MAX_OBJECTS                     (48u)

/*!
 * \brief Max number threshold objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_THRESHOLD_MAX_OBJECTS                  (48u)

/*!
 * \brief Max number error objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_ERROR_MAX_OBJECTS                      (30u)

/*!
 * \brief Max number object arrays supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_OBJ_ARRAY_MAX_OBJECTS                  (256u)

/*!
 * \brief Max number parameter objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_PARAMETER_MAX_OBJECTS                  (48u)

/*!
 * \brief Max number data reference queue objects supported
 *
 * \ingroup group_tivx_obj_cfg
 */
#define TIVX_DATA_REF_Q_MAX_OBJECTS                 (128u)

/*!
 * \brief Max number of Target that can exist on a CPU
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_TARGET_MAX_TARGETS_IN_CPU  (16u)

/*!
 * \brief Max depth of queue associated with target
 * \note Since this parameter is only used on target, it can not yet
 *       be tracked for max usage, so generated report reports static value
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_TARGET_MAX_JOB_QUEUE_DEPTH         (96u)

/*!
 * \brief Max possible references in a context
 *
 * \ingroup group_vx_context_cfg
 */
#define TIVX_CONTEXT_MAX_REFERENCES     (512u + 256u + 64u + 8u + 32 + 64 + 32)


/*!
 * \brief Max possible unique kernels in a context
 *
 * \ingroup group_vx_context_cfg
 */
#define TIVX_CONTEXT_MAX_KERNELS     (128u)

/*!
 * \brief Max possible user structs in a context
 *
 * \ingroup group_vx_context_cfg
 */
#define TIVX_CONTEXT_MAX_USER_STRUCTS   (128u)

/*! \brief Maximum targets a kernel can run on
 * \ingroup group_tivx_target_kernel_cfg
 */
#define TIVX_MAX_TARGETS_PER_KERNEL     (32u)

/*! \brief Maximum possible modules in system \
 * \ingroup group_vx_module_cfg
 */
#define TIVX_MODULE_MAX        (16u)

/*!
 * \brief Max target kernel that will be active on a CPU
 *       at the same time
 * \ingroup group_tivx_target_kernel_cfg
 */
#define TIVX_TARGET_KERNEL_MAX     (256u)

/*!
 * \brief Max levels supported for the pyramid
 *        Note: If this macro is changed, change #gOrbScaleFactor also
 *              in vx_pyramid file.
 * \ingroup group_vx_pyramid_cfg
 */
#define TIVX_PYRAMID_MAX_LEVELS_ORB             (17u)

/*!
 * \brief Max target kernel instances that will be active on a CPU
 *       at the same time
 * \ingroup group_tivx_target_kernel_instance_cfg
 */
#define TIVX_TARGET_KERNEL_INSTANCE_MAX     (64u)

/*!
 * \brief Max possible mapping via vxMapArray supported
 *
 * \ingroup group_vx_array_cfg
 */
#define TIVX_ARRAY_MAX_MAPS     (16u)

/*!
 * \brief Max possible mapping via vxMapUserDataObject supported
 *
 * \ingroup group_vx_user_data_object_cfg
 */
#define TIVX_USER_DATA_OBJECT_MAX_MAPS     (16u)

/*!
 * \brief Max possible mapping via vxMapRawImagePatch supported
 *
 * \ingroup group_vx_raw_image_cfg
 */
#define TIVX_RAW_IMAGE_MAX_MAPS            (16u)

/*! \brief Max possible head nodes in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_HEAD_NODES          (8u)

/*! \brief Max possible leaf nodes in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_LEAF_NODES          (16u)

/*! \brief Max possible parameters in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_PARAMS              (16u)

/*! \brief Max data ref queue in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_DATA_REF_QUEUE      (32u)

/*! \brief Max possible delays in graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_DELAYS              (8u)

/*! \brief Max possible data references in a graph
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_GRAPH_MAX_DATA_REF            (128u)

/*!
 * \brief Max possible sub images from a image
 *
 * \ingroup group_vx_image_cfg
 */
#define TIVX_IMAGE_MAX_SUBIMAGES     (16u)

/*!
 * \brief Max possible sub images from a raw image
 *
 * \ingroup group_tivx_raw_image_cfg
 */
#define TIVX_RAW_IMAGE_MAX_SUBIMAGES     (16u)

/*!
 * \brief Max possible mapping via vxMapImagePatch supported
 *
 * \ingroup group_vx_image_cfg
 */
#define TIVX_IMAGE_MAX_MAPS     (16u)

/*!
 * \brief Max possible mapping via vxMapTensorPatch supported
 *
 * \ingroup group_vx_tensor_cfg
 */
#define TIVX_TENSOR_MAX_MAPS     (16u)

/*! \brief Maximum number of objects supported inside delay object
 * \ingroup group_vx_delay_cfg
 */
#define TIVX_DELAY_MAX_OBJECT           (8u)

/*! \brief Maximum number of parameter objects that can be associated with a delay
 * \ingroup group_vx_delay_cfg
 */
#define TIVX_DELAY_MAX_PRM_OBJECT       (16u)

/*! \brief Max size of event queue
 * \ingroup group_vx_event_cfg
 */
#define TIVX_EVENT_QUEUE_MAX_SIZE       (256u)

/*! \brief Max number of BAM user plugins on DSP (in addition to builtin vxlib plugins)
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_MAX_DSP_BAM_USER_PLUGINS   (8u)

/*! \brief Max number of nodes per super node
 * \ingroup group_tivx_supernode_cfg
 */
#define TIVX_SUPER_NODE_MAX_NODES   (16u)

/*! \brief Max number of edges per super node
 * \ingroup group_tivx_supernode_cfg
 */
#define TIVX_SUPER_NODE_MAX_EDGES   (16u)

/*! \brief Default tile width
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_DEFAULT_TILE_WIDTH   (64u)

/*! \brief Default tile height
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_DEFAULT_TILE_HEIGHT  (48u)

/*! \brief Default timeout value for kernel level control event ACK waits.
 * This is the default timeout value used for all node instances of this kernel
 * when calling the node CREATE and DELETE target kernel functions, which can be
 * overwritten for specific node instances by setting TIVX_NODE_TIMEOUT attribute.
 * \ingroup group_tivx_target_kernel_instance_cfg
 */
#define TIVX_DEFAULT_KERNAL_TIMEOUT (TIVX_EVENT_TIMEOUT_WAIT_FOREVER)

/*! \brief Default timeout value for graph level control event ACK waits.
 * This is the default timeout value used within the following APIs
 * - vxWaitGraph()
 * - vxGraphParameterDequeueDoneRef()
 * \ingroup group_vx_graph_cfg
 */
#define TIVX_DEFAULT_GRAPH_TIMEOUT  (TIVX_EVENT_TIMEOUT_WAIT_FOREVER)

/*! \brief Max size of control command objects
 * \ingroup group_vx_event_cfg
 */
#define TIVX_MAX_CTRL_CMD_OBJECTS (4u)

/*! \brief Max number of kernel ID's
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_MAX_KERNEL_ID (VX_KERNEL_MASK)

/*! \brief Max number of kernel library ID's
 * \ingroup group_tivx_target_cfg
 */
#define TIVX_MAX_LIBRARY_ID (VX_LIBRARY(VX_LIBRARY_MASK))

#ifdef __cplusplus
}
#endif

#endif
