'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

from . import *

## Distribution object (OpenVX equivalent = vx_distribution)
#
#
# \par Example Usage: Create a distribution object of 16 bins, 0 offset, 256 range
#
# \code
# from tiovx import *
#
# my_dist1 = Distribution(16, 0, 256, name="mydist")
# my_dist2 = Distribution(16, 0, 256)
# \endcode
#
# \ingroup DATA
class Distribution (Reference) :
    ## Constructor used to create this object
    #
    # See vxCreateDistribution for more details about the parameters
    #
    # \param num_bins [in] Number of bins
    # \param offset [in] Offset
    # \param range [in] Range
    # \param name [in] [optional] Name of the object
    def __init__(self, num_bins, offset, range, name="default") :
        Reference.__init__(self, Type.DISTRIBUTION, name)
        self.num_bins = num_bins
        self.offset = offset
        self.range = range

    def __str__(self):
        return Reference.__str__(self) + ' [ ' + str(self.num_bins) + ', ' + str(self.offset) + ', ' + str(self.range) + ' ]'

