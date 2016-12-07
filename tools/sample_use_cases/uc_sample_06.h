/*
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#ifndef UC_SAMPLE_06
#define UC_SAMPLE_06

#include <VX/vx.h>
#include <TI/tivx.h>

typedef struct _uc_sample_06_t *uc_sample_06;

typedef struct _uc_sample_06_t
{
    vx_context context;
    
    
    vx_scalar scalar_0;
    vx_scalar scalar_1;
    vx_scalar scalar_2;
    vx_scalar scalar_3;
    vx_scalar scalar_4;
    vx_scalar scalar_5;
    vx_scalar scalar_6;
    vx_scalar scalar_7;
    vx_scalar scalar_8;
    vx_scalar scalar_9;
    vx_scalar scalar_10;
    vx_scalar scalar_11;
    vx_scalar scalar_12;
    vx_scalar scalar_13;
    vx_scalar scalar_14;
    
    
} uc_sample_06_t;

vx_status uc_sample_06_data_create(uc_sample_06 usecase);
vx_status uc_sample_06_data_delete(uc_sample_06 usecase);

vx_status uc_sample_06_create(uc_sample_06 usecase);
vx_status uc_sample_06_delete(uc_sample_06 usecase);
vx_status uc_sample_06_verify(uc_sample_06 usecase);
vx_status uc_sample_06_run(uc_sample_06 usecase);

#endif /* UC_SAMPLE_06 */


