'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

class ImagePatchAddress :
    def __init__(self, dim_x, dim_y, stride_x, stride_y, scale_x=1024, scale_y=1024, step_x=1, step_y=1) :
        self.dim_x = dim_x
        self.dim_y = dim_y
        self.stride_x = stride_x
        self.stride_y = stride_y
        self.step_x = step_x
        self.step_y = step_y

class Image (Reference) :
    def __init__(self, width, height, df_image, name="default") :
        Reference.__init__(self, Type.IMAGE, name)
        self.width = width
        self.height = height
        self.df_image = df_image

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.width) + 'x' + str(self.height) + ':' + self.df_image.name + ' ]'

class ImageFromChannel (Image) :
    def __init__(self, image, channel, name="default") :
        df_image = DfImage.INVALID
        if(channel == Channel.Y) :
            if(image.df_image in [ DfImage.NV12, DfImage.NV21, DfImage.IYUV, DfImage.YUV4]) :
                df_image = DfImage.U8
        if(channel == Channel.U or channel == Channel.V ) :
            if(fimage.df_image in [ DfImage.IYUV, DfImage.YUV4]) :
                df_image = DfImage.U8
        Image.__init__(self, image.width, image.height, df_image, name)

class ImageFromHandle (Image) :
    def __init__(self, df_image, image_patch_address, name="default") :
        Image.__init__(self, image_patch_address.dim_x, image_patch_address.dim_y, df_image, name)
        self.image_patch_address = image_patch_address
        

