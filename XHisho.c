/**
 * 画像張り付け & 時計表示 Widget 「XHisho Widget」 Widget本体処理ファイル
 *
 * copyright(c) 1998,1999  Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 *
 * 指定された(XtNcgFile)xpmデータを張り付ける。その下にxpmと同じ幅で指
 * 定された(XtNfontSet)フォントと同じ高さの余白を作り、そこに時計を表
 * 示する。時計の表示フォーマットはリソースXtNclockFormatで指定できる。
 * また、リソースXtNfocusWinをTrueにすると、フォーカスの当っているウイ
 * ンドウのタイトルに「おすわり」するようになる。
 **/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/extensions/shape.h>
#include <X11/Xaw/XawInit.h>

#include "XHishoP.h"
#include "config.h"

#define DISPLAY (xhw->xhisho.d)
#define WINDOW  (xhw->xhisho.w)
#define XH_GC      (xhw->xhisho.i_info->gc)
#define BCG     (xhw->xhisho.cg_file)
#define WIDTH  (xhw->xhisho.i_info->width)
#define HEIGHT  (xhw->xhisho.i_info->height)
#define PIXMAP(n)  (((xhw->xhisho.i_info->image) + n)->pixmap)
#define CG_NUM (xhw->xhisho.cg_number)
#define FRAME_WIDTH 1

static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Redraw(Widget, XEvent *, Region);
static void ClockDraw(XHishoWidget);
static void MoveFocusWindow(XHishoWidget);
static void Destroy(Widget);
static void ClassInit();
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void NewInterval(XHishoWidget);
static void FocusInterval(XHishoWidget);
static void SetSize(XHishoWidget);
static void Animation(XHishoWidget);
static void ChangeAnimType(XHishoWidget);

static XtResource resources[] = {
  {
    XtNcgFile,
    XtCCgFile,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.cg_file),
    XtRImmediate,
    CGFILE,
  },
  {
    XtNmessageFile,
    XtCMessageFile,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.m_file),
    XtRImmediate,
    RCFILE,
  },
  {
    XtNextFilter,
    XtCExtFilter,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.ext_filter),
    XtRImmediate,
    (XtPointer) FILTER
  },
  {
    XtNextSoundCommand,
    XtCExtSoundCommand,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.ext_soundcommand),
    XtRImmediate,
    (XtPointer) NULL
  },
  {
    XtNpetnameFile,
    XtCPetnameFile,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.petname_f),
    XtRImmediate,
    (XtPointer) PETNAME_F
  },
  {
    XtNclockText,
    XtCClockText,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.clock_text),
    XtRImmediate,
    "00:00:00",
  },
  {
    XtNclockFormat,
    XtCClockFormat,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.clock_f),
    XtRImmediate,
    "%s:%s:%s"
  },
  {
    XtNclockArg,
    XtCClockArg,
    XtRString,
    sizeof(String),
    XtOffset(XHishoWidget, xhisho.clock_arg),
    XtRImmediate,
    "hMs"
  },
  {
    XtNdrawClock,
    XtCDrawClock,
    XtRBoolean,
    sizeof(Boolean),
    XtOffset(XHishoWidget, xhisho.c_draw),
    XtRImmediate,
    (XtPointer) True
  },
  {
    XtNfocusWin,
    XtCFocusWin,
    XtRBoolean,
    sizeof(Boolean),
    XtOffset(XHishoWidget, xhisho.focuswin),
    XtRImmediate,
    (XtPointer) False
  },
  {
    XtNfocusYoff,
    XtCFocusYoff,
    XtRInt,
    sizeof(int),
    XtOffset(XHishoWidget, xhisho.yoff),
    XtRImmediate,
    (XtPointer) 0
  },
  {
    XtNjustify,
    XtCJustify,
    XtRJustify,
    sizeof(XtJustify),
    XtOffset(XHishoWidget, xhisho.just),
    XtRImmediate,
    (XtPointer) XtJustifyLeft
  },
  {
    XtNisShape,
    XtCIsShape,
    XtRBoolean,
    sizeof(Boolean),
    XtOffset(XHishoWidget, xhisho.is_shape),
    XtRImmediate,
    (XtPointer) False
  },
  {
    XtNfocusWinInterval,
    XtCFocusWinInterval,
    XtRInt,
    sizeof(int),
    XtOffset(XHishoWidget, xhisho.focus_interval),
    XtRImmediate,
    (XtPointer) 1
  },
  {
    XtNanimType,
    XtCAnimType,
    XtRInt,
    sizeof(int),
    XtOffset(XHishoWidget, xhisho.anim_type),
    XtRImmediate,
    (XtPointer) 0
  },
};


XHishoClassRec xHishoClassRec = {
  /**
   * Core Class
   **/
  {
    (WidgetClass) (&labelClassRec),	/** superclass **/
    "XHisho",			/** class_name **/
    sizeof(XHishoRec),		/** size **/
    ClassInit,			/** class_initialize **/
    NULL,			/** class_part_initialize **/
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
    XtInheritResize,		/** resize **/
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
   * SimplePart
   **/
  {
    XtInheritChangeSensitive,
  },
  /**
   * LabelPart
   **/
  {
    (int) NULL,
  },
  /**
   * XHishoPart
   **/
  {
    (int) NULL,
  },
};


WidgetClass xHishoWidgetClass = (WidgetClass) & xHishoClassRec;

static void Initialize(Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  int i;
  XHishoWidget xhw = (XHishoWidget) new;

  xhw->xhisho.i_info = (ImageInfo*)malloc(sizeof(ImageInfo));
  WIDTH = HEIGHT = 0;

  XH_GC = XCreateGC(XtDisplay(xhw), RootWindowOfScreen(XtScreen(xhw)), (unsigned long) NULL, NULL);

  xhw->xhisho.intervalId = 0;
  CG_NUM = 0;

  for(i = 0; i < NUM_OF_ARRAY(xhw->xhisho.i_info->anim_number);i++)
    xhw->xhisho.i_info->anim_number[i] = -1;

}

static void Realize(Widget w, XtValueMask * valueMask, XSetWindowAttributes * attrs)
{
  XHishoWidget xhw = (XHishoWidget) w;
  int clock_height;

  (labelClassRec.core_class.realize) (w, valueMask, attrs);

  XtCreateWindow(w, (unsigned) InputOutput, (Visual *) CopyFromParent, *valueMask, attrs);

  DISPLAY = XtDisplay(w);
  WINDOW = XtWindow(w);

  if (!xhw->xhisho.c_draw) {
    xhw->label.label_height = 0;
    clock_height = 0;
  } else {
    clock_height = xhw->label.label_height + 4;
  }

  /**
   * ImageInfoに必要なデータを与える
   **/

  xhw->xhisho.i_info->d = XtDisplay(XtParent(w));
  xhw->xhisho.i_info->w = XtWindow(XtParent(w));
  xhw->xhisho.i_info->filename = malloc(strlen(BCG) + 1);
  strcpy(xhw->xhisho.i_info->filename,BCG);
  xhw->xhisho.i_info->is_shape = xhw->xhisho.is_shape;
  xhw->xhisho.i_info->ext_height = clock_height;

  if (LoadImage(xhw->xhisho.i_info) != 0) {
    fprintf(stderr, "fail read CG data,%s\n", BCG);
    exit(1);
  }

  if (xhw->xhisho.i_info->is_shape){
    XShapeCombineMask(DISPLAY, XtWindow(XtParent(xhw)), ShapeBounding, 0, 0
		      ,((xhw->xhisho.i_info->image) + CG_NUM)->mask, ShapeSet);
  }

  if(xhw->xhisho.i_info->num_of_images > 1)
    xhw->xhisho.cg_sec = ((xhw->xhisho.i_info->image) + CG_NUM)->secs;

  SetSize(xhw);
  XtResizeWidget(XtParent(xhw), WIDTH, HEIGHT + clock_height, FRAME_WIDTH);
  XtResizeWidget((Widget) xhw, WIDTH, HEIGHT + clock_height
		 ,FRAME_WIDTH);
}


static void Redraw(Widget w, XEvent * event, Region region)
{
  XHishoWidget xhw = (XHishoWidget) w;

  XCopyArea(DISPLAY, PIXMAP(CG_NUM), WINDOW, XH_GC, 0, 0, WIDTH, HEIGHT, 0, 0);

  if (xhw->xhisho.focuswin || xhw->xhisho.i_info->num_of_images > 1) {
    MoveFocusWindow(xhw);
  }

  if (xhw->xhisho.c_draw){
    ClockDraw(xhw);
  }
}

static void ClockDraw(XHishoWidget xhw)
{
  time_t now;
  struct tm *tmp;
  char year[5], month[3], day[3], hour[3], min[3], sec[3], clock[64];
  int width, x, i, height, y;
  char *argarray[6];

  if (xhw->xhisho.c_draw) {
    time(&now);
    tmp = localtime(&now);
    strftime(year, sizeof(year), "%G", tmp);
    strftime(month, sizeof(month), "%m", tmp);
    strftime(day, sizeof(day), "%d", tmp);
    strftime(hour, sizeof(hour), "%H", tmp);
    strftime(min, sizeof(min), "%M", tmp);
    strftime(sec, sizeof(sec), "%S", tmp);

    for (i = 0; i < strlen(xhw->xhisho.clock_arg) && i < 6; i++) {
      switch (xhw->xhisho.clock_arg[i]) {
      case 'y':
	argarray[i] = year;
	break;
      case 'm':
	argarray[i] = month;
	break;
      case 'd':
	argarray[i] = day;
	break;
      case 'h':
	argarray[i] = hour;
	break;
      case 'M':
	argarray[i] = min;
	break;
      case 's':
	argarray[i] = sec;
	break;
      default:
	argarray[i] = "Invarid argument format";
      }
    }

    sprintf(clock, xhw->xhisho.clock_f, argarray[0], argarray[1], argarray[2],
	    argarray[3], argarray[4], argarray[5]);

    XSetForeground(DISPLAY, XH_GC, WhitePixel(DISPLAY, 0));
    XFillRectangle(DISPLAY, WINDOW, XH_GC, 0, HEIGHT, WIDTH
		   ,xhw->core.height - HEIGHT);
    XSetForeground(DISPLAY, XH_GC, BlackPixel(DISPLAY, 0));

    width = XmbTextEscapement(xhw->label.fontset, clock, strlen(clock));

    x = (width > xhw->core.width) ? 0 : (int) ((xhw->core.width - width) / 2);

    XmbDrawString(DISPLAY, WINDOW, xhw->label.fontset, XH_GC
		,x, HEIGHT + xhw->label.label_height, clock, strlen(clock));
  }

  NewInterval(xhw);
}


static void Destroy(Widget w)
{
  XHishoWidget xhw = (XHishoWidget) w;

  XFreeGC(DISPLAY, XH_GC);
}


static void ClassInit()
{
  XawInitializeWidgetSet();
}

static Boolean SetValues(Widget current, Widget request, Widget new, ArgList args, Cardinal * num_args)
{
  Boolean ret;
  XHishoWidget iold = (XHishoWidget) current;
  XHishoWidget inew = (XHishoWidget) new;

  if (!strcmp(iold->xhisho.clock_text, inew->xhisho.clock_text)) {
    ClockDraw(inew);
  }

  if (iold->xhisho.anim_type != inew->xhisho.anim_type){
    ChangeAnimType(inew);
  }

  ret = (labelClassRec.core_class.set_values) (current, request, new, args, num_args);
  return ret;
}

static void NewInterval(XHishoWidget xhw)
{
  if (xhw->xhisho.intervalId) {
    XtRemoveTimeOut(xhw->xhisho.intervalId);
    xhw->xhisho.intervalId = 0;
  }
  xhw->xhisho.intervalId = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) xhw)
			   ,1 * 1000, (XtTimerCallbackProc) ClockDraw, xhw);
}

static void FocusInterval(XHishoWidget xhw)
{
  if (xhw->xhisho.focus_intervalId) {
    XtRemoveTimeOut(xhw->xhisho.focus_intervalId);
    xhw->xhisho.intervalId = 0;
  }

  if(xhw->xhisho.focus_interval == 0)
    xhw->xhisho.focus_interval = 1;

  xhw->xhisho.focus_intervalId = 
    XtAppAddTimeOut(XtWidgetToApplicationContext((Widget) xhw)
		    ,100 * xhw->xhisho.focus_interval
		    , (XtTimerCallbackProc) MoveFocusWindow, xhw);
}
  

static void MoveFocusWindow(XHishoWidget xhw)
{
  int width, x, i, height, y;
  Window focus, root, child, work;
  Widget tmp, tmp2;
  int dummy;

  if(xhw->xhisho.focuswin){

    XGetInputFocus(DISPLAY, &focus, &dummy);
    work = focus;
    tmp = XtWindowToWidget(DISPLAY, focus);
    
    while (tmp != NULL) {
      if (XtWindow(tmp) == XtWindow(XtParent(xhw)))
	focus = None;
      tmp2 = XtParent(tmp);
      tmp = tmp2;
    }
    
    if (focus == None || focus == PointerRoot || focus == XtWindow(XtParent(xhw)))
      focus = xhw->xhisho.focus;
    
    if (focus != None) {
      XGetGeometry(DISPLAY, focus, &root, &x, &y, &width, &height
		   ,&dummy, &dummy);
      XTranslateCoordinates(DISPLAY, focus, root, 0, 0, &x, &y, &child);
      
      switch (xhw->xhisho.just) {
      case XtJustifyLeft:
	break;
      case XtJustifyCenter:
	x += (width / 2 - xhw->core.width / 2);
	break;
      case XtJustifyRight:
	x += (width - xhw->core.width);
	break;
      }
      y -= xhw->core.height;
      y += xhw->xhisho.yoff;
      
      if (xhw->xhisho.old_x != x || xhw->xhisho.old_y != y) {
	XtMoveWidget(XtParent(xhw), x, y);
	xhw->core.x = x;
	xhw->core.y = y;
      }
      xhw->xhisho.old_x = x;
      xhw->xhisho.old_y = y;
    }
    xhw->xhisho.focus = focus;
  }

  if(xhw->xhisho.i_info->num_of_images > 1){
    Animation(xhw);
  }

  FocusInterval(xhw);
}
  
void SetSize(XHishoWidget xhw)
{
  if(xhw->xhisho.i_info->num_of_images > 1){
    WIDTH = (((xhw->xhisho.i_info->image) + CG_NUM)->width);
    HEIGHT = (((xhw->xhisho.i_info->image) + CG_NUM)->height);
  }
}

void Animation(XHishoWidget xhw)
{
  unsigned int change;
  int clock_height;
  
  change = 0;
  if(xhw->xhisho.cg_sec == -1) return;
  if(--xhw->xhisho.cg_sec <= 0){
    /**
     * 規定の時間が過ぎたらCG変更
     **/
    CG_NUM = CG_NUM + 1;
    if(CG_NUM >= xhw->xhisho.i_info->num_of_images) CG_NUM = 0;
    change = 1;
  }

  /**
   * ラベルの処理。ラベルは単純に読み飛ばす
   **/

  if(!strcmp(((xhw->xhisho.i_info->image) + CG_NUM)->filename,"SCHEDULE")||
     !strcmp(((xhw->xhisho.i_info->image) + CG_NUM)->filename,"MAIL")){
    CG_NUM = CG_NUM + 1;
    if(CG_NUM >= xhw->xhisho.i_info->num_of_images) CG_NUM = 0;
    change = 1;
  }
    
  while(!strcmp(((xhw->xhisho.i_info->image) + CG_NUM)->filename,"GOTO")){
    /**
     * GOTO の処理。GOTOの先がGOTOの時をまとめて処理するためにwhileにする
     **/

    if(((xhw->xhisho.i_info->image) + CG_NUM)->loop_b == -1){

      /**
       * loop_bが -1 なら永久に飛び続ける
       **/
      CG_NUM = ((xhw->xhisho.i_info->image) + CG_NUM)->secs -1;
      ((xhw->xhisho.i_info->image) + CG_NUM)->loop_c = 0;
      change = 1;
    } else {
      if(++((xhw->xhisho.i_info->image) + CG_NUM)->loop_c
	 >= ((xhw->xhisho.i_info->image) + CG_NUM)->loop_b){
	/**
	 * ループカウンタが既定値を越えたらGOTOを無視して次の行へ
	 **/
	((xhw->xhisho.i_info->image) + CG_NUM)->loop_c = 0;
	CG_NUM = CG_NUM + 1;
	change = 1;
      } else {
	/**
	 * 通常のGOTOジャンプ
	 **/
	CG_NUM = ((xhw->xhisho.i_info->image) + CG_NUM)->secs - 1;
	change = 1;
      }
    }
    if(CG_NUM >= xhw->xhisho.i_info->num_of_images - 1 ||
       CG_NUM < 0) CG_NUM = 0;
  }

  if(change){
    if (!xhw->xhisho.c_draw) {
      xhw->label.label_height = 0;
      clock_height = 0;
    } else {
      clock_height = xhw->label.label_height + 4;
    }

    xhw->xhisho.cg_sec = ((xhw->xhisho.i_info->image) + CG_NUM)->secs;
    SetSize(xhw);
    XtResizeWidget(XtParent(xhw), WIDTH, HEIGHT + clock_height, FRAME_WIDTH);
    XtResizeWidget((Widget) xhw, WIDTH, HEIGHT + clock_height
		 ,FRAME_WIDTH);
    XCopyArea(DISPLAY, PIXMAP(CG_NUM), WINDOW, XH_GC, 0, 0, WIDTH, HEIGHT, 0, 0);
    if (xhw->xhisho.i_info->is_shape){
      XShapeCombineMask(DISPLAY, XtWindow(XtParent(xhw)), ShapeBounding, 0, 0
			,((xhw->xhisho.i_info->image) + CG_NUM)->mask, ShapeSet);
    }
    XFlush(DISPLAY);
  }
}

static void ChangeAnimType(XHishoWidget xhw){
  /**
   * XtNanimTypeがセットされたときの処理
   **/

  if(xhw->xhisho.anim_type > 3 || xhw->xhisho.anim_type < 0) return;
  
  /**
   * テーブルの値が -1 → そのラベルは未定義だから無視
   **/

  if(xhw->xhisho.i_info->anim_number[xhw->xhisho.anim_type] == -1)
    return;

  /**
   * USUAL → 他のtypeの変更の時は現在のアニメーション処理の番号を
   * テーブルに覚えておく
   **/

  if(xhw->xhisho.anim_type){
    xhw->xhisho.i_info->anim_number[0] = CG_NUM;
  }
  
  CG_NUM = xhw->xhisho.i_info->anim_number[xhw->xhisho.anim_type];

  /**
   * Animation()の中ですぐに処理されるように,タイマーを0にしておく
   **/

  xhw->xhisho.cg_sec = 0;
  Animation(xhw);
}
