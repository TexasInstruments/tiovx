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

from . import *
import os, sys, re

## Export objects from context to C source code
#
#
# \par Example Usage: Export objects from context to C source code
#
# \code
#
# from tiovx import *
#
# my_context = Context("my_usecase")
#
# ...
#
# ExportCode(my_context, VISION_APPS_PATH).export()
# \endcode
#
# Output files from the above parameters:
# \code
# <VISION_APPS_PATH>/apps/my_usecase/DEVELOPER_TODO.txt
# <VISION_APPS_PATH>/apps/my_usecase/concerto_inc.mak
# <VISION_APPS_PATH>/apps/my_usecase/app_my_usecase/concerto.mak
# <VISION_APPS_PATH>/apps/my_usecase/app_my_usecase/my_usecase.c
# <VISION_APPS_PATH>/apps/my_usecase/app_my_usecase/my_usecase.h
# \endcode
#
# Output files from the above parameters in the case that the environment variable CUSTOM_APPLICATION_PATH was given:
# \code
# <CUSTOM_APPLICATION_PATH>/DEVELOPER_TODO.txt
# <CUSTOM_APPLICATION_PATH>/concerto_inc.mak
# <CUSTOM_APPLICATION_PATH>/custom_tools_path.mak
# <CUSTOM_APPLICATION_PATH>/kernels/custom_app_kernel_library_tests.h
# <CUSTOM_APPLICATION_PATH>/app_my_usecase/concerto.mak
# <CUSTOM_APPLICATION_PATH>/app_my_usecase/my_usecase.c
# <CUSTOM_APPLICATION_PATH>/app_my_usecase/my_usecase.h
# \endcode
#
# \ingroup FRAMEWORK
class ExportCode (Export) :
    ## Constructor used to create this object
    #
    # \param context [in] Context object. tiovx::context::Context
    # \param env_var [in] [optional] Path to the directory where these should be outputted; Default="VISION_APPS_PATH"
    def __init__(self, context, env_var='VISION_APPS_PATH') :
        self.env_var = env_var
        Export.__init__(self, context)
        self.usecase_code = UsecaseCode(context, self.env_var)

    ## Export object as C source code
    def export(self) :
        print ('Generating C code from OpenVX context ...')
        print ('Files [%s] and [%s]' % (self.context.name + '.h', self.context.name + '.c'))
        self.usecase_code.generate_code()
        print ('Generating C code from OpenVX context ... DONE !!!')

