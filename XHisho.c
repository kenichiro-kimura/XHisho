/**
 * 画像張り付け & 時計表示 Widget 「XHisho Widget」 Widget本体処理ファイル
 *
 * copyright(c) 1998,1999  Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 *
 * 指定された(XtNcgFile)xpmデータを張り付ける。その下にxpmと同じ幅で指
 * 定された(XtNfontSet)フォントと同じ高さの余白を作り、そこに時計を表
 * 示する。時計の表示フォーマットはリソースXtNclockFormatで指定できる。
 * また、リソースXtNfocusWinをTrueにすると、フォーカスの当っているウイ
 * ンドウのタイトルに「おすわり」するようになる。focusWinへの追随は時
 * 計の書き換えと同時(つまり1秒毎)に行っているので、やや反応が鈍い。速
 * くすることも可能だが、負荷をあげてまでこの反応をあげることに意味を
 * 見いだせない。
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
#define XH_GC      (xhw->xhisho.gc)
#define BCG     (xhw->xhisho.cg_file)
#define WIDTH  (xhw->xhisho.width)
#define HEIGHT  (xhw->xhisho.height)
#define PIXMAP  (xhw->xhisho.pixmap)
#define PIXMASK (xhw->xhisho.pmask)
#define FRAME_WIDTH 1

static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Redraw(Widget, XEvent *, Region);
static void ClockDraw(XHishoWidget);
static void Destroy(Widget);
static void ClassInit();
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static void NewInterval(XHishoWidget);

extern int LoadImage(Widget, GC *, Pixmap *, char *, int *, int *, int, Boolean);

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
  XHishoWidget xhw = (XHishoWidget) new;

  XH_GC = XCreateGC(XtDisplay(xhw), RootWindowOfScreen(XtScreen(xhw)), (unsigned long) NULL, NULL);

  xhw->xhisho.intervalId = 0;
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

  if (LoadImage(XtParent((Widget) xhw), &(XH_GC), &(PIXMAP), BCG, &WIDTH
		,&HEIGHT, clock_height, xhw->xhisho.is_shape) != 0) {
    fprintf(stderr, "fail read CG data,%s\n", BCG);
    exit(1);
  }
  XtResizeWidget(XtParent(xhw), WIDTH, HEIGHT + clock_height, FRAME_WIDTH);
  XtResizeWidget((Widget) xhw, WIDTH, HEIGHT + clock_height
		 ,FRAME_WIDTH);
}


static void Redraw(Widget w, XEvent * event, Region region)
{
  XHishoWidget xhw = (XHishoWidget) w;

  XCopyArea(DISPLAY, PIXMAP, WINDOW, XH_GC, 0, 0, WIDTH, HEIGHT, 0, 0);

  ClockDraw(xhw);
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
  if (xhw->xhisho.focuswin) {
    /**
     * move to focus window ..
     **/

    Window focus, root, child, work;
    Widget tmp, tmp2;
    int dummy;

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
