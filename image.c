#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/extensions/shape.h>
#include "image.h"
#include "config.h"

extern int LoadBmp(ImageInfo *, char *);

#ifdef HAVE_LIBJPEG
#ifdef WITH_JPEG
extern int LoadJpeg(ImageInfo *, char *);
#endif
#endif

#ifdef WITH_XPM
extern int LoadXpm(ImageInfo *, char *);
#endif

static struct {
  int (*loader) (ImageInfo *i_info, char *filename);
}   loaders[] = {

  {
    LoadBmp
  },
#ifdef HAVE_LIBJPEG
#ifdef WITH_JPEG
  {
    LoadJpeg
  },
#endif
#endif

#ifdef WITH_XPM
  {
    LoadXpm
  },
#endif
};

static inline int highbit(unsigned long mask)
{
  unsigned long hb = 1UL << (ULONG_HIGH_BITS - 1);
  int i;

  for (i = ULONG_HIGH_BITS - 1; i >= 0; i--, mask <<= 1)
    if (mask & hb)
      break;
  return i;
}

static unsigned long GetPixelFromRGB(struct palette * pal, XVisualInfo vinfo)
{
  u_long rmask, gmask, bmask;
  int rshift, gshift, bshift;
  unsigned long rv, gv, bv, pixel_value;

  /**
   *  $B<+J,$NM}2r$N$?$a$N3P$(=q$-(B.....
   *
   * mask$B$N0lHV>e0L$N%S%C%H$,2?%S%C%HL\$+$rD4$Y!"$=$3$K(Bu_char(8bit)$B$N(B
   * $BCM$N:G>e0L%S%C%H$r9g$o$;$k$?$a$K%7%U%H!#8N$K(B,highbit(mask)-7$B$@$1(B
   * $B%7%U%H$9$k!#(Bshift$B$NCM$,%^%$%J%9$K$J$k$H!"MW$9$k$K$=$l$G$O$_=P$7B?(B
   * $BJ,$O;H$o$l$J$$$N$G5$$K$;$:@dBPCM$@$11&%7%U%H$9$k!#Nc$($P(B
   * bpp=15(16)$B$@$H:G>e0L(B1bit$B$r;D$7$F(Brgb$B$K(B5bit$B$:$DM?$($i$l$k!#$D$^$j!"(B
   * $B%Q%l%C%H$NCM(B8bit$B$N$&$A>e0L(B5bit$B$@$1$,;H$o$l$k!#$3$N;~(Bbmask = 31$B!#(B
   * $B%T%/%;%kCM$N:G2<0L(Bbit$B$+$i(B5bit$B$,(Bblue$B$K;H$o$l$k$3$H$,J,$+$k!#(Bbmask
   * $B$N:G>e0L%S%C%H$O(B4bit$BL\(B($B:G2<0L$r(B0$B$H$9$k(B)$B!#8N$K%7%U%HNL$O(B4-7=-3,$B$D(B
   * $B$^$j1&$K(B3bit$B%7%U%H$9$k$3$H$K$J$k!#$3$l$K$h$C$FEv=i$NL\E*DL$j%Q%l%C(B
   * $B%H$N(B8bit$BCf>e0L(B5bit$B$,;H$o$l$k!#(B
   *
   * mask$B$NCM$O(BXVisualInfo$B$+$i<h$j=P$9!#(B
   **/

  rshift = highbit(rmask = vinfo.red_mask) - 7;
  gshift = highbit(gmask = vinfo.green_mask) - 7;
  bshift = highbit(bmask = vinfo.blue_mask) - 7;

  rv = rshift < 0 ? pal->r >> -rshift : pal->r << rshift;
  gv = gshift < 0 ? pal->g >> -gshift : pal->g << gshift;
  bv = bshift < 0 ? pal->b >> -bshift : pal->b << bshift;
  pixel_value = (rv & rmask) | (gv & gmask) | (bv & bmask);

  return (pixel_value);
}

static void GetPseudoPixelFromRGB(Display * d, Colormap cm, struct palette * pal, unsigned long pixel_value[], int colorsuu, int cells)
{
  int i, j;
  XColor scol[256], *ccel;
  int alloc_fail[256];


  /**
   * $B$H$j$"$($:!"6&M-%;%k$K5$9g$$$G<h$l$k$@$1<h$C$F$_$k(B
   **/

  for (i = 0; i < colorsuu; i++) {
    scol[i].blue = (pal + i)->b << 8;
    scol[i].red = (pal + i)->r << 8;
    scol[i].green = (pal + i)->g << 8;
    scol[i].flags = DoRed | DoGreen | DoBlue;

    if (XAllocColor(d, cm, &scol[i])) {
      pixel_value[i] = scol[i].pixel;
      alloc_fail[i] = 0;
    } else {
      pixel_value[i] = 0L;
      alloc_fail[i] = 1;
    }
  }

  for (; i < 256; i++) {
    pixel_value[i] = 0L;
    alloc_fail[i] = 0;
  }
  /**
   * $B$J$s$H$+6a$$?'$r3d$jEv$F$h$&$H$9$k(B
   **/

  if ((ccel = (XColor *) malloc(sizeof(XColor) * cells)) == NULL) {
    fprintf(stderr, "error: can't alloc RO color\n");
    exit(1);
  }
  for (i = 0; i < cells; i++)
    ccel[i].pixel = i;
  XQueryColors(d, cm, ccel, cells);

  for (i = 0; i < colorsuu; i++) {
    int min, close;
    int ri, gi, bi;

    if (!alloc_fail[i])
      continue;

    min = 1000000;
    close = -1;
    ri = (pal + i)->r;
    gi = (pal + i)->g;
    bi = (pal + i)->b;

    for (j = 0; j < cells; j++) {
      int dist, rdist, gdist, bdist;
      rdist = ri - (ccel[j].red >> 8);
      gdist = gi - (ccel[j].green >> 8);
      bdist = bi - (ccel[j].blue >> 8);
      dist = rdist * rdist + gdist * gdist + bdist * bdist;
      if (dist < min) {
	min = dist;
	close = j;
      }
    }

    if (close != -1 && XAllocColor(d, cm, &ccel[close])) {
      scol[i] = ccel[close];
      alloc_fail[i] = 0;
      pixel_value[i] = ccel[close].pixel;
    } else {
      pixel_value[i] = scol[i].pixel = 0;
    }
  }

  /**
   * $B$@$a$@$C$?$i(BAllocate$B$5$l$?Cf$+$i6a$$$N$rC5$9(B
   **/
  for (i = 0; i < colorsuu; i++) {
    int min, close, ri, gi, bi;

    if (!alloc_fail[i])
      continue;

    min = 1000000;
    close = -1;
    ri = (pal + i)->r;
    gi = (pal + i)->g;
    bi = (pal + i)->b;

    for (j = 0; j < cells; j++) {
      int dist, rdist, gdist, bdist;

      rdist = ri - (scol[j].red >> 8);
      gdist = gi - (scol[j].green >> 8);
      bdist = bi - (scol[j].blue >> 8);
      dist = rdist * rdist + gdist * gdist + bdist * bdist;
      if (dist < min) {
	min = dist;
	close = pixel_value[j];
      }
    }
    scol[i] = scol[close];
    pixel_value[i] = scol[i].pixel;
  }

}

int ExecLoader(ImageInfo * i_info, char *filename)
{
  size_t n_loader = sizeof loaders / sizeof loaders[0];
  int i;
  for (i = 0; i < n_loader; i++) {
    if (!loaders[i].loader(i_info, filename))
      return 0;
  }
  return -1;
}

int LoadImage(Widget xhw, GC * gc, Pixmap * pmap, char *fname, int *width, int *height, int ext_height, Boolean is_shape)
{
  /**
   * fname$B$G;XDj$5$l$?%U%!%$%k$rFI$_9~$`!#IA2hBP>]$K$J$k(B
   * Widget,GC,Pixmap$B$,I,MW!#(Bwidth$B$H(Bheight$B$O%U%!%$%k$+$i<hF@$5$l!"(B
   * $B%;%C%H$5$l$k!#(Bext_height$B$O(Bmask$B$r$+$1$k$H$-!"(Bmask$B0J30$NItJ,$N9b$5!#(B
   * $B$h!<$9$k$K(BXHishoWidget$B$+$i8F$V$H$-$K;~7W$NJ,$N9b$5$rL5;k$9$k$?$a(B
   * $B$K(B,$B;~7W$N9b$5$r;XDj$9$k$@$1!#IaDL$O(B0$B$G$h$m$7!#(Bis_shape $B$O(Bshape$B$9(B
   * $B$k$+$I$&$+!#(B0$B0J30$G(Bshape,
   *
   * $BFI$_9~$_$K@.8y$7$?$i(B0,$B<:GT$7$?$i(B-$B#1$rJV$9!#(B
   **/

  Display *d;
  Window w;
  Colormap cm;
  XImage *image;
  XVisualInfo vinfo, *vinfolist;
  Visual *vis;
  XWindowAttributes attr;
  XImage *mask_image;
  Pixmap mask;
  GC  mask_gc;
  struct palette *pal;
  FILE *fp;
  Window mask_win;
  Widget top;
  int i, j, depth, matched;
  int colorsuu;
  unsigned char *pdata;
  unsigned long pixel_value[256];
  int cells;
  unsigned long trans_pix;
  ImageInfo i_info;

  i_info.d = d = XtDisplay(xhw);
  i_info.w = w = XtWindow(xhw);
  vis = DefaultVisual(d, 0);
  i_info.depth = depth = DefaultDepth(d, 0);
  vinfo.screen = DefaultScreen(d);
  *height = trans_pix = 0;
  i_info.ImagePalette = (struct palette *) malloc(sizeof(struct palette));
  pal = (struct palette *) malloc(sizeof(struct palette));

  /**
   * $B%m!<%@$r8F$S=P$9(B
   */

  if (ExecLoader(&i_info, fname) == -1)
    return -1;

  *width = i_info.width;
  *height = i_info.height;
  colorsuu = i_info.colorsuu;

  /**
   * VisualInfo$B$r<hF@$7!":GE,$J$b$N$rA*Br$9$k!#(B
   **/

  XGetWindowAttributes(d, w, &attr);
  vinfo.visualid = XVisualIDFromVisual(attr.visual);
  vinfolist = XGetVisualInfo(d, VisualIDMask, &vinfo, &matched);
  if (!matched) {
    fprintf(stderr, "can't get visual info.");
    return -1;
  }
  vinfo = vinfolist[0];
  XFree(vinfolist);

  cells = vinfo.colormap_size;	/** size of colormap **/

  *pmap = XCreatePixmap(d, w, *width, *height, depth);
  image = XGetImage(d, *pmap, 0, 0, *width, *height, AllPlanes, ZPixmap);

  if (is_shape) {
    top = xhw;
    while (XtParent(top))
      top = XtParent(top);
    mask_win = XtWindow(top);
    mask = XCreatePixmap(d, w, *width, *height + ext_height, 1);
    mask_image = XGetImage(d, mask, 0, 0, *width, *height + ext_height, 1, ZPixmap);
  }
  switch (i_info.BitCount) {
  case 1:
  case 4:
  case 8:
    if (vinfo.class == TrueColor) {
      int i;

      for (i = 0; i < colorsuu; i++)
	pixel_value[i] = GetPixelFromRGB(i_info.ImagePalette + i, vinfo);

      for (; i < 256; i++)
	pixel_value[i] = 0L;

      for (i = 0; i < *height; i++) {
	for (j = 0; j < *width; j++) {
	  XPutPixel(image, j, i, pixel_value[i_info.ImageData[i * *width + j]]);
	  if (is_shape) {
	    if (i_info.ImageData[i * *width + j] == i_info.ImageData[0])
	      XPutPixel(mask_image, j, i, 0);
	    else
	      XPutPixel(mask_image, j, i, 1);
	  }
	}
      }

      if (is_shape) {
	for (i = *height; i < *height + ext_height; i++)
	  for (j = 0; j < *width; j++)
	    XPutPixel(mask_image, j, i, 1);
      }
    } else {
      /**
       * for 8-bit pseudo colors
       **/

      cm = DefaultColormap(d, 0);

      GetPseudoPixelFromRGB(d, cm, i_info.ImagePalette, pixel_value, colorsuu, cells);

      /**
       * change pixels to matched color index
       **/

      for (i = 0; i < *height; i++)
	for (j = 0; j < *width; j++) {
	  XPutPixel(image, j, i, pixel_value[i_info.ImageData[i * *width + j]]);

	  if (is_shape) {
	    if (i_info.ImageData[i * *width + j] == i_info.ImageData[0]) {
	      XPutPixel(mask_image, j, i, 0);
	    } else {
	      XPutPixel(mask_image, j, i, 1);
	    }
	  }
	}

      if (is_shape) {
	for (i = *height; i < *height + ext_height; i++)
	  for (j = 0; j < *width; j++)
	    XPutPixel(mask_image, j, i, 1);
      }
    }
    break;
  case 24:
    if (vinfo.class != TrueColor) {
      fprintf(stderr, "no supported mode...\n");
      return -1;
    }
    for (i = 0; i < *height; i++) {
      char *ptr;

      ptr = (char *) i_info.ImageData + (*height - i - 1) * *width * 3;

      for (j = 0; j < *width; j++) {
	pal->r = (unsigned char) ptr[0];
	pal->g = (unsigned char) ptr[1];
	pal->b = (unsigned char) ptr[2];

	ptr += 3;
	pixel_value[0] = GetPixelFromRGB(pal, vinfo);

	if (i == 0 && j == 0)
	  trans_pix = pixel_value[0];

	if (is_shape) {
	  if (pixel_value[0] == trans_pix)
	    XPutPixel(mask_image, j, *height - i - 1, 0);
	  else
	    XPutPixel(mask_image, j, *height - i - 1, 1);
	}
	XPutPixel(image, j, *height - i - 1, pixel_value[0]);
      }
    }
    if (is_shape) {
      for (i = *height; i < *height + ext_height; i++)
	for (j = 0; j < *width; j++)
	  XPutPixel(mask_image, j, i, 1);
    }
    break;
  }

  XFreeGC(d, *gc);
  *gc = XCreateGC(d, *pmap, 0, NULL);

  XPutImage(d, *pmap, *gc, image, 0, 0, 0, 0, *width, *height);

  if (is_shape) {
    mask_gc = XCreateGC(d, mask, 0, NULL);
    XPutImage(d, mask, mask_gc, mask_image, 0, 0, 0, 0, *width, *height + ext_height);
    XShapeCombineMask(d, mask_win, ShapeBounding, 0, 0, mask, ShapeSet);
    XFreePixmap(d, mask);
    XFreeGC(d, mask_gc);
  }
  free(i_info.ImageData);
  free(i_info.ImagePalette);
  free(pal);

  return 0;
}
