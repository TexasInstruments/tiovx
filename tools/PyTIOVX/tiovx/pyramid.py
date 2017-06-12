'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Pyramid object (OpenVX equivalent = vx_pyramid)
#
# \par Example Usage: Create a Pyramid object having 10 levels
#
# \code
# from tiovx import *
#
# my_pyramid1 = Pyramid(10, PyramidScale.HALF, 640, 480, name="mypyramid")
# my_pyramid2 = Pyramid(10, PyramidScale.HALF, 640, 480)
# \endcode
#
# \ingroup DATA
class Pyramid (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreatePyramid for more details about the parameters
    #
    # \param levels [in] Data type. tiovx::enums::Type
    # \param scale [in] tiovx::enums::PyramidScale
    # \param width [in] Width of highest pyramid level image
    # \param height [in] Height of highest pyramid level image
    # \param df_image [in] Image data format. tiovx::enums::DfImage
    # \param name [in] [optional] Name of the object
    def __init__(self, levels, scale, width, height, df_format, name="default") :
        Reference.__init__(self, Type.PYRAMID, name)
        self.num_levels = levels;
        self.width = width;
        self.height = height;
        self.scale = scale;
        self.format = df_format;

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.format.name + ':' + str(self.scale) + ', ' + str(self.width) + ', ' + str(self.height) + ' ]'
