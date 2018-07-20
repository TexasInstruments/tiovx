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

from tiovx import *

context = Context("uc_superset2")
graph = Graph()

#creating references
in_meanstdev = Image(640, 480, DfImage.U8)
out_mean     = Scalar(Type.FLOAT32, 0)
out_stddev   = Scalar(Type.FLOAT32, 0)

in_minmax    = Image(640, 480, DfImage.U8)
sc_min1      = Scalar(Type.UINT8, 0)
sc_max1      = Scalar(Type.UINT8, 255)
arr1_minmax  = Array(Type.COORDINATES2D, 100)
arr2_minmax  = Array(Type.COORDINATES2D, 100)
sc_min_cnt   = Scalar(Type.UINT32, 10)
sc_max_cnt   = Scalar(Type.UINT32, 100)

in_dist      = Image(640, 480, DfImage.U8)
dist         = Distribution(10, 1, 100)

in_color     = Image(640, 480, DfImage.RGB)
out_color    = Image(640, 480, DfImage.RGBX)

in_harris           = Image(640, 480, DfImage.U8)
harris_strength_thr = Scalar(Type.FLOAT32, 0) # issue in c code w/ this param
fast_strength_thr   = Scalar(Type.FLOAT32, 0) # issue in c code w/ this param
harris_min_dist     = Scalar(Type.FLOAT32, 0)
harris_sensitivity  = Scalar(Type.FLOAT32, 0)
harris_corners      = Array(Type.KEYPOINT, 100)
harris_num_corners  = Scalar(Type.SIZE, 0)
in_gaussian_pyr     = Image(640, 480, DfImage.U8)
gaussian_pyr        = Pyramid(5, PyramidScale.HALF, 640, 480, DfImage.U8)
in_fast             = Image(640, 480, DfImage.U8)
fast_corners        = Array(Type.KEYPOINT, 100)
fast_num_corners    = Scalar(Type.SIZE, 0)
nonmax              = Scalar(Type.BOOL, True)
in_opt_flow         = Pyramid(5, PyramidScale.HALF, 640, 480, DfImage.U8)
out_corners         = Array(Type.KEYPOINT, 100)
use_initial_est     = Scalar(Type.BOOL, True)

in_lpl_pyr          = Image(640, 480, DfImage.U8)
out_lpl_pyr         = Pyramid(5, PyramidScale.HALF, 640, 480, DfImage.S16)
out_lpl_img         = Image(20, 15, DfImage.U8)
out_lpl_rec         = Image(640, 480, DfImage.U8)

#creating nodes
node_gausspyr = NodeGaussianPyramid(in_gaussian_pyr, gaussian_pyr)
node_harris   = NodeHarrisCorners(in_harris, harris_strength_thr, harris_min_dist, harris_sensitivity, 3, 5, harris_corners, harris_num_corners)
node_fast     = NodeFastCorners(in_fast, fast_strength_thr, nonmax, fast_corners, fast_num_corners)
node_opt_flow = NodeOpticalFlowPyrLK(gaussian_pyr, in_opt_flow, harris_corners, fast_corners, out_corners, TermCriteria.EPSILON, 1, 10, True, 9)

node_color    = NodeColorConvert(in_color, out_color)
node_hist     = NodeHistogram(in_dist, dist)
node_mnstd    = NodeMeanStdDev(in_meanstdev, out_mean, out_stddev)
node_minmax   = NodeMinMaxLoc(in_minmax, sc_min1, sc_max1, arr1_minmax, arr2_minmax, sc_min_cnt, sc_max_cnt)

node_lpl_pyr  = NodeLaplacianPyramid(in_lpl_pyr, out_lpl_pyr, out_lpl_img)
node_lpl_rec  = NodeLaplacianReconstruct(out_lpl_pyr, out_lpl_img, out_lpl_rec)

#adding nodes to graph
graph.add ( node_harris )
graph.add ( node_gausspyr )
graph.add ( node_fast )
graph.add ( node_opt_flow )
graph.add ( node_color )
graph.add ( node_hist )
graph.add ( node_mnstd )
graph.add ( node_minmax )
graph.add ( node_lpl_pyr )
graph.add ( node_lpl_rec )

context.add ( graph )

ExportImage(context).export()
ExportCode(context, "CUSTOM_APPLICATION_PATH").export()
