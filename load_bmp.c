#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/shape.h>
#include "image.h"

static long bfSize;
static short bfReserved1, bfReserved2;
static long bfOffBits, biSize;
static short biPlanes;
static long biCompression, biSizeImage;
static long biXPelsPerMeter, biYPelsPerMeter;
static long biClrUsed, biClrImportant;

static inline unsigned GetShort(FILE *);
static inline unsigned long GetLong(FILE *);
static int ReadHeader(FILE *, ImageInfo *);
static int Load1(FILE *, ImageInfo *);
static int Load4(FILE *, ImageInfo *);
static int Load8(FILE *, ImageInfo *);
static int Load24(FILE *, ImageInfo *);
static int Load4RGB(FILE *, ImageInfo *);
static int Load8RGB(FILE *, ImageInfo *);

int LoadBmp(ImageInfo *, char *);

static inline unsigned GetShort(FILE * fp)
{
  unsigned c, c1;

  c = getc(fp), c1 = getc(fp);
  return (c1 << 8) | c;
}

static inline unsigned long GetLong(FILE * fp)
{
  unsigned long c, c1, c2, c3;

  c = getc(fp), c1 = getc(fp), c2 = getc(fp), c3 = getc(fp);
  return (c3 << 24) | (c2 << 16) | (c1 << 8) | c;
}

static int ReadHeader(FILE * fp, ImageInfo * i_info)
{
  int i, c, bPad;

  /**
   * read the file type (first two bytes)
   **/

  if (getc(fp) != 'B' || getc(fp) != 'M')
    return -1;

  bfSize = GetLong(fp);
  bfReserved1 = GetShort(fp);
  bfReserved2 = GetShort(fp);
  bfOffBits = GetLong(fp);
  biSize = GetLong(fp);

  if (biSize == WIN_NEW || biSize == OS2_NEW) {
    i_info->width = GetLong(fp);
    i_info->height = GetLong(fp);
    biPlanes = GetShort(fp);
    i_info->BitCount = GetShort(fp);
    biCompression = GetLong(fp);
    biSizeImage = GetLong(fp);
    biXPelsPerMeter = GetLong(fp);
    biYPelsPerMeter = GetLong(fp);
    biClrUsed = GetLong(fp);
    biClrImportant = GetLong(fp);
  } else {
    /**
     * old bitmap
     **/
    i_info->width = GetShort(fp);
    i_info->height = GetShort(fp);
    biPlanes = GetShort(fp);
    i_info->BitCount = GetShort(fp);
    biCompression = BI_RGB;
    biSizeImage = (((biPlanes * i_info->BitCount * (i_info->width)) + 31) / 32) * 4 * (i_info->height);
    biXPelsPerMeter = biYPelsPerMeter = 0;
    biClrUsed = biClrImportant = 0;
  }
  /**
   * error checking
   **/
  if ((i_info->BitCount != 1 && i_info->BitCount != 4 && i_info->BitCount != 8 && i_info->BitCount != 24)
      || biPlanes != 1 || biCompression > BI_RLE4) {
    fprintf(stderr, "Bogus BMP File! "
	    "(bitCount=%d, Planes=%d, Compression=%ld)\n",
	    i_info->BitCount, biPlanes, biCompression);
    return -1;
  }
  if (((i_info->BitCount == 1 || i_info->BitCount == 24) && biCompression != BI_RGB)
      || (i_info->BitCount == 4 && biCompression == BI_RLE8)
      || (i_info->BitCount == 8 && biCompression == BI_RLE4)) {
    fprintf(stderr, "Bogus BMP File! (bitCount=%d, Compression=%ld)",
	    i_info->BitCount, biCompression);
    return -1;
  }
  bPad = 0;
  if (biSize != WIN_OS2_OLD) {
    /**
     * skip ahead to colormap, using biSize
     * 40 bytes read from biSize to biClrImportant
     **/

    c = biSize - 40;
    for (i = 0; i < c; i++)
      getc(fp);

    bPad = bfOffBits - (biSize + 14);
  }
  if (i_info->BitCount != 24) {
    int i, cmaplen;

    cmaplen = (biClrUsed) ? biClrUsed : 1 << i_info->BitCount;

    i_info->ImagePalette = i_info->ImagePalette ? realloc(i_info->ImagePalette, sizeof(i_info->ImagePalette[0]) * cmaplen)
      : malloc(sizeof(i_info->ImagePalette[0]) * cmaplen);
    if (!i_info->ImagePalette) {
      fprintf(stderr, "bmp:read palette failed.\n");
      return -1;
    }
    for (i = 0; i < cmaplen; i++) {
      ((i_info->ImagePalette) + i)->b = getc(fp);
      ((i_info->ImagePalette) + i)->g = getc(fp);
      ((i_info->ImagePalette) + i)->r = getc(fp);
      if (biSize != WIN_OS2_OLD) {
	getc(fp);
	bPad -= 4;
      }
    }
  }
  if (biSize != WIN_OS2_OLD) {
    /**
     * Waste any unused bytes between the colour map (if present)
     * and the start of the actual bitmap data.
     **/
    while (bPad-- > 0)
      getc(fp);
  }
  return 0;
}

/**
 * BMPファイル読み込みサブルーチン
 **/

static int Load1(FILE * fp, ImageInfo * i_info)
{
  int i, j, c, ct, padw;
  char *pic8;

  /**
   * 'padw', padded to be a multiple of 32
   **/
  c = 0;
  padw = (((i_info->width + 31) / 32) * 32 - i_info->width) / 8;

  for (i = 0; i < i_info->height; i++) {
    pic8 = (char *) i_info->ImageData + (i_info->height - i - 1) * i_info->width;
    for (j = ct = 0; j < i_info->width; j++, ct++) {
      if ((ct & 7) == 0) {
	if (c = getc(fp), c == EOF)
	  return -1;
	ct = 0;
      }
      *pic8++ = (c & 0x80) ? 1 : 0;
      c <<= 1;
    }
    for (j = 0; j < padw; j++)
      getc(fp);
  }
  return 0;
}

static int Load4RGB(FILE * fp, ImageInfo * i_info)
{
  int i, j, c, padw;
  char *pic8;

  /**
   * 'padw' padded to a multiple of 8pix (32 bits)
   **/
  c = 0;
  padw = (((i_info->width + 7) / 8) * 8 - i_info->width) / 2;

  for (i = 0; i < i_info->height; i++) {
    pic8 = (char *) i_info->ImageData + (i_info->height - i - 1) * i_info->width;

    for (j = 0; j < i_info->width; j++) {
      if ((j & 1) == 1)
	*pic8++ = (c & 0x0f);
      else {
	if (c = getc(fp), c == EOF)
	  return -1;
	*pic8++ = (c & 0xf0) >> 4;
      }
    }
    for (j = 0; j < padw; j++)
      getc(fp);
  }
  return 0;
}

static int Load4(FILE * fp, ImageInfo * i_info)
{
  int i, c, c1, x, y;
  char *pic8;

  switch (biCompression) {
  case BI_RGB:
    /**
     * read uncompressed data
     **/
    return Load4RGB(fp, i_info);
  case BI_RLE4:
    /**
     * read RLE4 compressed data
     **/
    break;
  default:
    fprintf(stderr, "unknown BMP compression type 0x%0lx\n",
	    biCompression);
    return -1;
  }
  c = c1 = 0;
  x = y = 0;
  pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;

  while (y < i_info->height) {
    if ((c = getc(fp)) == EOF)
      return -1;
    if (c) {
      /**
       * encoded mode
       **/
      c1 = getc(fp);
      for (i = 0; i < c; i++, x++)
	*pic8++ = (i & 1) ? (c1 & 0x0f) : ((c1 >> 4) & 0x0f
	  );
    } else {
      /**
       * c==0x00  :  escape codes
       **/
      if ((c = getc(fp)) == EOF)
	return -1;

      if (c == 0x00) {
	/**
         * end of line
         **/
	x = 0;
	y++;
	pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;
      } else if (c == 0x01)
	/**
         * end of pic8
         **/
	break;
      else if (c == 0x02) {
	/**
         * delta
         **/
	c = getc(fp);
	x += c;
	c = getc(fp);
	y += c;
	pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;
      } else {
	/**
         * absolute mode
         **/
	for (i = 0; i < c; i++, x++) {
	  if ((i & 1) == 1)
	    *pic8++ = (c1 & 0x0f);
	  else {
	    c1 = getc(fp);
	    *pic8++ = (c1 & 0xf0) >> 4;
	  }
	}
	/**
	 * read pad byte
	 **/
	if (((c & 3) == 1) || ((c & 3) == 2))
	  getc(fp);
      }
    }				/** escape processing **/
  }				/** while **/
  return 0;
}

static int Load8RGB(FILE * fp, ImageInfo * i_info)
{
  int i, j, c, padw;
  char *pic8;

  /**
   * 'padw' padded to a multiple of 4pix (32 bits)
   **/
  padw = ((i_info->width + 3) / 4) * 4 - i_info->width;

  for (i = 0; i < i_info->height; i++) {
    pic8 = (char *) i_info->ImageData + (i_info->height - i - 1) * i_info->width;
    for (j = 0; j < i_info->width; j++) {
      if ((c = getc(fp)) == EOF)
	return -1;
      *pic8++ = c;
    }
    for (j = 0; j < padw; j++)
      getc(fp);
  }
  return 0;
}

static int Load8(FILE * fp, ImageInfo * i_info)
{
  int i, c, c1, x, y;
  char *pic8;

  switch (biCompression) {
  case BI_RGB:
    /**
     * read uncompressed data
     **/
    return Load8RGB(fp, i_info);
  case BI_RLE8:
    /**
     * read RLE8 compressed data
     **/
    break;
  default:
    fprintf(stderr, "unknown BMP compression type 0x%0lx\n"
	    ,biCompression);
    return -1;
  }
  x = y = 0;
  pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;

  while (y < i_info->height) {
    if ((c = getc(fp)) == EOF)
      return -1;
    if (c) {
      /**
       * encoded mode
       **/
      c1 = getc(fp);
      for (i = 0; i < c; i++, x++)
	*pic8++ = c1;
    } else {
      /**
       * c==0x00  :  escape codes
       **/
      if ((c = getc(fp)) == EOF)
	return -1;
      if (c == 0x00) {
	/**
         * end of line
         **/
	x = 0;
	y++;
	pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;
      } else if (c == 0x01)
	/**
         * end of pic8
         **/
	break;
      else if (c == 0x02) {
	/**
         * delta
         **/
	c = getc(fp);
	x += c;
	c = getc(fp);
	y += c;
	pic8 = (char *) i_info->ImageData + x + (i_info->height - y - 1) * i_info->width;
      } else {
	/**
         * absolute mode
         **/
	for (i = 0; i < c; i++, x++)
	  *pic8++ = getc(fp);
	if (c & 1) {
	  /**
           * odd length run: read an extra pad
           * byte
           **/
	  getc(fp);
	}
      }
    }				/** escape processing **/

  }				/** while **/

  return 0;
}

static int Load24(FILE * fp, ImageInfo * i_info)
{
  int i, j, pad;
  char *pic24;

  /**
   * # of pad bytes to read at EOscanline
   **/
  pad = (4 - ((i_info->width * 3) % 4)) & 0x03;

  for (i = 0; i < i_info->height; i++) {
    pic24 = (char *) i_info->ImageData + (i_info->height - i - 1) * i_info->width * 3;

    for (j = 0; j < i_info->width; j++) {
      pic24[2] = getc(fp);	/** blue **/
      pic24[1] = getc(fp);	/** green **/
      pic24[0] = getc(fp);	/** red **/
      pic24 += 3;
    }
    for (j = 0; j < pad; j++)
      getc(fp);
  }
  return 0;
}

int LoadBmp(ImageInfo * i_info, char *fname)
{
  FILE *fp;

  if (NULL == (fp = fopen(fname, "r"))) {
    return -1;
  }
  if (ReadHeader(fp, i_info) < 0)
    return -1;

  switch (i_info->BitCount) {
  case 1:
  case 4:
  case 8:
    i_info->ImageData = malloc(i_info->width * i_info->height);
    break;
  case 24:
    i_info->ImageData = malloc(i_info->width * i_info->height * 3);
    break;
  }

  i_info->colorsuu = (biClrUsed) ? biClrUsed : 1 << i_info->BitCount;

  switch (i_info->BitCount) {
  case 1:
    Load1(fp, i_info);
    break;
  case 4:
    Load4(fp, i_info);
    break;
  case 8:
    Load8(fp, i_info);
    break;
  case 24:
    Load24(fp, i_info);
    break;
  }

  fclose(fp);
  return 0;
}
