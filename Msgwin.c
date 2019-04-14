/**
 * ふきだし型Form Widget 「Msgwin Widget」 Widget本体処理ファイル
 *
 * copyright(c) 1998-2019   Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 *
 * Msgwinでは、From Widgetをふきだしの形にshapeして表示する。親になるウ
 * インドの位置によって画面を4分割し、それをWindowModeとする。Modeによっ
 * てふきだしの位置(三角形の部分)の位置が変化する。WindowModeは外部から
 * SetValueすることもできるが、PopupやRealizeでRedrawがかかるときに自動
 * 的に再計算するのでおそらく意味がない。意味があるとすれば,「再計算のト
 * リガー」として使うことであろう。SetValue時に呼ばれる関数の中で必ず
 * WindowModeの再計算を行う。故に,何らかのタイミングでふきだしの形が思っ
 * たものにならない場合はSetValueしてみるのも有効であろう。また、
 * PopupShellとして使っていて、しょっちゅうmodeが変わるような使い方をす
 * る(例えばXMaidのように親ウインドウが動きまわるなど)場合、一瞬ウインド
 * ウのPopup位置がずれて見えるかも知れない。その時は,Popupの前にModeを
 * SetValueしてからPopupしてやるとよい。出来ればこれもどうにかしたいのだ
 * が。
 *
 * リソース forceMode をTrueにすると、WindowModeの自動設定が効かなくなる。
 **/

#define ARC_WIDTH (int)(mask_height / 10)

#include <stdlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/extensions/shape.h>

#include "MsgwinP.h"

#ifdef Xaw3D
#include <X11/Xaw3d/XawInit.h>
#else
#include <X11/Xaw/XawInit.h>
#endif

static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Redraw(Widget, XEvent *, Region);
static void ShapeWindow(MsgwinWidget);
static void DrawFrame(MsgwinWidget);
static void Destroy(Widget);
static void ClassInit();
static void ClassPartInit();
static void ManageChild(Widget);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static Boolean SetConstValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void ChangeManaged(Widget);
static XtGeometryResult GeometryManager();
static void ConstraintInitialize(Widget, Widget, ArgList, Cardinal *);
static void SetWindowLocate(MsgwinWidget);
static XtGeometryResult GeometryManager();
static void SwapFrameInfo(Dimension *, Dimension *);

static CompositeClassExtensionRec extension_rec = {
  NULL,				/** next extension **/
  NULLQUARK,			/** record type **/
  XtCompositeExtensionVersion,	/** version **/
  sizeof(CompositeClassExtensionRec),	/** record size **/
  TRUE,				/** accepts objects **/
};

static ConstraintClassExtensionRec constraints_rec = {
  NULL,
  NULLQUARK,
  XtConstraintExtensionVersion,
  sizeof(ConstraintClassExtensionRec),
  NULL,
};

static XtResource resources[] = {
  {
    XtNwindowMode,
    XtCWindowMode,
    XtRInt,
    sizeof(int),
    XtOffset(MsgwinWidget, msgwin.WindowMode),
    XtRImmediate,
    0,
  },
  {
    XtNframeMode,
    XtCFrameMode,
    XtRInt,
    sizeof(int),
    XtOffset(MsgwinWidget, msgwin.FrameMode),
    XtRImmediate,
    0,
  },
  {
    XtNyoff,
    XtCYoff,
    XtRInt,
    sizeof(int),
    XtOffset(MsgwinWidget, msgwin.yoff),
    XtRImmediate,
    0,
  },
  {
    XtNxoff,
    XtCXoff,
    XtRInt,
    sizeof(int),
    XtOffset(MsgwinWidget, msgwin.xoff),
    XtRImmediate,
    0,
  },
  {
    XtNforceMode,
    XtCForceMode,
    XtRBoolean,
    sizeof(Boolean),
    XtOffset(MsgwinWidget, msgwin.force_m),
    XtRImmediate,
    False,
  },
};


MsgwinClassRec msgwinClassRec = {
  /**
   * Core Class
   **/
  {
    (WidgetClass) (&formClassRec),	/** superclass **/
    "Msgwin",			/** class_name **/
    sizeof(MsgwinRec),		/** size **/
    ClassInit,			/** class_initialize **/
    ClassPartInit,		/** class_part_initialize **/
    FALSE,			/** class_inited **/
    (XtInitProc) Initialize,	/** initialize **/
    NULL,			/** initialize_hook **/
    (XtRealizeProc) Realize,	/** realize **/
    NULL,			/** actions **/
    0,				/** num_actions **/
    resources,			/** resources **/
    XtNumber(resources),	/** num_resources **/
    NULLQUARK,			/** xrm_class **/
    TRUE,			/** compress_motion **/
    TRUE,			/** compress_exposure **/
    TRUE,			/** compress_enterleave **/
    TRUE,			/** visible_interest **/
    (XtWidgetProc) Destroy,	/** destroy **/
    (XtWidgetProc) ManageChild,	/** resize **/
    (XtExposeProc) Redraw,	/** expose **/
    SetValues,			/** set_values **/
    NULL,			/** set_values_hook **/
    XtInheritSetValuesAlmost,	/** set_values_almost **/
    NULL,			/** get_values_hook **/
    NULL,			/** accept_focus **/
    XtVersion,			/** version **/
    NULL,			/** callback_private **/
    NULL,			/** tm_table **/
    XtInheritQueryGeometry,	/** query_geometry **/
    NULL,			/** display_accelerator **/
    NULL,			/** extension **/
  },
  /**
   * CompositePart
   **/
  {
    GeometryManager,		/** geometry_manager   **/
    ChangeManaged,		/** change_managed     **/
    XtInheritInsertChild,	/** insert_child   **/
    XtInheritDeleteChild,	/** delete_child   **/
    NULL,			/** extension      **/
  },
  /**
   * ConstraintPart
   **/
  {
    NULL,
    0,
    sizeof(MsgwinConstraintsRec),
    ConstraintInitialize,
    NULL,
    SetConstValues,
    NULL,
  },
  /**
   * FormPart
   **/
  {
    XtInheritLayout,
  },
  /**
   * MsgwinPart
   **/
  {
    (intptr_t) NULL,
  },
};


WidgetClass msgwinWidgetClass = (WidgetClass) & msgwinClassRec;


static void Initialize(Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  MsgwinWidget msw = (MsgwinWidget) new;

  msw->msgwin.gc = XCreateGC(XtDisplay(msw), RootWindowOfScreen(XtScreen(msw)), (unsigned long) NULL, NULL);
  msw->msgwin.is_shaped = FALSE;
}

static void Realize(Widget w, XtValueMask * valueMask, XSetWindowAttributes * attrs)
{
  MsgwinWidget msw = (MsgwinWidget) w;

  (formClassRec.core_class.realize) (w, valueMask, attrs);

  ManageChild((Widget) msw);

  if (msw->msgwin.is_shaped == FALSE)
    ShapeWindow(msw);
  DrawFrame(msw);
  SetWindowLocate(msw);

  XtCreateWindow(w, (unsigned) InputOutput, (Visual *) CopyFromParent, *valueMask, attrs);

}


static void ShapeWindow(MsgwinWidget msw)
{
  /**
   * arc[],rect[]の引数と実際の位置の対応
   *
   * 0----------------3  <- ここの円の幅がARC_WIDTH
   * |                |
   * |                |
   * 1----------------2
   *
   * point[]の引数と実際の位置の対応
   *
   *     0
   *    /|
   *   / |
   *  1  |
   *   \ |
   *    \|
   *     2
   * |<->| この幅がPOINT_WIDTH
   *
   *     ARC,POINTいずれもMsgwin.hで定義した定数
   *
   *     WindowMode は
   *
   *     0   |   1
   *    -----+------
   *     2   |   3
   *
   *          と画面を4分割する
   **/

  int i;
  Dimension mask_width, mask_height;
  GC  mask_gc;
  Pixmap window_mask;
  Display *d;
  Window w;
  int WindowMode, FrameMode;
  XArc mask_arc[4];
  XRectangle mask_rect[4];	/** 2,3 is dummy **/
  XPoint mask_point[4];		/** 3 is dummy **/

  d = XtDisplay(msw);
  w = XtWindow(msw);
  WindowMode = msw->msgwin.WindowMode;
  FrameMode = msw->msgwin.FrameMode;

  /**
   * maskを作るためにウインドウの大きさを取得
   **/

  mask_width = msw->core.width;
  mask_height = msw->core.height;

  /**
   * maskの生成
   **/

  for (i = 0; i < 2; i++) {
    mask_rect[i].x = (!i) * (int) (ARC_WIDTH / 2) + POINT_WIDTH;
    mask_rect[i].y = i * (int) (ARC_WIDTH / 2);
    mask_rect[i].width = mask_width - (POINT_WIDTH * 2) + 1;
    mask_rect[i].height = mask_height - i * (ARC_WIDTH - 1);
  }

  mask_rect[0].width -= ARC_WIDTH;

  for (i = 0; i < 4; i++) {
    mask_arc[i].width = ARC_WIDTH;
    mask_arc[i].height = ARC_WIDTH;
    mask_arc[i].angle2 = 90 * 64;
  }

  mask_point[0].x = (WindowMode == 1 || WindowMode == 3) ?
    mask_rect[1].x + mask_rect[1].width : mask_rect[1].x;
  mask_point[0].y = (WindowMode == 2 || WindowMode == 3) ?
    mask_height - ARC_WIDTH : ARC_WIDTH;
  mask_point[1].x = (WindowMode == 1 || WindowMode == 3) ? mask_width : 0;
  mask_point[1].y = (WindowMode == 2 || WindowMode == 3) ?
    mask_height - (int) (ARC_WIDTH / 2) - ARC_WIDTH :
    (int) (ARC_WIDTH / 2) + ARC_WIDTH;
  mask_point[2].x = mask_point[0].x;
  mask_point[2].y = (WindowMode == 2 || WindowMode == 3) ?
    mask_height - ARC_WIDTH - ARC_WIDTH : ARC_WIDTH + ARC_WIDTH;

  mask_arc[0].x = POINT_WIDTH;
  mask_arc[0].y = 0;
  mask_arc[0].angle1 = 90 * 64;

  mask_arc[1].x = POINT_WIDTH;
  mask_arc[1].y = mask_height - ARC_WIDTH - 1;
  mask_arc[1].angle1 = 180 * 64;

  mask_arc[2].x = mask_width - POINT_WIDTH - ARC_WIDTH;
  mask_arc[2].y = mask_height - ARC_WIDTH - 1;
  mask_arc[2].angle1 = 270 * 64;

  mask_arc[3].x = mask_width - POINT_WIDTH - ARC_WIDTH;
  mask_arc[3].y = 0;
  mask_arc[3].angle1 = 0 * 64;


  window_mask = XCreatePixmap(d, w
			      ,mask_width + POINT_WIDTH * 2, mask_height, 1);
  mask_gc = XCreateGC(d, window_mask, 0, NULL);

  XSetGraphicsExposures(d, mask_gc, FALSE);

  /**
   *とりあえずマスクする領域をクリアするために黒で塗り潰す
   **/

  XSetForeground(d, mask_gc, 0);

  XFillRectangle(d, window_mask, mask_gc, 0, 0, mask_width + POINT_WIDTH * 2
		 ,mask_height);

  /**
   * FrameMode によって マスクの形を変える
   **/

  switch (FrameMode) {
  case 1:
    SwapFrameInfo(&mask_point[0].y, &mask_point[1].y);
    break;
  case 2:
    SwapFrameInfo(&mask_point[1].y, &mask_point[2].y);
    break;
  default:
    break;
  }

  /**
   * マスクを白で描画する
   **/

  XSetForeground(d, mask_gc, 1);

  XFillRectangles(d, window_mask, mask_gc, mask_rect, 2);
  XFillArcs(d, window_mask, mask_gc, mask_arc, 4);

  XSetFillRule(d, mask_gc, WindingRule);
  XFillPolygon(d, window_mask, mask_gc
	       ,mask_point, 3, Complex, CoordModeOrigin);

  XShapeCombineMask(d, XtWindow(XtParent(msw)), ShapeBounding
		    ,0, 0, window_mask, ShapeSet);
  XFreePixmap(d, window_mask);
  XFreeGC(d, mask_gc);
  XFlush(d);

  msw->msgwin.is_shaped = TRUE;

  /**
   * DrawFrameでこのマスクの形を使うので、mswに格納しておく
   **/

  for (i = 0; i < 4; i++) {
    msw->msgwin.mask_arc[i] = mask_arc[i];
    msw->msgwin.mask_rect[i] = mask_rect[i];
    msw->msgwin.mask_point[i] = mask_point[i];
  }
}

static void Redraw(Widget w, XEvent * event, Region region)
{
  MsgwinWidget msw;

  msw = (MsgwinWidget) w;

  /**
   * Redraw 毎にWindowModeをチェック
   **/

  /*  if(!msw->msgwin.force_m)*/
  SetWindowLocate(msw);

  /**
   * WindowModeが変っていたりすればShapeしなおす
   **/

  if (msw->msgwin.is_shaped == FALSE)
    ShapeWindow(msw);
  DrawFrame(msw);
}

static void DrawFrame(MsgwinWidget msw)
{
  GC  gc;
  Display *d;
  Window w;
  XArc mask_arc[4];
  XRectangle mask_rect[4];
  XPoint mask_point[4];
  int i;

  d = XtDisplay(msw);
  w = XtWindow(msw);
  gc = XCreateGC(d, w, 0, 0);

  /**
   * mswに待避しておいたマスクを取り出す。直接使ってもいいが、ここで値を
   * いじる可能性もあるので、とりあえずlocal valueとして取り出してそれを使う
   **/

  for (i = 0; i < 4; i++) {
    mask_arc[i] = msw->msgwin.mask_arc[i];
    mask_rect[i] = msw->msgwin.mask_rect[i];
    mask_point[i] = msw->msgwin.mask_point[i];
  }


  /**
   * 枠を描写
   **/

  XSetForeground(d, gc, BlackPixel(d, 0));
  XSetLineAttributes(d, gc, 2, LineSolid, CapButt, JoinMiter);

  XDrawLine(d, w, gc, mask_rect[1].x, mask_rect[1].y, mask_rect[1].x
	    ,mask_rect[1].y + mask_rect[1].height);
  XDrawLine(d, w, gc, mask_rect[1].x + mask_rect[1].width, mask_rect[1].y
	    ,mask_rect[1].x + mask_rect[1].width
	    ,mask_rect[1].y + mask_rect[1].height);
  XDrawLine(d, w, gc, mask_rect[0].x, mask_rect[0].y
	    ,mask_rect[0].x + mask_rect[0].width, mask_rect[0].y);

  XDrawLine(d, w, gc, mask_rect[0].x
	    , mask_rect[0].y + mask_rect[0].height
	    ,mask_rect[0].x + mask_rect[0].width
	    , mask_rect[0].y + mask_rect[0].height);

  XDrawArcs(d, w, gc, mask_arc, 4);

  XDrawLines(d, w, gc, mask_point, 3, CoordModeOrigin);

  XSetForeground(d, gc, msw->core.background_pixel);
  XDrawLine(d, w, gc, mask_point[0].x, mask_point[0].y, mask_point[2].x, mask_point[2].y);
  XFreeGC(d, gc);
}

static void Destroy(Widget w)
{
  MsgwinWidget msw = (MsgwinWidget) w;

  XFreeGC(XtDisplay(w), msw->msgwin.gc);
}

static void ClassInit()
{
  XawInitializeWidgetSet();
}

static void ClassPartInit(WidgetClass wc)
{
  MsgwinWidgetClass msw = (MsgwinWidgetClass) wc;

  extension_rec.next_extension = msw->composite_class.extension;
  constraints_rec.next_extension = msw->constraint_class.extension;
  msw->composite_class.extension = (XtPointer) (&extension_rec);
  msw->constraint_class.extension = (XtPointer) (&constraints_rec);
}


static Boolean SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  Boolean ret;
  MsgwinWidget mswold = (MsgwinWidget) current;
  MsgwinWidget mswnew = (MsgwinWidget) new;

  /**
   * もしWindowModeが変わっていたら枠の形が変わるので、再描写するために
   * is_shapedをFALSEにする
   **/

  SetWindowLocate(mswnew);
  if(mswnew->msgwin.force_m)
    mswnew->msgwin.WindowMode =  mswold->msgwin.WindowMode;

  if (mswold->msgwin.WindowMode != mswnew->msgwin.WindowMode) {
    mswnew->msgwin.is_shaped = FALSE;
  }

  ret = (formClassRec.core_class.set_values) (current, request, new, args, num_args);

  if (XtIsRealized((Widget) mswnew)) {
    if (mswnew->msgwin.is_shaped == FALSE)
      ShapeWindow(mswnew);
    DrawFrame(mswnew);
  }
  return ret;

}

static void ChangeManaged(Widget w)
{
  (formClassRec.composite_class.change_managed) (w);
}

static XtGeometryResult GeometryManager(Widget w, XtWidgetGeometry * request, XtWidgetGeometry * reply)
{
  XtGeometryResult ret;

  ret = (formClassRec.composite_class.geometry_manager) (w, request, reply);
  return ret;
}

static void ConstraintInitialize(Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  MsgwinConstraints constraint = (MsgwinConstraints) new->core.constraints;

  constraint->form.left = XtChainLeft;
  constraint->form.right = XtChainLeft;
}

static Boolean SetConstValues(Widget current, Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  Boolean ret;

  ret = (formClassRec.core_class.set_values) (current, request, new, args, num_args);

  return ret;
}

static void SetWindowLocate(MsgwinWidget msw)
{
  int WindowMode, Old;
  Dimension get_x, get_y, main_width, main_height, new_x, new_y;
  int main_x, main_y;
  Widget top = (Widget) msw;

  Old = msw->msgwin.WindowMode;

  while (XtParent(top))
    top = XtParent(top);

  XtVaGetValues(top, XtNx, &get_x, XtNy, &get_y, XtNwidth, &main_width, XtNheight, &main_height, NULL);

  main_x = get_x;
  main_y = get_y;

  /**
   * 左上隅が画面外にあるときの対処
   **/

  if (main_x > DisplayWidth(XtDisplay(top), 0))
    main_x -= (Dimension) (-1);
  if (main_y > DisplayHeight(XtDisplay(top), 0))
    main_y -= (Dimension) (-1);

  WindowMode = (main_x > DisplayWidth(XtDisplay(top), 0) - main_x - main_width) ? 1 : 0;
  WindowMode = (main_y > DisplayHeight(XtDisplay(top), 0) - main_y - main_height) ?
    WindowMode + 2 : WindowMode;

  new_x = (WindowMode == 1 || WindowMode == 3) ?
    main_x - (msw->core.width) : main_x + main_width;

  new_y = (WindowMode == 0 || WindowMode == 1) ?
    main_y + (int) (main_height / 2) - (int) ((msw->core.height) / 10) :
    main_y + (int) (main_height / 2) - 8 * (int) ((msw->core.height) / 10);

  new_y += YPOS_OFFSET;

  (XtParent(msw))->core.width = msw->core.width;
  (XtParent(msw))->core.height = msw->core.height;

  switch(WindowMode){
  case 0:
    new_x += msw->msgwin.xoff;
    new_y += msw->msgwin.yoff;
    break;
  case 1:
    new_x -= msw->msgwin.xoff;
    new_y += msw->msgwin.yoff;
    break;
  case 2:
    new_x += msw->msgwin.xoff;
    new_y -= msw->msgwin.yoff;
    break;
  case 3:
    new_x -= msw->msgwin.xoff;
    new_y -= msw->msgwin.yoff;
    break;
  }
  XtMoveWidget(XtParent(msw), new_x, new_y);

  if (Old != WindowMode)
    msw->msgwin.is_shaped = FALSE;


  if(!msw->msgwin.force_m)
    msw->msgwin.WindowMode = WindowMode;
}

static void ManageChild(Widget parent)
{
  int Longest_width, Form_height, i, ypos, b_ypos, tmp_width, b_height,
      b_width, tmp_height;
  int same_line;
  Widget child, b_child;
  MsgwinWidget msw = (MsgwinWidget) parent;
  MsgwinConstraints mct;

  Longest_width = Form_height = b_ypos = tmp_width = b_width = tmp_height = 0;
  same_line = 0;
  ypos = XtParent(msw)->core.border_width + 5;
  Form_height += 5;

  child = (Widget) NULL;
  b_child = (Widget) NULL;
  mct = (MsgwinConstraints) NULL;

  for (i = 0; i < msw->composite.num_children; i++) {
    child = msw->composite.children[i];
    mct = (MsgwinConstraints) child->core.constraints;

    if (i == 0) {
      Longest_width = tmp_width = child->core.width + mct->form.dx;
      tmp_height = child->core.height + child->core.border_width * 2 + mct->form.dy;
    } else {
      b_child = msw->composite.children[i - 1];
      if (b_child->core.y == child->core.y) {
	same_line++;
	tmp_width += child->core.width + mct->form.dx;
	tmp_height = (tmp_height > child->core.height +
		 child->core.border_width * 2 + mct->form.dy) ? tmp_height :
	  child->core.height + mct->form.dy + child->core.border_width * 2;
      } else {
	if (tmp_height == 0) {
	  tmp_height = b_child->core.height;
	  tmp_height += b_child->core.border_width * 2;
	  tmp_height += ((MsgwinConstraints) (b_child->core.constraints))->form.dy;
	}
	Form_height += tmp_height;

	Longest_width = (Longest_width > tmp_width) ? Longest_width : tmp_width;
	tmp_width = child->core.width + child->core.x;
	tmp_height = same_line = 0;
      }
    }
  }

  Form_height += child->core.height + child->core.border_width * 2 + mct->form.dy;
  Longest_width = (Longest_width > tmp_width) ? Longest_width : tmp_width;

  XtResizeWidget(XtParent(msw)
		 ,Longest_width + (LABEL_OFFSET + POINT_WIDTH)
		 ,Form_height + 20, 0);
  XtResizeWidget((Widget)msw
		 ,Longest_width + (LABEL_OFFSET + POINT_WIDTH)
		 ,Form_height + 20, 0);
  b_ypos = b_height = 0;

  for (i = 0; i < msw->composite.num_children; i++) {
    child = msw->composite.children[i];
    mct = (MsgwinConstraints) child->core.constraints;

    if (child->core.managed) {
      if (b_ypos != child->core.y) {
	ypos += b_height + mct->form.dy;
	b_ypos = child->core.y;
	XtMoveWidget(child, child->core.x, ypos);
	b_height = child->core.height + child->core.border_width * 2;
      } else {
	b_ypos = child->core.y;
	XtMoveWidget(child, child->core.x, ypos);
	b_height = (b_height > child->core.height + child->core.border_width * 2) ?
	  b_height : child->core.height + child->core.border_width * 2;
      }
    }
  }
  if(!msw->msgwin.force_m)
    SetWindowLocate(msw);
}

static void SwapFrameInfo(Dimension * a, Dimension * b)
{
  Dimension tmp;

  tmp = *a;
  *a = *b;
  *b = tmp;
}
