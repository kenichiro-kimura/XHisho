#ifndef _IMAGE_H
#define _IMAGE_H

enum {
  BI_RGB = 0,
  BI_RLE8 = 1,
  BI_RLE4 = 2,
  WIN_OS2_OLD = 12,
  WIN_NEW = 40,
  OS2_NEW = 64,
  ULONG_HIGH_BITS = 32
};

struct palette {
  /**
   * 画像パレットテーブル 
   **/
  unsigned char r,g,b,pad;
};

typedef struct _ImageInfo{
  /**
   * 画像データ
   */

  Display *d;
  Window w;
  struct palette *ImagePalette;
  int colorsuu;
  int width,height;
  int depth;
  short BitCount;
  unsigned char* ImageData;
} ImageInfo;
  

#endif
