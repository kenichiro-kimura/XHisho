/**
 * 画像張り付け & 時計表示 Widget 「XHisho Widget」 プライベートヘッダファイル
 * 
 * copyright(c) 1998-2001  Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 **/

#ifndef _XHISHOP_H
#define _XHISHOP_H

#include "XHisho.h"
#include "image.h"

#include <X11/Xaw/LabelP.h>
#include <X11/Xmu/Converters.h>

typedef struct _XHishoClassPart {
  int dummy;
}   XHishoClassPart;

typedef struct _XHishoClassRec {
  CoreClassPart core_class;
  SimpleClassPart simple_class;
  LabelClassPart label_class;
  XHishoClassPart xhisho_class;
}   XHishoClassRec;

extern XHishoClassRec xHishoClassRec;

typedef struct _XHishoPart {
  Display *d;
  Window w;
  String cg_file;
  String m_file;
  String ext_filter;
  String clock_text;
  String petname_f;
  XtIntervalId intervalId;
  XtIntervalId focus_intervalId;
  String clock_f;
  String clock_arg;
  Boolean c_draw;
  Boolean focuswin;
  Boolean is_shape;
  Boolean adjust;
  int yoff;
  XtJustify just;
  String ext_soundcommand;
  int focus_interval;
  String ext_editcommand;
  int f_cg_number;
  int uf_cg_number;
  int ucg_off;
  Boolean force_cg;
  Boolean use_unyuu;

  /* private data */
  ImageInfo* i_info;
  Window focus;
  int old_x, old_y;
  int cg_number;
  int cg_sec;
  int anim_type;
  int ucg_number;
  Pixmap mask_p;
  Pixmap u_p;
  GC mask_gc;
}   XHishoPart;

typedef struct _XHishoRec {
  CorePart core;
  SimplePart simple;
  LabelPart label;
  XHishoPart xhisho;
}   XHishoRec;

#endif
