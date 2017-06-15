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

class NodePhaseRgb (Node) :
    def __init__(self, image_in, image_out, name="default", target=Target.DEFAULT) :
        Node.__init__(self, "vx_tutorial_graph.phase_rgb", image_in, image_out)
        self.setParams(1, 1, Type.IMAGE, Type.IMAGE)
        self.setTarget(target)
        self.setKernelEnumName("VX_USER_KERNEL")

    def checkParams(self, *param_type_args) :
        Node.checkParams(self, *param_type_args)
        # additional error conditions over the basic ones
        assert ( self.ref[0].width    == self.ref[1].width ), "Input and Output width MUST match"
        assert ( self.ref[0].height   == self.ref[1].height ), "Input and Output height MUST match"
        assert ( self.ref[0].df_image == DfImage.U8 ), "Input data format must be U8"
        assert ( self.ref[1].df_image == DfImage.RGB ), "Output data format must be RGB"


context = Context("vx_tutorial_graph_user_kernel_pytiovx_uc")
graph = Graph()

width = 640
height = 480

in_image = Image(width, height, DfImage.U8, name="input")
grad_x = Image(width, height, DfImage.S16, name="grad_x")
grad_y = Image(width, height, DfImage.S16, name="grad_y")
phase = Image(width, height, DfImage.U8, name="phase")
phase_rgb = Image(width, height, DfImage.RGB, name="phase_rgb")

graph.add ( NodeSobel3x3(in_image, grad_x, grad_y, target=Target.DSP1) )
graph.add ( NodePhase(grad_x, grad_y, phase, target=Target.DSP1) )
graph.add ( NodePhaseRgb(phase, phase_rgb, target=Target.DSP1) )

context.add ( graph )

ExportImage(context).export()
ExportCode(context).export()
