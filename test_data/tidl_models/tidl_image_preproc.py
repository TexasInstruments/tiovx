"""
TEXAS INSTRUMENTS (INC)
=======================

File: tidl_image_preproc.py
Description: Performs image preprocessing of arbitrary sized images to raw binary \
             format as needed by different inference networks supported by TIDL

Supported networks:
"inception_v1"
"mobilenetv1"
"resnet10"
"squeez1"
"jacintonet11v2"

EXAMPLE USAGE:
-------------

python tidl_image_preproc.py --input_file=<path of input .jpg/.png/.bmp, mandatory> \
                             --network=<choice of network, mandatory> \
                             --output_file=<path of output binary file, optional>


python tidl_image_preproc.py --input_file="airshow.jpg" --network="inception_v1"

By default, the resulting output pre-processed binary file will be named as "input.rgb"
This raw-binary file can be provided as input to TIDL OpenVx node.

"""
import sys
import cv2
import numpy as np

error_msg1 = "python tidl_image_preproc.py --input_file=<path of input .jpg/.png/.bmp, mandatory> \
--network=<choice of network, mandatory> \
--output_file=<path of output binary file, optional>"
error_msg2 = "python tidl_image_preproc.py --input_file=airshow.jpg --network=inception_v1"
error_msg3 = "Supported Networks: "
net_list = ["inception_v1", "mobilenetv1", "resnet10", "squeez1", "jacintonet11v2"]

def tidl_image_preproc(input_file, network, output_file):
    TIDL_RAW_IMG_BGR_PLANAR = 0
    TIDL_RAW_IMG_RGB_PLANAR = 1

    if (network == "jacintonet11v2"):
        preProcType = 0
        width = 224
        height = 224
    
    if (network == "inception_v1"):
        preProcType = 2
        width = 224
        height = 224
    
    if (network == "mobilenetv1"):
        preProcType = 1
        width  = 224
        height = 224
    
    if (network == "resnet10"):
        preProcType = 0
        width = 224
        height = 224
    
    if (network == "squeez1"):
        preProcType = 1
        width  = 227
        height = 227

    if (preProcType == 0):
        resizeWidth  = 256
        resizeHeight = 256
        cropWidth    = width
        cropHeight   = height
        outFormat    = TIDL_RAW_IMG_BGR_PLANAR
        enableMeanSub = 0
        scale = 1

    if (preProcType == 1):
        resizeWidth = 256
        resizeHeight = 256
        cropWidth = width
        cropHeight = height
        outFormat = TIDL_RAW_IMG_BGR_PLANAR
        enableMeanSub = 1
        mean_val_ch0 = 104
        mean_val_ch1 = 117
        mean_val_ch2 = 123
        scale = 2

    if (preProcType == 2):
        factor = 0.875
        cropWidth = width
        cropHeight = height
        resizeWidth = int(width / factor)
        resizeHeight = int(height / factor)
        outFormat = TIDL_RAW_IMG_RGB_PLANAR
        enableMeanSub = 1
        mean_val_ch0 = 128
        mean_val_ch1 = 128
        mean_val_ch2 = 128
        scale = 1

    if (preProcType == 3):
        resizeWidth = 32
        resizeHeight = 32
        cropWidth = width
        cropHeight = height
        outFormat = TIDL_RAW_IMG_RGB_PLANAR
        enableMeanSub = 0
        scale = 1

    if (preProcType == 4):
        resizeWidth = width
        resizeHeight = height
        cropWidth = width
        cropHeight = height
        outFormat = TIDL_RAW_IMG_RGB_PLANAR
        enableMeanSub = 0
        scale = 1

    img = cv2.imread(input_file)

    if (outFormat == TIDL_RAW_IMG_RGB_PLANAR):
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)

    halfWidth = resizeWidth / 2;
    halfHeight = resizeHeight / 2;

    startX = halfWidth - cropWidth / 2;
    startY = halfHeight - cropHeight / 2;

    resize_img = cv2.resize(img, (resizeWidth, resizeHeight), 0, 0, cv2.INTER_AREA);

    crop_img = resize_img[int(startX):(int(startX) + int(cropWidth)), int(startY):(int(startY) + int(cropHeight)), :]
    
    ch0 = crop_img[:,:,0].astype(int)
    ch1 = crop_img[:,:,1].astype(int)
    ch2 = crop_img[:,:,2].astype(int)

    if (enableMeanSub):
        tmp0 = (ch0 - mean_val_ch0) / scale
        tmp1 = (ch1 - mean_val_ch1) / scale
        tmp2 = (ch2 - mean_val_ch2) / scale
    
        out0 = np.clip(tmp0.astype(int), -128, 127)
        out1 = np.clip(tmp1.astype(int), -128, 127)
        out2 = np.clip(tmp2.astype(int), -128, 127)
    else:
        tmp0 = ch0 / scale
        tmp1 = ch1 / scale
        tmp2 = ch2 / scale

        out0 = tmp0.astype(int)
        out1 = tmp1.astype(int)
        out2 = tmp2.astype(int)

    out_img = np.concatenate([out0, out1, out2])
    out_img = out_img.astype(np.dtype('B'))
    out_img.tofile(output_file)

if __name__ == '__main__':
    args = sys.argv[1:]
    
    if(len(args) < 2):
        print("Example Usage:")
        print(error_msg1)
        print(error_msg2)
        print(error_msg3 + str(net_list))

    else:
        output_file = "input.rgb"
        input_file = ""
        network = ""
    
        for arg in args:
            opt, value = arg.split("=")
        
            if(opt == "--input_file"):
                input_file = value
            if(opt == "--network"):
                network = value
            if(opt == "--output_file"):
                output_file = value
    
        network_found = 0
        for net in net_list:
            if (network == net):
                network_found = 1

        if(network_found == 1):
            print("Preprocessing input for " + network)
            tidl_image_preproc(input_file, network, output_file)
            print("Done!")
        else:
            print(error_msg3 + str(net_list))
        
