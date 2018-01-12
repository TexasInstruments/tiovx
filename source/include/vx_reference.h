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



#ifndef VX_REFERENCE_H_
#define VX_REFERENCE_H_



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
typedef enum _tivx_reftype_e {
    VX_INTERNAL = 1,
    VX_EXTERNAL = 2,
    VX_BOTH = 3,
} tivx_reftype_e;

/*! \brief Callback type used to register different
 *    callbacks from object dervied from references
 * \ingroup group_vx_reference
 */
typedef vx_status (*tivx_reference_callback_f)(vx_reference ref);

/*! \brief Callback type used to register release
 *    callbacks from object dervied from references
 * \ingroup group_vx_reference
 */
typedef vx_status (* VX_API_CALL tivx_reference_release_callback_f)(vx_reference *ref);

/*! \brief The most basic type in the OpenVX system. Any type that inherits
 *  from tivx_reference_t must have a vx_reference_t as its first memeber
 *  to allow casting to this type.
 * \ingroup group_vx_reference
 */
typedef struct _vx_reference {

    /*! \brief Magic code which confirms this is a reference */
    uint32_t magic;

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

    /*! \brief Reference name */
    char name[VX_MAX_REFERENCE_NAME];

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
    tivx_reference_callback_f mem_alloc_callback;

    /*! \brief Object specific function that is called to destroy an object
     * when refernce count reaches zero
     */
    tivx_reference_callback_f destructor_callback;

    /*! \brief Object specific function that is called to release an object
     */
    tivx_reference_release_callback_f release_callback;

    /*! \brief Lock to take for the reference */
    tivx_mutex lock;

    /* \brief This indicates if the object belongs to a delay */
    vx_delay delay;
    /* \brief This indicates the original delay slot index when the object belongs to a delay */
    vx_int32 delay_slot_index;

    /*! \brief This indicates if the object is virtual or not */
    vx_bool is_virtual;

    /*! \brief Can this reference be accessed by user application
     *         Used for virtual reference's only
     */
    vx_bool is_accessible;

    /*! \brief object descriptor */
    tivx_obj_desc_t *obj_desc;
} tivx_reference_t;

/**
 * \brief Create a reference object
 *
 * \param [in] context The context to which this reference will belong
 * \param [in] type    The \ref vx_type_e type desired.
 * \param [in] reftype The \ref tivx_reftype_e reference type desired.
 * \param [in] scope   The scope to which this reference belongs.
 *
 * \ingroup group_vx_reference
 */
vx_reference ownCreateReference(vx_context context, vx_enum type, vx_enum reftype, vx_reference scope);

/*! \brief Used to destroy a reference.
 * \param [in] ref The reference to release.
 * \param [in] type The \ref vx_type_e to check against.
 * \param [in] reftype The \ref tivx_reftype_e reference type
 * \param [in] destructor The function to call after the total count has reached zero
 * \ingroup group_vx_reference
 */
vx_status ownReleaseReferenceInt(vx_reference *ref,
                        vx_enum type,
                        vx_enum reftype,
                        tivx_reference_callback_f destructor);

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
 * \param [in] reftype see \ref tivx_reftype_e
 * \ingroup group_vx_reference
 */
vx_uint32 ownIncrementReference(vx_reference ref, vx_enum reftype);

/*! \brief Decrements the ref count.
 * \param [in] ref The reference.
 * \param [in] reftype see \ref tivx_reftype_e
 * \ingroup group_vx_reference
 */
vx_uint32 ownDecrementReference(vx_reference ref, vx_enum reftype);

/*! \brief Returns the total reference count of the object.
 * \param [in] ref The reference to print.
 * \ingroup group_vx_reference
 */
vx_uint32 ownTotalReferenceCount(vx_reference ref);

/*! \brief Print reference information
 * \param [in] ref The reference.
 * \ingroup group_vx_reference
 */
void ownPrintReference(vx_reference ref);

/*! \brief This returns true if the type is within the definition of types in OpenVX.
 * \note VX_TYPE_INVALID is not valid for determining a type.
 * \param [in] type The \ref vx_type_e value.
 * \ingroup group_vx_reference
 */
vx_bool ownIsValidType(vx_enum type);

/**
 * \brief Init a reference object
 *
 * \param [in] ref     The reference
 * \param [in] context The context to which this reference will belong
 * \param [in] type    The \ref vx_type_e type desired.
 * \param [in] scope   The scope to which this reference belongs.
 *
 * \ingroup group_vx_reference
 */
vx_status ownInitReference(vx_reference ref, vx_context context, vx_enum type, vx_reference scope);


/**
 * \brief Check if reference is valid
 *
 * \param [in] ref     The reference
 *
 * \ingroup group_vx_reference
 */
vx_bool ownIsValidReference(vx_reference ref);

/*! \brief Used to initialize any vx_reference as a delay element
 * \param [in] ref The pointer to the reference object.
 * \param [in] d The delay to which the object belongs
 * \param [in] index the index in the delay
 * \ingroup group_vx_reference
 */
void ownInitReferenceForDelay(vx_reference ref, vx_delay d, vx_int32 index);

/*! \brief Alloc memory associated with this reference, typically data reference
 * \ingroup group_vx_reference
 */
vx_status ownReferenceAllocMem(vx_reference ref);

/*! \brief Get the size of the Type enum
 * \ingroup group_vx_reference
 */
vx_size ownSizeOfEnumType(vx_enum item_type);


/*! \brief Set scope of a reference
 * \ingroup group_vx_reference
 */
void ownReferenceSetScope(vx_reference ref, vx_reference scope);


/*! \brief Create reference from a exemplar object
 * \ingroup group_vx_reference
 */
vx_reference ownCreateReferenceFromExemplar(
    vx_context context, vx_reference exemplar);


/*! \brief Return reference given a obj desc ID
 * \ingroup group_vx_reference
 */
vx_reference ownReferenceGetHandleFromObjDescId(uint16_t obj_desc_id);

#ifdef __cplusplus
}
#endif

#endif
