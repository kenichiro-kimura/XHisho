#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>

#include "image.h"

int LoadXpm(ImageInfo * i_info, char *filename)
{
  int i, j;
  XpmImage image;
  XpmInfo info;
  char col[3];

  if (XpmReadFileToXpmImage(filename, &image, &info) != XpmSuccess)
    return -1;

  i_info->width = image.width;
  i_info->height = image.height;
  i_info->colorsuu = image.ncolors;
  i_info->BitCount = image.cpp;

  i_info->ImageData = malloc(i_info->width * i_info->height);

  for (i = 0; i < i_info->height; i++)
    for (j = 0; j < i_info->width; j++)
      i_info->ImageData[i * i_info->width + j] = (unsigned char) image.data[i * i_info->width + j];

  i_info->ImagePalette = i_info->ImagePalette ?
    realloc(i_info->ImagePalette, sizeof(i_info->ImagePalette[0]) * i_info->colorsuu)
    : malloc(sizeof(i_info->ImagePalette[0]) * i_info->colorsuu);

  for (i = 0; i < i_info->colorsuu; i++) {
    strncpy(col, (image.colorTable + i)->c_color + 1, 2);
    (i_info->ImagePalette + i)->r = (int) strtol(col, (char **) NULL, 16);
    strncpy(col, (image.colorTable + i)->c_color + 3, 2);
    (i_info->ImagePalette + i)->g = (int) strtol(col, (char **) NULL, 16);
    strncpy(col, (image.colorTable + i)->c_color + 5, 2);
    (i_info->ImagePalette + i)->b = (int) strtol(col, (char **) NULL, 16);
  }
  return 0;
}
