'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

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
    # \param name [in] [optional] Name of the object
    def __init__(self, width, height, df_image, name="default") :
        Reference.__init__(self, Type.IMAGE, name)
        self.width = width
        self.height = height
        self.df_image = df_image

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


