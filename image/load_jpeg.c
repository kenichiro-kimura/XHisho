#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "image.h"

/**
 * function definition
 **/

static int JpegDecode(ImageInfo *, FILE *);
int LoadJpeg(ImageInfo *);

/**
 * local variable
 **/

static jmp_buf Error_jmp;
static int _Error = 0;

/**
 * JPEG error handler
 **/

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
  /*  my_error_ptr myerr = (my_error_ptr) cinfo->err;*/

  /**
   * _Errorを1にして,Error_jmpの場所に復帰。Error_jmpを設定したところでは
   * _Errorの値を見て判断する。
   **/
  _Error = 1;
  longjmp(Error_jmp, 1);
}


static int JpegDecode(ImageInfo * i_info, FILE * infile)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  int row_stride, cmap_entries, i;
  JSAMPARRAY buffer, colormap;

  _Error = 0;
  cinfo.err = jpeg_std_error((struct jpeg_error_mgr *) (&jerr));
  jerr.pub.error_exit = my_error_exit;

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);

  /**
   * エラーハンドラから帰ってきたら-１を返すようにする
   **/

  setjmp(Error_jmp);
  if (_Error == 1){
    jpeg_destroy_decompress(&cinfo);
    return -1;
  }

  jpeg_read_header(&cinfo, 1);
  jpeg_start_decompress(&cinfo);


  i_info->width = cinfo.output_width;
  i_info->height = cinfo.output_height;

  row_stride = cinfo.output_width * cinfo.output_components;

  if (cinfo.out_color_space == JCS_RGB) {
    if (cinfo.quantize_colors) {
      /**
       * Colormapped RGB
       **/
      i_info->BitCount = 8;
      cmap_entries = 256;
    } else {
      /**
       * Unquantized, full color RGB
       **/
      i_info->BitCount = 24;
      cmap_entries = 0;
    }
  } else {
    /**
     * Grayscale output.  We need to fake a 256-entry colormap.
     **/
    i_info->BitCount = 8;
    cmap_entries = 256;
  }

  if (cmap_entries) {
    i_info->ImagePalette = i_info->ImagePalette ?
      realloc(i_info->ImagePalette, sizeof(i_info->ImagePalette[0]) * cmap_entries)
      : malloc(sizeof(i_info->ImagePalette[0]) * cmap_entries);
    if (!i_info->ImagePalette) {
      fprintf(stderr, "jpeg:read palette failed.\n");
      jpeg_destroy_decompress(&cinfo);
      return -1;
    }
  }
  /**
   * read colormap from cinfo
   */

  colormap = cinfo.colormap;
  for (i = 0; i < cinfo.actual_number_of_colors; i++) {
    (i_info->ImagePalette + i)->b = GETJSAMPLE(colormap[2][i]);
    (i_info->ImagePalette + i)->g = GETJSAMPLE(colormap[1][i]);
    (i_info->ImagePalette + i)->r = GETJSAMPLE(colormap[0][i]);
  }

  buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

  if ((i_info->ImageData = malloc(row_stride * cinfo.output_height)) == NULL) {
    fprintf(stderr, "can't alloc image buffer\n");
    jpeg_destroy_decompress(&cinfo);
    return -1;
  } else {
    char *b_ptr = i_info->ImageData;

    while (cinfo.output_scanline < cinfo.output_height) {
      jpeg_read_scanlines(&cinfo, buffer, 1);

      memcpy(b_ptr, buffer[0], row_stride);
      b_ptr += row_stride;
    }
  }

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  return 0;
}

int LoadJpeg(ImageInfo * i_info)
{
  FILE *fp;
  int r;

  if (NULL == (fp = fopen(i_info->filename, "r"))) {
    return -1;
  }
  r = JpegDecode(i_info, fp);

  fclose(fp);
  return r;
}
