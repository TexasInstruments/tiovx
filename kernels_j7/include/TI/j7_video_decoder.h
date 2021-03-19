/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef J7_VIDEO_DECODER_H_
#define J7_VIDEO_DECODER_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Video Decoder kernels in this kernel extension.
 */

/*! \brief video_decoder kernel name
 *  \ingroup group_vision_function_video_decoder
 */
#define TIVX_KERNEL_VIDEO_DECODER_NAME     "com.ti.hwa.video_decoder"

/*********************************
 *     VIDEO_DECODER Defines
 *********************************/

/*!
 * \defgroup group_vision_function_video_decoder_bitstream_format Bitstream Format Enumerations
 * \brief Enumerations for bitstream format in Video Decoder structures for \ref tivx_video_decoder_params_t.bitstream_format
 * \ingroup group_vision_function_video_decoder
 * @{*/

/*! H264 Bitstream format */
#define TIVX_BITSTREAM_FORMAT_H264  (0u)
/*! HEVC Bitstream format */
#define TIVX_BITSTREAM_FORMAT_HEVC  (1u)
/*! MJPEG Bitstream format */
#define TIVX_BITSTREAM_FORMAT_MJPEG (2u)

/*@}*/


/*! \brief Image alignment size
 *  \ingroup group_vision_function_video_decoder
 *  \details The width and height of the output video image should both be a multiple of this value
 */
#define TIVX_VDEC_ALIGN_SIZE     (64U)

/*! \brief Image alignment macro
 *  \ingroup group_vision_function_video_decoder
 *  \details Helper macro to properly size the width and height of the output image to properly satisfy the video codec HW constraints
 *  This can be used on the valid width and height of the output image when calling \ref vxCreateImage, to make sure the buffer is properly
 *  allocated \see tivxVideoDecoderNode output_image parameter
 */
#define TIVX_VDEC_ALIGN(size)    (((size + (TIVX_VDEC_ALIGN_SIZE-1)) / TIVX_VDEC_ALIGN_SIZE) * TIVX_VDEC_ALIGN_SIZE)


/*********************************
 *    VIDEO_DECODER STRUCTURES
 *********************************/

/*!
 * \brief The configuration data structure used by the VIDEO_DECODER kernel.
 *
 *  \details The configuration data structure used by the VIDEO_DECODER kernel; contains only the input format.
 *
 * \ingroup group_vision_function_video_decoder
 */
typedef struct {
    /*! Bitstream format, \see group_vision_function_video_decoder_bitstream_format
     *  \note Only \ref TIVX_BITSTREAM_FORMAT_H264 format supported in this release  */
    uint32_t bitstream_format;
} tivx_video_decoder_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the video_decoder Target
 * \ingroup group_vision_function_video_decoder
 */
void tivxRegisterHwaTargetVdecKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the video_decoder Target
 * \ingroup group_vision_function_video_decoder
 */
void tivxUnRegisterHwaTargetVdecKernels(void);

/*! \brief [Graph] Creates a VIDEO_DECODER Node.
 *  \note This node is only supported in graphs with pipelining enabled with 2 or more
 *        input and output buffers. \see \ref TIOVX_PIPELINING
 * \param [in] graph The reference to the graph.
 * \param [in] configuration The input object of a single params structure of
 *             type <tt>\ref tivx_video_decoder_params_t</tt>.
 * \param [in] input_bitstream The input object of a \ref vx_uint8 buffer.
 *             The \ref vx_user_data_object should be sized as per worst case (uncompressed) bitstream size.
 *             For each frame of input, the \ref TIVX_USER_DATA_OBJECT_VALID_SIZE attribute should be set on the
 *             input_bitstream \ref vx_user_data_object to reflect the size of the specific bitstream buffer frame.
 *             Must be formatted as an H264 i-frame only stream.
 * \param [out] output_image Decoded output. Use <tt>\ref VX_DF_IMAGE_NV12 </tt> dataformat.
 *             QCIF resolution (176x144) is not supported.  Image width and height both must be multiple of 64.
 * \see <tt>TIVX_KERNEL_VIDEO_DECODER_NAME</tt>
 * \ingroup group_vision_function_video_decoder
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVideoDecoderNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_user_data_object  input_bitstream,
                                      vx_image             output_image);
/*!
 * \brief Function to initialize Video Decoder Parameters
 *
 * \param prms  [in] Pointer to Video Decoder parameters
 *
 * \ingroup group_vision_function_video_decoder
 */
void tivx_video_decoder_params_init(tivx_video_decoder_params_t *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_VIDEO_DECODER_H_ */

