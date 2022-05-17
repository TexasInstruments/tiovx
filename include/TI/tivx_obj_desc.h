/*
*
* Copyright (c) 2017-2019 Texas Instruments Incorporated
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



#ifndef TIVX_OBJ_DESC_H_
#define TIVX_OBJ_DESC_H_

#include <TI/tivx_mem.h>
#include <TI/tivx_config.h>
#include <TI/tivx_ext_raw_image.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to object descriptor
 */

/*! \brief Macro to check max shared mem entry size
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE        (1024U)

/*!
 * \brief Max possible planes of data in an image
 *
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_IMAGE_MAX_PLANES   (3u)

/*!
 * \brief Max possible dimensions of data in a tensor
 *
 * \ingroup group_tivx_obj_desc_cfg
 */
#define TIVX_CONTEXT_MAX_TENSOR_DIMS (4)

/*! \brief Flag to indicate if run-time trace should be logged
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_REF_FLAG_LOG_RT_TRACE            (0x00000001u)

/*! \brief Flag to indicate if ref is invalid
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_REF_FLAG_IS_INVALID              (0x00000002u)

/*! \brief Flag to indicate if node is replicated
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_REPLICATED     (0x00000001u)

/*! \brief Flag to indicate if node is executed
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_EXECUTED       (0x00000002u)

/*! \brief Flag to indicate if user callback is requested after node complete
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_USER_CALLBACK  (0x00000004u)

/*! \brief Flag to indicate if kernel associated with this node is a
 *         user kernel or target kernel
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_TARGET_KERNEL  (0x00000008u)

/*! \brief Flag to indicate if this node is a supernode
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_SUPERNODE  (0x00000010u)

/*! \brief State of a node object descriptor to indicate it is IDLE
 *
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_OBJ_DESC_STATE_IDLE                    (0x0u)

/*! \brief State of a node object descriptor to indicate it is
 *         BLOCKED
 *
 *
 *         There are two condition for which a node can be block
 *         1. prev node obj desc in pipeline is waiting to be
 *         executed
 *         2. parameters could not be acquired
 *
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_OBJ_DESC_STATE_BLOCKED                  (0x1u)


/*! \brief Max size of the array host_port_id, in tivx_obj_desc_t
 *
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_OBJ_DESC_MAX_HOST_PORT_ID_CPU                (16u)

/*!
 * \brief Enum that list all possible object descriptor type's
 *
 * \ingroup group_tivx_obj_desc
 */
typedef enum _tivx_obj_desc_type_e {

    /*! \brief Object desciptor that has information related to image object */
    TIVX_OBJ_DESC_IMAGE         = 0x0,

    /*! \brief Object desciptor that has information related to scalar object */
    TIVX_OBJ_DESC_SCALAR        = 0x1,

    /*! \brief Object desciptor that has information related to remap object */
    TIVX_OBJ_DESC_REMAP         = 0x2,

    /*! \brief Object desciptor that has information related to node object */
    TIVX_OBJ_DESC_NODE          = 0x3,

    /*! \brief Object desciptor that has information related to command object */
    TIVX_OBJ_DESC_CMD           = 0x4,

    /*! \brief Object desciptor that has information related to matrix object */
    TIVX_OBJ_DESC_MATRIX        = 0x5,

    /*! \brief Object desciptor that has information related to Lut object */
    TIVX_OBJ_DESC_LUT           = 0x6,

    /*! \brief Object desciptor that has information related to
        Convolution object */
    TIVX_OBJ_DESC_CONVOLUTION   = 0x7,

    /*! \brief Object desciptor that has information related to
        Distribution object */
    TIVX_OBJ_DESC_DISTRIBUTION  = 0x8,

    /*! \brief Object desciptor that has information related to
        threshold object */
    TIVX_OBJ_DESC_THRESHOLD     = 0x9,

    /*! \brief Object desciptor that has information related to
        Array object */
    TIVX_OBJ_DESC_ARRAY         = 0xA,

    /*! \brief Object desciptor that has information related to
        Pyramid object */
    TIVX_OBJ_DESC_PYRAMID       = 0xB,

    /*! \brief Object desciptor that has information related to
        Object Array object */
    TIVX_OBJ_DESC_OBJARRAY      = 0xC,

    /*! \brief Object desciptor that has information related to
        kernel name */
    TIVX_OBJ_DESC_KERNEL_NAME   = 0xD,

    /*! \brief Object desciptor queue */
    TIVX_OBJ_DESC_QUEUE         = 0xE,

    /*! \brief Object desciptor data ref queue */
    TIVX_OBJ_DESC_DATA_REF_Q    = 0xF,

    /*! \brief Object desciptor graph */
    TIVX_OBJ_DESC_GRAPH         = 0x10,

    /*! \brief Object desciptor that has information related to
        tensor object */
    TIVX_OBJ_DESC_TENSOR        = 0x11,

    /*! \brief Object desciptor that has information related to
        user data object */
    TIVX_OBJ_DESC_USER_DATA_OBJECT = 0x12,

    /*! \brief Object desciptor that has information related to
        raw image object */
    TIVX_OBJ_DESC_RAW_IMAGE     = 0x13,

    /*! \brief Object desciptor that has information related to
        super node */
    TIVX_OBJ_DESC_SUPER_NODE   = 0x14,

    /*! \brief Value of a invalid object descriptor */
    TIVX_OBJ_DESC_INVALID       = 0xFFFFu

} tivx_obj_desc_type_e;

/*!
 * \brief Method by which image is created
 *
 * \ingroup group_tivx_obj_desc
 */
typedef enum _tivx_image_create_type_e
{
    /*! \brief Create using vxCreateImage() */
    TIVX_IMAGE_NORMAL,
    /*! \brief Create using vxCreateUniformImage() */
    TIVX_IMAGE_UNIFORM,
    /*! \brief Create using vxCreateVirtualImage() */
    TIVX_IMAGE_VIRTUAL,
    /*! \brief Create using vxCreateImageFromHandle() */
    TIVX_IMAGE_FROM_HANDLE,
    /*! \brief Create using vxCreateImageFromROI() */
    TIVX_IMAGE_FROM_ROI,
    /*! \brief Create using vxCreateImageFromChannel() */
    TIVX_IMAGE_FROM_CHANNEL

} tivx_image_create_type_e;

/*!
 * \brief Remap point in remap table
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_remap_point {

    vx_float32 src_x;
    vx_float32 src_y;

} tivx_remap_point_t;

/*!
 * \brief Object descriptor
 *
 *        Must be first element of any objects descriptors
 *        placed in shared memory
 *
 *        Make sure this structure is multiple of 64b
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_t {

    /*! \brief ID of object in shared memory */
    volatile uint16_t obj_desc_id;

    /*! \brief Type of object descritor, see \ref tivx_obj_desc_type_e */
    volatile uint16_t type;

    /*! \brief object descriptor ID of the scope in which this object is created
     *         For element of pyramid and object array, this is obj_desc_id
     *         of excompassing pyramid, object array
     */
    volatile uint16_t scope_obj_desc_id;

    /*! \brief number of input nodes that have consumed this obj_desc, used in pipelining mode only for data references */
    volatile uint16_t in_node_done_cnt;

    /*! \brief Host reference, accessible only on HOST side */
    volatile uint64_t host_ref;

    /*! \brief reference flags */
    volatile uint32_t flags;

    /*! \brief holds the index ID in the case that this is an element within a pyramid or object array */
    volatile uint32_t element_idx;

    /*! \brief reference timestamp */
    volatile uint64_t timestamp;

    /*! \brief holds the CPU ID of the OpenVX host for this object descriptor */
    volatile uint32_t host_cpu_id;

    /*! \brief reserved to make 64b aligned */
    volatile uint32_t rsv0;

    /*! \brief holds the index of the IPC port on OpenVX host for this object descriptor for each remote CPU
     *
     *         A connection to each remote CPU could be on a different host_port_id, hence this needs
     *         to hold host_port_id for each remote CPU, this array is indexed by TIOVX CPU ID.
     *         TIVX_OBJ_DESC_MAX_HOST_PORT_ID_CPU MUST be <= TIVX_CPU_ID_MAX
     * */
    volatile uint16_t host_port_id[TIVX_OBJ_DESC_MAX_HOST_PORT_ID_CPU];


} tivx_obj_desc_t;

/*!
 * \brief Node object descriptor
 *
 *        Fields with name target_* are only accessed by target CPU
 *        Fields with name host_* are only accessed by host CPU
 *        Other fields are accessed by both target and host
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_node
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;

    /*! \brief node flags see TIVX_NODE_FLAG_xxx
     */
    volatile uint32_t flags;

    /*! \brief ID of kernel to execute on target */
    volatile uint32_t kernel_id;

    /*! \brief kernel name object descriptor ID */
    volatile uint32_t kernel_name_obj_desc_id;

    /*! \brief ID of target to execute on. Set by host */
    volatile uint32_t target_id;

    /*! \brief command to send when node completes execution
     *
     *         This is set by host in following conditions
     *         - when user callback is attached and host wants intimation
     *           of node execution complete in order to call user callback
     *         - when this is a leaf node and host needs intimation of node
     *           execution complete in order to check for graph execution
     *           complete
     */
    volatile uint32_t node_complete_cmd_obj_desc_id;

    /*! \brief Index in target kernel table
     *
     *         Used for fast indexing to kernel functions during graph execute
     *         phase.
     *
     *         This is filled by target side during node create phase
     */
    volatile uint32_t target_kernel_index[TIVX_NODE_MAX_REPLICATE];

    /*! \brief node execution status */
    volatile uint32_t exe_status;

    /*! \brief node execution time */
    volatile uint32_t exe_time_beg_h;

    /*! \brief node execution time */
    volatile uint32_t exe_time_beg_l;

    /*! \brief node execution time */
    volatile uint32_t exe_time_end_h;

    /*! \brief node execution time */
    volatile uint32_t exe_time_end_l;

    /*! \brief number of parameters associated with this node */
    volatile uint32_t num_params;

    /*! \brief Number of nodes that are connected to output of this node */
    volatile uint32_t num_out_nodes;

    /*! \brief Number of nodes that are connected to output of this node */
    volatile uint32_t num_in_nodes;

    /*! \brief border mode */
    vx_border_t border_mode;

    /*! \brief bitmask which indicates if prm is replicated
     *         valid only when TIVX_NODE_FLAG_IS_REPLICATED is
     *         set in flags.
     *         bitN = 1 means param at index N is replicated
     */
    volatile uint32_t is_prm_replicated;

    /*! \brief number of times node is replicated
     *         valid only when TIVX_NODE_FLAG_IS_REPLICATED is
     *         set in flags.
     */
    volatile uint32_t num_of_replicas;

    /*! \brief parameter object descriptors */
    volatile uint16_t data_id[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief parameter data ref q object descriptors,
     * valid only when is_prm_data_ref_q is set */
    volatile uint16_t data_ref_q_id[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief parameter object descriptors */
    volatile uint16_t out_node_id[TIVX_NODE_MAX_OUT_NODES];

    /*! \brief parameter object descriptors */
    volatile uint16_t in_node_id[TIVX_NODE_MAX_IN_NODES];

    /*! \brief node state one of TIVX_NODE_OBJ_DESC_STATE_IDLE,
     *       TIVX_NODE_OBJ_DESC_STATE_BLOCKED
     */
    volatile uint16_t state;

    /*! \brief node ID that is blocked on this node to be IDLE
     */
    volatile uint16_t blocked_node_id;

    /*! \brief pipeline ID to which this node obj desc belongs */
    volatile uint16_t pipeline_id;

    /*! \brief node in the previous pipeline relative to this node */
    volatile uint16_t prev_pipe_node_id;

    /*! \brief bitmask which indicates if prm is input to a node
     *         bitN = 1 means param at index N is input to node
     */
    volatile uint32_t is_prm_input;

    /*! \brief bitmask which indicates if prm is data ref q
     *         bitN = 1 means param at index N is data ref q
     */
    volatile uint32_t is_prm_data_ref_q;

    /*! \brief bitmask which indicates if prm is array element
     *         bitN = 1 means param at index N is array element
     */
    volatile uint32_t is_prm_array_element;

    /*! \brief Flag indicating whether or not node is in pipeup
     *         state or steady state
     */
    volatile uint32_t source_state;

    /*! \brief number of buffers required for pipeup state
     */
    volatile uint32_t num_pipeup_bufs;

    /*! \brief Pipe buf idx used for when to begin graph
     *         processing on source node
     */
    volatile uint32_t pipeup_buf_idx;

    /*! \brief variable to store the tile width for the given node
     */
    volatile uint32_t block_width;

    /*! \brief variable to store the tile height for the given node
     */
    volatile uint32_t block_height;

} tivx_obj_desc_node_t;

/*!
 * \brief Image object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_image
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief image plane buffer addresses */
    tivx_shared_mem_ptr_t mem_ptr[TIVX_IMAGE_MAX_PLANES];
    /*! \brief Width of image in pixels */
    volatile uint32_t width;
    /*! \brief Height of image in lines */
    volatile uint32_t height;
    /*! \brief Data format of image, see \ref vx_df_image */
    volatile uint32_t format;
    /*! \brief Number of data planes in image */
    volatile uint32_t planes;
    /*! \brief Color space of the image, see \ref vx_color_space_e */
    volatile uint32_t color_space;
    /*! \brief Color range of the image channel, see \ref vx_channel_range_e */
    volatile uint32_t color_range;
    /*! \brief image plane buffer size */
    volatile uint32_t mem_size[TIVX_IMAGE_MAX_PLANES];
    /*! \brief reserved for 64b alignment */
    volatile uint32_t rsv[1];
    /*! \brief the value to use to fill for a uniform image.
     *
     * bit 0.. 7 - Component 0 - R or Y or U8.
     * bit 8..15 - Component 1 - G or U.
     * bit16..23 - Component 2 - B or V.
     * bit24..31 - Component 3 - X.
     *
     * bit0..15 - U16, S16.
     * bit0..31 - U32, S32.
     */
    volatile uint32_t uniform_image_pixel_value;
    /*! \brief method by which image was created, see \ref tivx_image_create_type_e */
    volatile uint32_t create_type;
    /*! \brief image plane addressing parameters */
    vx_imagepatch_addressing_t imagepatch_addr[TIVX_IMAGE_MAX_PLANES];
    /*! \brief valid region of image to use for processing */
    vx_rectangle_t valid_roi;
} tivx_obj_desc_image_t;


/*!
 * \brief Remap object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_remap
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief buffer address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief The source width */
    volatile uint32_t src_width;
    /*! \brief The source height */
    volatile uint32_t src_height;
    /*! \brief The destination width */
    volatile uint32_t dst_width;
    /*! \brief The destination height */
    volatile uint32_t dst_height;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;
} tivx_obj_desc_remap_t;

/*!
 * \brief matrix object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_matrix
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief matrix memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief number of rows */
    volatile uint32_t rows;
    /*! \brief number of columns */
    volatile uint32_t columns;
    /*! \brief position at which to place the mask */
    volatile uint32_t origin_x;
    /*! \brief position at which to place the mask  */
    volatile uint32_t origin_y;
    /*! \brief Pattern of the matrix \ref vx_pattern_e */
    volatile vx_enum pattern;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;
    /*! \brief From \ref vx_type_e */
    volatile vx_enum data_type;

} tivx_obj_desc_matrix_t;

/*!
 * \brief lut object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_lut
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief lut memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief The item type of the lut. */
    volatile vx_enum item_type;
    /*! \brief size of each item */
    volatile vx_uint32 item_size;
    /*! \brief number of items */
    volatile vx_uint32 num_items;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;

} tivx_obj_desc_lut_t;

/*!
 * \brief pyramid object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_pyramid
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief number of pyramid levels */
    volatile vx_uint32 num_levels;
    /*! \brief width of the level0 image */
    volatile vx_uint32 width;
    /*! \brief height of the level0 image */
    volatile uint32_t height;
    /*! \brief Scaling factor between levels of the pyramid. */
    volatile vx_float32 scale;
    /*! \brief image format */
    volatile vx_df_image format;
    /*! \brief array of object descriptor ids for the image object */
    volatile uint16_t obj_desc_id[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
} tivx_obj_desc_pyramid_t;

/*!
 * \brief convolution object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_convolution
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief convolution memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief number of rows */
    volatile uint32_t rows;
    /*! \brief number of columns */
    volatile uint32_t columns;
    /*! \brief scale factor */
    volatile uint32_t scale;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;
} tivx_obj_desc_convolution_t;

/*!
 * \brief threshold object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_threshold
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief type of threshold */
    volatile vx_enum type;
    /*! \brief threshold value*/
    volatile int32_t value;
    /*! \brief upper limit */
    volatile int32_t upper;
    /*! \brief lower limit*/
    volatile int32_t lower;
    /*! \brief threshold true value*/
    volatile int32_t true_value;
    /*! \brief threshold false value*/
    volatile int32_t false_value;
    /*! \brief data type of the threshold */
    volatile vx_enum data_type;
} tivx_obj_desc_threshold_t;

/*!
 * \brief distribution object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_distribution
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief distribution memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief number of rows */
    volatile uint32_t num_bins;
    /*! \brief number of columns */
    volatile uint32_t offset;
    /*! \brief scale factor */
    volatile uint32_t range;
    /*! \brief number of windows */
    volatile uint32_t num_win;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;

} tivx_obj_desc_distribution_t;

/*!
 * \brief array object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_array
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief array memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief The item type of the array. */
    volatile vx_enum item_type;
    /*! \brief Size of the array item */
    volatile uint32_t item_size;
    /*! \brief number of valid items in array */
    volatile uint32_t num_items;
    /*! \brief Max size of the array */
    volatile uint32_t capacity;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;

} tivx_obj_desc_array_t;

/*!
 * \brief user data object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_user_data_object
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief user data object memory address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief size of buffer pointed to by mem_ptr */
    volatile uint32_t mem_size;
    /*! \brief size of valid data within the buffer */
    volatile uint32_t valid_mem_size;
    /*! \brief The type name of the user data object. */
    volatile vx_char type_name[VX_MAX_REFERENCE_NAME];

} tivx_obj_desc_user_data_object_t;

/*!
 * \brief raw image descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_raw_image
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief allocated buffer addresses (max 1 for each combined image and meta data) */
    tivx_shared_mem_ptr_t mem_ptr[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief image data buffer addresses */
    tivx_shared_mem_ptr_t img_ptr[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief meta before buffer addresses */
    tivx_shared_mem_ptr_t meta_before_ptr[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief meta after buffer addresses */
    tivx_shared_mem_ptr_t meta_after_ptr[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief create parameters for raw image */
    tivx_raw_image_create_params_t params;
    /*! \brief image plane buffer size */
    volatile uint32_t mem_size[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief method by which raw image was created, see \ref tivx_image_create_type_e */
    volatile uint32_t create_type;
    /*! \brief raw image exposure addressing parameters */
    vx_imagepatch_addressing_t imagepatch_addr[TIVX_RAW_IMAGE_MAX_EXPOSURES];
    /*! \brief valid region of raw image to use for processing */
    vx_rectangle_t valid_roi;

} tivx_obj_desc_raw_image_t;

/*!
 * \brief object array object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_object_array
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief The item type of the lut. */
    volatile vx_enum item_type;
    /*! \brief number of valid items in array */
    volatile uint32_t num_items;
    /*! \brief array of descriptor ids of the objects */
    volatile uint16_t obj_desc_id[TIVX_OBJECT_ARRAY_MAX_ITEMS];

} tivx_obj_desc_object_array_t;

/*!
 * \brief Scalar object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_scalar
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief The value contained in the reference for a scalar type */
    volatile uint32_t  data_type;
    /*! \brief Reserved field to make below union on 64b aligned boundary */
    volatile uint32_t rsv;

    union {
        /*! \brief A character */
        volatile vx_char   chr;
        /*! \brief Signed 8 bit */
        volatile vx_int8   s08;
        /*! \brief Unsigned 8 bit */
        volatile vx_uint8  u08;
        /*! \brief Signed 16 bit */
        volatile vx_int16  s16;
        /*! \brief Unsigned 16 bit */
        volatile vx_uint16 u16;
        /*! \brief Signed 32 bit */
        volatile vx_int32  s32;
        /*! \brief Unsigned 32 bit */
        volatile vx_uint32 u32;
        /*! \brief Signed 64 bit */
        volatile vx_int64  s64;
        /*! \brief Unsigned 64 bit */
        volatile vx_uint64  u64;
#if defined(EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT)
        /*! \brief 16 bit float */
        volatile vx_float16 f16;
#endif
        /*! \brief 32 bit float */
        volatile vx_float32 f32;
        /*! \brief 64 bit float */
        volatile vx_float64 f64;
        /*! \brief 32 bit image format code */
        volatile vx_df_image  fcc;
        /*! \brief Signed 32 bit*/
        volatile vx_enum    enm;
        /*! \brief Architecture depth unsigned value */
        volatile vx_size    size;
        /*! \brief Boolean Values */
        volatile vx_bool    boolean;
    } data;

} tivx_obj_desc_scalar_t;

/*!
 * \brief Tensor object descriptor as placed in shared memory
 *
 * \ingroup group_tivx_obj_desc
 */
typedef struct _tivx_obj_desc_tensor
{
    /*! \brief base object descriptor */
    tivx_obj_desc_t base;
    /*! \brief buffer address */
    tivx_shared_mem_ptr_t mem_ptr;
    /*! \brief Number of dimensions in the tensor */
    volatile uint32_t number_of_dimensions;
    /*! \brief Data type of tensor */
    volatile uint32_t data_type;
    /*! \brief Fixed point precision of the tensor */
    volatile uint32_t fixed_point_position;
    /*! \brief each element of the tensor can be divided by this scaling value in order to obtain its real value */
    volatile uint32_t scaling_divisor;
    /*! \brief Fixed point precision of the scaling divisor */
    volatile uint32_t scaling_divisor_fixed_point_position;
    /*! \brief Size of all dimensions */
    volatile uint32_t dimensions[TIVX_CONTEXT_MAX_TENSOR_DIMS];
    /*! \brief Stride of all dimensions
     *
     * stride[0] is more like bytes per element
     * stride[1] is offset in bytes from one line to next
     * stride[2] is offset in bytes from one frame/channel to next
     * stride[3] is offset in bytes from one batch to next
     *
     * How to calculate:
     *
     * stride[0] = sizeof(data_type)
     * for (i > 0)
     *    stride[i] = stride[i-1] * dimensions[i-1]
     */
    volatile uint32_t stride[TIVX_CONTEXT_MAX_TENSOR_DIMS];
    /*! \brief Buffer size */
    volatile uint32_t mem_size;
} tivx_obj_desc_tensor_t;

/*!
 * \brief Function to get the pointer to object descriptors for the
 *        given object descriptor id
 *
 * \param [in]  obj_desc_id  list of object descriptor id
 * \param [out] obj_desc     object descriptor pointer
 * \param [in]  num_desc_id  number of valid entries in obj_desc_id
 *
 * \ingroup group_tivx_obj_desc_cfg
 */
void tivxGetObjDescList(volatile uint16_t obj_desc_id[],
    tivx_obj_desc_t *obj_desc[], uint32_t num_desc_id);

/*!
 * \brief Function to get the pointer to an element of an object array. If the
 *        object  being passed is a regular object descriptor then a pointer to
 *        the object is returned.
 *
 * \param [in]  obj_desc     object descriptor pointer
 * \param [in]  elem_idx     A valid index with in the object array. This is
 *                           used only if obj_desc refers to an object
 *                           descriptor array, otherwise it it ignored.
 *
 * \ingroup group_tivx_obj_desc
 */
tivx_obj_desc_t *tivxGetObjDescElement(tivx_obj_desc_t *obj_desc, uint16_t elem_idx);

/*!
 * \brief Utility function for memory/string copy/set operation on object descriptor pointers
 *
 *   IMPORTANT NOTE:
 *   On some SoCs, obj desc's are allocated in a memory region where
 *   unaligned access results in 'Bus Error'. The fields with a obj desc
 *   are properly aligned so making an access to obj desc fields is fine.
 *   But when APIs like strncpy, memcpy, memset are
 *   used, it could result in an unaligned access. To avoid this, below functions
 *   should be used in place of standard strncpy, memcpy, meset functions.
 *   Note, when a operation like *a_struct = *b_struct is done it results in a memcpy so
 *   this kind of structure assignment should be avoided and below function used instead
 *
 * \ingroup group_tivx_obj_desc
 */
void tivx_obj_desc_strncpy(volatile void *dst, volatile void *src, uint32_t size);

/*!
 * \brief Utility function for memory/string copy/set operation on object descriptor pointers
 *
 *   IMPORTANT NOTE:
 *   On some SoCs, obj desc's are allocated in a memory region where
 *   unaligned access results in 'Bus Error'. The fields with a obj desc
 *   are properly aligned so making an access to obj desc fields is fine.
 *   But when APIs like strncpy, memcpy, memset are
 *   used, it could result in an unaligned access. To avoid this, below functions
 *   should be used in place of standard strncpy, memcpy, meset functions.
 *   Note, when a operation like *a_struct = *b_struct is done it results in a memcpy so
 *   this kind of structure assignment should be avoided and below function used instead
 *
 * \ingroup group_tivx_obj_desc
 */
void tivx_obj_desc_memcpy(volatile void *dst, volatile void *src, uint32_t size);

/*!
 * \brief Utility function for memory/string copy/set operation on object descriptor pointers
 *
 *   IMPORTANT NOTE:
 *   On some SoCs, obj desc's are allocated in a memory region where
 *   unaligned access results in 'Bus Error'. The fields with a obj desc
 *   are properly aligned so making an access to obj desc fields is fine.
 *   But when APIs like strncpy, memcpy, memset are
 *   used, it could result in an unaligned access. To avoid this, below functions
 *   should be used in place of standard strncpy, memcpy, meset functions.
 *   Note, when a operation like *a_struct = *b_struct is done it results in a memcpy so
 *   this kind of structure assignment should be avoided and below function used instead
 *
 * \ingroup group_tivx_obj_desc
 */
void tivx_obj_desc_memset(volatile void *dst, uint8_t val, uint32_t size);


/*!
 * \brief Utility function for memory/string compare operation on object descriptor pointers
 *
 *   IMPORTANT NOTE:
 *   On some SoCs, obj desc's are allocated in a memory region where
 *   unaligned access results in 'Bus Error'. The fields with a obj desc
 *   are properly aligned so making an access to obj desc fields is fine.
 *   But when APIs like strncpy, memcpy, memset are
 *   used, it could result in an unaligned access. To avoid this, below functions
 *   should be used in place of standard strncpy, memcpy, meset functions.
 *   Note, when a operation like *a_struct = *b_struct is done it results in a memcpy so
 *   this kind of structure assignment should be avoided and below function used instead
 *
 * \ingroup group_tivx_obj_desc
 */
int32_t tivx_obj_desc_strncmp(volatile void *dst, volatile void *src, uint32_t size);

/*!
 * \brief Utility function for memory/string compare operation on object descriptor pointers with delimiter
 *
 *   The function only compares until there is a mismatch, it gets to the end of the string, or it gets to
 *   the delimiter.  If the strings are the same before one of them reaches a delimiter, the result is still 0
 *
 *   IMPORTANT NOTE:
 *   On some SoCs, obj desc's are allocated in a memory region where
 *   unaligned access results in 'Bus Error'. The fields with a obj desc
 *   are properly aligned so making an access to obj desc fields is fine.
 *   But when APIs like strncpy, memcpy, memset are
 *   used, it could result in an unaligned access. To avoid this, below functions
 *   should be used in place of standard strncpy, memcpy, meset functions.
 *   Note, when a operation like *a_struct = *b_struct is done it results in a memcpy so
 *   this kind of structure assignment should be avoided and below function used instead
 *
 * \ingroup group_tivx_obj_desc
 */
int32_t tivx_obj_desc_strncmp_delim(volatile void *dst, volatile void *src, uint32_t size, char delim);

#ifdef __cplusplus
}
#endif

#endif
