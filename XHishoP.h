/*
 * 画像張り付け & 時計表示 Widget 「XHisho Widget」 プライベートヘッダファイル
 *
 * copyright(c) 1998,1999  Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 */

#ifndef _XHISHOP_H
#define _XHISHOP_H

#include "XHisho.h"

#include <X11/Xaw/LabelP.h>
#include <X11/Xmu/Converters.h>

typedef struct _XHishoClassPart{
  int dummy;
} XHishoClassPart;

typedef struct _XHishoClassRec{
  CoreClassPart	core_class;
  SimpleClassPart	simple_class;
  LabelClassPart	label_class;
  XHishoClassPart   xhisho_class;
} XHishoClassRec;

extern XHishoClassRec xHishoClassRec;

typedef struct _XHishoPart{
  String cg_file;
  String m_file;  
  String ext_filter;
  String clock_text;
  String petname_f;
  XtIntervalId intervalId;
  String clock_f;
  String clock_arg;
  Boolean c_draw;
  Boolean focuswin;
  int yoff;
  XtJustify just;
  Boolean is_shape;
  String ext_soundcommand;

  /* private data */
  Display* d;
  Window w;
  GC gc;
  int width,height;
  Pixmap pixmap;
  Pixmap pmask;
  Window focus;
  int old_x,old_y;
} XHishoPart;

typedef struct _XHishoRec{
  CorePart	core;
  SimplePart	simple;
  LabelPart	label;
  XHishoPart xhisho;
} XHishoRec;

#endif
