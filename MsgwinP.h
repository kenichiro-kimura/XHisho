/**
 * ふきだし型Form Widget 「Msgwin Widget」 プライベートヘッダファイル
 * 
 * copyright(c) 1998-2019   Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 * 
 **/

#ifndef _MSGWINP_H
#define _MSGWINP_H

#include "Msgwin.h"

#include <X11/Xaw/FormP.h>

typedef struct _MsgwinClassPart {
  int dummy;
}   MsgwinClassPart;

typedef struct _MsgwinClassRec {
  CoreClassPart core_class;
  CompositeClassPart composite_class;
  ConstraintClassPart constraint_class;
  FormClassPart form_class;
  MsgwinClassPart msgwin_class;
}   MsgwinClassRec;

extern MsgwinClassRec msgwinClassRec;

typedef struct _MsgwinPart {
  /* Private data */
  GC  gc;
  XArc mask_arc[4];
  XRectangle mask_rect[4];
  XPoint mask_point[4];
  Boolean is_shaped;

  /* resource data */
  int WindowMode;
  int FrameMode;
  int yoff;
  int xoff;
  Boolean force_m;
}   MsgwinPart;

typedef struct _MsgwinRec {
  CorePart core;
  CompositePart composite;
  ConstraintPart constraint;
  FormPart form;
  MsgwinPart msgwin;
}   MsgwinRec;

typedef struct _MsgwinConstraintsPart {
  int dummy;
}   MsgwinConstraintsPart;

typedef struct _MsgwinConstraintsRec {
  FormConstraintsPart form;
  MsgwinConstraintsPart msgwin;
}   MsgwinConstraintsRec, *MsgwinConstraints;

#endif
