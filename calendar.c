#include "Msgwin.h"
#include "calendar.h"

/**
 * 各種変数の宣言
 **/

static Widget top, calendar, week[7], day[42], prev, next, ok, cal_label;

static const char wname[][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};

static const int wdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int Edited_Month, Edited_Year;
static CalendarRes cres;

extern Widget editwin, calendarwin;

/**
 * 関数のプロトタイプ
 **/

static void Destroy(Widget w, caddr_t client_data, caddr_t call_data);
static void EditorWindowPopup(Widget, caddr_t, caddr_t);
static void PrevMonth(Widget, caddr_t, caddr_t);
static void NextMonth(Widget, caddr_t, caddr_t);

Widget CreateCalendarWindow(Widget, int, struct tm);

extern Widget CreateEditorWindow(Widget, int, struct tm);	/** in edit.c  **/
extern int ExistSchedule(int, int);
extern int ExistHoliday(int, int, int);
extern unsigned long GetColor(Display *, char *);
extern void ReadRcdata(char *, char *, int);

static XtResource resources[] = {
  {
    XtNcalendarLabel,
    XtCCalendarLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(CalendarRes, label),
    XtRImmediate,
    (XtPointer) CAL_LABEL
  },
  {
    XtNprevButton,
    XtCButton,
    XtRString,
    sizeof(String),
    XtOffsetOf(CalendarRes, prev),
    XtRImmediate,
    (XtPointer) PREV_BUTTON
  },
  {
    XtNnextButton,
    XtCButton,
    XtRString,
    sizeof(String),
    XtOffsetOf(CalendarRes, next),
    XtRImmediate,
    (XtPointer) NEXT_BUTTON
  },
  {
    XtNexistColor,
    XtCExistColor,
    XtRString,
    sizeof(String),
    XtOffsetOf(CalendarRes, color),
    XtRImmediate,
    (XtPointer) EX_COLOR
  },
};


static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
  XtPopdown(XtParent(XtParent(w)));
}

static void EditorWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
  struct tm *tm_now;
  time_t tval;

  time(&tval);
  tm_now = localtime(&tval);
  tm_now->tm_mon = Edited_Month;
  tm_now->tm_mday = (int) client_data;
  tval = mktime(tm_now);
  tm_now = localtime(&tval);

  XtDestroyWidget(XtParent(editwin));
  editwin = CreateEditorWindow(XtParent(top), 3, *tm_now);
  XtPopdown(XtParent(XtParent(w)));
  XtPopup(XtParent(editwin), XtGrabNone);
}

static void PrevMonth(Widget w, caddr_t client_data, caddr_t call_data)
{
  time_t tval;
  struct tm *tm_now;
  int t_month, t_day;

  time(&tval);
  tm_now = localtime(&tval);
  tm_now->tm_year = Edited_Year;
  t_month = tm_now->tm_mon = Edited_Month - 1;
  t_day = tm_now->tm_mday;

  if (t_month < 0)
    t_month = 11;
  if (t_day > wdays[t_month])
    t_day = wdays[t_month];
  tm_now->tm_mday = t_day;

  tval = mktime(tm_now);
  tm_now = localtime(&tval);

  XtDestroyWidget(XtParent(calendarwin));
  calendarwin = CreateCalendarWindow(XtParent(top), tm_now->tm_mon, *tm_now);
  XtPopup(XtParent(calendarwin), XtGrabNone);
}

static void NextMonth(Widget w, caddr_t client_data, caddr_t call_data)
{
  time_t tval;
  struct tm *tm_now;
  int t_month, t_day;

  time(&tval);
  tm_now = localtime(&tval);
  tm_now->tm_year = Edited_Year;
  t_month = tm_now->tm_mon = Edited_Month + 1;
  t_day = tm_now->tm_mday;

  if (t_month > 11)
    t_month = 0;
  if (t_day > wdays[t_month])
    t_day = wdays[t_month];
  tm_now->tm_mday = t_day;

  tval = mktime(tm_now);
  tm_now = localtime(&tval);

  XtDestroyWidget(XtParent(calendarwin));
  calendarwin = CreateCalendarWindow(XtParent(top), tm_now->tm_mon, *tm_now);
  XtPopup(XtParent(calendarwin), XtGrabNone);
}


Widget CreateCalendarWindow(Widget w, int Month, struct tm tm_now)
{
  /**
   * (Month)月のカレンダを作る
   **/

  static XtPopdownIDRec pdrec;
  static char string_l[256];
  static char tmpstring[256];
  time_t now;
  struct tm *tm_tmp;
  int i, j, k, l, m, tmp_width, Longest_label, calendar_offset, NowYear,
      NowMonth, NowDay;
  Dimension Label_width;
  char *message;
  int uru_adjust;

  static Arg calargs[] = {
    {XtNwindowMode, 0},
    {XtNlabel, (XtArgVal) ""},
    {XtNwidth, 10},
    {XtNx, 100},
  };

  Arg labelargs[] = {
    {XtNlabel, (XtArgVal) ""},	/** 0 **/
    {XtNborderWidth, 0},	/** 1 **/
    {XtNinternational, TRUE},	/** 2 **/
    {XtNleft, XtChainLeft},	/** 3 **/
    {XtNright, XtChainLeft},	/** 4 **/
    {XtNhorizDistance, 0},	/** 5 **/
    {XtNfromVert, (XtArgVal) NULL},	/** 6 **/
    {XtNfromHoriz, (XtArgVal) NULL},	/** 7 **/
    {XtNvertDistance, 2},	/** 8 **/
    {XtNinternalHeight, FONT_OFFSET},	/** 9 **/
  };

  Arg weekargs[] = {
    {XtNborderWidth, 0},	/** 0 **/
    {XtNinternational, TRUE},	/** 1 **/
    {XtNleft, XtChainLeft},	/** 2 **/
    {XtNright, XtChainLeft},	/** 3 **/
    {XtNhorizDistance, 0},	/** 4 **/
    {XtNfromVert, (XtArgVal) NULL},	/** 5 **/
    {XtNfromHoriz, (XtArgVal) NULL},	/** 6 **/
    {XtNvertDistance, 2},	/** 7 **/
    {XtNinternalHeight, FONT_OFFSET},	/** 8 **/
  };


  time(&now);
  tm_tmp = localtime(&now);
  NowYear = tm_tmp->tm_year;
  NowMonth = tm_tmp->tm_mon;
  NowDay = tm_tmp->tm_mday;

  i = j = k = 0;
  l = 1;
  Edited_Month = Month;
  Edited_Year = tm_now.tm_year;

  message = malloc(BUFSIZ);

  /**
   * 指定された月の初日の曜日を取得する
   **/

  *tm_tmp = tm_now;
  tm_tmp->tm_mday = 1;
  now = mktime(tm_tmp);
  tm_tmp = localtime(&now);
  m = tm_tmp->tm_wday;

  /**
   * うるう年のチェック。本来の定義は
   *
   * ・4で割りきれる年はうるう
   * ・但し、世紀末(**00)かつ400で割りきれない年はうるうにしない
   *
   * だが、どうせ2100年にこのプログラムが動いてると思えないし、
   * 2000年はうるうになるので2つ目のルールは不適用 ^^;
   **/

  uru_adjust = ((tm_now.tm_year % 4) == 0 && Month == 1) ? 1 : 0;

  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  /**
   * toplevel Widgetの生成
   **/

  top = XtCreatePopupShell("Calendar", transientShellWidgetClass
			   ,w, calargs, XtNumber(calargs));

  XtGetApplicationResources(top, &cres, resources, XtNumber(resources), NULL, 0);

  ReadRcdata("calendar", message, BUFSIZ);

  calendar = XtCreateManagedWidget("calendar", msgwinWidgetClass, top
				   ,calargs, XtNumber(calargs));

  prev = XtVaCreateManagedWidget("calendarPrev", commandWidgetClass, calendar, XtNfromVert
				 ,NULL
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, cres.prev
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNvertDistance, 2
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
  XtAddCallback(prev, XtNcallback, (XtCallbackProc) PrevMonth, NULL);

  XtVaGetValues(prev, XtNwidth, &Label_width, NULL);
  tmp_width = Label_width;

  string_l[0] = '\0';
  if (*message) {
    sprintf(string_l, message, Month + 1);
  } else {
    sprintf(string_l, cres.label, Month + 1);
  }
  labelargs[0].value = (XtArgVal) string_l;
  labelargs[6].value = (XtArgVal) NULL;
  labelargs[7].value = (XtArgVal) prev;

  cal_label = XtCreateManagedWidget("label", labelWidgetClass, calendar
				    ,labelargs, XtNumber(labelargs));
  XtVaGetValues(cal_label, XtNwidth, &Label_width, NULL);
  tmp_width += Label_width;

  next = XtVaCreateManagedWidget("calendarNext", commandWidgetClass, calendar, XtNfromVert
				 ,NULL
				 ,XtNfromHoriz, cal_label
				 ,XtNlabel, cres.next
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNvertDistance, 2
				 ,XtNinternalHeight, FONT_OFFSET, NULL);

  XtAddCallback(next, XtNcallback, (XtCallbackProc) NextMonth, NULL);

  XtVaGetValues(next, XtNwidth, &Label_width, NULL);
  tmp_width += Label_width;
  Longest_label = tmp_width;
  tmp_width = 0;

  labelargs[8].value = (XtArgVal) 10;

  for (i = 0; i < 7; i++) {
    if (!i) {
      weekargs[4].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
      weekargs[6].value = (XtArgVal) NULL;
    } else {
      weekargs[4].value = (XtArgVal) 2;
      weekargs[6].value = (XtArgVal) week[i - 1];
    }

    weekargs[5].value = (XtArgVal) cal_label;

    week[i] = XtCreateManagedWidget(wname[i], labelWidgetClass, calendar
				    ,weekargs, XtNumber(weekargs));
    XtVaGetValues(week[i], XtNwidth, &Label_width, NULL);
    tmp_width += Label_width;

  }

  if (tmp_width >= Longest_label) {
    calendar_offset = (int) ((tmp_width - Longest_label) / 2);
    XtVaSetValues(prev, XtNhorizDistance, calendar_offset + POINT_WIDTH + LABEL_OFFSET
		  ,NULL);
    calendar_offset = 0;
  } else {
    calendar_offset = (int) ((Longest_label - tmp_width) / 2);
    XtVaSetValues(week[0], XtNhorizDistance, calendar_offset + POINT_WIDTH + LABEL_OFFSET
		  ,NULL);
  }

  labelargs[8].value = (XtArgVal) 2;

  for (i = 0; i < 6; i++) {
    for (j = 0; j < 7; j++) {
      if (j == 0) {
	labelargs[5].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET + calendar_offset;
	labelargs[7].value = (XtArgVal) NULL;
      } else {
	labelargs[5].value = 2;
	labelargs[7].value = (XtArgVal) day[k - 1];
      }


      if (k < 7) {
	labelargs[6].value = (XtArgVal) week[k];
      } else {
	labelargs[6].value = (XtArgVal) day[k - 7];
      }

      if (k < 7 && j < m) {
	strcpy(tmpstring, "  ");
	labelargs[0].value = (XtArgVal) tmpstring;
	day[k] = XtCreateManagedWidget("day", labelWidgetClass, calendar
				       ,labelargs, XtNumber(labelargs));
      } else {
	sprintf(tmpstring, "%2d", l);
	l++;
	if (wdays[Month] + uru_adjust < l - 1)
	  break;
	labelargs[0].value = (XtArgVal) tmpstring;
	day[k] = XtCreateManagedWidget("day", commandWidgetClass, calendar
				       ,labelargs, XtNumber(labelargs));
	XtAddCallback(day[k], XtNcallback, (XtCallbackProc) EditorWindowPopup
		      ,(XtPointer) (l - 1));

	/**
	 * 予定のある日の色を変える
	 **/

	if (ExistSchedule(Edited_Month, l - 1))
	  XtVaSetValues(day[k], XtNbackground, GetColor(XtDisplay(top), cres.color), NULL);

	/**
	 * 土曜と日曜の色を変える
	 **/

	if (j == 0) {
	  XtVaSetValues(week[0], XtNforeground, GetColor(XtDisplay(top), "red"), NULL);
	  XtVaSetValues(day[k], XtNforeground, GetColor(XtDisplay(top), "red"), NULL);
	}
	if (j == 6) {
	  XtVaSetValues(week[6], XtNforeground, GetColor(XtDisplay(top), "blue"), NULL);
	  XtVaSetValues(day[k], XtNforeground, GetColor(XtDisplay(top), "blue"), NULL);
	}
	/**
	 *祝日の色を変える
	 **/

	if (ExistHoliday(Edited_Year + 1900, Edited_Month, l - 1))
	  XtVaSetValues(day[k], XtNforeground, GetColor(XtDisplay(top), "red"), NULL);

	/**
	 * 振り替え休日のチェック
	 **/

	if (j == 1) {
	  int bm, bd;
	  bd = l - 2;
	  bm = Edited_Month;
	  if (bd < 1) {
	    bm = Edited_Month - 1;
	    if (bm < 0)
	      bm = 11;
	    bd = wdays[bm] + ((Edited_Month == 1) ? uru_adjust : 0);
	  }
	  if (ExistHoliday(Edited_Year + 1900, bm, bd))
	    XtVaSetValues(day[k], XtNforeground, GetColor(XtDisplay(top), "red"), NULL);
	}
	/**
	 * 今日の色を変える
	 **/

	if (Edited_Year == NowYear && Edited_Month == NowMonth && l - 1 == NowDay)
	  XtVaSetValues(day[k], XtNforeground, GetColor(XtDisplay(top), "navy"), NULL);
      }
      k++;
    }
  }

  ok = XtVaCreateManagedWidget("calendarOk", commandWidgetClass, calendar, XtNfromVert
			       ,day[k - 1]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			       ,XtNlabel, "OK"
			       ,XtNleft, XtChainLeft, XtNright, XtChainLeft
			       ,XtNvertDistance, 20
			       ,XtNinternalHeight, FONT_OFFSET, NULL);

  XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

  free(message);
  return (calendar);
}
