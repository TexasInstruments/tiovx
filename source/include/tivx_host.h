/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


#ifndef TIVX_HOST_H_
#define TIVX_HOST_H_


#ifdef __cplusplus
extern "C" {
#endif


/*!
 * \brief Function to initialize host
 *
 * \ingroup group_tivx_host
 */
void tivxHostInit(void);

/*!
 * \brief Function to de-initialize host
 *
 * \ingroup group_tivx_host
 */
void tivxHostDeInit(void);

#ifdef __cplusplus
}
#endif

#endif
