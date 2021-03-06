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
     * $B%Q%l%C%H>pJs$NJQ49!#(BXPM$B$G$N%Q%l%C%H>pJs$O(B char** image.colorTable$B$K(B
     * "#xxxxxx"$B$H(B16$B?J?t$N7A<0$GJ]B8$5$l$F$$$k!#(B
     * $B$=$3$G!"$H$j$"$($:(BR,G,B$B$N(B3$B$D$KJ,3d$7$F$+$i(Bstrtol$B$G(B10$B?J?t$N@0?tCM$KJQ49!#(B
     * $B$b$7(B"None"$B$@$C$?$iF)L@?'$J$N$G!"(Bi_info->trans_pix$B$KEPO?$7$F$*$/!#(B
     **/

    if (!strcmp("None", cptr)) {
      i_info->trans_pix = i;
      pal_ptr->r = pal_ptr->g = pal_ptr->b = 0;
    } else {
      unsigned int r,g,b;
      strncpy(col, cptr + 1, l);
      col[l] = '\0';
      r = (int) strtol(col, (char **) NULL, 16);
      strncpy(col, cptr + 1 + l, l);
      col[l] = '\0';
      g = (int) strtol(col, (char **) NULL, 16);
      strncpy(col, cptr + 1 + l * 2, l);
      col[l] = '\0';
      b = (int) strtol(col, (char **) NULL, 16);
      pal_ptr->r = r >> (l - 2);
      pal_ptr->g = g >> (l - 2);
      pal_ptr->b = b >> (l - 2);
    }
  }

  return 0;
}
