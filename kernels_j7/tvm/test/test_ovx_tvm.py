# Copyright (c) 2021 Texas Instruments Incorporated
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
# *       No reverse engineering, decompilation, or disassembly of this software is
# permitted with respect to any software provided in binary form.
#
# *       any redistribution and use are licensed by TI for use only with TI Devices.
#
# *       Nothing shall obligate TI to provide you with source code for the software
# licensed and provided to you in object code.
#
# If software source code is provided to you, modification and redistribution of the
# source code are permitted provided that the following conditions are met:
#
# *       any redistribution and use of the source code, including any resulting derivative
# works, are licensed by TI for use only with TI Devices.
#
# *       any redistribution and use of any object code compiled from the source code
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

"""Unit tests for TIDL compilation of TVM OpenVX dispatch."""

"""Usage:
TIDL_TOOLS_PATH=$PSDKR_PATH/tidl_j7_xx_yy_zz_ww/tidl_tools \
ARM64_GCC_PATH=$PSDKR_PATH/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu \
CGT7X_ROOT=$PSDKR_PATH/ti-cgt-c7000_x.y.z.www / \
python3 ./test_ovx_tvm.py --platform [J7, J721S2, J784S4, AM62A]

For EVM testing:
cp artifacts_ovx_tvm_c7x_target/tempDir/c7x_deploy_mod.out \
        /opt/vision_apps/test_data/tivx/tvm_models/ovx_tvm_test_small.out
cp artifacts_ovx_tvm_large_c7x_target/tempDir/c7x_deploy_mod.out \
        /opt/vision_apps/test_data/tivx/tvm_models/ovx_tvm_test_large.out
Repo for generated TVM models in OpenVX unit test:
        ssh://git@bitbucket.itg.ti.com/processor-sdk-vision/test_data.git
"""

import os
import numpy as np
import tvm
from tvm import relay
from tvm.relay.backend.contrib import tidl
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--target', action='store_true',
                    default=True,
                    help='generate code for target device (ARM core) (Default)')
parser.add_argument('--platform', action='store',
                      default="J7",
                      help='Compile model for which platform (J7, J721S2, J784S4, AM62A)')
parser.add_argument('--host', action='store_false',
                    dest="target",
                    help='generate code for host emulation (e.g. x86_64 core)')
parser.add_argument('--deny', dest='denylist', action='append',
                    help='force Relay operator to be unsupported by TIDL, comma-separated string')
parser.add_argument('--nooffload', action='store_true',
                    help='produce a host-only deployable module without TIDL offload')
args = parser.parse_args()

def get_tidl_tools_path():
    tidl_tools_path = os.getenv("TIDL_TOOLS_PATH")
    if tidl_tools_path is None:
        raise Exception("Environment variable TIDL_TOOLS_PATH is not set!")
    relay_import_lib = os.path.join(tidl_tools_path, "tidl_model_import_relay.so")
    if not os.path.exists(relay_import_lib):
        raise Exception("${TIDL_TOOLS_PATH}/tidl_model_import_relay.so does not exist!")
    return tidl_tools_path

def get_arm_compiler():
    arm_gcc_path = os.getenv("ARM64_GCC_PATH")
    if arm_gcc_path is None:
        raise Exception("Environment variable ARM64_GCC_PATH is not set!")
    arm_gcc = os.path.join(arm_gcc_path, "bin", "aarch64-none-linux-gnu-g++")
    if not os.path.exists(arm_gcc):
        raise Exception("${ARM64_GCC_PATH}/aarch64-none-linux-gnu-g++ does not exist!")
    return arm_gcc

def get_c7x_compiler_path():
    cgt7x_root = os.getenv("CGT7X_ROOT")
    if cgt7x_root is None:
        raise Exception("Environment variable CGT7X_ROOT is not set!")
    cl7x_bin = os.path.join(cgt7x_root, "bin", "cl7x")
    if not os.path.exists(cl7x_bin):
        raise Exception("${CGT7X_ROOT}/cl7x does not exist!")
    return cgt7x_root

def model_compile(model_name, mod_orig, params, model_input_list, max_num_subgraphs=16):
    """ Compile a model in Relay IR graph

    Parameters
    ----------
    model_name : string
        Name of the model
    mod_orig : tvm.relay.Module
        Original Relay IR graph
    params : dict of str to tvm.NDArray
        The parameter dict to be used by relay
    model_input_list : list of dictionary for multiple calibration data
        A dictionary where the key in input name and the value is input tensor
    max_num_subgraphs : int
        Max number of subgraphs to offload to TIDL
    Returns
    -------
    status: int
        Status of compilation:
            1  - compilation for TIDL offload succeeded
            -1 - compilation for TIDL offload failed - failure for CI testing
            0  - no compilation due to missing TIDL tools or ARM64 GCC tools
    """
    c7x_codegen = 1 if model_name.endswith('_c7x') else 0

    try:
        tidl_tools_path = get_tidl_tools_path()
        if args.target:
            arm_gcc = get_arm_compiler()
        if c7x_codegen == 1:
            cgt7x_root = get_c7x_compiler_path()
            if "large" in model_name:
                c7x_codegen = 1
    except Exception as ex:
        print(f"{__file__}: Skip compilation because: {ex}")
        return 0

    tidl_platform = args.platform   # or "AM57"
    tidl_version = "8.1"   # corresponding Processor SDK version
    tidl_artifacts_folder = "./artifacts_" + model_name +  ("_target" if args.target else "_host")
    os.makedirs(tidl_artifacts_folder, exist_ok = True)
    for root, dirs, files in os.walk(tidl_artifacts_folder, topdown=False):
        for f in files:
            os.remove(os.path.join(root, f))
        for d in dirs:
            os.rmdir(os.path.join(root, d))
    tidl_compiler = tidl.TIDLCompiler(platform=tidl_platform, version=tidl_version,
                                      tidl_tools_path=tidl_tools_path,
                                      artifacts_folder=tidl_artifacts_folder,
                                      tensor_bits=16,
                                      max_num_subgraphs=max_num_subgraphs,
                                      deny_list=args.denylist,
                                      c7x_codegen=c7x_codegen,
                                      accuracy_level=0,
                                      advanced_options={'calibration_iterations': 10}
                                     )

    if args.nooffload:
        mod, status = mod_orig, 0
    else:
        mod, status = tidl_compiler.enable(mod_orig, params, model_input_list)

    if status == 1: # TIDL compilation succeeded
        print("Graph execution with TIDL")
    else: # TIDL compilation failed or no TIDL compilation due to missing tools
        print("Graph execution without TIDL")

    if args.target:
        target = "llvm -device=arm_cpu -mtriple=aarch64-linux-gnu"
    else:
        target = "llvm"

    with tidl.build_config(tidl_compiler=tidl_compiler):
        graph, lib, params = relay.build_module.build(mod, target=target, params=params)
    tidl.remove_tidl_params(params)

    path_lib = os.path.join(tidl_artifacts_folder, "deploy_lib.so")
    path_graph = os.path.join(tidl_artifacts_folder, "deploy_graph.json")
    path_params = os.path.join(tidl_artifacts_folder, "deploy_param.params")
    if args.target:
        lib.export_library(path_lib, cc=arm_gcc)
    else:
        lib.export_library(path_lib)
    with open(path_graph, "w") as fo:
        fo.write(graph)
    with open(path_params, "wb") as fo:
        fo.write(relay.save_param_dict(params))

    print("Artifacts can be found at " + tidl_artifacts_folder)
    return status

def test_tidl_c7x(model_name):
    data = relay.var("data", relay.TensorType((1, 480, 14, 14), "float32"))
    weight = relay.var("weight", relay.TensorType((1, 480, 1, 1), "float32"))
    broadcast_mult = relay.multiply(data, weight)

    if "large" in model_name:
        for i in range(13):
            relu = relay.nn.relu(broadcast_mult)
            down = relay.nn.max_pool2d(relu, pool_size=(2,2))
            broadcast_mult = relay.multiply(down, weight)

    mod = tvm.IRModule.from_expr(relay.Function([data, weight], broadcast_mult))

    params = { "weight" : tvm.nd.array(np.ones((1, 480, 1, 1), "float32") * 2.0) }
    input_dict_list = [ { "data" : tvm.nd.array(np.ones((1, 480, 14, 14), "float32")) } ]

    status = model_compile(model_name, mod, params, input_dict_list, max_num_subgraphs=0)
    assert status != -1, "TIDL compilation failed"   # For CI test

if __name__ == '__main__':
    import os
    import logging
    #os.environ["DMLC_LOG_DEBUG"] = "1"
    #logging.getLogger("compile_engine").setLevel(logging.DEBUG)
    #logging.basicConfig(level=logging.DEBUG)

    test_tidl_c7x("ovx_tvm_c7x")
    test_tidl_c7x("ovx_tvm_large_c7x")

