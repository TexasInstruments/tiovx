/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */

#ifndef TIVX_KENREL_COMMON_
#define TIVX_KENREL_COMMON_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \brief Maximum number of images (input/output) supported in
 *        calculating valid rectangles
 */
#define TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE        (5u)

typedef struct
{
    /*! \brief List of input images */
    vx_image in_img[TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE];
    /*! \brief number Valid entries in in_img array */
    vx_uint32 num_input_images;

    /*! \brief List of output images */
    vx_image out_img[TIVX_KERNEL_COMMON_VALID_RECT_MAX_IMAGE];
    /*! \brief number of Valid entries in out_img array */
    vx_uint32 num_output_images;

    /*! \brief Padding requied by the kernel */
    vx_uint32 top_pad, bot_pad, right_pad, left_pad;

    /*! \brief Input Border Mode */
    vx_enum border_mode;
} tivxKernelValidRectParams;

vx_status tivxKernelValidateParametersNotNull(const vx_reference *parameters, vx_uint8 maxParams);

vx_status tivxKernelValidateInputSize(vx_uint32 inputWidth0, vx_uint32 inputWidth1,
                            vx_uint32 inputHeight0, vx_uint32 inputHeight1);

vx_status tivxKernelValidatePossibleFormat(vx_df_image inputFormat, vx_df_image possibleFormat);

vx_status tivxKernelValidateScalarType(vx_enum scalarType, vx_enum expectedScalarType);

vx_status tivxKernelValidateOutputSize(vx_uint32 expectedWidth, vx_uint32 outputWidth, vx_uint32 expectedHeight,
                             vx_uint32 outputHeight, vx_image outputImage);

void tivxKernelSetMetas(vx_meta_format *metas, vx_uint8 maxParams, vx_df_image fmt, vx_uint32 width, vx_uint32 height);

/*!
 * \brief Function to initialize Valid Rect Parameter structure
 *        Currently the entire structure is memset to 0
 *
 * \param prms [in] Valid Rectange Parameters
 */
static inline void tivxKernelValidRectParams_init(
    tivxKernelValidRectParams *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0, sizeof(tivxKernelValidRectParams));
        prms.border_mode = VX_BORDER_UNDEFINED;
    }
}

/*!
 * \brief Function to calculate and configure valid region
 *        This API loops over all the input and output image's valid
 *        rectangles and figures out overlapping rectangle and sets it
 *        as valid rectangle in the output image.
 *
 *        Each kernel may also require few lines/pixels of padding on
 *        each side of the image. Host side kernel can provide this
 *        information to this API and it will adjust valid
 *        rectangle considering padding requirement also.
 *
 *        This is utility API.
 *
 * \param prms [in] Valid Rectange Parameters
 */
vx_status tivxKernelConfigValidRect(tivxKernelValidRectParams *prms);

#endif
