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
static short biPlanes, biBitCount;
static long biCompression, biSizeImage;
static long biXPelsPerMeter, biYPelsPerMeter;
static long biClrUsed, biClrImportant;

unsigned char *ImageData;

struct palette *ImagePalette;

static int Load1(FILE *, int, int);
static int Load4(FILE *, int, int);
static int Load8(FILE *, int, int);
static int Load24(FILE *, int, int);
static int Load4RGB(FILE *, int, int);
static int Load8RGB(FILE *, int, int);
static unsigned long GetPixelFromRGB(struct palette *, XVisualInfo);
static void GetPseudoPixelFromRGB(Display *, Colormap, struct palette *, unsigned long *, int, int);


static inline int highbit(unsigned long mask)
{
    unsigned long hb = 1UL << (ULONG_HIGH_BITS - 1);
    int i;

    for (i = ULONG_HIGH_BITS - 1; i >= 0; i--, mask <<= 1)
	if (mask & hb)
	    break;
    return i;
}

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

static int ReadHeader(FILE * fp, int *biWidth, int *biHeight)
{
    int i, c, bPad;

    /** read the file type (first two bytes) **/

    if (getc(fp) != 'B' || getc(fp) != 'M')
	return -1;

    bfSize = GetLong(fp);
    bfReserved1 = GetShort(fp);
    bfReserved2 = GetShort(fp);
    bfOffBits = GetLong(fp);
    biSize = GetLong(fp);

    if (biSize == WIN_NEW || biSize == OS2_NEW) {
	*biWidth = GetLong(fp);
	*biHeight = GetLong(fp);
	biPlanes = GetShort(fp);
	biBitCount = GetShort(fp);
	biCompression = GetLong(fp);
	biSizeImage = GetLong(fp);
	biXPelsPerMeter = GetLong(fp);
	biYPelsPerMeter = GetLong(fp);
	biClrUsed = GetLong(fp);
	biClrImportant = GetLong(fp);
    } else {
	/** old bitmap **/
	*biWidth = GetShort(fp);
	*biHeight = GetShort(fp);
	biPlanes = GetShort(fp);
	biBitCount = GetShort(fp);
	biCompression = BI_RGB;
	biSizeImage = (((biPlanes * biBitCount * (*biWidth)) + 31) / 32) * 4 * (*biHeight);
	biXPelsPerMeter = biYPelsPerMeter = 0;
	biClrUsed = biClrImportant = 0;
    }
    /** error checking **/
    if ((biBitCount != 1 && biBitCount != 4 && biBitCount != 8 && biBitCount != 24)
	|| biPlanes != 1 || biCompression > BI_RLE4) {
	fprintf(stderr, "Bogus BMP File! "
		"(bitCount=%d, Planes=%d, Compression=%ld)\n",
		biBitCount, biPlanes, biCompression);
	return -1;
    }
    if (((biBitCount == 1 || biBitCount == 24) && biCompression != BI_RGB)
	|| (biBitCount == 4 && biCompression == BI_RLE8)
	|| (biBitCount == 8 && biCompression == BI_RLE4)) {
	fprintf(stderr, "Bogus BMP File! (bitCount=%d, Compression=%ld)",
		biBitCount, biCompression);
	return -1;
    }
    bPad = 0;
    if (biSize != WIN_OS2_OLD) {
	/** skip ahead to colormap, using biSize **/
	/** 40 bytes read from biSize to biClrImportant **/

	c = biSize - 40;
	for (i = 0; i < c; i++)
	    getc(fp);

	bPad = bfOffBits - (biSize + 14);
    }
    if (biBitCount != 24) {
	int i, cmaplen;

	cmaplen = (biClrUsed) ? biClrUsed : 1 << biBitCount;

	ImagePalette = ImagePalette ? realloc(ImagePalette, sizeof(ImagePalette[0]) * cmaplen)
	    : malloc(sizeof(ImagePalette[0]) * cmaplen);
	if (!ImagePalette) {
	    fprintf(stderr, "bmp:read palette failed.\n");
	    return -1;
	}
	for (i = 0; i < cmaplen; i++) {
	    (ImagePalette + i)->b = getc(fp);
	    (ImagePalette + i)->g = getc(fp);
	    (ImagePalette + i)->r = getc(fp);
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
 * --------------- BMP�ե������ɤ߹��ߥ��֥롼����
 **/

static int Load1(FILE * fp, int biWidth, int biHeight)
{
    int i, j, c, ct, padw;
    char *pic8;

    /** 'padw', padded to be a multiple of 32 **/
    c = 0;
    padw = (((biWidth + 31) / 32) * 32 - biWidth) / 8;

    for (i = 0; i < biHeight; i++) {
	pic8 = (char *) ImageData + (biHeight - i - 1) * biWidth;
	for (j = ct = 0; j < biWidth; j++, ct++) {
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

static int Load4RGB(FILE * fp, int biWidth, int biHeight)
{
    int i, j, c, padw;
    char *pic8;

    /** 'padw' padded to a multiple of 8pix (32 bits) **/
    c = 0;
    padw = (((biWidth + 7) / 8) * 8 - biWidth) / 2;

    for (i = 0; i < biHeight; i++) {
	pic8 = (char *) ImageData + (biHeight - i - 1) * biWidth;

	for (j = 0; j < biWidth; j++) {
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

static int Load4(FILE * fp, int biWidth, int biHeight)
{
    int i, c, c1, x, y;
    char *pic8;

    switch (biCompression) {
    case BI_RGB:
	/** read uncompressed data **/
	return Load4RGB(fp, biWidth, biHeight);
    case BI_RLE4:
	/** read RLE4 compressed data **/
	break;
    default:
	fprintf(stderr, "unknown BMP compression type 0x%0lx\n",
		biCompression);
	return -1;
    }
    c = c1 = 0;
    x = y = 0;
    pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;

    while (y < biHeight) {
	if ((c = getc(fp)) == EOF)
	    return -1;
	if (c) {		/** encoded mode **/
	    c1 = getc(fp);
	    for (i = 0; i < c; i++, x++)
		*pic8++ = (i & 1) ? (c1 & 0x0f) : ((c1 >> 4) & 0x0f
		    );
	} else {		/** c==0x00  :  escape codes **/
	    if ((c = getc(fp)) == EOF)
		return -1;

	    if (c == 0x00) {	/** end of line **/
		x = 0;
		y++;
		pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;
	    } else if (c == 0x01)
		break;		/** end of pic8 **/
	    else if (c == 0x02) {	/** delta **/
		c = getc(fp);
		x += c;
		c = getc(fp);
		y += c;
		pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;
	    } else {		/** absolute mode **/
		for (i = 0; i < c; i++, x++) {
		    if ((i & 1) == 1)
			*pic8++ = (c1 & 0x0f);
		    else {
			c1 = getc(fp);
			*pic8++ = (c1 & 0xf0) >> 4;
		    }
		}
		/** read pad byte **/
		if (((c & 3) == 1) || ((c & 3) == 2))
		    getc(fp);
	    }
	}			/** escape processing **/
    }				/** while **/
    return 0;
}

static int Load8RGB(FILE * fp, int biWidth, int biHeight)
{
    int i, j, c, padw;
    char *pic8;

    /** 'padw' padded to a multiple of 4pix (32 bits) **/
    padw = ((biWidth + 3) / 4) * 4 - biWidth;

    for (i = 0; i < biHeight; i++) {
	pic8 = (char *) ImageData + (biHeight - i - 1) * biWidth;
	for (j = 0; j < biWidth; j++) {
	    if ((c = getc(fp)) == EOF)
		return -1;
	    *pic8++ = c;
	}
	for (j = 0; j < padw; j++)
	    getc(fp);
    }
    return 0;
}

static int Load8(FILE * fp, int biWidth, int biHeight)
{
    int i, c, c1, x, y;
    char *pic8;

    switch (biCompression) {
    case BI_RGB:
	/** read uncompressed data **/
	return Load8RGB(fp, biWidth, biHeight);
    case BI_RLE8:
	/** read RLE8 compressed data **/
	break;
    default:
	fprintf(stderr, "unknown BMP compression type 0x%0lx\n"
		,biCompression);
	return -1;
    }
    x = y = 0;
    pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;

    while (y < biHeight) {
	if ((c = getc(fp)) == EOF)
	    return -1;
	if (c) {		/** encoded mode **/
	    c1 = getc(fp);
	    for (i = 0; i < c; i++, x++)
		*pic8++ = c1;
	} else {		/** c==0x00  :  escape codes **/
	    if ((c = getc(fp)) == EOF)
		return -1;
	    if (c == 0x00) {	/** end of line **/
		x = 0;
		y++;
		pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;
	    } else if (c == 0x01)
		break;		/** end of pic8 **/
	    else if (c == 0x02) {	/** delta **/
		c = getc(fp);
		x += c;
		c = getc(fp);
		y += c;
		pic8 = (char *) ImageData + x + (biHeight - y - 1) * biWidth;
	    } else {		/** absolute mode **/
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
	}			/** escape processing **/

    }				/** while **/

    return 0;
}

static int Load24(FILE * fp, int biWidth, int biHeight)
{
    int i, j, pad;
    char *pic24;

    /** # of pad bytes to read at EOscanline **/
    pad = (4 - ((biWidth * 3) % 4)) & 0x03;

    for (i = 0; i < biHeight; i++) {
	pic24 = (char *) ImageData + (biHeight - i - 1) * biWidth * 3;

	for (j = 0; j < biWidth; j++) {
	    pic24[2] = getc(fp);/** blue **/
	    pic24[1] = getc(fp);/** green **/
	    pic24[0] = getc(fp);/** red **/
	    pic24 += 3;
	}
	for (j = 0; j < pad; j++)
	    getc(fp);
    }
    return 0;
}


static unsigned long GetPixelFromRGB(struct palette * pal, XVisualInfo vinfo)
{
    u_long rmask, gmask, bmask;
    int rshift, gshift, bshift;
    unsigned long rv, gv, bv, pixel_value;

    /**
     *  ��ʬ������Τ���γФ���.....
     *
     * mask�ΰ��־�̤ΥӥåȤ����ӥå��ܤ���Ĵ�١�������u_char(8bit)��
     * �ͤκǾ�̥ӥåȤ��碌�뤿��˥��եȡ��Τ�,highbit(mask)-7����
     * ���եȤ��롣shift���ͤ��ޥ��ʥ��ˤʤ�ȡ��פ���ˤ���ǤϤ߽Ф�¿
     * ʬ�ϻȤ��ʤ��Τǵ��ˤ��������ͤ��������եȤ��롣�㤨��
     * bpp=15(16)���ȺǾ��1bit��Ĥ���rgb��5bit����Ϳ�����롣�Ĥޤꡢ
     * �ѥ�åȤ���8bit�Τ������5bit�������Ȥ��롣���λ�bmask = 31��
     * �ԥ������ͤκǲ���bit����5bit��blue�˻Ȥ��뤳�Ȥ�ʬ���롣bmask
     * �κǾ�̥ӥåȤ�4bit��(�ǲ��̤�0�Ȥ���)���Τ˥��ե��̤�4-7=-3,��
     * �ޤ걦��3bit���եȤ��뤳�Ȥˤʤ롣����ˤ�ä��������Ū�̤�ѥ��
     * �Ȥ�8bit����5bit���Ȥ��롣
     *
     * mask���ͤ�XVisualInfo������Ф���
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
     *�Ȥꤢ��������ͭ����˵��礤�Ǽ��������äƤߤ�
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
     * �ʤ�Ȥ��ᤤ���������Ƥ褦�Ȥ���
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

    /** ������ä���Allocate���줿�椫��ᤤ�Τ�õ�� **/
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


int LoadBmp(Widget xhw, GC * gc, Pixmap * pmap, char *fname, int *width, int *height, int ext_height, Boolean is_shape)
{
    /**
     * fname�ǻ��ꤵ�줿BMP�ե�������ɤ߹��ࡣ�����оݤˤʤ�
     * Widget,GC,Pixmap��ɬ�ס�width��height��BMP�ե����뤫��������졢
     * ���åȤ���롣ext_height��mask�򤫤���Ȥ���mask�ʳ�����ʬ�ι⤵��
     * �衼�����XHishoWidget����Ƥ֤Ȥ��˻��פ�ʬ�ι⤵��̵�뤹�뤿��
     * ��,���פι⤵����ꤹ����������̤�0�Ǥ����is_shape ��shape��
     * �뤫�ɤ�����0�ʳ���shape,
     *
     * �ɤ߹��ߤ�����������0,���Ԥ�����-�����֤���
     **/

    Display *d;
    Window w;
    Colormap cm;
    XImage *image;
    XVisualInfo vinfo, *vinfolist;
    Visual *vis;
    XWindowAttributes attr;
    FILE *fp;
    Window mask_win;
    XImage *mask_image;
    Pixmap mask;
    GC  mask_gc;
    Widget top;
    int i, j, depth, matched;
    int lwidth, colorsuu;
    unsigned char *pdata;
    unsigned long pixel_value[256];
    int cells;
    unsigned long trans_pix;
    struct palette *pal;

    d = XtDisplay(xhw);
    w = XtWindow(xhw);
    vis = DefaultVisual(d, 0);
    depth = DefaultDepth(d, 0);
    vinfo.screen = DefaultScreen(d);
    lwidth = *height = trans_pix = 0;
    pal = (struct palette *) malloc(sizeof(struct palette));

    image = XCreateImage(d, vis, depth,
			 ZPixmap, 0,
			 0, lwidth, *height, 32,
			 lwidth * 2);


    /**�⤷��ե����뤬�����ʤ��ä��齪λ**/
    if (NULL == (fp = fopen(fname, "r"))) {
	return -1;
    }
    if (ReadHeader(fp, width, height) < 0)
	return -1;

    switch (biBitCount) {
    case 1:
    case 4:
    case 8:
	ImageData = malloc((*width) * (*height));
	break;
    case 24:
	ImageData = malloc((*width) * (*height) * 3);
	break;
    }

    lwidth = *width;
    colorsuu = (biClrUsed) ? biClrUsed : 1 << biBitCount;

    pdata = malloc(lwidth * (*height));

    switch (biBitCount) {
    case 1:
	Load1(fp, *width, *height);
	break;
    case 4:
	Load4(fp, *width, *height);
	break;
    case 8:
	Load8(fp, *width, *height);
	break;
    case 24:
	Load24(fp, *width, *height);
	break;
    }

    /**
     * VisualInfo�����������Ŭ�ʤ�Τ����򤹤롣
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

    cells = vinfo.colormap_size;/** size of colormap **/

    *pmap = XCreatePixmap(d, w, lwidth, *height, depth);
    image = XGetImage(d, *pmap, 0, 0, lwidth, *height, AllPlanes, ZPixmap);

    if (is_shape) {
	top = xhw;
	while (XtParent(top))
	    top = XtParent(top);
	mask_win = XtWindow(top);
	mask = XCreatePixmap(d, w, lwidth, *height + ext_height, 1);
	mask_image = XGetImage(d, mask, 0, 0, lwidth, *height + ext_height, 1, ZPixmap);
    }
    switch (biBitCount) {
    case 1:
    case 4:
    case 8:
	if (vinfo.class == TrueColor) {
	    int i;

	    for (i = 0; i < colorsuu; i++)
		pixel_value[i] = GetPixelFromRGB(ImagePalette + i, vinfo);

	    for (; i < 256; i++)
		pixel_value[i] = 0L;

	    for (i = 0; i < *height; i++) {
		for (j = 0; j < lwidth; j++) {
		    XPutPixel(image, j, i, pixel_value[ImageData[i * lwidth + j]]);
		    if (is_shape) {
			if (ImageData[i * lwidth + j] == ImageData[0])
			    XPutPixel(mask_image, j, i, 0);
			else
			    XPutPixel(mask_image, j, i, 1);
		    }
		}
	    }

	    if (is_shape) {
		for (i = *height; i < *height + ext_height; i++)
		    for (j = 0; j < lwidth; j++)
			XPutPixel(mask_image, j, i, 1);
	    }
	} else {
	    /**
             * for 8-bit pseudo colors
             **/

	    cm = DefaultColormap(d, 0);

	    GetPseudoPixelFromRGB(d, cm, ImagePalette, pixel_value, colorsuu, cells);

	    /** change pixels to matched color index **/

	    for (i = 0; i < *height; i++)
		for (j = 0; j < lwidth; j++) {
		    XPutPixel(image, j, i, pixel_value[ImageData[i * lwidth + j]]);

		    if (is_shape) {
			if (ImageData[i * lwidth + j] == ImageData[0]) {
			    XPutPixel(mask_image, j, i, 0);
			} else {
			    XPutPixel(mask_image, j, i, 1);
			}
		    }
		}

	    if (is_shape) {
		for (i = *height; i < *height + ext_height; i++)
		    for (j = 0; j < lwidth; j++)
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

	    ptr = (char *) ImageData + (*height - i - 1) * lwidth * 3;

	    for (j = 0; j < lwidth; j++) {
		pal->r = (unsigned char) ptr[0];
		pal->g = (unsigned char) ptr[1];
		pal->b = (unsigned char) ptr[2];

		ptr += 3;
		pixel_value[0] = GetPixelFromRGB(pal, vinfo);

		if (i == 0 && j == 0)
		    trans_pix = pixel_value[0];

		if (is_shape) {
		    if (pixel_value[0] == trans_pix)
			XPutPixel(mask_image, j, *height - i, 0);
		    else
			XPutPixel(mask_image, j, *height - i, 1);
		}
		XPutPixel(image, j, *height - i - 1, pixel_value[0]);
	    }
	}
	if (is_shape) {
	    for (i = *height; i < *height + ext_height; i++)
		for (j = 0; j < lwidth; j++)
		    XPutPixel(mask_image, j, i, 1);
	}
	break;
    }

    XFreeGC(d, *gc);
    *gc = XCreateGC(d, *pmap, 0, NULL);

    XPutImage(d, *pmap, *gc, image, 0, 0, 0, 0, lwidth, *height);

    if (is_shape) {
	mask_gc = XCreateGC(d, mask, 0, NULL);
	XPutImage(d, mask, mask_gc, mask_image, 0, 0, 0, 0, lwidth, *height + ext_height);
	XShapeCombineMask(d, mask_win, ShapeBounding, 0, 0, mask, ShapeSet);
	XFreePixmap(d, mask);
	XFreeGC(d, mask_gc);
    }
    free(pdata);
    free(ImageData);
    free(ImagePalette);
    free(pal);

    return 0;
}
