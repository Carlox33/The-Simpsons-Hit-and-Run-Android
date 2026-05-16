//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


// PNG image loader
// 15-June-98, NV
// requires libpng.lib, and zlib.dll

#include <p3d/png.hpp>
#include <p3d/image.hpp>
#include <p3d/file.hpp>
#include <p3d/utility.hpp>

#include <png.h>

static void LoadPNG4(png_structp, png_infop, tImageHandler::Builder*);
static void LoadPNG8(png_structp, png_infop, tImageHandler::Builder*);
static void LoadPNG32(png_structp, png_infop, tImageHandler::Builder*);

// User I/O routines for libpng.  By default it uses stdio.
static void pngRead(png_structp png_ptr, png_bytep data, png_uint_32 length)
{
    tFile* read = (tFile*)png_get_io_ptr(png_ptr);
    //if(readPtr->pos + length > readPtr->size)
    //   return;
    read->GetData(data, length, tFile::BYTE);
}

// user error routines
static void p3d_png_err(png_structp, png_const_charp message)
{
    P3DASSERTMSG(0,message,"");
}

static void p3d_png_warn(png_structp, png_const_charp message)
{
    p3d::print(message);
    p3d::print("\n");
}

// user memory routines
static void * p3d_png_malloc(png_structp, png_size_t size)
{
    return p3d::MallocTemp(size);
}

static void p3d_png_free(png_structp, void* data)
{
    p3d::FreeTemp(data);
}

//-------------------------------------------------------------------
bool tPNGHandler::CheckFormat(Format format)
{
    return format == IMG_PNG;
}


void tPNGHandler::CreateImage(tFile* file, tImageHandler::Builder* builder)
{
    png_structp pPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);


    if(!pPNG)
    {
        return;
    }

    png_infop pngInfo = png_create_info_struct(pPNG);
    if(!pngInfo)
    {
        png_destroy_read_struct(&pPNG, 0, 0);
        return;
    }

    // install user i/o routines to parse the memory
    png_set_read_fn(pPNG, file, (png_rw_ptr)pngRead);

    // grab info
    png_read_info(pPNG, pngInfo);

    int channelDepth = png_get_bit_depth(pPNG, pngInfo);

    // check for supported colour type
    unsigned colourType = png_get_color_type(pPNG, pngInfo);
    if(colourType != PNG_COLOR_TYPE_PALETTE &&
        colourType != PNG_COLOR_TYPE_RGB &&
        colourType != PNG_COLOR_TYPE_RGB_ALPHA)
    {
        png_destroy_read_struct(&pPNG, 0, 0);
        return;
    }
    
    // we can't handle interlaced data
    unsigned interlaceType = png_get_interlace_type(pPNG, pngInfo);
    if(interlaceType != 0)
    {
        png_destroy_info_struct(pPNG, &pngInfo);
        png_destroy_read_struct(&pPNG, 0, 0);
        return;
    }
    
    // convert 16 bit/channel images to 8 bit/channel   
    if(channelDepth == 16)
    {
        png_set_strip_16(pPNG);
    }
    
    // convert RGB pixels to BGR
    png_set_bgr(pPNG);
    
    // expand 24 bit pixels to 32
    if(png_get_channels(pPNG, pngInfo) != 4)
    {
        png_set_filler(pPNG, 0xff, PNG_FILLER_AFTER);
    }

    // depalettise image
    if((colourType == PNG_COLOR_TYPE_PALETTE) &&
        (channelDepth != 4) && (channelDepth != 8))
    {
        png_set_expand(pPNG);
    }

    // transform data to appropriate format
    png_read_update_info(pPNG, pngInfo);

    if(channelDepth == 4 && colourType == PNG_COLOR_TYPE_PALETTE)
    {
        LoadPNG4(pPNG, pngInfo, builder);
    }
    else if(channelDepth == 8 && colourType == PNG_COLOR_TYPE_PALETTE)
    {
        LoadPNG8(pPNG, pngInfo, builder);
    }
    else
    {
        LoadPNG32(pPNG, pngInfo, builder);
    }

    // we are done!
    png_destroy_info_struct(pPNG, &pngInfo);
    png_destroy_read_struct(&pPNG, 0, 0);
}

bool tPNGHandler::SaveImage(tImage* image, char* filename)
{
    // someday, maybe
    return false;
}

void LoadPNG4(png_structp pPNG, png_infop pngInfo, tImageHandler::Builder* builder)
{
    const int bpp = 4;
    int width = png_get_image_width(pPNG, pngInfo);
    int height = png_get_image_height(pPNG, pngInfo);
    
    int numPalette = 0;
    png_color* srcPalette;
    png_get_PLTE(pPNG, pngInfo, &srcPalette, &numPalette);      
    
    pddiColour dstPalette[256];
    memset(dstPalette, 0, sizeof(pddiColour)*256);

    for(int i=0; i < numPalette; i++)
    {
        dstPalette[i].c = pddiColour(srcPalette[i].red, srcPalette[i].green, srcPalette[i].blue, 255);
    }

    // load transparency channel for palette
    if (png_get_valid(pPNG, pngInfo, PNG_INFO_tRNS) > 0)
    {
        int numAlpha;
        png_byte *srcAlpha;
        png_get_tRNS(pPNG, pngInfo, &srcAlpha, &numAlpha, 0);
        
        for(int i = 0; i < numAlpha; ++i)
        {
            dstPalette[i].SetAlpha(srcAlpha[i]);
        }
    }

    builder->BeginImage(width, height, bpp, tImageHandler::Builder::TOP, (pddiColour*)dstPalette);
    
    // create an image, read in the bits
    unsigned char* srcRow = (unsigned char*)p3d::MallocTemp(png_get_rowbytes(pPNG, pngInfo));
    unsigned char* dstRow = (unsigned char*)p3d::MallocTemp(width);
    
    for(int y = 0; y < height; y++)
    {
        png_read_row(pPNG, (unsigned char*)srcRow, (unsigned char*)NULL);

        // promote 4-bit palette indexes to 8-bit palette indexes
        for (int i=0; i<width/2; i++)
        {
            dstRow[(i*2)+0] = srcRow[i] >> 4;
            dstRow[(i*2)+1] = srcRow[i] & 0x0f;
        }
        
        builder->ProcessScanline8((unsigned char*)dstRow);
    }
    
    p3d::FreeTemp(srcRow);
    p3d::FreeTemp(dstRow);

    builder->EndImage();
}


void LoadPNG8(png_structp pPNG, png_infop pngInfo, tImageHandler::Builder* builder)
{
    const int bpp = 8;

    int width = png_get_image_width(pPNG, pngInfo);
    int height = png_get_image_height(pPNG, pngInfo);

    int numPalette = 0;
    png_color* srcPalette;
    png_get_PLTE(pPNG, pngInfo, &srcPalette, &numPalette);
    
    pddiColour dstPalette[256];
    for (int i=0; i<numPalette; i++)
    {
        dstPalette[i] = pddiColour(srcPalette[i].red, srcPalette[i].green, srcPalette[i].blue, 255);
    }

    // load transparency channel for palette
    if (png_get_valid(pPNG, pngInfo, PNG_INFO_tRNS) > 0)
    {
        int numAlpha;
        png_byte *srcAlpha;
        png_get_tRNS(pPNG, pngInfo, &srcAlpha, &numAlpha, 0);

        for(int i=0; i < numAlpha; ++i)
        {
            dstPalette[i].SetAlpha(srcAlpha[i]);
        }
    }

    builder->BeginImage(width, height, bpp, tImageHandler::Builder::TOP, (pddiColour*)dstPalette);

    // create an image, read in the bits
    unsigned char* row = (unsigned char*)p3d::MallocTemp(png_get_rowbytes(pPNG, pngInfo));

    for(int y = 0; y < height; y++)
    {
        png_read_row(pPNG, (unsigned char*)row, (unsigned char*)NULL);
        builder->ProcessScanline8((unsigned char*)row);
    }

    p3d::FreeTemp(row);

    builder->EndImage();
}


void LoadPNG32(png_structp pPNG, png_infop pngInfo, tImageHandler::Builder* builder)
{
    int bpp = (png_get_color_type(pPNG, pngInfo) == PNG_COLOR_TYPE_RGB_ALPHA) ? 32 : 24;
    int width = png_get_image_width(pPNG, pngInfo);
    int height = png_get_image_height(pPNG, pngInfo);

    builder->BeginImage(width, height, bpp, tImageHandler::Builder::TOP, (pddiColour*)NULL);
    
    // create an image, read in the bits
    unsigned* row = (unsigned*)p3d::MallocTemp(png_get_rowbytes(pPNG, pngInfo));  
    
    for(int y = 0; y < height; y++)
    {
        png_read_row(pPNG, (unsigned char*)row, (unsigned char*)NULL);
#ifdef RAD_GAMECUBE
        int a;
        for (a = 0; a < width; a++) row[a] = __lwbrx(&row[a], 0);
#endif
        builder->ProcessScanline32(row);
    }
    
    p3d::FreeTemp(row);
    builder->EndImage();
}

