#ifndef _IMAGE_H
#define _IMAGE_H

#include <X11/Xlib.h>
enum {
  /**
   * BMP用の定数定義
   **/

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

  unsigned char r, g, b, pad;
};

typedef struct _ImageInfo {
  /**
   * 画像データ受け渡し用
   */

  Display *d;
  Window w;
  GC  gc;
  Pixmap pixmap;
  unsigned char *ImageData;
  struct palette *ImagePalette;
  int colorsuu;
  unsigned int width, height;
  short BitCount;
  int trans_pix;
  int is_shape;
  int ext_height;
  char* filename;
} ImageInfo;


#endif
