#define _PNG_GLOBAL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/shape.h>
#include "image.h"
#include "png.h"

int LoadPng(ImageInfo * i_info)
{
  FILE* fp;
  unsigned char buf[8];
  png_structp png_ptr;
  png_infop info_ptr,end_info;
  png_bytep row_pointer;
  unsigned int width,height,bit_depth,color_type,interlace_type;
  int i,j;

  /**
   * check file whether PNG or not
   **/

  if((fp = fopen(i_info->filename,"rb")) == NULL)  return -1;

  fread(buf, 1, 8, fp);
  rewind(fp);
  if (buf [0] != 0x89 ||
      buf [1] != 'P' ||
      buf [2] != 'N' ||
      buf [3] != 'G' ||
      buf [4] != 0x0d ||
      buf [5] != 0x0a ||
      buf [6] != 0x1a ||
      buf [7] != 0x0a){
    return -1;
  }

  /**
   * read PNG data
   **/

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,(png_voidp)NULL
				   ,NULL,NULL);
  if(!png_ptr){
    fclose(fp);
    return -1;
  }

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr){
    png_destroy_read_struct(&png_ptr,NULL,NULL);
    fclose(fp);
    return -1;
  }

  end_info = png_create_info_struct(png_ptr);
  if(!end_info){
    png_destroy_read_struct(&png_ptr,NULL,NULL);
    fclose(fp);
    return -1;
  }

  if (setjmp(png_ptr->jmpbuf)){
    png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
    fclose(fp);
    return -1;
  }

  png_init_io(png_ptr, fp);
  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png_ptr,info_ptr, &width,&height,&bit_depth,&color_type,&interlace_type
	       ,NULL,NULL);

  i_info->width = width;
  i_info->height = height;
  i_info->BitCount = 24;

  i_info->ImageData = malloc(i_info->width * i_info->height * 3);

  if((color_type & PNG_COLOR_MASK_COLOR)){
    png_color* palette;
    int num_palette = -1;

    png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
    i_info->colorsuu = num_palette;

    if(num_palette > 256){
      if (color_type == PNG_COLOR_TYPE_PALETTE)
	png_set_expand(png_ptr);
      i_info->BitCount = 24;
    } else {
      i_info->BitCount = 8;
      i_info->ImagePalette = i_info->ImagePalette ?
	realloc(i_info->ImagePalette, sizeof(i_info->ImagePalette[0]) * i_info->colorsuu)
	: malloc(sizeof(i_info->ImagePalette[0]) * i_info->colorsuu);

      if(!i_info->ImagePalette){
	printf("Can't alloc palette:%d\n",i_info->colorsuu);
	png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
	fclose(fp);
	return -1;
      }

      for(i = 0; i < i_info->colorsuu;i++){
	i_info->ImagePalette[i].r = palette[i].red;
	i_info->ImagePalette[i].g = palette[i].green;
	i_info->ImagePalette[i].b = palette[i].blue;
      }
    }
  }

  png_set_strip_16(png_ptr);
  png_set_packing(png_ptr);

  if(png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
    png_set_expand(png_ptr);

  png_set_filler(png_ptr,0xff,PNG_FILLER_AFTER);

  if(bit_depth > 8)
    png_set_swap(png_ptr);

  for(i = 0; i < height;i++){
    unsigned char* ptr;

    row_pointer = malloc(width * 4);
    png_read_rows(png_ptr, &row_pointer, NULL, 1);
    ptr = row_pointer;
    switch(i_info->BitCount){
    case 8:
      for(j = 0 ; j < width;j++){
	i_info->ImageData[i * width + j ] = *ptr++;
      }
      break;
    case 24:
      for(j = 0; j < width * 3 ;j += 3){
	i_info->ImageData[i * width * 3 + j ] = *ptr++;
	i_info->ImageData[i * width * 3 + j + 1] = *ptr++;
	i_info->ImageData[i * width * 3 + j + 2] = *ptr++;
	ptr++;
      }
      break;
    }
    free(row_pointer);
  }

  png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
  fclose(fp);
  return 0;
}


