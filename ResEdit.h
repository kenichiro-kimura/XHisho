#ifndef _RESEDIT_H
#define _RESEDIT_H

#ifndef NeedWidePrototypes
#define NeedWidePrototypes 0
#endif

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/xpm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <X11/Xmu/Editres.h>

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Scrollbar.h>
#include "Msgwin.h"

typedef struct _Preference{
  String label;
  String name;
  float offset;
  float param;
  float max;
  int is_set;
} Preference;

#define XtNresEditLabel0 "resEditLabel0"
#define XtNresEditLabel1 "resEditLabel1"
#define XtNresEditLabel2 "resEditLabel2"
#define XtNresEditLabel3 "resEditLabel3"
#define XtNresEditLabel "resEditLabel"
#define XtCResEditLabel "ResEditLabel"

#define XtNresEditRes0 "resEditRes0"
#define XtNresEditRes1 "resEditRes1"
#define XtNresEditRes2 "resEditRes2"
#define XtNresEditRes3 "resEditRes3"
#define XtCResEditRes "ResEditRes"

#define XtNresEditMax0 "resEditMax0"
#define XtNresEditMax1 "resEditMax1"
#define XtNresEditMax2 "resEditMax2"
#define XtNresEditMax3 "resEditMax3"
#define XtCResEditMax "ResEditMax"

#define XtNresEditOffset0 "resEditOffset0"
#define XtNresEditOffset1 "resEditOffset1"
#define XtNresEditOffset2 "resEditOffset2"
#define XtNresEditOffset3 "resEditOffset3"
#define XtCResEditOffset "ResEditOffset"

#define TIMEOUT_RES  "mail_timeout"
#define TIMEOUT_MAX "20"
#define TIMEOUT_OFFSET "0"
#define INTERVAL_RES "mail_check"
#define INTERVAL_MAX "100"

#ifdef POP
#define INTERVAL_OFFSET "60"
#else
#define INTERVAL_OFFSET "1"
#endif

#define LENGTH_RES "mail_length"
#define LENGTH_MAX "100"
#define LENGTH_OFFSET "1"
#define LINES_RES "mail_lines"
#define LINES_MAX "8"
#define LINES_OFFSET "1"

#ifdef JP
#define TOP_LABEL "設定を変更してください"
#define TIMEOUT_LABEL "タイムアウト(秒)"
#define INTERVAL_LABEL "チェックインターバル(秒)"
#define LENGTH_LABEL "メッセージ長"
#define LINES_LABEL "メッセージ行数"
#else
#define TOP_LABEL "Please change preferences."
#define TIMEOUT_LABEL "mail window timeout(sec.)"
#define INTERVAL_LABEL "mail check interval(sec.)"
#define LENGTH_LABEL "message columns"
#define LINES_LABEL "message lines"
#endif

#define MAX_PREF_NUM 4

typedef struct {
  Preference Pref[MAX_PREF_NUM];
  String label;
} ResEditRes;

#endif





