#
# Copyright (c) 2017 Texas Instruments Incorporated
#
# All rights reserved not granted herein.
#
# Limited License.
#
# Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
# license under copyrights and patents it now or hereafter owns or controls to make,
# have made, use, import, offer to sell and sell ("Utilize") this software subject to the
# terms herein.  With respect to the foregoing patent license, such license is granted
# solely to the extent that any such patent is necessary to Utilize the software alone.
# The patent license shall not apply to any combinations which include this software,
# other than combinations with devices manufactured by or for TI ("TI Devices").
# No hardware patent is licensed hereunder.
#
# Redistributions must preserve existing copyright notices and reproduce this license
# (including the above copyright notice and the disclaimer and (if applicable) source
# code license limitations below) in the documentation and/or other materials provided
# with the distribution
#
# Redistribution and use in binary form, without modification, are permitted provided
# that the following conditions are met:
#
#       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
#       any redistribution and use are licensed by TI for use only with TI Devices.
#
#       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
#       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
#       any redistribution and use of any object code compiled from the source code
# and any resulting derivative works, are licensed by TI for use only with TI Devices.
#
# Neither the name of Texas Instruments Incorporated nor the names of its suppliers
#
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# DISCLAIMER.
#
# THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
# OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#

from . import *

## Image patch object (OpenVX equivalent = vx_imagepatch_addressing_t)
#
# \par Example Usage
#
# See ImageFromHandle example
#
# \ingroup DATA
class ImagePatchAddress :
    ## Constructor used to create this object
    #
    # See vx_imagepatch_addressing_t for more details about the parameters
    #
    # \param dim_x [in] Width of image
    # \param dim_y [in] Height of image
    # \param stride_x [in] Stride in X dimensions in bytes.
    # \param stride_y [in] Stride in Y dimensions in bytes.
    # \param scale_x [in] [optional]
    # \param scale_y [in] [optional]
    # \param step_x [in] [optional]
    # \param step_y [in] [optional]
    def __init__(self, dim_x, dim_y, stride_x, stride_y, scale_x=1024, scale_y=1024, step_x=1, step_y=1) :
        self.dim_x = dim_x
        self.dim_y = dim_y
        self.stride_x = stride_x
        self.stride_y = stride_y
        self.step_x = step_x
        self.step_y = step_y

## Image object (OpenVX equivalent = vx_image, vxCreateImage)
#
# \par Example Usage: Create a image object of 640x480
#
# \code
# from tiovx import *
#
# my_img1 = Image(640, 480, DfImage.YUYV, name="myimage")
# my_img2 = Image(640, 480, DfImage.U8)
# \endcode
#
# \ingroup DATA
class Image (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateImage for more details about the parameters
    #
    # \param width [in] Width of image
    # \param height [in] Height of image
    # \param df_image [in] Image data format. tiovx::enums::DfImage
    # \param access_type [in] [optional] Memory access type
    # \param in_file_addr [in] [optional] In the case of memory accessed from file, the path to the input file
    # \param out_file_addr [in] [optional] In the case of memory outputted to file, the path to the output file
    # \param name [in] [optional] Name of the object
    def __init__(self, width, height, df_image, access_type="Host", in_file_addr="./", out_file_addr="./", name="default") :
        Reference.__init__(self, Type.IMAGE, name)
        self.width = width
        self.height = height
        self.df_image = df_image
        self.access_type = access_type;
        self.in_file = in_file_addr;
        self.out_file = out_file_addr;

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.width) + 'x' + str(self.height) + ':' + self.df_image.name + ' ]'

## Image from channel object (OpenVX equivalent = vx_image, vxCreateImageFromChannel)
#
# \par Example Usage: Create a image object from R channel of another image object
#
# \code
# from tiovx import *
#
# my_img1 = Image(640, 480, DfImage.RGB)
# my_img2 = ImageFromChannel(my_img1, Channel.R)
# \endcode
#
# \ingroup DATA
class ImageFromChannel (Image) :
    ## Constructor used to create this object
    #
    # See vxCreateImageFromChannel for more details about the parameters
    #
    # \param image [in] Source image. Image
    # \param channel [in] Channel to use to create new image. tiovx::enums::Channel
    # \param name [in] [optional] Name of the object
    def __init__(self, image, channel, name="default") :
        df_image = DfImage.INVALID
        if(channel == Channel.Y) :
            if(image.df_image in [ DfImage.NV12, DfImage.NV21, DfImage.IYUV, DfImage.YUV4]) :
                df_image = DfImage.U8
        if(channel == Channel.U or channel == Channel.V ) :
            if(fimage.df_image in [ DfImage.IYUV, DfImage.YUV4]) :
                df_image = DfImage.U8
        Image.__init__(self, image.width, image.height, df_image, name)

## Image from handle object (OpenVX equivalent = vx_image, vxCreateImageFromHandle)
#
# \par Example Usage: Create a image object from handle
#
# \code
# from tiovx import *
#
# my_img_addr = ImagePatchAddress(640, 480, 1, 640)
# my_img1 = ImageFromHandle(DfImage.RGB, my_img_addr, name="myimage")
# \endcode
#
# \ingroup DATA
class ImageFromHandle (Image) :
    ## Constructor used to create this object
    #
    # See vxCreateImageFromHandle for more details about the parameters
    #
    # \param df_image [in] Image data format. tiovx::enums::DfImage
    # \param image_patch_address [in] Data arragement in memory. ImagePatchAddress
    # \param name [in] [optional] Name of the object
    def __init__(self, df_image, image_patch_address, name="default") :
        Image.__init__(self, image_patch_address.dim_x, image_patch_address.dim_y, df_image, name)
        self.image_patch_address = image_patch_address


