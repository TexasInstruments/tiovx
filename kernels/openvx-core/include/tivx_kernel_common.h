/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifdef __cplusplus
extern "C" {
#endif

vx_status tivxKernelValidateParametersNotNull(const vx_reference *parameters, vx_uint8 maxParams);

vx_status tivxKernelValidateInputSize(vx_uint32 inputWidth0, vx_uint32 inputWidth1,
                            vx_uint32 inputHeight0, vx_uint32 inputHeight1);

vx_status tivxKernelValidatePossibleFormat(vx_df_image inputFormat, vx_df_image possibleFormat);

vx_status tivxKernelValidateScalarType(vx_enum scalarType, vx_enum expectedScalarType);

vx_status tivxKernelValidateOutputSize(vx_uint32 expectedWidth, vx_uint32 outputWidth, vx_uint32 expectedHeight, 
                             vx_uint32 outputHeight, vx_image outputImage);

void tivxKernelSetMetas(vx_meta_format *metas, vx_uint8 maxParams, vx_df_image fmt, vx_uint32 width, vx_uint32 height);
