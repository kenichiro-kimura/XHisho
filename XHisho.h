/**
 * 画像張り付け & 時計表示 Widget 「XHisho Widget」 パブリックヘッダファイル
 * 
 * copyright(c) 1998,1999  Ken'ichirou Kimura(kimura@db.is.kyushu-u.ac.jp)
 **/


#ifndef _XHISHO_h
#define _XHISHO_h

#include <X11/Xaw/Label.h>
#include <X11/Xmu/Editres.h>

extern WidgetClass xHishoWidgetClass;

typedef struct _xHishoClassRec *XHishoWidgetClass;
typedef struct _XHishoRec *XHishoWidget;

#define XtNcgFile "cgFile"
#define XtCCgFile "CgFile"
#define XtNmessageFile "messageFile"
#define XtCMessageFile "MessageFile"
#define XtNextFilter "extFilter"
#define XtCExtFilter "ExtFilter"
#define XtNextSoundCommand "extSoundCommand"
#define XtCExtSoundCommand "ExtSoundCommand"
#define XtNclockText "clockText"
#define XtCClockText "ClockText"
#define XtNclockFormat "clockFormat"
#define XtCClockFormat "ClockFormat"
#define XtNclockArg "clockArg"
#define XtCClockArg "ClockArg"
#define XtNdrawClock "drawClock"
#define XtCDrawClock "DrawClock"
#define XtNfocusWin "focusWin"
#define XtCFocusWin "FocusWin"
#define XtNfocusYoff "focusYoff"
#define XtCFocusYoff "FocusYoff"
#define XtNfocusYoff "focusYoff"
#define XtCFocusYoff "FocusYoff"
#define XtNisShape "isShape"
#define XtCIsShape "IsShape"
#define XtNpetnameFile "petnameFile"
#define XtCPetnameFile "PetnameFile"
#define XtNfocusWinInterval "focusWinInterval"
#define XtCFocusWinInterval "FocusWinInterval"
#define XtNanimType "animType"
#define XtCAnimType "AnimType"
#define XtNadjust "adjust"
#define XtCAdjust "Adjust"
#define XtNextEditCommand "extEditCommand"
#define XtCExtEditCommand "ExtEditCommand"


#define CGFILE "hisho.xpm"
#define RCFILE "Messages"
#define FILTER "nkf -e -m"
#define PETNAME_F "Petname"

#endif
