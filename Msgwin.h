/**
 * ふきだし型Form Widget 「Msgwin Widget」 パブリックヘッダファイル
 * 
 * copyright(c) 1998-2001   Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 * 
 **/

#ifndef _MSGWIN_H
#define _MSGWIN_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>

#ifdef Xaw3D
#include <X11/Xaw3d/Form.h>
#else
#include <X11/Xaw/Form.h>
#endif

extern WidgetClass msgwinWidgetClass;

typedef struct _MsgwinClassRec *MsgwinWidgetClass;
typedef struct _MsgwinRec *MsgwinWidget;

#define XtNwindowMode "windowMode"
#define XtCWindowMode "WindowMode"
#define XtNframeMode "frameMode"
#define XtCFrameMode "FrameMode"

enum {
  /**
   * いくつかの定数定義をenumで書き直したが、XtNxxの定義などはXの流儀 
   * にしたがってそのまま。
   * 
   **/

  LABEL_OFFSET = 20,
  POINT_WIDTH = 30,
  FONT_OFFSET = 2,
  YPOS_OFFSET = 0
};

#endif
