![](https://raw.githubusercontent.com/TexasInstruments/texasinstruments.github.io/main/documents/TI_document_header.png)

# TI OpenVX (TIOVX)

TI OpenVX (TIOVX) is TI's implementation of OpenVX Standard.

TIOVX allows users to create vision and compute applications using OpenVX API. These
OpenVX applications can be executed on supported TI SoCs such as 
[TDA4x](https://www.ti.com/product/TDA4VM). TIOVX is fully conformant to
[OpenVX v1.1](https://registry.khronos.org/OpenVX/specs/1.1/html/index.html) 
specification, as well as [Khronos](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/RESOURCES.html#autotoc_md1) and [TI](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/group__group__tivx__ext__top.html) extensions. TIOVX also provides optimized OpenVX kernels for C66x DSP. 
An extension API allows users to integrate their own natively developed custom kernels 
and call them using OpenVX APIs. Examples showing usage of TIOVX as part of the larger 
system, ex, camera input and display output are also provided as part of Processor SDK 
RTOS.

## Dependencies

TIOVX is delivered as a component from a larger SDK delivered by Texas Instruments (e.g. [J721E](https://www.ti.com/tool/download/PROCESSOR-SDK-RTOS-J721E), [J721S2](https://www.ti.com/tool/download/PROCESSOR-SDK-RTOS-J721S2), & [J784S4](https://www.ti.com/tool/download/PROCESSOR-SDK-RTOS-J784S4)).  It has dependencies on a few other repositories and compilers which are downloaded together as part of device-specific TI Processor SDK's.

Therefore, users are expected to download the relevant Processor SDK for the TI device they are working on as part of building and developing with TIOVX.  A recommended workflow would be to start with a specific version of the Processor SDK, and replace the TIOVX folder in the download with a clone of this repository (other repos in the SDK may also be available to clone as well).

## More Information

* [TIOVX User Guide](https://software-dl.ti.com/jacinto7/esd/processor-sdk-rtos-jacinto7/latest/exports/docs/tiovx/docs/user_guide/) - Includes TI extension API documentation, support, references, and instructions for building.
* [License](LICENSE.MD) - Software manifest which details the license information included in this repository.
* [Contributions](CONTRIBUTING.MD) - Information about how to contribute to this project. 