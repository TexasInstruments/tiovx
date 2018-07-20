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

context = Context("uc_sample_06")

sc1 = Scalar(Type.CHAR, 'c')
sc2 = Scalar(Type.INT8, -0x12)
sc3 = Scalar(Type.UINT8, 0x12)
sc4 = Scalar(Type.INT16, -0x1234)
sc5 = Scalar(Type.UINT16, 0x1234)
sc6 = Scalar(Type.INT32, -0x12345678)
sc7 = Scalar(Type.UINT32, 0x12345678)
sc8 = Scalar(Type.INT64, -0x1234567890)
sc9 = Scalar(Type.UINT64, 0x1234567890)
sc10 = Scalar(Type.FLOAT32, -1234.1234)
sc11 = Scalar(Type.FLOAT64, -1234567890.1234567890)
sc12 = Scalar(Type.ENUM, Type.FLOAT32)
sc13 = Scalar(Type.SIZE, 1234)
sc14 = Scalar(Type.DF_IMAGE, DfImage.NV12)
sc15 = Scalar(Type.BOOL, True)

context.add ( sc1 )
context.add ( sc2 )
context.add ( sc3 )
context.add ( sc4 )
context.add ( sc5 )
context.add ( sc6 )
context.add ( sc7 )
context.add ( sc8 )
context.add ( sc9 )
context.add ( sc10 )
context.add ( sc11 )
context.add ( sc12 )
context.add ( sc13 )
context.add ( sc14 )
context.add ( sc15 )

ExportCode(context, "CUSTOM_APPLICATION_PATH").export()
