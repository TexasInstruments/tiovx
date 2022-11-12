/*
 * Copyright (c) 2012-2016 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#ifndef VX_PYRAMID_H_
#define VX_PYRAMID_H_


#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief Implementation of Pyramid object
 */


/*!
 * \brief Pyramid object internal state
 *
 * \ingroup group_vx_pyramid
 */
typedef struct _vx_pyramid
{
    /*! \brief reference object */
    tivx_reference_t base;

    /*! \brief array of image objects */
    vx_image img[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
} tivx_pyramid_t;


/*!
 * \brief function to initialize virtual pyramid's parameters
 *
 * \param prmd      [in] virtual pyramid reference
 * \param width     [in] base width of image pyramid
 * \param height    [in] base height of image pyramid
 * \param format    [in] image format
 *
 * \return VX_SUCCESS on success
 *
 * \ingroup group_tivx_array
 */
vx_status ownInitVirtualPyramid(
    vx_pyramid prmd, vx_uint32 width, vx_uint32 height, vx_df_image format);

#ifdef __cplusplus
}
#endif

#endif
