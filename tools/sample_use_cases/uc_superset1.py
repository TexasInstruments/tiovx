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

context = Context("uc_superset1")
graph = Graph()

#creating references
#First Branch
in1_absdiff  = Image(640, 480, DfImage.U8)
in2_absdiff  = Image(640, 480, DfImage.U8)
out_absdiff  = Image(640, 480, DfImage.U8)
in_add       = Image(640, 480, DfImage.U8)
out_add      = Image(640, 480, DfImage.U8)
in_sub       = Image(640, 480, DfImage.U8)
out_sub      = Image(640, 480, DfImage.U8)
in_and       = Image(640, 480, DfImage.U8)
out_and      = Image(640, 480, DfImage.U8)
in_or        = Image(640, 480, DfImage.U8)
out_or       = Image(640, 480, DfImage.U8)
in_xor       = Image(640, 480, DfImage.U8)
out_xor      = Image(640, 480, DfImage.U8)
out_not      = Image(640, 480, DfImage.U8)
out_box      = Image(640, 480, DfImage.U8)
conv         = Convolution(3, 3)
out_conv     = Image(640, 480, DfImage.U8)
out_dilate   = Image(640, 480, DfImage.U8)
out_gauss    = Image(640, 480, DfImage.U8)
nlf_matrix   = Matrix(Type.UINT8, 3, 3)
out_nlf      = Image(640, 480, DfImage.U8)
in_mult      = Image(640, 480, DfImage.U8)
mult_scalar  = Scalar(Type.FLOAT32, 1)
out_mult     = Image(640, 480, DfImage.U8)
out_conv_d1  = Image(640, 480, DfImage.S16)
in_mag       = Image(640, 480, DfImage.S16)
out_mag      = Image(640, 480, DfImage.S16)
shift_conv_d = Scalar(Type.INT32, 0)
in_phase     = Image(640, 480, DfImage.S16)
out_phase    = Image(640, 480, DfImage.U8)
in1_ch_comb  = Image(640, 480, DfImage.U8)
in2_ch_comb  = Image(640, 480, DfImage.U8)
in3_ch_comb  = Image(640, 480, DfImage.U8)
out_ch_comb  = Image(640, 480, DfImage.RGBX)
out_ch_ext   = Image(640, 480, DfImage.U8)
out_eq_hist  = Image(640, 480, DfImage.U8)
lut          = Lut(Type.UINT8, 255)
out_lut      = Image(640, 480, DfImage.U8)
out_median   = Image(640, 480, DfImage.U8)
out_sobel1   = Image(640, 480, DfImage.S16)
out_sobel2   = Image(640, 480, DfImage.S16)
out_conv_d2  = Image(640, 480, DfImage.U8)
out_warp_aff = Image(640, 480, DfImage.U8)
warp_aff_mat = Matrix(Type.FLOAT32, 2, 3)
out_warp_per = Image(640, 480, DfImage.U8)
warp_per_mat = Matrix(Type.FLOAT32, 3, 3)
thr          = Threshold(ThresholdType.BINARY, Type.UINT8)
out_thr      = Image(640, 480, DfImage.U8)
remap        = Remap(640, 480, 1280, 720)
out_remap    = Image(1280, 720, DfImage.U8)
out_scale    = Image(640, 480, DfImage.U8)
thr_canny    = Threshold(ThresholdType.BINARY, Type.UINT8)
out_canny    = Image(640, 480, DfImage.U8)
out_intgimg  = Image(640, 480, DfImage.U32)

in_lpl_pyr_rec      = Pyramid(5, PyramidScale.HALF, 640, 480, DfImage.S16)
in_lpl_img_rec      = Image(20, 15, DfImage.U8)
out_lpl_rec         = Image(640, 480, DfImage.U8)

#creating nodes
node_absdiff  = NodeAbsDiff(in1_absdiff, in2_absdiff, out_absdiff)
node_add      = NodeAdd(out_absdiff, in_add, Policy.WRAP, out_add)
node_sub      = NodeSubtract(out_add, in_sub, Policy.WRAP, out_sub)
node_and      = NodeAnd(out_sub, in_and, out_and)
node_or       = NodeOr(out_and, in_or, out_or)
node_xor      = NodeXor(out_or, in_xor, out_xor)
node_not      = NodeNot(out_xor, out_not)
node_box      = NodeBox3x3(out_not, out_box)
node_conv     = NodeConvolve(out_box, conv, out_conv)
node_dilate   = NodeDilate3x3(out_conv, out_dilate)
node_gauss    = NodeGaussian3x3(out_dilate, out_gauss)
node_nlf      = NodeNonLinearFilter(NonLinearFilter.MIN, out_gauss, nlf_matrix, out_nlf)
node_mult     = NodeMultiply(out_nlf, in_mult, mult_scalar, Policy.WRAP, Round.TO_ZERO, out_mult)
node_conv_d1  = NodeConvertDepth(out_mult, out_conv_d1, Policy.WRAP, shift_conv_d)
node_mag      = NodeMagnitude(out_conv_d1, in_mag, out_mag)
node_phase    = NodePhase(out_mag, in_phase, out_phase )
node_ch_comb  = NodeChannelCombine(out_phase, in1_ch_comb, in2_ch_comb, in3_ch_comb, out_ch_comb )
node_ch_ext   = NodeChannelExtract(out_ch_comb, Channel.R, out_ch_ext )
node_eq_hist  = NodeEqualizeHist(out_ch_ext, out_eq_hist )
node_lut      = NodeTableLookup(out_eq_hist, lut, out_lut)
node_median   = NodeMedian3x3(out_lut, out_median)
node_sobel    = NodeSobel3x3(out_median, out_sobel1, out_sobel2)
node_conv_d2  = NodeConvertDepth(out_sobel2, out_conv_d2, Policy.WRAP, shift_conv_d)
node_warp_aff = NodeWarpAffine(out_conv_d2, warp_aff_mat, InterpolationType.BILINEAR, out_warp_aff)
node_warp_per = NodeWarpPerspective(out_warp_aff, warp_per_mat, InterpolationType.BILINEAR, out_warp_per)
node_thr      = NodeThreshold(out_warp_per, thr, out_thr)
node_remap    = NodeRemap(out_thr, remap, InterpolationType.BILINEAR, out_remap)
node_scale    = NodeScaleImage(out_remap, out_scale, InterpolationType.BILINEAR)
node_canny    = NodeCannyEdgeDetector(out_scale, thr_canny, 3, Norm.L1, out_canny)
node_intgimg  = NodeIntegralImage(out_canny, out_intgimg)
node_lpl_rec  = NodeLaplacianReconstruct(in_lpl_pyr_rec, in_lpl_img_rec, out_lpl_rec)


#adding nodes to graph
graph.add ( node_absdiff )
graph.add ( node_add )
graph.add ( node_sub )
graph.add ( node_and )
graph.add ( node_or )
graph.add ( node_xor )
graph.add ( node_not )
graph.add ( node_box )
graph.add ( node_conv )
graph.add ( node_dilate )
graph.add ( node_gauss )
graph.add ( node_nlf )
graph.add ( node_mult )
graph.add ( node_conv_d1 )
graph.add ( node_mag )
graph.add ( node_phase )
graph.add ( node_ch_comb )
graph.add ( node_ch_ext )
graph.add ( node_eq_hist )
graph.add ( node_lut )
graph.add ( node_median )
graph.add ( node_sobel )
graph.add ( node_conv_d2 )
graph.add ( node_warp_aff )
graph.add ( node_warp_per )
graph.add ( node_thr )
graph.add ( node_remap )
graph.add ( node_scale )
graph.add ( node_canny )
graph.add ( node_intgimg )
graph.add ( node_lpl_rec )

context.add ( graph )

ExportImage(context).export()
ExportCode(context, "CUSTOM_APPLICATION_PATH").export()
