'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Threshold object (OpenVX equivalent = vx_threshold)
#
# \par Example Usage: Create a Threshold object
#
# \code
# from tiovx import *
#
# my_threshold1 = Threshold(10, ThresholdType.BINARY, Type.UINT8, name="mythreshold")
# my_threshold2 = Threshold(10, ThresholdType.RANGE, Type.UINT16)
# \endcode
#
# \ingroup DATA
class Threshold (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateThreshold for more details about the parameters
    #
    # \param thr_type [in] Type of threshold. tiovx::enums::ThresholdType
    # \param data_type [in] Data type of threshold value. tiovx::enums::Type
    # \param name [in] [optional] Name of the object
    def __init__(self, thr_type, data_type, name="default") :
        Reference.__init__(self, Type.THRESHOLD, name)
        self.thr_type = thr_type
        self.data_type = data_type

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + self.thr_type.name + ', ' + self.data_type.name + ' ] '

