'''
* Copyright (C) 2016 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
'''

'''
This is test case to test error detection
'''

from tiovx import *

context = Context("uc_node_test")
graph = Graph()

img_in = Image(640, 480, DfImage.U8)
img_out_acc = Image(640, 480, DfImage.S16)
node_acc = NodeAccumulateImage(img_in, img_out_acc);

img_out_acc_w = Image(640, 480, DfImage.U8)
alpha = Scalar(Type.UINT8)
node_acc_w = NodeAccumulateWeightedImage(img_in, alpha, img_out_acc_w );

img_out_conv = Image(640, 480, DfImage.S16)

conv = Convolution(3, 3)
node_conv = NodeConvolve(img_out_acc_w, conv, img_out_conv);


sc_min1 = Scalar(Type.ENUM, 0);
sc_max1 = Scalar(Type.ENUM, 255);
arr_minmax_out1 = Array(Type.ENUM, 100)
arr_minmax_out2 = Array(Type.ENUM, 100)
sc_min_cnt = Scalar(Type.ENUM, 0)
sc_max_cnt = Scalar(Type.ENUM, 0)

node_minmaxloc = NodeMinMaxLoc(img_out_conv, sc_min1, sc_max1, arr_minmax_out1, arr_minmax_out2, sc_min_cnt, sc_max_cnt);

gaussian_pyr = Pyramid(5, 1, 640, 480, DfImage.U8)
node_gaussian_pyramid = NodeGaussianPyramid(img_out_acc_w, gaussian_pyr);

img_out_phase = Image(640, 480, DfImage.U8)
node_phase = NodePhase(img_out_acc, img_out_conv, img_out_phase)

remap = Remap(640, 480, 1280, 720);
img_out_remap = Image(1280, 720, DfImage.U8);
node_remap = NodeRemap(img_out_phase, remap, InterpolationType.BILINEAR, img_out_remap);

img_out_laplacian = Image(1280, 720, DfImage.U8)
node_remap = NodeRemap(img_out_phase, remap, InterpolationType.BILINEAR, img_out_remap);

thr_canny = Threshold(0, Type.UINT8)
img_out_canny = Image(1280, 720, DfImage.U8);
node_canny_edge_detect = NodeCannyEdgeDetector(img_out_remap, thr_canny, 100, 0, img_out_canny)

img_out_nonlinear_filter = Image(1280, 720, DfImage.U8);
nonlinear_matrix = Matrix(Type.UINT8, 3, 3)
node_nonlinear_filter = NodeNonLinearFilter(NonLinearFilter.Min, img_out_canny, nonlinear_matrix, img_out_nonlinear_filter)

img_out_combine = Image(1280, 720, DfImage.U8)
node_combine = NodeChannelCombine(img_out_phase, img_out_remap, img_out_canny, img_out_nonlinear_filter, img_out_combine)

dist = Distribution(10, 1, 100)
node_histogram = NodeHistogram(img_out_combine, dist);

img_out_lut = Image(1280, 720, DfImage.U8)
lut = Lut(Type.UINT8, 255)
node_lut = NodeTableLookup(img_out_remap, lut, img_out_lut);

img_out_and = Image(1280, 720, DfImage.U8);
node_and = NodeAnd(img_out_lut, img_out_nonlinear_filter, img_out_and)

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
ExportCode(context).export()
