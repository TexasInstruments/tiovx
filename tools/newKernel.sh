#!/bin/sh

if [ $# -eq 6 ]; then

cp c66x/vx_$1_target.c c66x/vx_$2_target.c
cp host/vx_$1_host.c host/vx_$2_host.c
cp include/tivx_kernel_$1.h include/tivx_kernel_$2.h

ls c66x/vx_$2_target.c | xargs sed -i 's/'"$1"'/'"$2"'/g'
ls c66x/vx_$2_target.c | xargs sed -i 's/'"$3"'/'"$4"'/g'
ls c66x/vx_$2_target.c | xargs sed -i 's/'"$5"'/'"$6"'/g'
ls host/vx_$2_host.c | xargs sed -i 's/'"$1"'/'"$2"'/g'
ls host/vx_$2_host.c | xargs sed -i 's/'"$3"'/'"$4"'/g'
ls host/vx_$2_host.c | xargs sed -i 's/'"$5"'/'"$6"'/g'
ls include/tivx_kernel_$2.h | xargs sed -i 's/'"$1"'/'"$2"'/g'
ls include/tivx_kernel_$2.h | xargs sed -i 's/'"$3"'/'"$4"'/g'
ls include/tivx_kernel_$2.h | xargs sed -i 's/'"$5"'/'"$6"'/g'

git add .

else

echo "\nusage:\n newKernel.sh <Old file name root> <New file name root> <Old Kernel name camel case> <New Kernel Name camel case> <Old ALL CAPS> <New ALL CAPS>"
echo " e.g. newKernel.sh absdiff warp_affine AbsDiff WarpAffine ABSDIFF WARP_AFFINE"

fi
