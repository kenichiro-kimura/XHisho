#define _IMAGE_GLOBAL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include "image.h"
#include "globaldefs.h"

static struct {
/**
 * グラフィックローダ。関数ポインタの配列で表現。
 * 新しいローダは(*int)(ImageInfo*)の形で作ってここに登録するだけ。
 **/

  int (*loader) (ImageInfo * i_info);
}   loaders[] = {
  {
    LoadAnim
  },
  {
    LoadBmp
  },
#ifdef HAVE_LIBPNG
  {
    LoadPng
  },
#endif
#ifdef HAVE_LIBXPM
  {
    LoadXpm
  },
#endif
#ifdef HAVE_LIBJPEG
  {
    LoadJpeg
  },
#endif

};

static inline int highbit(unsigned long mask)
{
  /**
   * maskの最上位ビットが何ビット目かを調べる関数。
   * pixel値の計算で使う。
   **/

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
   *  自分の理解のための覚え書き.....
   *
   * maskの一番上位のビットが何ビット目かを調べ、そこにu_char(8bit)の
   * 値の最上位ビットを合わせるためにシフト。故に,highbit(mask)-7だけ
   * シフトする。shiftの値がマイナスになると、要するにそれではみ出し多
   * 分は使われないので気にせず絶対値だけ右シフトする。例えば
   * bpp=15(16)だと最上位1bitを残してrgbに5bitずつ与えられる。つまり、
   * パレットの値8bitのうち上位5bitだけが使われる。この時bmask = 31。
   * ピクセル値の最下位bitから5bitがblueに使われることが分かる。bmask
   * の最上位ビットは4bit目(最下位を0とする)。故にシフト量は4-7=-3,つ
   * まり右に3bitシフトすることになる。これによって当初の目的通りパレッ
   * トの8bit中上位5bitが使われる。
   *
   * maskの値はXVisualInfoから取り出す。
   *
   * で、この関数は上記のような動作をしてパレットのRGB値からpixel値を
   * 求める。
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
  /**
   * PseudoColor環境でパレットのRGB値からpixel値を求める。その際、
   * Colormap Cmを参照して以下のように動作する。
   *
   * 1.共有セルに取れるだけ取る
   * 2.近い色を共有セルに取ろうとする
   * 3.割り当てたうちから近いものを選ぶ
   **/

  int i, j;
  XColor scol[256], *ccel;
  int alloc_fail[256];


  /**
   * とりあえず、共有セルに気合いで取れるだけ取ってみる
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
   * なんとか近い色を割り当てようとする
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
   * だめだったらAllocateされた中から近いのを探す
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

int ExecLoader(ImageInfo * i_info)
{
  int i;

  for (i = 0; i < NUM_OF_ARRAY(loaders); i++) {
    if (!loaders[i].loader(i_info))
      return 0;
  }
  return -1;
}

int LoadImage(ImageInfo* i_info)
{
  /**
   * ImageInfoを引数に与える。ImageInfoに必要なデータをセットしておく
   * のを忘れない。必要なのは
   *
   * Display* i_info->d;
   * Window   i_info->w;
   * int      i_info->is_shape;
   * int      i_info->ext_height;
   * char*    filename;
   *
   * の5つ。filenameはちゃんとmallocしてからセットするよーに。     
   * 
   * i_info->ext_heightはmaskをかけるとき、mask以外の部分の高さ。よー
   * するにXHishoWidgetから呼ぶときに時計の分の高さを無視するために,時
   * 計の高さを指定するだけ。普通は0でよろし。i_info->is_shape はshape
   * するかどうか。0以外でshape。
   *
   * 読み込みに成功したら0,失敗したら-1を返す。
   *
   **/

  Display *d;
  Window w;
  Colormap cm;
  XImage *image;
  XVisualInfo vinfo, *vinfolist;
  Visual *vis;
  XWindowAttributes attr;
  XImage *mask_image;
  GC  mask_gc;
  struct palette *pal;
  int i,j, depth, matched;
  unsigned int width,height;
  int colorsuu;
  unsigned long pixel_value[256];
  int cells;
  unsigned long trans_pix;
  int loaded_num;
  unsigned int trans_index;

  i_info->trans_pix = -1;
  d = i_info->d;
  w = i_info->w;
  vis = DefaultVisual(d, 0);
  depth = DefaultDepth(d, 0);
  vinfo.screen = DefaultScreen(d);
  i = j = height = trans_pix = trans_index = 0;
  i_info->ImagePalette = (struct palette *) malloc(sizeof(struct palette));
  i_info->image = (AnimImage *) malloc(sizeof(AnimImage));
  if(!i_info->image) return -1;
    
  pal = (struct palette *) malloc(sizeof(struct palette));
  i_info->num_of_images = 1;
  i_info->loaded_images = 0;
  i_info->anim = 0;

  /**
   * ローダを呼び出す
   **/
  while(i_info->num_of_images > i_info->loaded_images){
    if(i_info->anim == 2){
	i_info->filename 
	  = malloc(strlen((i_info->image + i_info->loaded_images)->filename) + 1);	
      strcpy(i_info->filename,(i_info->image + i_info->loaded_images)->filename);
    }

    if(!strcmp(i_info->filename,"GOTO") ||
       !strcmp(i_info->filename,"MAIL") ||
       !strcmp(i_info->filename,"SCHEDULE")){
      i_info->loaded_images++;
      continue;
    }

    if((loaded_num = Search_files(i_info->filename)) != -1){
      memcpy(i_info->image + i_info->loaded_images
	     ,i_info->image + loaded_num,sizeof(AnimImage));
      i_info->loaded_images++;
      continue;
    }

    if (ExecLoader(i_info) == -1){
      printf("no supported format:%s\n",i_info->filename);
      return -1;
    }
    
    width = i_info->width;
    height = i_info->height;
    colorsuu = i_info->colorsuu;
    
    if(i_info->anim == 2){
      (i_info->image + i_info->loaded_images)->width = width;
      (i_info->image + i_info->loaded_images)->height = height;
    }

    if(i_info->anim == 1){
      realloc(i_info->filename
	      ,strlen((i_info->image + i_info->loaded_images)->filename));
      strcpy(i_info->filename,(i_info->image + i_info->loaded_images)->filename);
      i_info->anim = 2;
      continue;
    }

    /**
     * VisualInfoを取得し、最適なものを選択する。
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
    
    (i_info->image + i_info->loaded_images)->pixmap 
      = XCreatePixmap(d, w, width, height, depth);
    image = XGetImage(d, (i_info->image + i_info->loaded_images)->pixmap, 0, 0, width, height, AllPlanes, ZPixmap);
    
    if (i_info->is_shape) {
      (i_info->image + i_info->loaded_images)->mask
	= XCreatePixmap(d, w, width, height + i_info->ext_height, 1);
      mask_image = XGetImage(d, (i_info->image + i_info->loaded_images)->mask
			     , 0, 0, width, height + i_info->ext_height, 1, ZPixmap);
    }
    switch (i_info->BitCount) {
    case 1:
    case 4:
    case 8:
      if (vinfo.class == TrueColor) {
	int i;
	
	for (i = 0; i < colorsuu; i++)
	  pixel_value[i] = GetPixelFromRGB(i_info->ImagePalette + i, vinfo);
	
	for (; i < 256; i++)
	  pixel_value[i] = 0L;
	
	/**
	 * i_info->trans_pixがセットされていればそちらを透明色として使う。
	 * セットされていなければ、左上隅の色を透明色として使う。
	 *
	 * i_info->trans_pixはXPMの"None"で指定される。もしGIFローダを実
	 * 装したら、GIFの透明色(のパレット番号)をセットする。
	 **/
	
	if (i_info->trans_pix != -1) {
	  trans_index = i_info->trans_pix;
	} else {
	  trans_index = i_info->ImageData[0];
	}
	
	for (i = 0; i < height; i++) {
	  for (j = 0; j < width; j++) {
	    XPutPixel(image, j, i, pixel_value[i_info->ImageData[i * width + j]]);
	    if (i_info->is_shape) {
	      if (i_info->ImageData[i * width + j] == trans_index)
		XPutPixel(mask_image, j, i, 0);
	      else
		XPutPixel(mask_image, j, i, 1);
	    }
	  }
	}
	
	if (i_info->is_shape) {
	  for (i = height; i < height + i_info->ext_height; i++)
	    for (j = 0; j < width; j++)
	      XPutPixel(mask_image, j, i, 1);
	}
      } else {
	/**
	 * for 8-bit pseudo colors
	 **/
	
	cm = DefaultColormap(d, 0);
	
	GetPseudoPixelFromRGB(d, cm, i_info->ImagePalette, pixel_value, colorsuu, cells);
	
	/**
	 * change pixels to matched color index
	 **/
	
	if (i_info->trans_pix != -1) {
	  trans_index = i_info->trans_pix;
	} else {
	  trans_index = i_info->ImageData[0];
	}
	
	for (i = 0; i < height; i++)
	  for (j = 0; j < width; j++) {
	    XPutPixel(image, j, i, pixel_value[i_info->ImageData[i * width + j]]);
	    
	    if (i_info->is_shape) {
	      if (i_info->ImageData[i * width + j] == trans_index) {
		XPutPixel(mask_image, j, i, 0);
	      } else {
		XPutPixel(mask_image, j, i, 1);
	      }
	    }
	  }
	
	if (i_info->is_shape) {
	  for (i = height; i < height + i_info->ext_height; i++)
	    for (j = 0; j < width; j++)
	      XPutPixel(mask_image, j, i, 1);
	}
      }
      break;
    case 24:
      if (vinfo.class != TrueColor) {
	fprintf(stderr, "no supported mode...\n");
	return -1;
      }
      for (i = 0; i < height; i++) {
	char *ptr;
	
	ptr = (char *) i_info->ImageData + (height - i - 1) * width * 3;
	
	for (j = 0; j < width; j++) {
	  pal->r = (unsigned char) ptr[0];
	  pal->g = (unsigned char) ptr[1];
	  pal->b = (unsigned char) ptr[2];
	  
	  ptr += 3;
	  pixel_value[0] = GetPixelFromRGB(pal, vinfo);
	  
	  if (i == 0 && j == 0)
	    trans_pix = pixel_value[0];
	  
	  if (i_info->is_shape) {
	    if (pixel_value[0] == trans_pix)
	      XPutPixel(mask_image, j, height - i - 1, 0);
	    else
	      XPutPixel(mask_image, j, height - i - 1, 1);
	  }
	  XPutPixel(image, j, height - i - 1, pixel_value[0]);
	}
      }
      if (i_info->is_shape) {
	for (i = height; i < height + i_info->ext_height; i++)
	  for (j = 0; j < width; j++)
	    XPutPixel(mask_image, j, i, 1);
      }
      break;
    default:
      fprintf(stderr, "not supported format\n");
      return -1;
    }
    
    XFreeGC(d, i_info->gc);
    i_info->gc = XCreateGC(d, (i_info->image + i_info->loaded_images)->pixmap, 0, NULL);
    
    XPutImage(d, (i_info->image + i_info->loaded_images)->pixmap
	      , i_info->gc, image, 0, 0, 0, 0, width, height);
    
    if (i_info->is_shape) {
      mask_gc = XCreateGC(d, (i_info->image + i_info->loaded_images)->mask, 0, NULL);
      XPutImage(d, (i_info->image + i_info->loaded_images)->mask, mask_gc
		, mask_image, 0, 0, 0, 0, width, height + i_info->ext_height);
      XFreeGC(d, mask_gc);
    }
    free(i_info->ImageData);
    free(i_info->ImagePalette);
    free(pal);
    i_info->ImageData = NULL;
    i_info->ImagePalette = NULL;
    pal = NULL;
    Add_files(i_info->filename,i_info->loaded_images);
    i_info->loaded_images++;
  }
    
  return 0;
}
  
