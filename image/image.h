#ifndef _IMAGE_H
#define _IMAGE_H

#ifdef _IMAGE_GLOBAL
#define IMAGE_GLOBAL
#else
#define IMAGE_GLOBAL extern
#endif

#ifdef _JPEG_GLOBAL
#define JPEG_GLOBAL
#else
#define JPEG_GLOBAL extern
#endif

#ifdef _BMP_GLOBAL
#define BMP_GLOBAL
#else
#define BMP_GLOBAL extern
#endif

#ifdef _XPM_GLOBAL
#define XPM_GLOBAL
#else
#define XPM_GLOBAL extern
#endif

#ifdef _PNG_GLOBAL
#define PNG_GLOBAL
#else
#define PNG_GLOBAL extern
#endif

#include <X11/Xlib.h>
#include "../config.h"

enum {
  /**
   * BMP�Ѥ�������
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
   * �����ѥ�åȥơ��֥�
   **/

  unsigned char r, g, b, pad;
};

typedef struct _ImageInfo {
  /**
   * �����ǡ��������Ϥ���
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

/**
 * �ؿ�������ؿ����Τ��������Ȥ���ʳ��Ǥ�extern�ˤʤ�褦�˺ٹ���
 * �Ƥ���Τǡ�image�ޤ��δؿ���Ȥ��ץ����ϡ����Υե������
 * include��������Ǥ褤��
 */

IMAGE_GLOBAL int LoadImage(ImageInfo *);

BMP_GLOBAL int LoadBmp(ImageInfo *);

#ifdef HAVE_LIBJPEG
JPEG_GLOBAL int LoadJpeg(ImageInfo *);
#endif

#ifdef HAVE_LIBXPM
XPM_GLOBAL int LoadXpm(ImageInfo *);
#endif

#ifdef HAVE_LIBPNG
PNG_GLOBAL int LoadPng(ImageInfo *);
#endif

#endif
