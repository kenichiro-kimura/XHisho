#define _XPM_GLOBAL
#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/xpm.h>


int LoadXpm(ImageInfo * i_info)
{
  int i, j;
  XpmImage image;
  XpmInfo info;
  char col[3];

  if (XpmReadFileToXpmImage(i_info->filename, &image, &info) != XpmSuccess)
    return -1;

  i_info->width = image.width;
  i_info->height = image.height;
  i_info->colorsuu = image.ncolors;
  i_info->BitCount = 8;

  i_info->ImageData = malloc(i_info->width * i_info->height);
  if (!i_info->ImageData)
    return -1;

  for (i = 0; i < i_info->height; i++){
    for (j = 0; j < i_info->width; j++){
      i_info->ImageData[i * i_info->width + j] 
	= (unsigned char)image.data[i * i_info->width + j];
    }
  }

  i_info->ImagePalette = i_info->ImagePalette ? 
    realloc(i_info->ImagePalette, sizeof(struct palette) * i_info->colorsuu)
    : malloc(sizeof(struct palette) * i_info->colorsuu);

  if (!i_info->ImagePalette)
    return -1;


  for (i = 0; i < i_info->colorsuu; i++) {
    char *cptr = (image.colorTable + i)->c_color;
    struct palette *pal_ptr = i_info->ImagePalette + i;
    int l = (strlen(cptr) - 1) / 3;

    /**
     * パレット情報の変換。XPMでのパレット情報は char** image.colorTableに
     * "#xxxxxx"と16進数の形式で保存されている。
     * そこで、とりあえずR,G,Bの3つに分割してからstrtolで10進数の整数値に変換。
     * もし"None"だったら透明色なので、i_info->trans_pixに登録しておく。
     **/

    if (!strcmp("None", cptr)) {
      i_info->trans_pix = i;
      pal_ptr->r = pal_ptr->g = pal_ptr->b = 0;
    } else {
      unsigned int r,g,b;
      strncpy(col, cptr + 1, l);
      r = (int) strtol(col, (char **) NULL, 16);
      strncpy(col, cptr + 1 + l, l);
      g = (int) strtol(col, (char **) NULL, 16);
      strncpy(col, cptr + 1 + l * 2, l);
      b = (int) strtol(col, (char **) NULL, 16);
      pal_ptr->r = r >> (l - 2);
      pal_ptr->g = g >> (l - 2);
      pal_ptr->b = b >> (l - 2);
    }
  }

  return 0;
}
