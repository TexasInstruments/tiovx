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

'''
This is test case to test error detection
'''

from tiovx import *

context = Context("uc_sample_05")
graph = Graph()

img_in = Image(640, 480, DfImage.U8)
img_out_acc = Image(640, 480, DfImage.S16)
node_acc = NodeAccumulateImage(img_in, img_out_acc, target=Target.EVE1);

img_out_acc_w = Image(640, 480, DfImage.U8)
alpha = Scalar(Type.UINT8, 0)
node_acc_w = NodeAccumulateWeightedImage(img_in, alpha, img_out_acc_w );

img_out_conv = Image(640, 480, DfImage.S16)

conv = Convolution(3, 3)
node_conv = NodeConvolve(img_out_acc_w, conv, img_out_conv, target=Target.EVE2);


sc_min1 = Scalar(Type.UINT8, 0);
sc_max1 = Scalar(Type.UINT8, 255);
arr_minmax_out1 = Array(Type.ENUM, 100)
arr_minmax_out2 = Array(Type.ENUM, 100)
sc_min_cnt = Scalar(Type.UINT32, 10)
sc_max_cnt = Scalar(Type.UINT32, 100)

node_minmaxloc = NodeMinMaxLoc(img_out_conv, sc_min1, sc_max1, arr_minmax_out1, arr_minmax_out2, sc_min_cnt, sc_max_cnt);

gaussian_pyr = Pyramid(5, PyramidScale.HALF, 640, 480, DfImage.U8)
node_gaussian_pyramid = NodeGaussianPyramid(img_out_acc_w, gaussian_pyr, target=Target.EVE3);

img_out_phase = Image(640, 480, DfImage.U8)
node_phase = NodePhase(img_out_acc, img_out_conv, img_out_phase, target=Target.A15_0)

remap = Remap(640, 480, 1280, 720);
img_out_remap = Image(1280, 720, DfImage.U8);
node_remap = NodeRemap(img_out_phase, remap, InterpolationType.BILINEAR, img_out_remap, target=Target.DSP1);

img_out_laplacian = Image(1280, 720, DfImage.U8)
node_remap = NodeRemap(img_out_phase, remap, InterpolationType.BILINEAR, img_out_remap, target=Target.DSP2);

thr_canny = Threshold(ThresholdType.BINARY, Type.UINT8)
img_out_canny = Image(1280, 720, DfImage.U8);
node_canny_edge_detect = NodeCannyEdgeDetector(img_out_remap, thr_canny, 3, Norm.L1, img_out_canny, target=Target.EVE3)

img_out_nonlinear_filter = Image(1280, 720, DfImage.U8);
nonlinear_matrix = Matrix(Type.UINT8, 3, 3)
node_nonlinear_filter = NodeNonLinearFilter(NonLinearFilter.MIN, img_out_canny, nonlinear_matrix, img_out_nonlinear_filter, target=Target.EVE4)

img_out_combine = Image(1280, 720, DfImage.U8)
node_combine = NodeChannelCombine(img_out_phase, img_out_remap, img_out_canny, img_out_nonlinear_filter, img_out_combine, target=Target.A15_0)

dist = Distribution(10, 1, 100)
node_histogram = NodeHistogram(img_out_combine, dist, target=Target.IPU1_0);

img_out_lut = Image(1280, 720, DfImage.U8)
lut = Lut(Type.UINT8, 255)
node_lut = NodeTableLookup(img_out_remap, lut, img_out_lut, target=Target.IPU1_1);

img_out_and = Image(1280, 720, DfImage.U8);
node_and = NodeAnd(img_out_lut, img_out_nonlinear_filter, img_out_and, target=Target.IPU2)

graph.add ( node_acc )
graph.add ( node_acc_w )
graph.add ( node_conv )
graph.add ( node_minmaxloc )
graph.add ( node_gaussian_pyramid )
graph.add ( node_phase )
graph.add ( node_remap )
graph.add ( node_canny_edge_detect )
graph.add ( node_nonlinear_filter )
graph.add ( node_combine )
graph.add ( node_histogram )
graph.add ( node_lut )
graph.add ( node_and )

context.add ( graph )

ExportImage(context).export()
ExportCode(context, "CUSTOM_APPLICATION_PATH").export()
