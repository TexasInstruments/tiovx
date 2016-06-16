/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */
/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_CONTEXT_H_
#define _VX_CONTEXT_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Context object
 */

/*!
 * \brief Max possible references in a context
 *
 * \ingroup group_vx_context
 */
#define TIVX_CONTEXT_MAX_REFERENCES     (64u)


/*!
 * \brief Max possible user structs in a context
 *
 * \ingroup group_vx_context
 */
#define TIVX_CONTEXT_MAX_USER_STRUCTS   (16u)

/*! \brief The largest convolution matrix the specification requires support for is 15x15.
 * \ingroup group_vx_context
 */
#define TIVX_CONTEXT_MAX_CONVOLUTION_DIM (15)

/*! \brief The largest optical flow pyr LK window.
 * \ingroup group_vx_context
 */
#define TIVX_CONTEXT_OPTICALFLOWPYRLK_MAX_DIM (9)

/*! \brief The largest nonlinear filter matrix the specification requires support for is 9x9.
* \ingroup group_vx_context
*/
#define TIVX_CONTEXT_MAX_NONLINEAR_DIM (9)


/*! \brief The top level context data for the entire OpenVX instance
 * \ingroup group_vx_context
 */
typedef struct _vx_context {

    /*! \brief The base reference object */
    tivx_reference_t      base;

    /*! \brief References associated with a context */
    vx_reference        reftable[TIVX_CONTEXT_MAX_REFERENCES];
    /*! \brief The number of references in the table. */
    vx_uint32           num_references;
    /*! \brief The combined number of unique kernels in the system */
    vx_uint32           num_unique_kernels;
    /*! \brief The number of kernel libraries loaded */
    vx_uint32           num_modules;
    /*! \brief Callback to call for logging messages from framework */
    vx_log_callback_f   log_callback;
    /*! \brief The log enable toggle. */
    vx_bool             log_enabled;
    /*! \brief Lock to use for locking log print's */
    tivx_mutex          log_lock;
    /*! \brief The performance counter enable toggle. */
    vx_bool             perf_enabled;
    /*! \brief The immediate mode border */
    vx_border_t         imm_border;
    /*! \brief The unsupported border mode policy for immediate mode functions */
    vx_enum             imm_border_policy;
    /*! \brief The next available dynamic user kernel ID */
    vx_uint32           next_dynamic_user_kernel_id;
    /*! \brief The next available dynamic user library ID */
    vx_uint32           next_dynamic_user_library_id;
    /*! \brief The immediate mode enumeration */
    vx_enum             imm_target_enum;
    /*! \brief The immediate mode target string */
    vx_char             imm_target_string[TIVX_MAX_TARGET_NAME];
    /*! \brief The list of user defined structs. */
    struct {
        /*! \brief Type constant */
        vx_enum type;
        /*! \brief Size in bytes */
        vx_size size;
    } user_structs[TIVX_CONTEXT_MAX_USER_STRUCTS];


} tivx_context_t;

/**
 * \brief Check if 'context' is valid
 *
 * \param [in] context The reference to context
 *
 * \return A <tt>\ref vx_status_e</tt> enumeration.
 * \retval VX_SUCCESS No errors.
 *
 * \ingroup group_vx_context
 */
vx_bool ownIsValidContext(vx_context context);


/*! \brief Add reference to a context
 * \param [in] context The overall context.
 * \param [in] ref The reference to add.
 * \return vx_true_e on success
 * \return vx_false_e on failure
 * \ingroup group_vx_context
 */
vx_bool ownAddReferenceToContext(vx_context context, vx_reference ref);


/*! \brief Remove reference from a context
 * \param [in] context The overall context.
 * \param [in] ref The reference to remove.
 * \return vx_true_e on success
 * \return vx_false_e on failure
 * \ingroup group_vx_context
 */
vx_bool ownRemoveReferenceFromContext(vx_context context, vx_reference ref);

#ifdef __cplusplus
}
#endif

#endif
