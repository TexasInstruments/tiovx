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



#ifndef TIVX_OBJ_DESC_H_
#define TIVX_OBJ_DESC_H_

#include <TI/tivx_mem.h>
#include <TI/tivx_config.h>

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
#define TIVX_OBJ_DESC_MAX_SHM_ENTRY_SIZE        (384U)

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
    uint16_t obj_desc_id;

    /*! \brief Type of object descritor, see \ref tivx_obj_desc_type_e */
    uint16_t type;

    /*! \brief object descriptor ID of the scope in which this object is created
     *         For element of pyramid and object array, this is obj_desc_id
     *         of excompassing pyramid, object array
     */
    uint16_t scope_obj_desc_id;

    /*! \brief reserved field to make this structure a multiple of 64b */
    uint16_t rsv1[1];

    /*! \brief Host reference, accessible only on HOST side */
    uint64_t host_ref;

    /*! \brief reference flags */
    uint32_t flags;
    
    /*! \brief reserved field to make this structure a multiple of 64b */
    uint32_t rsv2[1];

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
    uint32_t flags;

    /*! \brief ID of kernel to execute on target */
    uint32_t kernel_id;

    /*! \brief kernel name object descriptor ID */
    uint32_t kernel_name_obj_desc_id;

    /*! \brief ID of target to execute on. Set by host */
    uint32_t target_id;

    /*! \brief command to send when node completes execution
     *
     *         This is set by host in following conditions
     *         - when user callback is attached and host wants intimation
     *           of node execution complete in order to call user callback
     *         - when this is a leaf node and host needs intimation of node
     *           execution complete in order to check for graph execution
     *           complete
     */
    uint32_t node_complete_cmd_obj_desc_id;

    /*! \brief Index in target kernel table
     *
     *         Used for fast indexing to kernel functions during graph execute
     *         phase.
     *
     *         This is filled by target side during node create phase
     */
    uint32_t target_kernel_index[TIVX_NODE_MAX_REPLICATE];

    /*! \brief node execution status */
    uint32_t exe_status;

    /*! \brief node execution time */
    uint32_t exe_time_beg_h;

    /*! \brief node execution time */
    uint32_t exe_time_beg_l;

    /*! \brief node execution time */
    uint32_t exe_time_end_h;

    /*! \brief node execution time */
    uint32_t exe_time_end_l;

    /*! \brief number of parameters associated with this node */
    uint32_t num_params;

    /*! \brief Number of nodes that are connected to output of this node */
    uint32_t num_out_nodes;

    /*! \brief Number of nodes that are connected to output of this node */
    uint32_t num_in_nodes;

    /*! \brief border mode */
    vx_border_t border_mode;

    /*! \brief bitmask which indicates if prm is replicated
     *         valid only when TIVX_NODE_FLAG_IS_REPLICATED is
     *         set in flags.
     *         bitN = 1 means param at index N is replicated
     */
    uint32_t is_prm_replicated;

    /*! \brief number of times node is replicated
     *         valid only when TIVX_NODE_FLAG_IS_REPLICATED is
     *         set in flags.
     */
    uint32_t num_of_replicas;

    /*! \brief parameter object descriptors */
    uint16_t data_id[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief parameter data ref q object descriptors,
     * valid only when is_prm_data_ref_q is set */
    uint16_t data_ref_q_id[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief parameter object descriptors */
    uint16_t out_node_id[TIVX_NODE_MAX_OUT_NODES];

    /*! \brief parameter object descriptors */
    uint16_t in_node_id[TIVX_NODE_MAX_IN_NODES];

    /*! \brief node state one of TIVX_NODE_OBJ_DESC_STATE_IDLE,
     *       TIVX_NODE_OBJ_DESC_STATE_BLOCKED
     */
    uint16_t state;

    /*! \brief node ID that is blocked on this node to be IDLE
     */
    uint16_t blocked_node_id;

    /*! \brief pipeline ID to which this node obj desc belongs */
    uint16_t pipeline_id;

    /*! \brief node in the previous pipeline relative to this node */
    uint16_t prev_pipe_node_id;

    /*! \brief bitmask which indicates if prm is input to a node
     *         bitN = 1 means param at index N is input to node
     */
    uint32_t is_prm_input;

    /*! \brief bitmask which indicates if prm is data ref q
     *         bitN = 1 means param at index N is data ref q
     */
    uint32_t is_prm_data_ref_q;

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
    uint32_t width;
    /*! \brief Height of image in lines */
    uint32_t height;
    /*! \brief Data format of image, see \ref vx_df_image */
    uint32_t format;
    /*! \brief Number of data planes in image */
    uint32_t planes;
    /*! \brief Color space of the image, see \ref vx_color_space_e */
    uint32_t color_space;
    /*! \brief Color range of the image channel, see \ref vx_channel_range_e */
    uint32_t color_range;
    /*! \brief image plane buffer size */
    uint32_t mem_size[TIVX_IMAGE_MAX_PLANES];
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
    uint32_t uniform_image_pixel_value;
    /*! \brief method by which image was created, see \ref tivx_image_create_type_e */
    uint32_t create_type;
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
    uint32_t src_width;
    /*! \brief The source height */
    uint32_t src_height;
    /*! \brief The destination width */
    uint32_t dst_width;
    /*! \brief The destination height */
    uint32_t dst_height;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;
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
    uint32_t rows;
    /*! \brief number of columns */
    uint32_t columns;
    /*! \brief position at which to place the mask */
    uint32_t origin_x;
    /*! \brief position at which to place the mask  */
    uint32_t origin_y;
    /*! \brief Pattern of the matrix \ref vx_pattern_e */
    vx_enum pattern;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;
    /*! \brief From \ref vx_type_e */
    vx_enum data_type;

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
    vx_enum item_type;
    /*! \brief size of each item */
    vx_size item_size;
    /*! \brief number of items */
    vx_size num_items;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;

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
    vx_size num_levels;
    /*! \brief width of the level0 image */
    vx_uint32 width;
    /*! \brief height of the level0 image */
    uint32_t height;
    /*! \brief Scaling factor between levels of the pyramid. */
    vx_float32 scale;
    /*! \brief image format */
    vx_df_image format;
    /*! \brief array of object descriptor ids for the image object */
    uint16_t obj_desc_id[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
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
    uint32_t rows;
    /*! \brief number of columns */
    uint32_t columns;
    /*! \brief scale factor */
    uint32_t scale;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;
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
    vx_enum type;
    /*! \brief threshold value*/
    int32_t value;
    /*! \brief upper limit */
    int32_t upper;
    /*! \brief lower limit*/
    int32_t lower;
    /*! \brief threshold true value*/
    int32_t true_value;
    /*! \brief threshold false value*/
    int32_t false_value;
    /*! \brief data type of the threshold */
    vx_enum data_type;
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
    uint32_t num_bins;
    /*! \brief number of columns */
    uint32_t offset;
    /*! \brief scale factor */
    uint32_t range;
    /*! \brief number of windows */
    uint32_t num_win;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;

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
    vx_enum item_type;
    /*! \brief Size of the array item */
    uint32_t item_size;
    /*! \brief number of valid items in array */
    uint32_t num_items;
    /*! \brief Max size of the array */
    uint32_t capacity;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;

} tivx_obj_desc_array_t;

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
    vx_enum item_type;
    /*! \brief number of valid items in array */
    uint32_t num_items;
    /*! \brief array of descriptor ids of the objects */
    uint16_t obj_desc_id[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    
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
    uint32_t  data_type;
    /*! \brief Reserved field to make below union on 64b aligned boundary */
    uint32_t rsv;
    
    union {
        /*! \brief A character */
        vx_char   chr;
        /*! \brief Signed 8 bit */
        vx_int8   s08;
        /*! \brief Unsigned 8 bit */
        vx_uint8  u08;
        /*! \brief Signed 16 bit */
        vx_int16  s16;
        /*! \brief Unsigned 16 bit */
        vx_uint16 u16;
        /*! \brief Signed 32 bit */
        vx_int32  s32;
        /*! \brief Unsigned 32 bit */
        vx_uint32 u32;
        /*! \brief Signed 64 bit */
        vx_int64  s64;
        /*! \brief Unsigned 64 bit */
        vx_int64  u64;
#if defined(EXPERIMENTAL_PLATFORM_SUPPORTS_16_FLOAT)
        /*! \brief 16 bit float */
        vx_float16 f16;
#endif
        /*! \brief 32 bit float */
        vx_float32 f32;
        /*! \brief 64 bit float */
        vx_float64 f64;
        /*! \brief 32 bit image format code */
        vx_df_image  fcc;
        /*! \brief Signed 32 bit*/
        vx_enum    enm;
        /*! \brief Architecture depth unsigned value */
        vx_size    size;
        /*! \brief Boolean Values */
        vx_bool    boolean;
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
    uint32_t number_of_dimensions;
    /*! \brief Data type of tensor */
    uint32_t data_type;
    /*! \brief Fixed point precision of the tensor */
    uint32_t fixed_point_position;
    /*! \brief Size of all dimensions */
    uint32_t dimensions[TIVX_CONTEXT_MAX_TENSOR_DIMS];
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
    uint32_t stride[TIVX_CONTEXT_MAX_TENSOR_DIMS];
    /*! \brief Buffer size */
    uint32_t mem_size;
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
void tivxGetObjDescList(uint16_t obj_desc_id[],
    tivx_obj_desc_t *obj_desc[], uint32_t num_desc_id);

#ifdef __cplusplus
}
#endif

#endif
