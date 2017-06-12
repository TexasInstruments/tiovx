'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Remap object (OpenVX equivalent = vx_remap)
#
# \par Example Usage: Create a Remap object
#
# \code
# from tiovx import *
#
# my_remap1 = Remap(640, 480, 320, 240, name="myremap")
# my_remap2 = Remap(640, 480, 320, 240)
# \endcode
#
# \ingroup DATA
class Remap (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateRemap for more details about the parameters
    #
    # \param src_width [in] Width of source image
    # \param src_height [in] Height of source image
    # \param dst_width [in] Width of destination image
    # \param dsy_height [in] Height of destination image
    # \param name [in] [optional] Name of the object
    def __init__(self, src_width, src_height, dst_width, dst_height, name="default") :
        Reference.__init__(self, Type.REMAP, name)
        self.src_width = src_width
        self.src_height = src_height
        self.dst_width = dst_width
        self.dst_height = dst_height

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.src_width) + 'x' + str(self.src_height) + ' ] ' + ' [ ' + str(self.dst_width) + 'x' + str(self.dst_height) + ' ]'

