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

#include <vx_internal.h>

#if defined(BUILD_DEV)
#define TIVX_MEM_CHAR_WIDTH_DEFAULT (30)
#define TIVX_MEM_CHAR_WIDTH_OBJECT_DESCRIPTOR (30)
#define TIVX_MEM_CHAR_WIDTH_GLOBAL (38)
#define TIVX_MEM_CHAR_WIDTH_ALL (48)

static int32_t findMacroSize(const char *resource_name);
static int32_t getNumDigits(int32_t value);
static char *test_file_path(void);
static char * applyMemoryUnit(vx_float64 * size_bytes, const char * unit);
static void printOutput (FILE *ofp, const char* format, ...);

static tivx_mutex g_tivx_log_resource_lock;

static tivx_shm_obj_count_t g_tivx_obj_desc_shm_table[] = {
    {
        0, 0, "TIVX_OBJ_DESC_GRAPH", (vx_enum)TIVX_OBJ_DESC_GRAPH, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_NODE", (vx_enum)TIVX_OBJ_DESC_NODE, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_ARRAY", (vx_enum)TIVX_OBJ_DESC_ARRAY, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_CONVOLUTION", (vx_enum)TIVX_OBJ_DESC_CONVOLUTION, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_DISTRIBUTION", (vx_enum)TIVX_OBJ_DESC_DISTRIBUTION, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_IMAGE", (vx_enum)TIVX_OBJ_DESC_IMAGE, (vx_bool)vx_true_e,
    },
    {
        0, 0, "TIVX_OBJ_DESC_LUT", (vx_enum)TIVX_OBJ_DESC_LUT, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_MATRIX", (vx_enum)TIVX_OBJ_DESC_MATRIX, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_OBJARRAY", (vx_enum)TIVX_OBJ_DESC_OBJARRAY, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_PYRAMID", (vx_enum)TIVX_OBJ_DESC_PYRAMID, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_RAW_IMAGE", (vx_enum)TIVX_OBJ_DESC_RAW_IMAGE, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_REMAP", (vx_enum)TIVX_OBJ_DESC_REMAP, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_SCALAR", (vx_enum)TIVX_OBJ_DESC_SCALAR, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_TENSOR", (vx_enum)TIVX_OBJ_DESC_TENSOR, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_THRESHOLD", (vx_enum)TIVX_OBJ_DESC_THRESHOLD, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_USER_DATA_OBJECT", (vx_enum)TIVX_OBJ_DESC_USER_DATA_OBJECT, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_DATA_REF_Q", (vx_enum)TIVX_OBJ_DESC_DATA_REF_Q, (vx_bool)vx_true_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_CMD", (vx_enum)TIVX_OBJ_DESC_CMD, (vx_bool)vx_false_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_KERNEL_NAME", (vx_enum)TIVX_OBJ_DESC_KERNEL_NAME, (vx_bool)vx_false_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_QUEUE", (vx_enum)TIVX_OBJ_DESC_QUEUE, (vx_bool)vx_false_e
    },
    {
        0, 0, "TIVX_OBJ_DESC_SUPER_NODE", (vx_enum)TIVX_OBJ_DESC_SUPER_NODE, (vx_bool)vx_false_e
    }
};

#define TIVX_OBJ_DESC_SHM_TABLE_SIZE (dimof(g_tivx_obj_desc_shm_table))

/*
 * Parameters with vx_[data object] data types store references to their repsective data objects,
 * not the actual data objects. TIVX_TYPE_UINTPTR is used to represent the size of a vx_reference
 * in these cases.
 */
tivx_resource_stats_t g_tivx_resource_stats_table[] = {
    {
        TIVX_CONTEXT_MAX_KERNELS, 0, 0, 1, "TIVX_CONTEXT_MAX_KERNELS", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                         {TIVX_CONTEXT_MAX_OBJECTS} }, \
                                                                       (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_CONTEXT_MAX_USER_STRUCTS, 0, 0, 1, "TIVX_CONTEXT_MAX_USER_STRUCTS", { {(uint32_t)TIVX_TYPE_CONTEXT_USER_STRUCTS}, \
                                                                                   {TIVX_CONTEXT_MAX_OBJECTS} }, \
                                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_CONTEXT_MAX_REFERENCES, 0, 0, 25, "TIVX_CONTEXT_MAX_REFERENCES", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                                {TIVX_CONTEXT_MAX_OBJECTS} }, \
                                                                              (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_CONTEXT_MAX_OBJECTS, 0, 0, 1, "TIVX_CONTEXT_MAX_OBJECTS", { {(uint32_t)VX_TYPE_CONTEXT, (uint32_t)VX_TYPE_BOOL}, \
                                                                         {1, 1} }, \
                                                                       (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_GRAPH_MAX_DELAYS, 0, 0, 1, "TIVX_GRAPH_MAX_DELAYS", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                   {TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_HEAD_NODES, 0, 0, 1, "TIVX_GRAPH_MAX_HEAD_NODES", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                           {TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_PIPELINE_DEPTH, 0, 0, 2, "TIVX_GRAPH_MAX_PIPELINE_DEPTH", { {(uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR, \
                                                                                    (uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR, \
                                                                                    (uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                                   {TIVX_DATA_REF_Q_MAX_OBJECTS, TIVX_DATA_REF_Q_MAX_OBJECTS, \
                                                                                    TIVX_GRAPH_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS, \
                                                                                    TIVX_NODE_MAX_OBJECTS, TIVX_NODE_MAX_OBJECTS} }, \
                                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_LEAF_NODES, 0, 0, 1, "TIVX_GRAPH_MAX_LEAF_NODES", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                           {TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_PARAMS, 0, 0, 1, "TIVX_GRAPH_MAX_PARAMS", { {(uint32_t)TIVX_TYPE_GRAPH_PARAMETERS}, \
                                                                   {TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_DATA_REF_QUEUE, 0, 0, 1, "TIVX_GRAPH_MAX_DATA_REF_QUEUE", { {(uint32_t)TIVX_TYPE_DATA_REF_QUEUE_LIST, \
                                                                                    (uint32_t)TIVX_TYPE_DELAY_DATA_REF_QUEUE_LIST}, \
                                                                                   {TIVX_GRAPH_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_NODES, 0, 0, 1, "TIVX_GRAPH_MAX_NODES", { {(uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                 {TIVX_CONTEXT_MAX_OBJECTS, TIVX_CONTEXT_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS} }, \
                                                               (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_DATA_REF, 0, 0, 1, "TIVX_GRAPH_MAX_DATA_REF", { {(uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)VX_TYPE_UINT8, (uint32_t)VX_TYPE_UINT8}, \
                                                                       {TIVX_GRAPH_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_GRAPH_MAX_OBJECTS, 0, 0, 1, "TIVX_GRAPH_MAX_OBJECTS", { {(uint32_t)VX_TYPE_GRAPH, (uint32_t)VX_TYPE_BOOL}, \
                                                                     {1, 1} }, \
                                                                   (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_GRAPH_MAX_PARAM_REFS, 0, 0, 1, "TIVX_GRAPH_MAX_PARAM_REFS", { {(uint32_t)VX_TYPE_UINT32, (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                           {TIVX_GRAPH_MAX_PARAMS+TIVX_GRAPH_MAX_OBJECTS, \
                                                                            TIVX_GRAPH_MAX_PARAMS+TIVX_GRAPH_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_NODE_MAX_OUT_NODES, 0, 0, 2, "TIVX_NODE_MAX_OUT_NODES", { {(uint32_t)VX_TYPE_UINT16}, \
                                                                       {TIVX_NODE_MAX_OBJECTS} }, \
                                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_NODE_MAX_IN_NODES, 0, 0, 2, "TIVX_NODE_MAX_IN_NODES", { {(uint32_t)VX_TYPE_UINT16}, \
                                                                     {TIVX_NODE_MAX_OBJECTS} }, \
                                                                   (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_NODE_MAX_REPLICATE, 0, 0, 0, "TIVX_NODE_MAX_REPLICATE", { {(uint32_t)VX_TYPE_UINT32}, \
                                                                       {TIVX_NODE_MAX_OBJECTS} }, \
                                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_NODE_MAX_OBJECTS, 0, 0, 2, "TIVX_NODE_MAX_OBJECTS", { {(uint32_t)VX_TYPE_NODE, (uint32_t)VX_TYPE_BOOL}, \
                                                                   {1, 1}}, \
                                                                 (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_ARRAY_MAX_MAPS, 0, 0, 1, "TIVX_ARRAY_MAX_MAPS", { {(uint32_t)TIVX_TYPE_ARRAY_MAP_INFO}, \
                                                               {TIVX_ARRAY_MAX_OBJECTS} }, \
                                                             (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_ARRAY_MAX_OBJECTS, 0, 0, 1, "TIVX_ARRAY_MAX_OBJECTS", { {(uint32_t)VX_TYPE_ARRAY, (uint32_t)VX_TYPE_BOOL}, \
                                                                     {1, 1} }, \
                                                                   (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_CONVOLUTION_MAX_OBJECTS, 0, 0, 1, "TIVX_CONVOLUTION_MAX_OBJECTS", { {(uint32_t)VX_TYPE_CONVOLUTION, (uint32_t)VX_TYPE_BOOL}, \
                                                                                 {1, 1} }, \
                                                                               (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_DISTRIBUTION_MAX_OBJECTS, 0, 0, 1, "TIVX_DISTRIBUTION_MAX_OBJECTS", { {(uint32_t)VX_TYPE_DISTRIBUTION, (uint32_t)VX_TYPE_BOOL}, \
                                                                                   {1, 1} }, \
                                                                                 (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_DELAY_MAX_OBJECT, 0, 0, 0, "TIVX_DELAY_MAX_OBJECT", { {(uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_DELAY_PARAM}, \
                                                                   {TIVX_DELAY_MAX_OBJECTS, TIVX_DELAY_MAX_OBJECTS} }, \
                                                                 (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_DELAY_MAX_PRM_OBJECT, 0, 0, 1, "TIVX_DELAY_MAX_PRM_OBJECT", { {(uint32_t)TIVX_TYPE_DELAY_PARAM}, \
                                                                           {TIVX_DELAY_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_DELAY_MAX_OBJECTS, 0, 0, 1, "TIVX_DELAY_MAX_OBJECTS", { {(uint32_t)VX_TYPE_DELAY, (uint32_t)VX_TYPE_BOOL}, \
                                                                     {1, 1} }, \
                                                                   (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_IMAGE_MAX_MAPS, 0, 0, 1, "TIVX_IMAGE_MAX_MAPS", { {(uint32_t)TIVX_TYPE_IMAGE_MAP_INFO}, \
                                                               {TIVX_IMAGE_MAX_OBJECTS} }, \
                                                             (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_IMAGE_MAX_SUBIMAGE_DEPTH, 0, 0, 2, "TIVX_IMAGE_MAX_SUBIMAGE_DEPTH", { {0}, {0} }, \
                                                             (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_IMAGE_MAX_SUBIMAGES, 0, 0, 1, "TIVX_IMAGE_MAX_SUBIMAGES", { {(uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                         {TIVX_IMAGE_MAX_OBJECTS} }, \
                                                                       (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_IMAGE_MAX_OBJECTS, 0, 0, 1, "TIVX_IMAGE_MAX_OBJECTS", { {(uint32_t)VX_TYPE_IMAGE, (uint32_t)VX_TYPE_BOOL}, \
                                                                     {1, 1} }, \
                                                                   (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_LUT_MAX_OBJECTS, 0, 0, 1, "TIVX_LUT_MAX_OBJECTS", { {(uint32_t)VX_TYPE_LUT, (uint32_t)VX_TYPE_BOOL}, \
                                                                 {1, 1}}, \
                                                               (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_MATRIX_MAX_OBJECTS, 0, 0, 1, "TIVX_MATRIX_MAX_OBJECTS", { {(uint32_t)VX_TYPE_MATRIX, (uint32_t)VX_TYPE_BOOL}, \
                                                                       {1, 1} }, \
                                                                     (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_OBJECT_ARRAY_MAX_ITEMS, 0, 0, 1, "TIVX_OBJECT_ARRAY_MAX_ITEMS", { {(uint32_t)TIVX_TYPE_UINTPTR, \
                                                                                (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                               {TIVX_DELAY_MAX_OBJECTS, \
                                                                                TIVX_OBJ_ARRAY_MAX_OBJECTS} }, \
                                                                             (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_OBJ_ARRAY_MAX_OBJECTS, 0, 0, 1, "TIVX_OBJ_ARRAY_MAX_OBJECTS", { {(uint32_t)VX_TYPE_OBJECT_ARRAY, (uint32_t)VX_TYPE_BOOL}, \
                                                                             {1, 1} }, \
                                                                           (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_PYRAMID_MAX_LEVEL_OBJECTS, 0, 0, 1, "TIVX_PYRAMID_MAX_LEVEL_OBJECTS", { {(uint32_t)TIVX_TYPE_UINTPTR, \
                                                                                      (uint32_t)VX_TYPE_RECTANGLE, (uint32_t)TIVX_TYPE_UINTPTR, \
                                                                                      (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                                     {TIVX_DELAY_MAX_OBJECTS, \
                                                                                      TIVX_GRAPH_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS, \
                                                                                      TIVX_PYRAMID_MAX_OBJECTS} }, \
                                                                                   (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_PYRAMID_MAX_OBJECTS, 0, 0, 1, "TIVX_PYRAMID_MAX_OBJECTS", { {(uint32_t)VX_TYPE_PYRAMID, (uint32_t)VX_TYPE_BOOL}, \
                                                                         {1, 1} }, \
                                                                       (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_RAW_IMAGE_MAX_MAPS, 0, 0, 1, "TIVX_RAW_IMAGE_MAX_MAPS", { {(uint32_t)TIVX_TYPE_RAW_IMAGE_MAP_INFO}, \
                                                                       {TIVX_RAW_IMAGE_MAX_OBJECTS} }, \
                                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_RAW_IMAGE_MAX_OBJECTS, 0, 0, 1, "TIVX_RAW_IMAGE_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_RAW_IMAGE, (uint32_t)VX_TYPE_BOOL}, \
                                                                             {1, 1} }, \
                                                                           (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_REMAP_MAX_OBJECTS, 0, 0, 1, "TIVX_REMAP_MAX_OBJECTS", { {(uint32_t)VX_TYPE_REMAP, (uint32_t)VX_TYPE_BOOL}, \
                                                                     {1, 1} }, \
                                                                   (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_SCALAR_MAX_OBJECTS, 0, 0, 1, "TIVX_SCALAR_MAX_OBJECTS", { {(uint32_t)VX_TYPE_SCALAR, (uint32_t)VX_TYPE_BOOL}, \
                                                                       {1, 1} }, \
                                                                     (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_TENSOR_MAX_MAPS, 0, 0, 1, "TIVX_TENSOR_MAX_MAPS", { {(uint32_t)TIVX_TYPE_TENSOR_MAP_INFO}, \
                                                                 {TIVX_TENSOR_MAX_OBJECTS} }, \
                                                               (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_TENSOR_MAX_OBJECTS, 0, 0, 1, "TIVX_TENSOR_MAX_OBJECTS", { {(uint32_t)VX_TYPE_TENSOR, (uint32_t)VX_TYPE_BOOL}, \
                                                                       {1, 1} }, \
                                                                     (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_THRESHOLD_MAX_OBJECTS, 0, 0, 1, "TIVX_THRESHOLD_MAX_OBJECTS", { {(uint32_t)VX_TYPE_THRESHOLD, (uint32_t)VX_TYPE_BOOL}, \
                                                                             {1, 1} }, \
                                                                           (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_USER_DATA_OBJECT_MAX_MAPS, 0, 0, 1, "TIVX_USER_DATA_OBJECT_MAX_MAPS", { {(uint32_t)TIVX_TYPE_USER_DATA_OBJECT_MAP_INFO}, \
                                                                                     {TIVX_USER_DATA_OBJECT_MAX_OBJECTS} }, \
                                                                                   (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_USER_DATA_OBJECT_MAX_OBJECTS, 0, 0, 1, "TIVX_USER_DATA_OBJECT_MAX_OBJECTS", { {VX_TYPE_USER_DATA_OBJECT, (uint32_t)VX_TYPE_BOOL}, \
                                                                                           {1, 1} }, \
                                                                                         (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_ERROR_MAX_OBJECTS, 0, 0, 25, "TIVX_ERROR_MAX_OBJECTS", { {(uint32_t)VX_TYPE_ERROR, (uint32_t)VX_TYPE_BOOL}, \
                                                                      {1, 1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_EVENT_QUEUE_MAX_SIZE, 0, 0, 1, "TIVX_EVENT_QUEUE_MAX_SIZE", { {(uint32_t)TIVX_TYPE_EVENT_QUEUE_ELEMENT, (uint32_t)TIVX_TYPE_UINTPTR, \
                                                                            (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                           {TIVX_GRAPH_MAX_OBJECTS+TIVX_CONTEXT_MAX_OBJECTS, \
                                                                            TIVX_GRAPH_MAX_OBJECTS+TIVX_CONTEXT_MAX_OBJECTS, \
                                                                            TIVX_GRAPH_MAX_OBJECTS+TIVX_CONTEXT_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_KERNEL_MAX_PARAMS, 0, 0, 2, "TIVX_KERNEL_MAX_PARAMS", { {(uint32_t)VX_TYPE_UINT16, (uint32_t)VX_TYPE_UINT16, (uint32_t)VX_TYPE_UINT16, \
                                                                      (uint32_t)VX_TYPE_RECTANGLE, (uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)VX_TYPE_ENUM, \
                                                                      (uint32_t)VX_TYPE_ENUM, (uint32_t)VX_TYPE_ENUM, (uint32_t)TIVX_TYPE_UINTPTR, \
                                                                      (uint32_t)VX_TYPE_BOOL, (uint32_t)VX_TYPE_UINT32}, \
                                                                     {TIVX_TARGET_KERNEL_INSTANCE_MAX+TIVX_NODE_MAX_OBJECTS, \
                                                                      TIVX_TARGET_KERNEL_INSTANCE_MAX+TIVX_NODE_MAX_OBJECTS, \
                                                                      TIVX_DATA_REF_Q_MAX_OBJECTS+TIVX_CONTEXT_MAX_OBJECTS+ \
                                                                      TIVX_NODE_MAX_OBJECTS, TIVX_GRAPH_MAX_OBJECTS, \
                                                                      TIVX_GRAPH_MAX_OBJECTS, TIVX_KERNEL_MAX_OBJECTS+ \
                                                                      TIVX_KERNEL_MAX_OBJECTS+TIVX_KERNEL_MAX_OBJECTS, \
                                                                      TIVX_NODE_MAX_OBJECTS, TIVX_NODE_MAX_OBJECTS, \
                                                                      TIVX_NODE_MAX_OBJECTS} }, \
                                                                   (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_KERNEL_MAX_OBJECTS, 0, 0, 0, "TIVX_KERNEL_MAX_OBJECTS", { {(uint32_t)VX_TYPE_KERNEL, (uint32_t)VX_TYPE_BOOL}, \
                                                                       {1, 1} }, \
                                                                     (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_META_FORMAT_MAX_OBJECTS, 0, 0, 0, "TIVX_META_FORMAT_MAX_OBJECTS", { {(uint32_t)VX_TYPE_META_FORMAT, (uint32_t)VX_TYPE_BOOL}, \
                                                                                 {1, 1} }, \
                                                                               (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_MODULE_MAX, 0, 0, 1, "TIVX_MODULE_MAX", { {(uint32_t)VX_TYPE_CHAR}, \
                                                       {1} }, \
                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST, 0, 0, 1, "TIVX_PLATFORM_MAX_OBJ_DESC_SHM_INST", {{0}, {0}}, (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_PARAMETER_MAX_OBJECTS, 0, 0, 1, "TIVX_PARAMETER_MAX_OBJECTS", { {(uint32_t)VX_TYPE_PARAMETER, (uint32_t)VX_TYPE_BOOL}, \
                                                                             {1, 1} }, \
                                                                           (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_DATA_REF_Q_MAX_OBJECTS, 0, 0, 1, "TIVX_DATA_REF_Q_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_DATA_REF_Q, (uint32_t)VX_TYPE_BOOL}, \
                                                                               {1, 1} }, \
                                                                             (vx_bool)vx_true_e, (vx_bool)vx_true_e
    },
    {
        TIVX_MAX_CTRL_CMD_OBJECTS, 0, 0, 1, "TIVX_MAX_CTRL_CMD_OBJECTS", { {(uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_EVENT_HANDLE, \
                                                                            (uint32_t)TIVX_TYPE_UINTPTR, (uint32_t)TIVX_TYPE_UINTPTR}, \
                                                                           {TIVX_CONTEXT_MAX_OBJECTS, TIVX_CONTEXT_MAX_OBJECTS, \
                                                                            TIVX_CONTEXT_MAX_OBJECTS, TIVX_CONTEXT_MAX_OBJECTS} }, \
                                                                         (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
#if 0
    {
        TIVX_MAX_DSP_BAM_USER_PLUGINS, 0, 0, 0, "TIVX_MAX_DSP_BAM_USER_PLUGINS"
    },
    {
        TIVX_SUPER_NODE_MAX_NODES, 0, 0, 16, "TIVX_SUPER_NODE_MAX_NODES"
    },
    {
        TIVX_SUPER_NODE_MAX_EDGES, 0, 0, 16, "TIVX_SUPER_NODE_MAX_EDGES"
    },
    {
        TIVX_SUPER_NODE_MAX_OBJECTS, 0, 0, 16, "TIVX_SUPER_NODE_MAX_OBJECTS"
    },
#endif
    {
        TIVX_TARGET_MAX_TARGETS_IN_CPU, 0, 0, TIVX_TARGET_MAX_TARGETS_IN_CPU, "TIVX_TARGET_MAX_TARGETS_IN_CPU", { {(uint32_t)TIVX_TYPE_TARGET}, \
                                                                                     {1} }, \
                                                                                   (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_MAX_TARGETS_PER_KERNEL, 0, 0, TIVX_MAX_TARGETS_PER_KERNEL, "TIVX_MAX_TARGETS_PER_KERNEL", { {(uint32_t)VX_TYPE_CHAR}, \
                                                                               {TIVX_TARGET_MAX_NAME*TIVX_KERNEL_MAX_OBJECTS} }, \
                                                                             (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_TARGET_KERNEL_INSTANCE_MAX, 0, 0, TIVX_TARGET_KERNEL_INSTANCE_MAX, "TIVX_TARGET_KERNEL_INSTANCE_MAX", { {(uint32_t)TIVX_TYPE_TARGET_KERNEL_INSTANCE}, \
                                                                                       {1} }, \
                                                                                     (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_TARGET_KERNEL_MAX, 0, 0, 1 , "TIVX_TARGET_KERNEL_MAX", { {(uint32_t)TIVX_TYPE_TARGET_KERNEL}, \
                                                                      {1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_false_e
    },
    {
        TIVX_EVENT_MAX_OBJECTS, 0, 0, 1 , "TIVX_EVENT_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_EVENT}, \
                                                                      {1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_MUTEX_MAX_OBJECTS, 0, 0, 1 , "TIVX_MUTEX_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_MUTEX}, \
                                                                      {1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_QUEUE_MAX_OBJECTS, 0, 0, 1 , "TIVX_QUEUE_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_QUEUE}, \
                                                                      {1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_true_e
    },
    {
        TIVX_TASK_MAX_OBJECTS, 0, 0, 1 , "TIVX_TASK_MAX_OBJECTS", { {(uint32_t)TIVX_TYPE_TASK}, \
                                                                      {1} }, \
                                                                    (vx_bool)vx_false_e, (vx_bool)vx_true_e
    }
};

static int32_t findMacroSize(const char *resource_name)
{
    int32_t i, size = (int32_t)TIVX_RESOURCE_NAME_MAX;

    for (i = 0; i < (int32_t)TIVX_RESOURCE_NAME_MAX; i++)
    {
        if (resource_name[i] == '\0')
        {
            size = i;
            break;
        }
    }

    return size;
}

static int32_t getNumDigits(int32_t value)
{
    int32_t temp = value;
    int32_t numDigits = 0;
    if (temp < 1)
    {
        numDigits = (temp < 0) ? 2 : 1;
    }
    while (temp > 0) {
        temp /= 10;
        numDigits++;
    }
    return numDigits;
}

static void ownLogUpdateTargetValues(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t i, j;
    tivx_resource_stats_t resource;
    char target_resources[TIVX_TARGET_RESOURCE_COUNT][TIVX_RESOURCE_NAME_MAX] = {"TIVX_TARGET_MAX_TARGETS_IN_CPU",
                                                                                       "TIVX_MAX_TARGETS_PER_KERNEL",
                                                                                       "TIVX_TARGET_KERNEL_INSTANCE_MAX",
                                                                                       "TIVX_TARGET_KERNEL_MAX"};
    uint32_t target_values[TIVX_TARGET_RESOURCE_COUNT] = {0, 0, 0, 0};
    uint32_t cpu_map[TIVX_CPU_ID_MAX];

    ownIpcGetCpuMap(cpu_map);

    for (i = 0; i < (uint32_t)TIVX_CPU_ID_MAX; i++)
    {
        if ((cpu_map[i] != 0) && (ownIsCpuEnabled(cpu_map[i])))
        {
            ownPlatformGetTargetPerfStats(cpu_map[i], target_values);

            for (j = 0; j < TIVX_TARGET_RESOURCE_COUNT; j++)
            {
                status = tivxQueryResourceStats(target_resources[j], &resource);

                if ((status == (vx_status)VX_SUCCESS) && (target_values[j] > resource.max_used_value) && (target_values[j] < resource.max_value))
                {
                    ownLogSetResourceUsedValue(resource.name, (uint16_t)target_values[j]);
                }
            }
        }
    }
}

static char *test_file_path(void)
{
    char *tivxPlatformGetEnv(char *env_var);

    #if defined(FREERTOS) || defined(SAFERTOS) || defined(THREADX)
    return tivxPlatformGetEnv("VX_TEST_DATA_PATH");
    #else
    return getenv("VX_TEST_DATA_PATH");
    #endif
}

static char * applyMemoryUnit(vx_float64 * size_bytes, const char * unit)
{
    char * output = "B ";
    if (strncmp(unit, "B", 1)==0)
    {
        output = "B ";
    }
    else if (strncmp(unit, "KB", 2)==0)
    {
        *size_bytes /= (vx_float64)1024;
        output = "KB";
    }
    else if (strncmp(unit, "MB", 2)==0)
    {
        *size_bytes /= (vx_float64)(1024*1024);
        output = "MB";
    }
    else
    {
        if (*size_bytes < (vx_float64)1024)
        {
            output = "B ";
        }
        else if (*size_bytes < (vx_float64)(1024*1024))
        {
            *size_bytes /= (vx_float64)1024;
            output = "KB";
        }
        else if (*size_bytes < (vx_float64)(1024*1024*1024))
        {
            *size_bytes /= (vx_float64)(1024*1024);
            output = "MB";
        }
        else
        {
            /* do nothing */
        }
    }
    return output;
}

static void printOutput (FILE *ofp, const char* format, ...)
{
    va_list argptr = {0};
    (void)va_start(argptr, format);
    if (ofp == NULL)
    {
        (void)vfprintf(stderr, format, argptr);
    }
    else
    {
        (void)vfprintf(ofp, format, argptr);
    }
    (void)va_end(argptr);
}
#endif /* #if defined(BUILD_DEV) */

void ownLogResourceInit(void)
{
#if defined(BUILD_DEV)
    /*
    * Compile time checks to ensure parameter cohesion in case they are configured
    */
    /* Head nodes can't exceed the defined graph node maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_HEAD_NODES <= TIVX_GRAPH_MAX_NODES)? 1 : 0));

    /* Leaf nodes can't exceed the defined graph node maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_LEAF_NODES <= TIVX_GRAPH_MAX_NODES)? 1 : 0));

    /* Graph parameters can't exceed the defined parameter maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_PARAMS <= TIVX_PARAMETER_MAX_OBJECTS)? 1 : 0));

    /* Graph data ref queues can't exceed the defined data ref queue maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_DATA_REF_QUEUE <= TIVX_DATA_REF_Q_MAX_OBJECTS)? 1 : 0));

    /* Graph nodes can't exceed the defined node maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_NODES <= TIVX_NODE_MAX_OBJECTS)? 1 : 0));

    /* Graph references can't exceed the defined context maximum */
    BUILD_ASSERT(((TIVX_GRAPH_MAX_DATA_REF <= TIVX_CONTEXT_MAX_REFERENCES)? 1 : 0));

    /* Out nodes can't exceed the defined node maximum */
    BUILD_ASSERT(((TIVX_NODE_MAX_OUT_NODES <= TIVX_NODE_MAX_OBJECTS)? 1 : 0));

    /* In nodes can't exceed the defined node maximum */
    BUILD_ASSERT(((TIVX_NODE_MAX_IN_NODES <= TIVX_NODE_MAX_OBJECTS)? 1 : 0));

    /* Subimages can't exceed the defined image object maximum */
    BUILD_ASSERT(((TIVX_IMAGE_MAX_SUBIMAGES <= TIVX_IMAGE_MAX_OBJECTS)? 1 : 0));
#ifdef TIVX_RESOURCE_LOG_ENABLE
    if((vx_status)VX_SUCCESS != tivxMutexCreate(&g_tivx_log_resource_lock))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to create a mutex\n");
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownLogResourceDeInit(void)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    if((vx_status)VX_SUCCESS != tivxMutexDelete(&g_tivx_log_resource_lock))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to delete mutex\n");
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownLogResourceAlloc(const char *resource_name, uint16_t num_allocs)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                g_tivx_resource_stats_table[i].cur_used_value += num_allocs;
                if (g_tivx_resource_stats_table[i].cur_used_value > g_tivx_resource_stats_table[i].max_used_value)
                {
                    g_tivx_resource_stats_table[i].max_used_value = g_tivx_resource_stats_table[i].cur_used_value;
                }
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownLogResourceFree(const char *resource_name, uint16_t num_frees)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                g_tivx_resource_stats_table[i].cur_used_value -= num_frees;
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownLogSetResourceUsedValue(const char *resource_name, uint16_t value)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                if (value > g_tivx_resource_stats_table[i].max_used_value)
                {
                    g_tivx_resource_stats_table[i].max_used_value = value;
                }
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownTableIncrementValue(vx_enum resource_name)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    uint32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < TIVX_OBJ_DESC_SHM_TABLE_SIZE; i++)
        {
            if (g_tivx_obj_desc_shm_table[i].object_type == resource_name)
            {
                g_tivx_obj_desc_shm_table[i].value++;
                if (g_tivx_obj_desc_shm_table[i].value > g_tivx_obj_desc_shm_table[i].max_value)
                {
                    g_tivx_obj_desc_shm_table[i].max_value = g_tivx_obj_desc_shm_table[i].value;
                }
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

void ownTableDecrementValue(vx_enum resource_name)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    uint32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < TIVX_OBJ_DESC_SHM_TABLE_SIZE; i++)
        {
            if (g_tivx_obj_desc_shm_table[i].object_type == resource_name)
            {
                g_tivx_obj_desc_shm_table[i].value--;
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

vx_status tivxQueryResourceStats(const char *resource_name, tivx_resource_stats_t *stats)
{
    vx_status status = (vx_status)VX_FAILURE;
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    int32_t i;
    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {
        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            if ( strncmp(g_tivx_resource_stats_table[i].name, resource_name, TIVX_RESOURCE_NAME_MAX) == 0 )
            {
                *stats = g_tivx_resource_stats_table[i];
                status = (vx_status)VX_SUCCESS;
                break;
            }
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */

    return status;
}

void tivxPrintAllResourceStats(void)
{
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    ownLogUpdateTargetValues();

    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {

        int32_t i, j;
        tivx_resource_stats_t stat;
        (void)printf("\n\n  MAX VALUE NAME:                         MAX VALUE:  VALUE USED:  REQ. VALUE:\n");
        (void)printf("-------------------------------------------------------------------------------\n");

        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            int32_t name_length, numDigits;
            stat = g_tivx_resource_stats_table[i];
            name_length = findMacroSize(stat.name);
            (void)printf("| ");
            (void)printf("%s ", stat.name);
            for (j = 0; j < ((int32_t)TIVX_RESOURCE_NAME_MAX - name_length); j++)
            {
                (void)printf(" ");
            }
            (void)printf("|");
            numDigits = getNumDigits((int32_t)stat.max_value);
            for (j = 0; j < (7 - numDigits); j++)
            {
                (void)printf(" ");
            }
            (void)printf("%d", stat.max_value);
            for (j = 0; j < 4; j++)
            {
                (void)printf(" ");
            }
            (void)printf("|");
            numDigits = getNumDigits((int32_t)stat.max_used_value);
            for (j = 0; j < (7 - numDigits); j++)
            {
                (void)printf(" ");
            }
            (void)printf("%d", stat.max_used_value);
            for (j = 0; j < 4; j++)
            {
                (void)printf(" ");
            }
            (void)printf("|");
            numDigits = getNumDigits((int32_t)stat.min_required_value);
            for (j = 0; j < (7 - numDigits); j++)
            {
                (void)printf(" ");
            }
            (void)printf("%d", stat.min_required_value);
            for (j = 0; j < 4; j++)
            {
                (void)printf(" ");
            }
            (void)printf("|\n");
        }
        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */
}

vx_status tivxExportAllResourceMaxUsedValueToFile(void)
{
    vx_status status = (vx_status)VX_FAILURE;
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    ownLogUpdateTargetValues();

    if((vx_status)VX_SUCCESS == tivxMutexLock(g_tivx_log_resource_lock))
    {

        int32_t i;
        FILE *ofp;
        tivx_resource_stats_t stat;
        status = (vx_status)VX_SUCCESS;

    char outputFilename[TIVX_CONFIG_PATH_LENGTH];
    const char *env;
    env = test_file_path();

    if (NULL != env)
    {
        (void)snprintf(outputFilename, TIVX_CONFIG_PATH_LENGTH, "%s/%s", env, "output/tivx_config_generated.h");

            ofp = fopen(outputFilename, "w");

            if (ofp == NULL)
            {
                (void)fprintf(stderr, "Can't open output file!\n");
                status = (vx_status)VX_FAILURE;
                if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
                }
            }
            else
            {
                (void)fprintf(ofp, "#ifndef TIVX_CONFIG_H_\n");
                (void)fprintf(ofp, "#define TIVX_CONFIG_H_\n\n");

                (void)fprintf(ofp, "#ifdef __cplusplus\n");
                (void)fprintf(ofp, "extern \"C\" {\n");
                (void)fprintf(ofp, "#endif\n\n");

                for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
                {
                    stat = g_tivx_resource_stats_table[i];
                    if (stat.max_used_value < stat.min_required_value) {
                        (void)fprintf(ofp, "/*This parameter was used less than its minimum value");
                        (void)fprintf(ofp, " requires. It has been set to its minimum value.*/\n");
                        stat.max_used_value = stat.min_required_value;
                    }
                    (void)fprintf(ofp, "#define ");
                    (void)fprintf(ofp, "%s ", stat.name);
                    (void)fprintf(ofp, "(%du)\n\n", stat.max_used_value);
                }

                (void)fprintf(ofp, "#ifdef __cplusplus\n");
                (void)fprintf(ofp, "}\n");
                (void)fprintf(ofp, "#endif\n\n");
                (void)fprintf(ofp, "#endif\n");

                (void)fclose(ofp);
                if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "VX_TEST_DATA_PATH has not been set!\n");
            if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
            {
                VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
            }
        }
    }
#endif
#endif /* #if defined(BUILD_DEV) */

    return status;
}

vx_status tivxExportMemoryConsumption(char * outputFile, const char * unit, vx_enum displayMode)
{
    vx_status status = (vx_status)VX_FAILURE;
#if defined(BUILD_DEV)
#ifdef TIVX_RESOURCE_LOG_ENABLE
    ownLogUpdateTargetValues();
    
    if((vx_status)VX_SUCCESS != tivxMutexLock(g_tivx_log_resource_lock))
    {
        VX_PRINT(VX_ZONE_ERROR,"Failed to lock mutex\n");
    }
    int32_t i, j, k, obj_count=0, char_width;
    uint32_t name_length, numDigits, useCount, shm_size;
    vx_float64 size, total_size = 0.0, rem_size, total_rem = 0.0, percent_used;
    vx_float64 obj_size, total_obj_size=0.0, rem_obj_size;
    char prefix[TIVX_RESOURCE_NAME_MAX] = {'\0'};
    char * size_units, * rem_units, * obj_units, * rem_obj_units;
    tivx_resource_stats_t row;
    tivx_shm_obj_count_t obj_row;
    status = (vx_status)VX_SUCCESS;
    FILE * ofp = NULL;
    const char *env;
    env = test_file_path();

    if (outputFile != NULL)
    {
        if (NULL != env)
        {
            char outputFileName[TIVX_CONFIG_PATH_LENGTH];
            (void)snprintf(outputFileName, TIVX_CONFIG_PATH_LENGTH, "%s/output/%s", env, outputFile);
            ofp = fopen(outputFileName, "w");
            if (ofp == NULL)
            {
                if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
                {
                    VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
                }
                status = (vx_status)VX_FAILURE;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "\nVX_TEST_DATA_PATH has not been set! Displaying to console instead.\n");
        }
    }
    if ((vx_status)VX_SUCCESS==status)
    {
        if (displayMode == (vx_enum)TIVX_MEM_LOG_GLOBAL)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_GLOBAL;
        }
        else if (displayMode == (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_OBJECT_DESCRIPTOR;
        }
        else if (displayMode == (vx_enum)TIVX_MEM_LOG_ALL)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_ALL;
        }
        else
        {
            char_width = TIVX_MEM_CHAR_WIDTH_DEFAULT;
            displayMode = (vx_enum)TIVX_MEM_LOG_DEFAULT;
            printOutput(ofp, "\n\n");
        }
        if (displayMode != (vx_enum)TIVX_MEM_LOG_DEFAULT)
        {
            printOutput(ofp, "\n\n|");
            for (k = 0; k < ((char_width*2)+23); k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n|");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "CONFIGURATION PARAMETER");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            for (k = 0; k < (char_width+4); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "MEMORY CARVEOUT");
            for (k = 0; k < (char_width+4); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            for (k = 0; k < ((char_width*2)+23); k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n|");
            for (k = 0; k < ((char_width*2)+23); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            for (k = 0; k < ((char_width*2)+23); k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|              PARAMETER               |    UNITS USED FROM     |");
            if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
            {
                printOutput(ofp, " PARAMETER MEMORY| PARAMETER MEMORY|");
            }
            if (displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL)
            {
                printOutput(ofp, " OBJECT DESCRIPTOR |");
            }
            printOutput(ofp, "\n");
            printOutput(ofp, "|                NAME                  |FRAMEWORK OR APPLICATION|");
            if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
            {
                printOutput(ofp, "     CONSUMED    |    REMAINING    |");
            }
            if (displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL)
            {
                printOutput(ofp, "  MEMORY AT CALL   |");
            }
            printOutput(ofp, "\n|--------------------------------------|------------------------|");
            if (displayMode == (vx_enum)TIVX_MEM_LOG_GLOBAL)
            {
                char_width = TIVX_MEM_CHAR_WIDTH_GLOBAL-3;
            }
            else if (displayMode == (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
            {
                char_width = TIVX_MEM_CHAR_WIDTH_OBJECT_DESCRIPTOR-11;
            }
            else if (displayMode == (vx_enum)TIVX_MEM_LOG_ALL)
            {
                char_width = TIVX_MEM_CHAR_WIDTH_ALL+7;
            }
            else
            {
                /* no new value to char_width */
            }
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n");
        }
        for (i = 0; i < (int32_t)TIVX_RESOURCE_STATS_TABLE_SIZE; i++)
        {
            size = 0.0;
            rem_size = 0.0;
            row = g_tivx_resource_stats_table[i];
            useCount = row.max_used_value;
            for (j = 0; j < (int32_t)TIVX_MAX_CONFIG_PARAM_OBJECTS; j++)
            {
                if ((int32_t)row.object_types[0][j] > 0)
                {
                    size += (vx_float64)useCount * (vx_float64)ownSizeOfEnumType((vx_enum)row.object_types[0][j]) * (vx_float64)row.object_types[1][j];
                    rem_size += ((vx_float64)row.max_value - (vx_float64)useCount) * (vx_float64)ownSizeOfEnumType((vx_enum)row.object_types[0][j]) * (vx_float64)row.object_types[1][j];
                }
                else
                {
                    break;
                }
            }
            if (row.is_size_cumulative == (vx_bool)vx_true_e)
            {
                total_size += size;
                total_rem += rem_size;
            }
            if (strncmp(row.name, "TIVX_PLAT_MAX_OBJ_DESC_SHM_INST", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                total_obj_size = (vx_float64)row.max_used_value * (vx_float64)(sizeof(tivx_obj_desc_shm_entry_t));
            }
            if (displayMode == (vx_enum)TIVX_MEM_LOG_DEFAULT)
            {
                continue;
            }
            if (strncmp(row.name, "TIVX_CONTEXT_MAX_KERNELS", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                printOutput(ofp, "|                                      | APPLICATION REQUESTED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|           CORE APPLICATION           |------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|               OBJECTS                | UNITS |  MAX  |   %%    |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |  USED | UNITS |  USED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|--------------------------------------|-------|-------|--------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
            }
            else if (strncmp(row.name, "TIVX_ARRAY_MAX_MAPS", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                printOutput(ofp, "|--------------------------------------|------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      | APPLICATION REQUESTED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|           APPLICATION DATA           |------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|               OBJECTS                | UNITS |  MAX  |   %%    |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |  USED | UNITS |  USED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|--------------------------------------|-------|-------|--------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
            }
            else if (strncmp(row.name, "TIVX_ERROR_MAX_OBJECTS", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                printOutput(ofp, "|--------------------------------------|------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |   FRAMEWORK REQUIRED   |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|             MISCELLANEOUS            |------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                OBJECTS               | UNITS |  MAX  |   %%    |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |  USED | UNITS |  USED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|--------------------------------------|-------|-------|--------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
            }
            else if (strncmp(row.name, "TIVX_PARAMETER_MAX_OBJECTS", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                printOutput(ofp, "|--------------------------------------|------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |   FRAMEWORK REQUIRED   |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                  TI                  |------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|              EXTENSIONS              | UNITS |  MAX  |   %%    |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |  USED | UNITS |  USED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|--------------------------------------|-------|-------|--------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
            }
            else if (strncmp(row.name, "TIVX_EVENT_MAX_OBJECTS", TIVX_RESOURCE_NAME_MAX) == 0)
            {
                printOutput(ofp, "|--------------------------------------|------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |   FRAMEWORK REQUIRED   |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                POSIX                 |------------------------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|               OBJECTS                | UNITS |  MAX  |   %%    |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|                                      |  USED | UNITS |  USED  |");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "|\n");
                printOutput(ofp, "|--------------------------------------|-------|-------|--------|");
                for (k = 0; k < char_width; k++)
                {
                    printOutput(ofp, "-");
                }
                printOutput(ofp, "|\n");
            }
            else
            {
                /* do nothing */
            }
            if ((displayMode == (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR) && (row.is_obj_desc == (vx_bool)vx_false_e))
            {
                continue;
            }
            if (strncmp(row.name, prefix, 8) != 0)
            {
                printOutput(ofp, "|                                      |       |       |        |");
                if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
                {
                    printOutput(ofp, "                 |                 |");
                }
                if (displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL)
                {
                    printOutput(ofp, "                   |");
                }
                printOutput(ofp, "\n");
            }
            if (row.is_size_cumulative == (vx_bool)vx_false_e)
            {
                printOutput(ofp, "| %s *", row.name);
            }
            else
            {
                printOutput(ofp, "| %s  ", row.name);
            }
            (void)strncpy(prefix, row.name, TIVX_RESOURCE_NAME_MAX);
            name_length = findMacroSize(row.name);
            for (k = 0; k < ((int32_t)TIVX_RESOURCE_NAME_MAX - (int32_t)name_length - 4); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|");
            numDigits = getNumDigits((int32_t)useCount);
            for (k = 0; k < (5 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "%u  |", useCount);
            numDigits = getNumDigits((int32_t)(row.max_value));
            for (k = 0; k < (5 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "%u  |", (row.max_value));
            percent_used = (vx_float64)100 * ((vx_float64)useCount / (vx_float64)row.max_value);
            numDigits = getNumDigits((int32_t)percent_used);
            for (k = 0; k < (3 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "%.2f%% |", percent_used);

            if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
            {
                size_units = applyMemoryUnit(&size, unit);
                numDigits = getNumDigits((int32_t)size);
                for (k = 0; k < (7 - (int32_t)numDigits); k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "%.2f %s    |", size, size_units);
                rem_units = applyMemoryUnit(&rem_size, unit);
                numDigits = getNumDigits((int32_t)rem_size);
                for (k = 0; k < (7 - (int32_t)numDigits); k++)
                {
                    printOutput(ofp, " ");
                }
                printOutput(ofp, "%.2f %s    |", rem_size, rem_units);
            }
            if (displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL)
            {
                if ((row.is_obj_desc == (vx_bool)vx_true_e) && (strncmp(row.name, "TIVX_PLAT_MAX_OBJ_DESC_SHM_INST", TIVX_RESOURCE_NAME_MAX)!=0))
                {
                    obj_size = (vx_float64)(sizeof(tivx_obj_desc_shm_entry_t)) * (vx_float64)(g_tivx_obj_desc_shm_table[obj_count].value);
                    obj_units = applyMemoryUnit(&obj_size, unit);
                    numDigits = getNumDigits((int32_t)obj_size);
                    for (k = 0; k < (7 - (int32_t)numDigits); k++)
                    {
                        printOutput(ofp, " ");
                    }
                    printOutput(ofp, "%.2f %s      |", obj_size, obj_units);
                    obj_count++;
                }
                else
                {
                    printOutput(ofp, "       -----       |");
                }
            }
            printOutput(ofp, "\n");
        }
        if ((displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL) && (displayMode != (vx_enum)TIVX_MEM_LOG_DEFAULT))
        {
            printOutput(ofp, "|--------------------------------------|------------------------|");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|                                      |                        |");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|            MISCELLANEOUS             |          UNITS         |");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|          OBJECT DESCRIPTORS          |        ALLOCATED       |");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|          (NOT CONFIGURABLE)          |                        |");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|--------------------------------------|------------------------|");
            for (k = 0; k < char_width; k++)
            {
                printOutput(ofp, "-");
            }
            printOutput(ofp, "|\n");
            printOutput(ofp, "|                                      |                        |");
            if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
            {
                printOutput(ofp, "                 |                 |                   |");
            }
            else
            {
                printOutput(ofp, "                   |");
            }
            /* removed condition check and print
             * as it is already confirmed at the start of this if condition
             */
            printOutput(ofp, "\n");

            while (obj_count < (int32_t)TIVX_OBJ_DESC_SHM_TABLE_SIZE)
            {
                obj_row = g_tivx_obj_desc_shm_table[obj_count];
                if (obj_row.is_configured == (vx_bool)vx_false_e)
                {
                    obj_size = (vx_float64)(obj_row.value) * (vx_float64)(sizeof(tivx_obj_desc_shm_entry_t));
                    obj_units = applyMemoryUnit(&obj_size, unit);
                    printOutput(ofp, "| %s", obj_row.name);
                    name_length = findMacroSize(obj_row.name);
                    for (k = 0; k < ((int32_t)TIVX_RESOURCE_NAME_MAX - (int32_t)name_length - 2); k++)
                    {
                        printOutput(ofp, " ");
                    }
                    printOutput(ofp, "|");
                    numDigits = getNumDigits((int32_t)obj_row.value);
                    for (k = 0; k < (13 - (int32_t)numDigits); k++)
                    {
                        printOutput(ofp, " ");
                    }
                    printOutput(ofp, "%u           |", obj_row.value);
                    if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
                    {
                        printOutput(ofp, "      -----      |      -----      |");
                    }
                    numDigits = getNumDigits((int32_t)obj_size);
                    for (k = 0; k < (7 - (int32_t)numDigits); k++)
                    {
                        printOutput(ofp, " ");
                    }
                    printOutput(ofp, "%.2f %s", obj_size, obj_units);
                    printOutput(ofp, "      |\n");
                    printOutput(ofp, "|                                      |                        |");
                    if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
                    {
                        printOutput(ofp, "                 |                 |");
                    }
                    printOutput(ofp, "                   |\n");
                }
                obj_count++;
            }
        }
        if (displayMode == (vx_enum)TIVX_MEM_LOG_GLOBAL)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_GLOBAL-4;
        }
        else if (displayMode == (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_OBJECT_DESCRIPTOR-4;
        }
        else if (displayMode == (vx_enum)TIVX_MEM_LOG_ALL)
        {
            char_width = TIVX_MEM_CHAR_WIDTH_ALL-4;
        }
        else
        {
            /* do nothing */
        }
        printOutput(ofp, "|");
        for (k = 0; k < ((char_width*2)+31); k++)
        {
            printOutput(ofp, "-");
        }
        printOutput(ofp, "|\n");
        if (displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR)
        {
            size_units = applyMemoryUnit(&total_size, unit);
            rem_units = applyMemoryUnit(&total_rem, unit);
            numDigits = getNumDigits((int32_t)total_size);
            printOutput(ofp, "|");
            for (k = 0; k < (char_width-1); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "TIOVX GLOBAL MEMORY USED: %.2f %s", total_size, size_units);
            for (k = 0; k < (char_width - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            numDigits = getNumDigits((int32_t)total_rem);
            for (k = 0; k < (char_width-4); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "TIOVX GLOBAL MEMORY REMAINING: %.2f %s", total_rem, rem_units);
            for (k = 0; k < (char_width-2 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
        }
        if (displayMode != (vx_enum)TIVX_MEM_LOG_GLOBAL)
        {
            (void)tivxPlatformGetShmSize(&shm_size);
            rem_obj_size = (vx_float64)shm_size - total_obj_size;
            rem_obj_units = applyMemoryUnit(&rem_obj_size, unit);
            obj_units = applyMemoryUnit(&total_obj_size, unit);
            printOutput(ofp, "|");
            for (k = 0; k < ((char_width*2)+31); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            numDigits = getNumDigits((int32_t)total_obj_size);
            for (k = 0; k < (char_width-3); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "OBJECT DESCRIPTOR MEMORY USED: %.2f %s", total_obj_size, obj_units);
            for (k = 0; k < (char_width-3 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n|");
            numDigits = getNumDigits((int32_t)rem_obj_size);
            for (k = 0; k < (char_width-6); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "OBJECT DESCRIPTOR MEMORY REMAINING: %.2f %s", rem_obj_size, rem_obj_units);
            for (k = 0; k < (char_width-5 - (int32_t)numDigits); k++)
            {
                printOutput(ofp, " ");
            }
            printOutput(ofp, "|\n");
        }
        printOutput(ofp, "|");
        for (k = 0; k < ((char_width*2)+31); k++)
        {
            printOutput(ofp, "-");
        }
        printOutput(ofp, "|\n\n");
        if ((displayMode != (vx_enum)TIVX_MEM_LOG_OBJECT_DESCRIPTOR) && (displayMode != (vx_enum)TIVX_MEM_LOG_DEFAULT))
        {
            printOutput(ofp, "* The global memory consumed by these parameters is counted in other resources. They aren't included\n");
            printOutput(ofp, "  in the total memory used to avoid multiple counts.\n\n");
        }
        printOutput(ofp, "Memory Management Documentation:\n");
        printOutput(ofp, "https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/TIOVX_MEMORY_MANAGEMENT.html\n\n");
        printOutput(ofp, "Memory Map Documentation:\n");
        printOutput(ofp, "https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/psdk_rtos/docs/user_guide/developer_notes_memory_map.html\n\n");

        if (ofp != NULL)
        {
            (void)fclose(ofp);
        }

        if((vx_status)VX_SUCCESS != tivxMutexUnlock(g_tivx_log_resource_lock))
        {
            VX_PRINT(VX_ZONE_ERROR,"Failed to unlock mutex\n");
        }

    /* Note: this requires access to the filesystem in order to generate this, and thus excluding from RTOS builds */
    #if defined(LINUX) || defined(QNX)
        const char *env;
        env = test_file_path();
        if (NULL != env)
        {
            char outputFileName[TIVX_CONFIG_PATH_LENGTH];
            uint32_t new_size;
            (void)snprintf(outputFileName, TIVX_CONFIG_PATH_LENGTH, "%s/output/%s", env, "gen_new_mem_map.sh");
            new_size = ((uint32_t)total_obj_size + ((uint32_t)16777216 - (uint32_t)total_obj_size))/(uint32_t)(1024*1024);
            ofp = fopen(outputFileName, "w");
            if (ofp == NULL)
            {
                status = (vx_status)VX_FAILURE;
            }
            else
            {
                printOutput(ofp, "if [[ -z \"${VX_TEST_DATA_PATH}\" ]]; then\n");
                printOutput(ofp, "    echo \"ERROR: VX_TEST_DATA_PATH has not been defined.\"\n");
                printOutput(ofp, "    exit 1\n");
                printOutput(ofp, "fi\n");
                printOutput(ofp, "if [[ -z \"${PATH_VISION_APPS}\" ]]; then\n");
                printOutput(ofp, "    echo \"ERROR: PATH_VISION_APPS has not been defined.\"\n");
                printOutput(ofp, "    exit 1\n");
                printOutput(ofp, "fi\n");
                printOutput(ofp, "if [[ -z \"${SOC}\" ]]; then\n");
                printOutput(ofp, "    echo \"ERROR: SOC has not been defined.\"\n");
                printOutput(ofp, "    exit 1\n");
                printOutput(ofp, "fi\n\n");
                printOutput(ofp, "cd ${PATH_VISION_APPS}/platform/${SOC}/rtos\n\n");
                printOutput(ofp, "yes | cp gen_linker_mem_map.py ${VX_TEST_DATA_PATH}/output/gen_new_linker_mem_map.py\n\n");
                printOutput(ofp, "cd ${VX_TEST_DATA_PATH}/output\n\n");
                printOutput(ofp, "sed -i \"s/tiovx_obj_desc_mem_size\\ =\\ ..\\*MB/tiovx_obj_desc_mem_size\\ =\\ %d\\*MB/\" gen_new_linker_mem_map.py\n\n", new_size);
                printOutput(ofp, "python gen_new_linker_mem_map.py\n");

                char buffer[TIVX_CONFIG_PATH_LENGTH];
                #pragma GCC diagnostic push
                #pragma GCC diagnostic ignored "-Wformat-truncation"
                (void)snprintf(buffer, TIVX_CONFIG_PATH_LENGTH, "chmod +x %s", outputFileName);
                #pragma GCC diagnostic pop
                status = system(buffer);
                (void)fclose(ofp);
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "VX_TEST_DATA_PATH has not been set!\n");
        }
#endif
    } /* if ((vx_status)VX_SUCCESS==status) */

#endif
#endif /* #if defined(BUILD_DEV) */

    return status;
}
