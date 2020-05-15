/*

 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
 *
 */

#include <TI/tivx_mem.h>
#include <TI/tivx_debug.h>

#include <tivx_utils_bmp.h>

#define TIVX_UTILS_BMP_MEM_ALLOC_ALIGN      (255U)
#define TIVX_UTILS_BMP_HDR_SIZE             (14U)
#define TIVX_UTILS_BMP_INFO_HDR_SIZE        (40U)
#define TIVX_UTILS_BMP_BASE_HDR_SIZE        (TIVX_UTILS_BMP_HDR_SIZE+\
                                             TIVX_UTILS_BMP_INFO_HDR_SIZE)

/* Number of colors in the palette table. */
#define TIVX_UTILS_BMP_MAX_NUM_COLORS       (256U)

/* Red   - 1 byte
 * Green - 1 byte
 * Blue  - 1 byte
 * Rsvd  - 1 byte (unused = 0)
 */
#define TIVX_UTILS_BMP_NUM_BYTES_PER_COLOR  (4U)

/* Pallette size. */
#define TIVX_UTILS_BMP_PALETTE_SIZE         (TIVX_UTILS_BMP_MAX_NUM_COLORS *\
                                             TIVX_UTILS_BMP_NUM_BYTES_PER_COLOR)
/* RGB no compression. */
#define TIVX_UTILS_BMP_COMPR_RGB            (0U)

/* 8-bit RLE encoding. */
#define TIVX_UTILS_BMP_COMPR_RLE8           (1U)

/* 4-bit RLE encoding. */
#define TIVX_UTILS_BMP_COMPR_RLE4           (2U)
#define TIVX_UTILS_BMP_COMPR_BITFIELDS      (3U)

#define TIVX_UTILS_BMP_WRITE_32BITS(p, v) \
{                                         \
    p[0] = (uint8_t)(v);                \
    p[1] = (uint8_t)((v) >>  8);        \
    p[2] = (uint8_t)((v) >> 16);        \
    p[3] = (uint8_t)((v) >> 24);        \
    p += 4;                               \
}

#define TIVX_UTILS_BMP_READ_32BITS(p, v)                 \
{                                                        \
    v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);\
    p += 4;                                              \
}

#define  TIVX_UTILS_BMP_SCALE   14
#define  TIVX_UTILS_BMP_CR      (int32_t)(0.299*(1<< TIVX_UTILS_BMP_SCALE)+0.5)
#define  TIVX_UTILS_BMP_CG      (int32_t)(0.587*(1<< TIVX_UTILS_BMP_SCALE)+0.5)
#define  TIVX_UTILS_BMP_CB      ((1<<TIVX_UTILS_BMP_SCALE)-TIVX_UTILS_BMP_CR-TIVX_UTILS_BMP_CG)

typedef struct
{
    /** Blue. */
    uint8_t     b;

    /** Green. */
    uint8_t     g;

    /** Red. */
    uint8_t     r;

    /** Reserved. */
    uint8_t     a;

} tivx_utils_palette_t;

typedef struct
{
    /** Signature - 2 bytes: B (0x42) M (0x4D). */
    uint8_t                 sig[2];

    /** File size - 4 bytes. */
    uint32_t                fileSize;

    /** Reserved - 4 bytes. */
    uint32_t                rsvd0;

    /** File offset to start of image data. */
    uint32_t                offset;

    /** Info header size - 4 bytes (40 bytes). */
    uint32_t                infoHdrSize;

    /** Image width - 4 bytes. */
    uint32_t                width;

    /** Image height - 4 bytes. */
    uint32_t                height;

    /** Number of planes. */
    uint16_t                numPlanes;

    /** Number of bits per pixel. */
    uint16_t                bpp;

    /** Compression type - 4 bytes. */
    uint32_t                compType;

    /** Size of compressed image - 4 bytes (0 = if uncompressed). */
    uint32_t                compImgSize;

    /** Width resolution - 4 bytes. */
    uint32_t                widthRes;

    /** Height resolution - 4 bytes. */
    uint32_t                heightRes;

    /** Num colors actually used - 4 bytes. */
    uint32_t                numColorsUsed;

    /** Num important colors - 4 bytes (0 = all colors). */
    uint32_t                numImpColors;

    /** Color palette. */
    tivx_utils_palette_t    palette[TIVX_UTILS_BMP_MAX_NUM_COLORS];

} tivx_utils_bmp_hdr_t;

typedef struct
{
    /** Header context. */
    tivx_utils_bmp_hdr_t    hdr;

    /** Ram BMP file content. */
    const uint8_t          *data;

    /** Flag to indicate if it is a color image. */
    uint8_t                 isColor;

} tivx_utils_bmp_rd_context_t;

static int32_t isColorPalette(const tivx_utils_palette_t   *pal,
                              int32_t                       clrused)
{
    int32_t status;
    int32_t i;

    status = 0;
    for (i = 0; i < clrused; i++)
    {
        if ((pal[i].b != pal[i].g) || (pal[i].b != pal[i].r))
        {
            status = 1;
            break;
        }
    }

    return status;
}

static void cvtRGBToGray(uint8_t       *gray,
                         const uint8_t *src,
                         int32_t        n,
                         int32_t        scn,
                         int32_t        blueIdx)
{
    int32_t i;

    for ( i = 0; i < n; i++, src += scn )
    {
        gray[i] = (uint8_t)((src[blueIdx]*TIVX_UTILS_BMP_CB +
                             src[1]*TIVX_UTILS_BMP_CG +
                             src[blueIdx^2]*TIVX_UTILS_BMP_CR +
                             (1 << (TIVX_UTILS_BMP_SCALE-1))) >>
                           TIVX_UTILS_BMP_SCALE);
    }
}

static void cvtRGBToRGB(uint8_t        *dst,
                        const uint8_t  *src,
                        int32_t         n,
                        int32_t         scn,
                        int32_t         sblueIdx,
                        int32_t         dcn,
                        int32_t         dblueIdx)
{
    int32_t i;

    for (i = 0; i < n; i++, src += scn, dst += dcn)
    {
        dst[dblueIdx]   = src[sblueIdx];
        dst[1]          = src[1];
        dst[dblueIdx^2] = src[sblueIdx^2];

        if ( dcn == 4 )
        {
            dst[3] = scn < 4 ? 255 : src[3];
        }
    }
}

static void fillColorRow8(uint8_t                      *data,
                          const uint8_t                *indices,
                          int32_t                       n,
                          const tivx_utils_palette_t   *palette,
                          int32_t                       dcn,
                          int32_t                       dblueIdx)
{
    int32_t i;

    for ( i = 0; i < n; i++, data += dcn )
    {
        const tivx_utils_palette_t *p = palette + indices[i];

        data[dblueIdx]   = p->b;
        data[1]          = p->g;
        data[dblueIdx^2] = p->r;

        if ( dcn == 4 )
        {
            data[3] = 255;
        }
    }
}

static void fillGrayRow8(uint8_t        *data,
                         const uint8_t *indices,
                         int32_t        n,
                         const uint8_t *palette)
{
    int32_t i;

    for (i = 0; i < n; i++)
    {
        data[i] = palette[indices[i]];
    }
}

static void fillColorRow4(uint8_t                      *data,
                          const uint8_t                *indices,
                          int32_t                       n,
                          const tivx_utils_palette_t   *palette,
                          int32_t                       dcn,
                          int32_t                       dblueIdx)
{
    int32_t i = 0;

    for (; i <= n - 2; i += 2, data += dcn*2)
    {
        int32_t idx = *indices++;
        const tivx_utils_palette_t *p0 = palette + (idx >> 4);
        const tivx_utils_palette_t *p1 = palette + (idx & 15);

        data[dblueIdx]           = p0->b;
        data[1]                  = p0->g;
        data[dblueIdx^2]         = p0->r;
        data[dcn + dblueIdx]     = p1->b;
        data[dcn + 1]            = p1->g;
        data[dcn + (dblueIdx^2)] = p1->r;

        if (dcn == 4)
        {
            data[3] = data[7] = 255;
        }
    }

    if (i < n)
    {
        const tivx_utils_palette_t *p0 = palette + (indices[0] >> 4);

        data[dblueIdx]   = p0->b;
        data[1]          = p0->g;
        data[dblueIdx^2] = p0->r;

        if (dcn == 4)
        {
            data[3] = 255;
        }
    }
}

static void fillGrayRow4(uint8_t       *data,
                         const uint8_t *indices,
                         int32_t        n,
                         const uint8_t *palette)
{
    int32_t i = 0;

    for (; i <= n - 2; i += 2)
    {
        int32_t idx = *indices++;

        data[i]   = palette[idx >> 4];
        data[i+1] = palette[idx & 15];
    }

    if (i < n)
    {
        data[i] = palette[indices[0] >> 4];
    }
}

static void fillColorRow1(uint8_t                      *data,
                          const uint8_t                *indices,
                          int32_t                       n,
                          const tivx_utils_palette_t   *palette,
                          int32_t                       dcn,
                          int32_t                       dblueIdx)
{
    int32_t i = 0;
    int32_t mask = 0;
    int32_t idx = 0;

    for (i = 0; i < n; i++, data += dcn, mask >>= 1)
    {
        const tivx_utils_palette_t* p;

        if (mask == 0)
        {
            idx = *indices++;
            mask = 128;
        }

        p                = palette + ((idx & mask) != 0);
        data[dblueIdx]   = p->b;
        data[1]          = p->g;
        data[dblueIdx^2] = p->r;
        data[3]          = 255;
    }
}

static void fillGrayRow1(uint8_t       *data,
                         const uint8_t *indices,
                         int32_t        n,
                         const uint8_t *palette)
{
    int32_t i = 0;
    int32_t mask = 0;
    int32_t idx = 0;

    for (i = 0; i < n; i++, mask >>= 1)
    {
        if (mask == 0)
        {
            idx  = *indices++;
            mask = 128;
        }

        data[i] = palette[(idx & mask) != 0];
    }
}

static uint32_t tivx_utils_bmp_stride_mul_factor(vx_df_image format)
{
    uint32_t factor = 0;

    switch (format)
    {
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_NV21:
        case VX_DF_IMAGE_NV12:
        case VX_DF_IMAGE_YUV4:
        case VX_DF_IMAGE_IYUV:
            factor = 1;
            break;

        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_UYVY:
            factor = 2;
            break;

        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_RGBX:
            factor = 4;
            break;

        case VX_DF_IMAGE_RGB:
            factor = 3;
            break;

        default:
            break;
    }

    return factor;
}

static int32_t tivx_utils_bmp_read_hdr(tivx_utils_bmp_rd_context_t *cntxt)
{
    tivx_utils_bmp_hdr_t   *hdr;
    const uint8_t          *data;
    const uint8_t          *p;
    int32_t                 clrused;
    int32_t                 isColor;
    int32_t                 status;
    uint16_t                bpp;
    uint16_t                ct;

    data    = cntxt->data;
    hdr     = &cntxt->hdr;
    clrused = 0;
    isColor = 0;
    status  = 0;
    p       = data + 10;

    if ((data[0] != 'B') || (data[1] != 'M') )
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Invalid BMP header. Found: [0x%2.2X 0x%2.2X] "
                 "Expecting: [0x42 0x4D]\n",
                 data[0], data[1]);
        status = -1;
    }
    else if (hdr->fileSize < TIVX_UTILS_BMP_BASE_HDR_SIZE)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Invalid BMP header size: %d bytes.\n",
                 hdr->fileSize);
        status = -1;
    }

    /* Read the offset. */
    TIVX_UTILS_BMP_READ_32BITS(p, hdr->offset);

    /* Read the info header size. */
    TIVX_UTILS_BMP_READ_32BITS(p, hdr->infoHdrSize);

    /* Validate the data size. */
    if ((hdr->infoHdrSize + (int32_t)(p-data)) >= hdr->fileSize)
    {
        status = -1;
    }
    else
    {
        uint32_t    tmp;

        /* Read width. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->width);

        /* Read height. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->height);

        /* Read plane and bpp information. */
        TIVX_UTILS_BMP_READ_32BITS(p, tmp);
        hdr->numPlanes = (uint16_t)(tmp & 0xFFFF);
        hdr->bpp      = (uint16_t)((tmp >> 16) & 0xFFFF);
    }

    bpp = hdr->bpp;
    ct  = TIVX_UTILS_BMP_COMPR_RGB;

    if ((bpp != 1) && (bpp != 4) && (bpp != 8) &&
        (bpp != 16) && (bpp != 24) && (bpp != 32))
    {
        status = -1;
    }

    if ((status == 0) && (hdr->infoHdrSize >= 36))
    {
        /* Read compression type. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->compType);

        /* Read compImgSize. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->compImgSize);

        /* Read width resolution. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->widthRes);

        /* Read height resolution. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->heightRes);

        /* Read Num colors actually used. */
        TIVX_UTILS_BMP_READ_32BITS(p, hdr->numColorsUsed);

        /* Read Num important colors. */
        //TIVX_UTILS_BMP_READ_32BITS(p, hdr->numImpColors);
        p += hdr->infoHdrSize - 36;

        ct = hdr->compType;

        if ((hdr->width > 0)  &&
            (hdr->height != 0) &&
            (((bpp != 16) && (ct == TIVX_UTILS_BMP_COMPR_RGB)) ||
             ((bpp == 16) &&
              ((ct == TIVX_UTILS_BMP_COMPR_RGB) ||
               (ct == TIVX_UTILS_BMP_COMPR_BITFIELDS))) ||
             ((bpp == 4) && (ct == TIVX_UTILS_BMP_COMPR_RLE4)) ||
             ((bpp == 8) && (ct == TIVX_UTILS_BMP_COMPR_RLE8))))
        {
            isColor = 1;

            if (bpp <= 8)
            {
                clrused = clrused == 0 ? (1 << bpp) : hdr->numColorsUsed;
                memcpy(hdr->palette, p, clrused*4);
                p += clrused*4;
                isColor = isColorPalette(hdr->palette, clrused);
            }
            else if ((bpp == 16) && (ct == TIVX_UTILS_BMP_COMPR_BITFIELDS))
            {
                int32_t redMask;
                int32_t greenMask;
                int32_t blueMask;

                TIVX_UTILS_BMP_READ_32BITS(p, redMask);
                TIVX_UTILS_BMP_READ_32BITS(p, greenMask);
                TIVX_UTILS_BMP_READ_32BITS(p, blueMask);

                if ((blueMask == 0x1f) &&
                    (greenMask == 0x3e0) &&
                    (redMask == 0x7c00))
                {
                    bpp = 15;
                }
                else if (!((blueMask == 0x1f) &&
                           (greenMask == 0x7e0) &&
                           (redMask == 0xf800)))
                {
                    status = -1;
                }
            }
            else if ((bpp == 16) && (ct == TIVX_UTILS_BMP_COMPR_RGB))
            {
                bpp = 15;
            }
        }
    }
    else if ((status == 0) && (hdr->infoHdrSize == 12))
    {
        hdr->compType = ct;

        if ((hdr->width > 0) && (hdr->height != 0) && (bpp != 16))
        {
            if (bpp <= 8)
            {
                int32_t i;

                clrused = 1 << bpp;

                for (i = 0; i < clrused; i++, p += 3)
                {
                    hdr->palette[i].b = p[0];
                    hdr->palette[i].g = p[1];
                    hdr->palette[i].r = p[2];
                    hdr->palette[i].a = 255;
                }

                isColor = isColorPalette(hdr->palette, clrused);
            }
        }
    }

    cntxt->isColor = (uint8_t)isColor;

    if ((bpp == 15) || (bpp == 16) ||
        ((ct != TIVX_UTILS_BMP_COMPR_RGB) &&
         (ct != TIVX_UTILS_BMP_COMPR_BITFIELDS)))
    {
        status = -1;
    }

    return status;
}

static int32_t
tivx_utils_bmp_read_data(tivx_utils_bmp_rd_context_t   *cntxt,
                         int32_t                        dcn,
                         tivx_utils_bmp_image_params_t *imgParams)
{
    uint8_t                 grayPalette[TIVX_UTILS_BMP_MAX_NUM_COLORS];
    tivx_utils_palette_t   *palette;
    tivx_utils_bmp_hdr_t   *hdr;
    const uint8_t          *srcData;
    uint8_t                *dstData;
    int32_t                 status;
    uint16_t                bpp;
    int32_t                 srcStep;
    int32_t                 width;
    int32_t                 height;
    int32_t                 stride_y;
    int32_t                 memBlkSize;
    int32_t                 color;
    int32_t                 i;

    hdr      = &cntxt->hdr;
    bpp      = hdr->bpp;
    height   = imgParams->height;
    width    = imgParams->width;
    stride_y = imgParams->stride_y;
    palette  = hdr->palette;
    status   = 0;
    color    = dcn > 1;

    /* Allocate the memory to hold the decoded BMP image data. */
    memBlkSize = imgParams->height * stride_y;
    imgParams->data = (uint8_t *)tivxMemAlloc(memBlkSize, TIVX_MEM_EXTERNAL);

    if (imgParams->data == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR,
                 "Failed to allocate memory of size %lu bytes.\n",
                 memBlkSize);

        status = -1;
    }

    if (status == 0)
    {
        imgParams->dataSize = memBlkSize;

        /* Set the data pointers. */
        srcData = cntxt->data + hdr->offset;
        dstData = imgParams->data;

        srcStep = ((width*(bpp != 15 ? bpp : 16) + 7)/8 + 3) & -4;
        if (height > 0)
        {
            dstData += (height-1) * stride_y;
            stride_y = -stride_y;
        }

        if ((color == 0) && (bpp <= 8))
        {
            cvtRGBToGray(grayPalette, &palette[0].b, (1 << bpp), 4, 0);
        }

        if (color)
        {
            switch (bpp)
            {
                case 1:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillColorRow1(dstData, srcData, width, palette, dcn, 2);
                    }
                    break;

                case 4:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillColorRow4(dstData, srcData, width, palette, dcn, 2);
                    }
                    break;

                case 8:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillColorRow8(dstData, srcData, width, palette, dcn, 2);
                    }
                    break;

                case 24:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        cvtRGBToRGB(dstData, srcData, width, 3, 0, dcn, 2);
                    }
                    break;

                case 32:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        cvtRGBToRGB(dstData, srcData, width, 4, 0, dcn, 2);
                    }
                    break;

                default:
                    status = -1;
                    break;

            } /* switch (bpp) */
        }
        else
        {
            switch (bpp)
            {
                case 1:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillGrayRow1(dstData, srcData, width, grayPalette);
                    }
                    break;

                case 4:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillGrayRow4(dstData, srcData, width, grayPalette);
                    }
                    break;

                case 8:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        fillGrayRow8(dstData, srcData, width, grayPalette);
                    }
                    break;

                case 24:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        cvtRGBToGray(dstData, srcData, width, 3, 0);
                    }
                    break;

                case 32:
                    for (i = 0; i < height; i++, dstData += stride_y, srcData += srcStep)
                    {
                        cvtRGBToGray(dstData, srcData, width, 4, 0);
                    }
                    break;

                default:
                    status = -1;
                    break;

            } /* switch (bpp) */
        }
    }

    return status;
}

static int32_t tivx_utils_get_channels(vx_df_image format)
{
    int32_t channels;

    if (format == VX_DF_IMAGE_RGB)
    {
        channels = 3;
    }
    else if (format == VX_DF_IMAGE_RGBX)
    {
        channels = 4;
    }
    else
    {
        /* VX_DF_IMAGE_U8
         * VX_DF_IMAGE_U16
         * VX_DF_IMAGE_S16
         * VX_DF_IMAGE_U32
         * VX_DF_IMAGE_S32
         */
        channels = 1;
    }

    return channels;
}

/* Public APIs. */
int32_t tivx_utils_bmp_read_mem(const uint8_t                  *data,
                                uint32_t                        dataSize,
                                int32_t                         dcn,
                                tivx_utils_bmp_image_params_t  *imgParams)
{
    tivx_utils_bmp_rd_context_t     cntxt;
    tivx_utils_bmp_hdr_t           *hdr;
    int32_t                         status;

    status = 0;

    if (data == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Data NULL.\n");
        status = -1;
    }
    else if (dataSize == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid dataSize: %d bytes.\n", dataSize);
        status = -1;
    }

    if (status == 0)
    {
        memset(&cntxt, 0, sizeof(tivx_utils_bmp_rd_context_t));

        cntxt.data = data;

        /* Initialize the header fields. */
        hdr = &cntxt.hdr;
        hdr->fileSize = dataSize;

        status = tivx_utils_bmp_read_hdr(&cntxt);
    }

    if (status == 0)
    {
        imgParams->width  = hdr->width;
        imgParams->height = hdr->height;

        if (dcn <= 0)
        {
            dcn = cntxt.isColor ? 3 : 1;
        }

        if (dcn == 1)
        {
            imgParams->format = VX_DF_IMAGE_U8;
            imgParams->bpp    = 1; /* Out Image bpp. */
        }
        else if (dcn == 3)
        {
            imgParams->format = VX_DF_IMAGE_RGB;
            imgParams->bpp    = 3; /* Out Image bpp. */
        }
        else
        {
            imgParams->format = VX_DF_IMAGE_RGBX;
            imgParams->bpp    = 4; /* Out Image bpp. */
        }

        imgParams->stride_y =
            imgParams->width *
            tivx_utils_bmp_stride_mul_factor(imgParams->format);

        status = tivx_utils_bmp_read_data(&cntxt, dcn, imgParams);
    }

    return status;
}

int32_t tivx_utils_bmp_read(const char                     *filename,
                            int32_t                         dcn,
                            tivx_utils_bmp_image_params_t  *imgParams)
{
    FILE       *f;
    uint8_t    *buf;
    size_t      memBlkSize;
    int32_t     status;

    f      = NULL;
    buf    = NULL;
    status = 0;

    if (filename == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Filename NULL.\n");

        status = -1;
    }

    if (status == 0)
    {
        /* Open the file to read the data. */
        f = fopen(filename, "rb");

        if (!f)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to open file <%s>.\n", filename);
            status = -1;
        }
    }

    if (status == 0)
    {
        /* Get the file size. */
        fseek(f, 0, SEEK_END);

        memBlkSize = (size_t)ftell(f);

        fseek(f, 0, SEEK_SET);

        if (memBlkSize <= 0)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "Invalid file size: %lu bytes.\n",
                     memBlkSize);

            status = -1;
        }
        else
        {
            buf = (uint8_t *)tivxMemAlloc(memBlkSize, TIVX_MEM_EXTERNAL);

            if (buf == NULL)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Failed to allocate memory of size %lu bytes.\n",
                         memBlkSize);

                status = -1;
            }
        }

        if (status == 0)
        {
            size_t  bytesRead;

            bytesRead = fread(buf, 1, memBlkSize, f);

            if (bytesRead != memBlkSize)
            {
                VX_PRINT(VX_ZONE_ERROR,
                         "Could only read %lu bytes of %lu bytes.\n",
                         bytesRead, memBlkSize);

                status = -1;
            }
        }
    }

    if (status == 0)
    {
        status = tivx_utils_bmp_read_mem(buf, memBlkSize, dcn, imgParams);
    }

    if (f)
    {
        fclose(f);
    }

    if (buf)
    {
        tivxMemFree(buf, memBlkSize, TIVX_MEM_EXTERNAL);
    }

    return status;
}

int32_t tivx_utils_bmp_read_release(tivx_utils_bmp_image_params_t  *imgParams)
{
    int32_t status;

    status = 0;

    if (imgParams == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "imgParams NULL.\n");
        status = -1;
    }
    else if (imgParams->data == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid data pointer.\n");
        status = -1;
    }
    else if (imgParams->dataSize == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid data size.\n");
        status = -1;
    }

    if (status == 0)
    {
        tivxMemFree(imgParams->data, imgParams->dataSize, TIVX_MEM_EXTERNAL);
    }

    return status;
}

int32_t tivx_utils_bmp_write(const char    *filename,
                             const uint8_t *data,
                             int32_t        width,
                             int32_t        height,
                             int32_t        stride_y,
                             vx_df_image    df)
{
    FILE       *f;
    uint8_t    *buf;
    uint8_t    *p;
    int32_t     numChans;
    int32_t     channels;
    int32_t     width3;
    int32_t     fileStep;
    int32_t     headerSize;
    int32_t     fileSize;
    int32_t     memBlkSize;
    int32_t     status;
    int32_t     y;
    int32_t     i;

    status = 0;
    f      = NULL;
    buf    = NULL;

    if (filename == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Filename NULL.\n");
        status = -1;
    }
    else if (data == NULL)
    {
        VX_PRINT(VX_ZONE_ERROR, "Data NULL.\n");
        status = -1;
    }
    else if (stride_y == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid stride_y: %d\n", stride_y);
        status = -1;
    }
    else if (width == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid width: %d\n", width);
        status = -1;
    }
    else if (height == 0)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid height: %d\n", height);
        status = -1;
    }

    numChans   = tivx_utils_get_channels(df);;
    channels   = numChans == 4 ? 3 : numChans;
    width3     = width * channels;
    fileStep   = (width3 + 3) & -4;
    headerSize = TIVX_UTILS_BMP_HDR_SIZE + TIVX_UTILS_BMP_INFO_HDR_SIZE;

    if (channels == 1)
    {
        /* The color table is present if bitsPerPexel <= 8. The colors should
         * be ordered by importance.
         */
        headerSize += TIVX_UTILS_BMP_PALETTE_SIZE;
    }

    fileSize   = (fileStep * height) + headerSize;
    memBlkSize = (fileSize + TIVX_UTILS_BMP_MEM_ALLOC_ALIGN) &
                 ~(TIVX_UTILS_BMP_MEM_ALLOC_ALIGN);

    if (status == 0)
    {
        f = fopen(filename, "wb");

        if (!f)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to open file <%s>.\n", filename);
            status = -1;
        }
    }

    if (status == 0)
    {
        buf = (uint8_t *)tivxMemAlloc(memBlkSize, TIVX_MEM_EXTERNAL);

        if (buf == NULL)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "Failed to allocate memory of size %d bytes.\n",
                     memBlkSize);

            status = -2;
        }
    }

    if (status == 0)
    {
        size_t  bytesWrote;

        p = buf;

        /*** HEADAR (14 bytes) ****/
        /* Signature - 2 bytes: B (0x42) M (0x4D). */
        p[0] = 'B';
        p[1] = 'M';
        p   += 2;

        /* File size - 4 bytes. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, fileSize);

        /* Reserved - 4 bytes. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, 0);

        /* File offset to start of image data. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, headerSize);

        /*** INFO HEADAR (40 bytes) ****/
        /* Info Header size - 4 bytes. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, TIVX_UTILS_BMP_INFO_HDR_SIZE);

        /* Image width - 4 bytes. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, width);

        /* Image height - 4 bytes. */
        TIVX_UTILS_BMP_WRITE_32BITS(p, height);

        /* Bits per pixel + num planes - 4 bytes:
         * - bits 0-15:  num planes (= 1)
         * - bits 16-31: bits per pixel
         *   -  1 = monochrome palette. NumColors = 1
         *   -  4 = 4-bit palletized. NumColors = 2^4 = 16
         *   -  8 = 8-bit palletized. NumColors = 2^8 = 256
         *   - 16 = 16-bit palletized. NumColors = 2^16 = 65536
         *   - 24 = 24-bit palletized. NumColors = 2^24 = 16 Million
         */
        /* At this point 'channels' is either 1 or 3 which should get
         * mapped to 8 or 24. The logic (channels << 19) does the
         * mapping (1 --> 8, 3 --> 24) and places it in the upper
         * 16 bits.
         */
        TIVX_UTILS_BMP_WRITE_32BITS(p, 1 | (channels << 19));

        /* Compression type - 4 bytes.
         * - Only 4 and 8 bit images may be compressed.
         */
        TIVX_UTILS_BMP_WRITE_32BITS(p, TIVX_UTILS_BMP_COMPR_RGB);

        /* Rest of the bytes:
         * - Size of compressed image - 4 bytes (0 = if uncompressed)
         * - Width resolution         - 4 bytes
         * - Height resolution        - 4 bytes
         * - Num colors actually used - 4 bytes
         * - Num important colors     - 4 bytes (0 = all colors)
         */
        memset(p, 0, 20);
        p += 20;

        if (channels == 1)
        {
            for (i = 0; i < TIVX_UTILS_BMP_MAX_NUM_COLORS; i++, p += 4)
            {
                p[0] = (uint8_t)i;
                p[1] = (uint8_t)i;
                p[2] = (uint8_t)i;
                p[3] = 255;
            }
        }

        for (y = height - 1; y >= 0; y--, p += fileStep)
        {
            if (numChans == 1)
            {
                memcpy(p, data + stride_y*y, width);
            }
            else
            {
                const uint8_t *imgrow = data + stride_y*y;
                uint8_t       *dst = p;

                for (i = 0; i < width; i++, imgrow += numChans, dst += 3)
                {
                    dst[0] = imgrow[2];
                    dst[1] = imgrow[1];
                    dst[2] = imgrow[0];
                }
            }

            if (fileStep > width3)
            {
                memset(p + width3, 0, fileStep - width3);
            }
        }

        bytesWrote = fwrite(buf, 1, fileSize, f);

        if (bytesWrote != fileSize)
        {
            VX_PRINT(VX_ZONE_ERROR,
                     "Could only write %lu bytes of %d bytes.\n",
                     bytesWrote, fileSize);

            status = -3;
        }
    }

    if (f)
    {
        fclose(f);
    }

    if (buf)
    {
        tivxMemFree(buf, memBlkSize, TIVX_MEM_EXTERNAL);
    }

    return status;
}

