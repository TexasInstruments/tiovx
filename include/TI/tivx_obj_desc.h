/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _TIVX_OBJ_DESC_H_
#define _TIVX_OBJ_DESC_H_

#include <TI/tivx_mem.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Interface to object descriptor
 */

/*!
 * \brief Max possible planes of data in an image
 *
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_IMAGE_MAX_PLANES   (3u)

/*! \brief Max parameters in a kernel
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_KERNEL_MAX_PARAMS      (8u)

/*! \brief Max nodes taking output form a given node
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_MAX_OUT_NODES      (8u)

/*! \brief Max nodes feeding input to a given node
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_MAX_IN_NODES      (8u)

/*! \brief Flag to indicate if node is replicated
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_REPLICATED     (0x00000001u)

/*! \brief Flag to indicate if node is executed
 * \ingroup group_tivx_obj_desc
 */
#define TIVX_NODE_FLAG_IS_EXECUTED       (0x00000002u)

/*!
 * \brief Enum that list all possible object descriptor type's
 *
 * \ingroup group_tivx_obj_desc
 */
typedef enum _tivx_obj_desc_type_e {

    /*! \brief Object desciptor that has information related to image object */
    TIVX_OBJ_DESC_IMAGE,

    /*! \brief Object desciptor that has information related to scalar object */
    TIVX_OBJ_DESC_SCALAR,

    /*! \brief Object desciptor that has information related to remap object */
    TIVX_OBJ_DESC_REMAP,

    /*! \brief Object desciptor that has information related to node object */
    TIVX_OBJ_DESC_NODE,

    /*! \brief Object desciptor that has information related to command object */
    TIVX_OBJ_DESC_CMD,

    /*! \brief Object desciptor that has information related to matrix object */
    TIVX_OBJ_DESC_MATRIX,

    /*! \brief Object desciptor that has information related to matrix object */
    TIVX_OBJ_DESC_LUT,

    /*! \brief Value of a invalid object descriptor */
    TIVX_OBJ_DESC_INVALID = 0xFFFFu

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

    /*! \brief reserved field to make this structure a multiple of 64b */
    uint16_t rsv[2];

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

    /*! \brief Handle to OpenVX Node reference, valid only on host side
     */
    uint32_t host_node_ref;

    /*! \brief Handle to kernel allocated by target during node create phase */
    uint32_t target_kernel_handle;

    /*! \brief Size of Handle created by target */
    uint32_t target_kernel_handle_size;

    /*! \brief Index in target kernel table
     *
     *         Used for fast indexing to kernel functions during graph execute
     *         phase.
     *
     *         This is filled by target side during node create phase
     */
    uint32_t target_kernel_index;

    /*! \brief node execution status */
    uint32_t exe_status;

    /*! \brief node execution time in units of usecs */
    uint32_t exe_time_usecs;

    /*! \brief number of parameters associated with this node */
    uint32_t num_params;

    /*! \brief Number of nodes that are connected to output of this node */
    uint32_t num_out_nodes;

    /*! \brief Number of nodes that are connected to output of this node */
    uint32_t num_in_nodes;

    /*! \brief border mode */
    vx_border_t border_mode;

    /*! \brief parameter object descriptors */
    uint16_t data_id[TIVX_KERNEL_MAX_PARAMS];

    /*! \brief parameter object descriptors */
    uint16_t out_node_id[TIVX_MAX_OUT_NODES];

    /*! \brief parameter object descriptors */
    uint16_t in_node_id[TIVX_MAX_IN_NODES];

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
    /*! \brief image plane buffer addresses */
    tivx_shared_mem_ptr_t mem_ptr[TIVX_IMAGE_MAX_PLANES];
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
    /*! \brief buffer address */
    tivx_shared_mem_ptr_t mem_ptr;
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
    /*! \brief The source width */

    /*! \brief number of rows */
    uint32_t rows;
    /*! \brief number of columns */
    uint32_t columns;
    /*! \brief Pattern of the matrix */
    vx_enum pattern;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;

    /*! \brief From \ref vx_type_e */
    vx_enum data_type;
    /*! \brief matrix memory address */
    tivx_shared_mem_ptr_t mem_ptr;
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

    /*! \brief The item type of the array. */
    vx_enum item_type;
    /*! \brief size of each item */
    vx_size item_size;
    /*! \brief number of items */
    vx_size num_items;
    /*! \brief size of buffer pointed to by mem_ptr */
    uint32_t mem_size;

    /*! \brief access type while mapping the buffer */
    uint32_t map_access_type;

    /*! \brief matrix memory address */
    tivx_shared_mem_ptr_t mem_ptr;
} tivx_obj_desc_lut_t;

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

#ifdef __cplusplus
}
#endif

#endif
