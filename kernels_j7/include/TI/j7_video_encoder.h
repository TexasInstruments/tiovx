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

#ifndef J7_VIDEO_ENCODER_H_
#define J7_VIDEO_ENCODER_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \brief The Video Encoder kernels in this kernel extension.
 */

/*! \brief video_encoder kernel name
 *  \ingroup group_vision_function_video_encoder
 */
#define TIVX_KERNEL_VIDEO_ENCODER_NAME     "com.ti.hwa.video_encoder"

/*********************************
 *     VIDEO_ENCODER Defines
 *********************************/

/*!
 * \defgroup group_vision_function_video_encoder_bitstream_format Enumerations
 * \brief Enumerations for bitstream format in Video Encoder structures
 * \ingroup group_vision_function_video_encoder
 * @{*/

#define TIVX_BITSTREAM_FORMAT_H264  (0u)

#define CODED_BUFFER_INFO_SECTION_SIZE       (64)
#define WORST_QP_SIZE                  (400)

/*@}*/

/*********************************
 *    VIDEO_ENCODER STRUCTURES
 *********************************/

/**
 *  MM Encoder supported features \n
 *  Application shall use this type to configure encoder features
 */
typedef enum {
    /**
     *  Enable CABAC encoding. Forces a minimum of MAIN PROFILE.
     */
    TIVX_ENC_FEATURE_CABAC = 0x0001,
    /**
     *  Enable 8x8 transform. Forces a minimum of HIGH PROFILE.
     */
    TIVX_ENC_FEATURE_8x8 = 0x0002,
    /**
     *  Disable Intra 4x4 prediction.
     */
    TIVX_ENC_FEATURE_DISABLE_INTRA4x4 = 0x0004,
    /**
     *  Disable Intra 8x8 prediction.
     */
    TIVX_ENC_FEATURE_DISABLE_INTRA8x8 = 0x0008,
    /**
     *  Disable Intra 16x16 prediction.
     */
    TIVX_ENC_FEATURE_DISABLE_INTRA16x16 = 0x0010,
    /**
     *  Disable Inter 8x8 prediction.
     */
    TIVX_ENC_FEATURE_DISABLE_INTER8x8 = 0x0020,
    /**
     *  Only one 8x8 block may be divided into 4x4 block per MB
     */
    TIVX_ENC_FEATURE_RESTRICT_INTER4x4 = 0x0040,
    /**
     *  Disable 8x16 motion vector block size detection
     */
    TIVX_ENC_FEATURE_DISABLE_8x16_MV_DETECT = 0x0080,
    /**
     *  Disable 16x8 motion vector block size detection
     */
    TIVX_ENC_FEATURE_DISABLE_16x8_MV_DETECT = 0x0100
} tivx_enc_features_e;

/**
 *  MM Encoder supported RC Modes \n
 *  Application shall use this type to configure encoder RC mode
 */
typedef enum {
    /**
     *  Use Variable Bit Rate (VBR) rate control. \n
     *  This allows different bits allocated per frame, meeting the overall
     *  bitrate requirement over a sliding window.
     */
    TIVX_ENC_VBR,
    /**
     *  Use Streaming Variable Bit Rate (SVBR) rate control. \n
     *  This is similar to VBR, but due to disabling bit stuffing a constant
     *  output bitrate is not guaranteed. However, this does still guarantee
     *  that a decoder fed at the specified bitrate will be able to decode
     *  successfully.
     */
    TIVX_ENC_SVBR
} tivx_enc_rcmode_e;

/**
 *  Enum describing smallest blocksize used during motion search \n
 */
typedef enum {
    /**
     *  Driver picks the best possible block size for this encode session
     */
    TIVX_ENC_BLK_SZ_DEFAULT = 0,
    /**
     *  Use 16x16 block size for motion search. This is the smallest for h.263
     */
    TIVX_ENC_BLK_SZ_16x16,
    /**
     *  Use 'upto' 8x8 block size for motion search. This is the smallest for MPEG-4
     */
    TIVX_ENC_BLK_SZ_8x8,
    /**
     *  Use 'upto' 4x4 block size for motion search. This is the smallest for H.264
     */
    TIVX_ENC_BLK_SZ_4x4
} tivx_enc_minblocksize_e;

/*!
 * \brief The configuration data structure used by the VIDEO_ENCODER kernel.
 *
 *  \details The configuration data structure used by the VIDEO_ENCODER kernel; contains only the input format.
 *
 * \ingroup group_vision_function_video_encoder
 */
typedef struct {
    /**
     *  Bitstream format for encoded output. Valid values: \n
     *    TIVX_BITSTREAM_FORMAT_H264
     */
    uint32_t bitstream_format;
    /**
     *  Bit flags for encoding features, see \ref tivx_enc_features_e \n
     *  Note that this is used to control H264 profile as well, with profile set
     *  to the minimum required for the features enabled.
     */
    uint32_t features;
    /** RC Mode, see \ref tivx_enc_rcmode_e */
    uint32_t rcmode;
    /**
     *  IDR period is the number of frames between Instantaneous Decode Refresh
     *  (IDR) frames. IDR frames guarantee that no frames after the IDR frame
     *  will reference any frames prior to the IDR frame. \n
     *  A typical value can be around 1-2 seconds worth of frames \n
     *    ex. at 30fps, idr_period = 30 for 1 second, idr_period = 60 for 2 seconds
     */
    uint32_t idr_period;
    /**
     *  I period is the number of frames between Intra frames (I-frames).
     *  I-frames are frames encoded with all of the image data for the entire
     *  frame, without needing reference to any other frames. \n
     *  Setting any I-frame period higher than one fills the frames in between
     *  with Predicted frames (P-frames). P-frames can reference data in frames
     *  prior to themselves, so rely on prior decoded frames for decoding. \n
     *    ex. i_period = 30 means there will be a pattern of one I-frame followed by 29 P-frames, then repeat. \n
     *  Setting i_period = 1 sets up I-frame only encoding.
     */
    uint32_t i_period;
    /**
     *  Bitrate is the number of bits per second of encoding. This is set in bps, not kbps or mbps. \n
     *  For a 1920x1080 VBR stream, a bitrate of around 10000000 is normal.
     */
    uint32_t bitrate;
    /**
     *  Framerate sets how many frames per second the stream is encoded for.
     *  This primarily impacts the bits the RC control will try to allocate per
     *  frame based on this and the bitrate. \n
     *  Typical framerates are 30 and 60 fps.
     */
    uint32_t framerate;
    /**
     *  Crop settings set how much to crop off of each side from the original image. \n\n
     *  NOTE: Width and Height of the video buffers must be 16-aligned to work with the driver.
     *    By default if a non-16-aligned value is input for buffer width/height, the driver will round down to 16-align.
     *    In order to get a full width/height from a non-16-aligned stream size the application must read the data into the buffer
     *    starting at the "upper left" corner, making sure lines are width aligned. Then an appropriate crop must be set for the
     *    encoder to crop out the empty data along the edges. \n\n
     *  ex. 1920x1080 \n
     *    Set to 1920x1088 width/height. Upon filling the input data this will leave 8 lines of empty data in the bottom of the buffer. \n
     *    Set crop_bottom = 8 to crop the empty 8 lines at the bottom of the buffer to avoid green lines. \n\n
     *  NOTE: crop values can only be set in multiples of 2. Any odd numbered crop will be rounded down in the driver. \n
     *  These should be set to 0 by default.
     */
    uint32_t crop_left;
    /**  See \ref crop_left */
    uint32_t crop_right;
    /**  See \ref crop_left */
    uint32_t crop_top;
    /**  See \ref crop_left */
    uint32_t crop_bottom;
    /**
     *  The number of slices to divide each frame into for encoding.
     *  Under normal circumstances, this should be set to 1. \n
     *  This can be set to 2 to take advantage of the encoder's 2 hardware pipes
     *  for lower latency encoding, but this latency advantage is only evident
     *  if this is the only stream being encoded.
     */
    uint32_t nslices;
    /**
     *  Sets which base pipe to use within the encoder. Valid values are 0 and 1. \n
     *  The encoder has 2 hardware encoding pipes. For best performance,
     *  multiple streams should be split among both pipes to minimize time spent
     *  waiting for a different stream to finish encoding a frame.\n\n
     *  For 2-slice encoding targeting minimal latency per frame, this should be set to 0 to allow it to use both pipe 0 and pipe 1.
     */
    uint32_t base_pipe;
    /**
     *  Qp Settings set the Quantization Parameters (QP). By default leaving
     *  these at 0 will allow the driver to automatically calculate appropriate
     *  settings based on the stream. \n
     *  These settings impact how the encoder targets encoding for more static or dynamic streams. \n
     *  A lower QP value allows for a high quality encoding but less compression,
     *  while a higher QP value allows for greater compression but lower quality encoding. \n
     *  If QP is too low for the amount of motion in the stream, this can produce
     *  artifacts as the encoder runs out of bits for frame data at the quality
     *  that QP is specifying. \n
     *  As a stream is encoded, QP values will be automatically adjusted to
     *  values between mix_qp and max_qp based on the stream contents, attempting
     *  to maximize quality for a given stream. \n\n
     *
     *  Initial QP settings (initial_qp_i/p/b) set the initial QP values for I/P/B frames respectively.
     *  This impacts the stream quality at the start of the stream, before the encoder has had a chance to adapt to the stream. \n\n
     *
     *  min_qp/max_qp set the minimum and maximum QP values respectively. Values are between 2 and 49.
     */
    uint32_t initial_qp_i;
    /** See \ref initial_qp_i */
    uint32_t initial_qp_p;
    /** See \ref initial_qp_i */
    uint32_t initial_qp_b;
    /** See \ref initial_qp_i */
    uint32_t min_qp;
    /** See \ref initial_qp_i */
    uint32_t max_qp;
    /** Min Block Size for motion search. See \ref tivx_enc_minblocksize_e */
    uint32_t min_blk_size;
    /** Controls H264COMP_INTRA_PRED_MODES register. Leave 0 for default. See TRM for details. */
    uint32_t intra_pred_modes;
} tivx_video_encoder_params_t;

/*********************************
 *      Function Prototypes
 *********************************/

/*!
 * \brief Function to register HWA Kernels on the video_encoder Target
 * \ingroup group_vision_function_video_encoder
 */
void tivxRegisterHwaTargetVencKernels(void);

/*!
 * \brief Function to un-register HWA Kernels on the video_encoder Target
 * \ingroup group_vision_function_video_encoder
 */
void tivxUnRegisterHwaTargetVencKernels(void);

/*! \brief [Graph] Creates a VIDEO_ENCODER Node.
 * \param [in] graph             The reference to the graph.
 * \param [in] configuration     The input object of a single params structure of
 *                               type <tt>\ref tivx_video_encoder_params_t</tt>.
 * \param [in] input_image       The input image to be encoded. Use <tt>\ref VX_DF_IMAGE_NV12 </tt> dataformat.
 *                               Width and Height of vx_image object must each be a multiple of 16 (\see tivx_video_encoder_params_t::crop_left)
 *                               QCIF resolution (176x144) is not supported.
 * \param [out] output_bitstream The output object of a uint8_t buffer.
 *                               Formatted as an H264 i-frame only stream.
 * \see <tt>TIVX_KERNEL_VIDEO_ENCODER_NAME</tt>
 * \ingroup group_vision_function_video_encoder
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxVideoEncoderNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_user_data_object  output_bitstream
                                      );
/*!
 * \brief Function to initialize Video Encoder Parameters
 *
 * \param prms  [in] Pointer to Video Encoder parameters
 *
 * \ingroup group_vision_function_video_encoder
 */
void tivx_video_encoder_params_init(tivx_video_encoder_params_t *prms);


#ifdef __cplusplus
}
#endif

#endif /* J7_VIDEO_ENCODER_H_ */

