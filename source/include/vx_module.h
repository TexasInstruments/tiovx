/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef _VX_MODULE_H_
#define _VX_MODULE_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Module APIs
 */

/*! \brief Maximum possible modules in system \
 * \ingroup group_vx_module
 */
#define TIVX_MAX_MODULES        (10u)

/*! \brief Maximum possible name of a module in system
 * \ingroup group_vx_module
 */
#define TIVX_MAX_MODULE_NAME    (256u)

/*!
 * \brief Module object internal state
 *
 * \ingroup group_vx_module
 */
typedef struct _vx_module
{
    /*! \brief names of modules */
    vx_char        name[TIVX_MAX_MODULE_NAME];

    /*! \brief callback to publish nodes in a module */
    vx_publish_kernels_f publish;

    /*! \brief callback to unpublish nodes in a module */
    vx_unpublish_kernels_f unpublish;

} tivx_module_t;


/*!
 * \brief Returns number of resgistered modules
 *
 * \ingroup group_vx_module
 */
uint32_t ownGetModuleCount();


#ifdef __cplusplus
}
#endif

#endif
