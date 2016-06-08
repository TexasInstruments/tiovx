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


#ifndef _VX_REFERENCE_H_
#define _VX_REFERENCE_H_



#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Reference object
 */



/*! \brief An internal enum for notating which sort of reference type we need.
 * \ingroup group_vx_reference
 */
typedef enum _vx_reftype_e {
    VX_INTERNAL = 1,
    VX_EXTERNAL = 2,
    VX_BOTH = 3,
} vx_reftype_e;

/*! \brief Used to destroy an object in a generic way.
 * \ingroup group_vx_reference
 */
typedef void (*vx_destructor_f)(vx_reference ref);

/*! \brief Used to allocate memory for an object in a generic way.
 * \ingroup group_vx_reference
 */
typedef vx_status (*vx_mem_alloc_f)(vx_reference ref);

/*! \brief The most basic type in the OpenVX system. Any type that inherits
 *  from vx_reference_t must have a vx_reference_t as its first memeber
 *  to allow casting to this type.
 * \ingroup group_vx_reference
 */
typedef struct _vx_reference {

    /*! \brief Set to an enum value in \ref vx_type_e. */
    vx_enum type;

    /*! \brief Pointer to the top level context.
     * If this reference is the context, this will be NULL.
     */
    vx_context context;

    /*! \brief The pointer to the object's scope parent. When virtual objects
     * are scoped within a graph, this will point to that parent graph. This is
     * left generic to allow future scoping variations. By default scope should
     * be the same as context.
     */
    vx_reference scope;

    /*! \brief The count of the number of users with this reference. When
     * greater than 0, this can not be freed. When zero, the value can be
     * considered inaccessible.
     */
    vx_uint32 external_count;

    /*! \brief The count of the number of framework references. When
     * greater than 0, this can not be freed.
     */
    vx_uint32 internal_count;

    /*! \brief Object specific function that is called to allcoate object memory
     */
    vx_mem_alloc_f mem_alloc_callback;

    /*! \brief Object specific function that is called to destroy an object
     * when refernce count reaches zero
     */
    vx_destructor_f destructor_callback;

} vx_reference_t;

/**
 * \brief Create a reference object
 *
 * \param [in] context The context to which this reference will belong
 * \param [in] type    The \ref vx_type_e type desired.
 * \param [in] reftype The \ref vx_reftype_e reference type desired.
 * \param [in] scope   The scope to which this reference belongs.
 *
 * \ingroup group_vx_reference
 */
vx_reference ownCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope);

/*! \brief Used to destroy a reference.
 * \param [in] ref The reference to release.
 * \param [in] type The \ref vx_type_e to check against.
 * \param [in] reftype The \ref vx_reftype_e reference type
 * \param [in] destructor The function to call after the total count has reached zero
 * \ingroup group_vx_reference
 */
vx_status ownReleaseReferenceInt(vx_reference *ref,
                        vx_enum type,
                        vx_enum reftype,
                        vx_destructor_f destructor);

/*! \brief Used to validate everything but vx_context, vx_image
 * \param [in] ref The reference to validate.
 * \param [in] type The \ref vx_type_e to check for.
 * \ingroup group_vx_reference
 */
vx_bool ownIsValidSpecificReference(vx_reference ref, vx_enum type);

/*! \brief Lock the reference
 * \param [in] ref The reference to lock
 * \ingroup group_vx_reference
 */
vx_status ownReferenceLock(vx_reference ref);

/*! \brief Unlock the reference
 * \param [in] ref The reference to unlock
 * \ingroup group_vx_reference
 */
vx_status ownReferenceUnlock(vx_reference ref);

/*! \brief Increments the ref count.
 * \param [in] ref The reference.
 * \param [in] reftype see \ref vx_reftype_e
 * \ingroup group_vx_reference
 */
vx_uint32 ownIncrementReference(vx_reference ref, vx_enum reftype);

/*! \brief Print reference information
 * \param [in] ref The reference.
 * \ingroup group_vx_reference
 */
void ownPrintReference(vx_reference ref);


#ifdef __cplusplus
}
#endif

#endif
