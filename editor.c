#define _EDITOR_GLOBAL
#include "globaldefs.h"
#include "openwin.h"
#include "Msgwin.h"

/**
 * local variable
 **/

static Widget top, editor, local_open, list[MAX_SCHED_NUM], editlist[MAX_SCHED_NUM];
static Widget day[MAX_SCHED_NUM], kara[MAX_SCHED_NUM], leave_t[MAX_SCHED_NUM];
static int Schedule_num, past_index;
static int virgine = 1;
static Schedule *schedule;
static int Edited_Month, Edited_Day;
static XtIntervalId LeaveWindowID = 0;
static const char ResName[][128] = {"open1", "open2", "open3", "alert1", "alert2"
,"alertformat", "schedule", "messagearg"};

/**
 * function definition
 **/

static int WriteSchedule(struct tm);
static void Destroy(Widget w, caddr_t client_data, caddr_t call_data);
static void DestroyEdit(Widget w, caddr_t client_data, caddr_t call_data);
static void ParseConfigFile(int, char *);
static int ScheduleComp(Schedule *, Schedule *);
static int CheckIsSchedPast(int, Schedule *);
static void ChangeReturn(String, char *);
static void ScheduleSort();
static int ChangeColorPastSched();
static void OpenPopup();

/**
 * resources
 **/

static XtResource resources[] = {
  {
    XtNopenMessageF,
    XtCOpenMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, open_f),
    XtRImmediate,
    (XtPointer) OPENF
  },
  {
    XtNopenMessageL,
    XtCOpenMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, open_l),
    XtRImmediate,
    (XtPointer) OPENL
  },
  {
    XtNopenMessageN,
    XtCOpenMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, open_n),
    XtRImmediate,
    (XtPointer) OPENN
  },
  {
    XtNalertMessageF,
    XtCAlertMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, alert_f),
    XtRImmediate,
    (XtPointer) ALERTF
  },
  {
    XtNalertMessageL,
    XtCAlertMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, alert_l),
    XtRImmediate,
    (XtPointer) ALERTL
  },
  {
    XtNmessageFormat,
    XtCMessageFormat,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, message_f),
    XtRImmediate,
    (XtPointer) FORMAT
  },
  {
    XtNmessageArg,
    XtCMessageArg,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, message_arg),
    XtRImmediate,
    (XtPointer) ARG
  },
  {
    XtNpastSchedColor,
    XtCPastSchedColor,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, past_c),
    XtRImmediate,
    (XtPointer) PAST_C
  },
  {
    XtNalertSchedColor,
    XtCAlertSchedColor,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, alert_c),
    XtRImmediate,
    (XtPointer) ALERT_C
  },
  {
    XtNnormalSchedColor,
    XtCNormalSchedColor,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, normal_c),
    XtRImmediate,
    (XtPointer) NORMAL_C
  },
  {
    XtNconfigFile,
    XtCConfigFile,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, cfg_file),
    XtRImmediate,
    (XtPointer) CFGFILE
  },
  {
    XtNextFilter,
    XtCExtFilter,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, ext_filter),
    XtRImmediate,
    (XtPointer) FILTER
  },
  {
    XtNleaveTime,
    XtCLeaveTime,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OpenMessageRes, leave_t),
    XtRImmediate,
    (XtPointer) LEAVE_TIME
  },
  {
    XtNscheduleAlertSound,
    XtCScheduleAlertSound,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, sound_f),
    XtRImmediate,
    (XtPointer) OPEN_SOUND_F
  },
  {
    XtNscheduleEditMessage,
    XtCScheduleEditMessage,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, edit_m),
    XtRImmediate,
    (XtPointer) EDIT_M
  },
  {
    XtNscheduleDir,
    XtCScheduleDir,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, sched_dir),
    XtRImmediate,
    (XtPointer) SCHED_DIR
  },
  {
    XtNscheduleSeparator,
    XtCScheduleSeparator,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, sched_sep),
    XtRImmediate,
    (XtPointer) SCHED_SEP
  },
  {
    XtNxcalendarCompatible,
    XtCXcalendarCompatible,
    XtRBoolean,
    sizeof(Boolean),
    XtOffsetOf(OpenMessageRes, xcalendar),
    XtRImmediate,
    FALSE,
  },
  {
    XtNextHelloCommand,
    XtCExtHelloCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(OpenMessageRes, ext_hello),
    XtRImmediate,
    (XtPointer) NULL
  },
  {
    XtNzeroChime,
    XtCZeroChime,
    XtRBoolean,
    sizeof(Boolean),
    XtOffsetOf(OpenMessageRes, chime),
    XtRImmediate,
    FALSE
  },
};


static void DestroyEdit(Widget w, caddr_t client_data, caddr_t call_data)
{
  int ret, i;
  struct tm *tm_now;
  time_t tval;

  time(&tval);
  tm_now = localtime(&tval);

  tm_now->tm_mon = Edited_Month;
  tm_now->tm_mday = Edited_Day;
  ret = WriteSchedule(*tm_now);
  if (ret) {
    fprintf(stderr, "fail write schedule file\n");
  }
  for (i = 0; i < MAX_SCHED_NUM; i++)
    (int) schedule[i].is_checked = 0;

  XtPopdown(XtParent(XtParent(w)));
  OpenWindowShown = 0;
  CloseEditWindow();
}

static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
  XtPopdown(XtParent(XtParent(w)));
  OpenWindowShown = 0;
}

static void Dismiss(Widget w, caddr_t client_data, caddr_t call_data)
{
  int i;

  XtPopdown(XtParent(XtParent(w)));
  OpenWindowShown = 0;

  for (i = 0; i < past_index + 1; i++) {
    schedule[i].is_checked = 1;
  }
}





Widget CreateEditorWindow(Widget w, int Mode, struct tm tm_now)
{

  /**
   *  Mode = 0 ... Open Window (あいさつ付)
   *  Mode = 1 ... Schedule Window (あいさつ無し)
   *  Mode = 2 ... Schedule Alert (メッセージ通知用。必要なスケジュールのみ)
   *  Mode = 3 ... Schedule Editor (通常スケジュール)
   *  Mode = 4 ... Schedule Editor (曜日スケジュール:未実装)
   **/

  Widget ok, label_f, label_e, cancel, dismiss;
  XFontSet fset;
  XRectangle ink, log;
  static XtPopdownIDRec pdrec;
  char *string_f;
  char *string_e;
  char *tmpstring;
  char *sched_list[MAX_SCHED_NUM];
  char *argarray[4];
  int i, j, k, l, Longest_sched, schedules;
  char *since_minutes[MAX_SCHED_NUM];
  Dimension Label_width;
  char *messages[NUM_OF_ARRAY(ResName)];
  char daystring[MAX_SCHED_NUM][256], leavestring[MAX_SCHED_NUM][256];
  char *mesarg;

  static Arg editargs[] = {
    {XtNwindowMode, 0},
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

  Arg dargs[] = {
    {XtNstring, (XtArgVal) NULL},
    {XtNfromVert, (XtArgVal) NULL},
    {XtNfromHoriz, (XtArgVal) NULL},
    {XtNborderWidth, 1},
    {XtNinternational, TRUE},
    {XtNleft, XtChainLeft},
    {XtNright, XtChainLeft},
    {XtNhorizDistance, 0},
    {XtNvertDistance, 2},
    {XtNwidth, 0},
    {XtNheight, 0},
    {XtNsensitive, True},
    {XtNeditType, XawtextEdit},
    {XtNlength, 256},
    {XtNwrap, XawtextWrapWord},
    {XtNtopMargin, 0},
    {XtNbottomMargin, 5},
  };

  /**
   * 変数等の初期化。mainの大きさや位置も取得し、Argにセットする
   **/

  labelargs[5].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
  dargs[7].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
  Longest_sched = 0;
  i = j = 0;
  label_e = NULL;
  past_index = 0;
  Edited_Month = tm_now.tm_mon;
  Edited_Day = tm_now.tm_mday;

  string_f = malloc(BUFSIZ * 2);
  string_e = malloc(256);
  tmpstring = malloc(BUFSIZ * 2);

  memset(string_f, 0, BUFSIZ * 2);
  memset(tmpstring, 0, BUFSIZ * 2);
  memset(string_e, 0, 256);


  for (i = 0; i < MAX_SCHED_NUM; i++) {
    sched_list[i] = malloc(BUFSIZ * 3);
    since_minutes[i] = malloc(BUFSIZ);
    memset(sched_list[i], 0, BUFSIZ * 3);
    memset(since_minutes[i], 0, BUFSIZ);
  }

  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  /**
   * toplevel Widgetの生成
   **/

  top = XtCreatePopupShell("OpenMessage", transientShellWidgetClass
			   ,w, editargs, XtNumber(editargs));

  XtGetApplicationResources(top, &omr, resources, XtNumber(resources), NULL, 0);

  if (virgine) {
    virgine = 0;
    schedule = malloc(sizeof(Schedule) * MAX_SCHED_NUM);
    memset(schedule, 0, sizeof(Schedule) * MAX_SCHED_NUM);
    ReadHoliday();
  }
  for (i = 0; i < NUM_OF_ARRAY(ResName); i++) {
    messages[i] = malloc(BUFSIZ);
    memset(messages[i], 0, BUFSIZ);
    ReadRcdata(ResName[i], messages[i], BUFSIZ);
  }

  if (Mode == 3 || Mode == 4) {
    editor = XtCreateManagedWidget("editor", msgwinWidgetClass, top
				   ,editargs, XtNumber(editargs));

  } else {
    local_open = XtCreateManagedWidget("open", msgwinWidgetClass, top
				 ,editargs, XtNumber(editargs));
  }

  /**
   * read Schedule
   **/

  *string_f = '\0';
  *string_e = '\0';
  *tmpstring = '\0';
  schedules = 0;

  switch (Mode) {
  case 0:
    Schedule_num = schedules = CheckSchedule(&omr, schedule, 1, tm_now);
    break;
  case 1:
    schedules = CheckSchedule(&omr, schedule, 1, tm_now);
    break;
  case 2:
    schedules = CheckSchedule(&omr, schedule, 1, tm_now);
    break;
  case 3:
    schedules = CheckSchedule(&omr, schedule, 0, tm_now);
    break;
  case 4:
    schedules = CheckSchedule(&omr, schedule, 2, tm_now);
    break;
  }

  ScheduleSort();

  /**
   * 最初のメッセージを表示するlabelWidgetを作る
   **/

  switch (Mode) {
  case 0:
    ParseConfigFile(tm_now.tm_hour, string_f);
    strcat(string_f, "\n\n");
    if (*messages[0]) {
      sprintf(tmpstring, messages[0], omr.month, omr.day);
    } else {
      sprintf(tmpstring, omr.open_f, omr.month, omr.day);
    }
    break;
  case 1:
    if (*messages[0]) {
      sprintf(tmpstring, messages[0], omr.month, omr.day);
    } else {
      sprintf(tmpstring, omr.open_f, omr.month, omr.day);
    }
    break;
  case 2:
    if (*messages[3]) {
      sprintf(tmpstring, messages[3], omr.month, omr.day);
    } else {
      sprintf(tmpstring, omr.alert_f, omr.month, omr.day);
    }
    break;
  case 3:
    strcat(string_f, "\n");
    if (*messages[6]) {
      sprintf(tmpstring, messages[6], omr.month, omr.day);
    } else {
      sprintf(tmpstring, omr.edit_m, omr.month, omr.day);
    }
    break;
  case 4:
    strcat(string_f, "\n");
    sprintf(tmpstring, omr.edit_w, tm_now.tm_wday);
    break;
  }

  strcat(string_f, tmpstring);
  strcat(string_f, "\n\n");
  labelargs[0].value = (XtArgVal) string_f;
  labelargs[6].value = '\0';

  if (Mode == 3 || Mode == 4) {
    label_f = XtCreateManagedWidget("editorLabel", labelWidgetClass, editor
				    ,labelargs, XtNumber(labelargs));
  } else {
    label_f = XtCreateManagedWidget("editorLabel", labelWidgetClass, local_open
				    ,labelargs, XtNumber(labelargs));
  }

  XtVaGetValues(label_f, XtNfontSet, &fset, NULL);

  /**
   * labelWidgetを作ってlistの大きさを決める。Mode = 0,1,2ならここで作った
   * LabelWidgetをそのまま使う。Mode=3,4ならここで決めた大きさを元に
   * asciiTextWidgetを作る。
   **/

  i = (Mode == 0 || Mode == 1 || Mode == 2) ? schedules : MAX_SCHED_NUM;


  for (j = 0; j < i; j++) {
    memset(tmpstring, 0, BUFSIZ * 2);
    if (j < schedules) {
      /**
       * もしスケジュールがあるならそっちの幅優先
       **/
      strcpy(tmpstring, schedule[j].ev);
    } else {
      strcpy(tmpstring, "12345678901234567890");
    }

    labelargs[0].value = (XtArgVal) tmpstring;

    /**
     * Mode =0,1,2ならそのままlabelWidgetを作ってしまう
     **/

    if (messages[7][0] != '\0')
      mesarg = messages[7];
    else
      mesarg = omr.message_arg;

    if (Mode == 0 || Mode == 1 || Mode == 2) {
      for (k = 0; k < strlen(omr.message_arg) && k < 4; k++) {
	switch (mesarg[k]) {
	case 'h':
	  argarray[k] = schedule[j].hour;
	  break;
	case 'm':
	  argarray[k] = schedule[j].min;
	  break;
	case 'e':
	  argarray[k] = schedule[j].ev;
	  break;
	case 'l':
	  argarray[k] = since_minutes[j];
	  break;
	default:
	  argarray[k] = "Invarid argument format";
	}
      }

      l = CheckIsSchedPast(j, schedule);
      sprintf(since_minutes[j], "%d", l);

      if (*messages[5]) {
	sprintf(tmpstring, messages[5], argarray[0], argarray[1], argarray[2], argarray[3]);
      } else {
	sprintf(tmpstring, omr.message_f, argarray[0], argarray[1], argarray[2], argarray[3]);
      }

      /**
       * Mode2(Schedule Alert)のとき、「もうすぐ」なSchedule(5分以内)以外は
       * 表示しない
       **/

      if (Mode != 2 || (l >= 0 && l <= schedule[j].leave)) {
	strcat(sched_list[j], tmpstring);

	/**
	 * 「もうすぐ」なスケジュールのうち、添字が最大のものを記録しておく
	 **/
	past_index = j;

	/**
	 * １分前だったら強制的にチェックをはずす(表示させる)
	 **/
	if (l <= 1)
	  schedule[j].is_checked = 0;
      }
      labelargs[0].value = (XtArgVal) sched_list[j];
      if (j) {
	labelargs[6].value = (XtArgVal) list[j - 1];
	labelargs[8].value = (XtArgVal) 2;
      } else {
	labelargs[6].value = (XtArgVal) label_f;
	labelargs[8].value = (XtArgVal) 20;
      }

      list[j] = XtCreateManagedWidget("schedList", labelWidgetClass, local_open
				      ,labelargs, XtNumber(labelargs));

      log.width = 0;
    } else {
      XmbTextExtents(fset, tmpstring, strlen(tmpstring), &ink, &log);
    }
    Longest_sched = MAX(Longest_sched, log.width);
  }

  if (Mode == 3 || Mode == 4) {
    /**
     * Mode = 3,4 のときはasciiTextWidgetを作る
     **/


    for (j = 0; j < MAX_SCHED_NUM; j++) {

      dargs[7].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;

      /**
       * 開始時間表示textWidgetの作成
       **/

      memset(tmpstring, 0, BUFSIZ * 2);
      strcpy(tmpstring, "00000");
      memset(daystring[j], 0, 256);

      XmbTextExtents(fset, tmpstring, strlen(tmpstring), &ink, &log);

      /**
       * 取得した大きさを元にdayなるtextWidgetを作る
       **/

      if (j < schedules)
	sprintf(daystring[j], "%s%s", schedule[j].hour, schedule[j].min);

      dargs[0].value = (XtArgVal) daystring[j];
      dargs[9].value = (XtArgVal) log.width;

      if (j) {
	dargs[1].value = (XtArgVal) editlist[j - 1];
	dargs[2].value = (XtArgVal) NULL;
      } else {
	dargs[1].value = (XtArgVal) label_f;
      }

      dargs[10].value = (XtArgVal) (log.height + 6);

      day[j] = XtCreateManagedWidget("day", asciiTextWidgetClass, editor
				     ,dargs, XtNumber(dargs));

      /**
       * 開始時間とスケジュールの間にはいるlabelWidgetを作る
       **/

      labelargs[0].value = (XtArgVal) omr.sched_sep;

      if (j) {
	labelargs[6].value = (XtArgVal) editlist[j - 1];
      } else {
	labelargs[6].value = (XtArgVal) label_f;
      }

      labelargs[7].value = (XtArgVal) day[j];
      labelargs[5].value = (XtArgVal) 2;

      kara[j] = XtCreateManagedWidget("kara", labelWidgetClass, editor
				      ,labelargs, XtNumber(labelargs));

      /**
       * スケジュール用のtextWidgetを作る
       **/

      /**
       * 取得した大きさを元にtextWidgetを作る
       **/

      if (j < schedules) {
	dargs[0].value = (XtArgVal) schedule[j].ev;
      } else {
	dargs[0].value = (XtArgVal) NULL;
      }

      dargs[7].value = (XtArgVal) 2;

      /**
       * 1文字分余裕を取るために +8
       **/
      dargs[9].value = (XtArgVal) (Longest_sched + 8);

      if (j) {
	dargs[1].value = (XtArgVal) editlist[j - 1];
      } else {
	dargs[1].value = (XtArgVal) label_f;
      }
      dargs[2].value = (XtArgVal) kara[j];
      dargs[10].value = (XtArgVal) (log.height + 5);

      editlist[j] = XtCreateManagedWidget("schedList", asciiTextWidgetClass, editor
					  ,dargs, XtNumber(dargs));

      XtVaSetValues(editlist[j], XtNwrap, XawtextWrapNever, XtNscrollHorizontal
		    ,XawtextScrollAlways, NULL);

      /**
       * leave time 入力用 textWidgetの作成
       **/

      strcpy(tmpstring, "00000");

      XmbTextExtents(fset, tmpstring, strlen(tmpstring), &ink, &log);

      /**
       * 取得した大きさを元にdayなるtextWidgetを作る
       **/

      memset(leavestring[j], 0, 256);
      if (j < schedules) {
	sprintf(leavestring[j], "%d", schedule[j].leave);
      } else {
	schedule[j].leave = omr.leave_t;
	sprintf(leavestring[j], "%d", schedule[j].leave);
      }

      dargs[0].value = (XtArgVal) leavestring[j];
      dargs[9].value = (XtArgVal) log.width;

      if (j) {
	dargs[1].value = (XtArgVal) editlist[j - 1];
      } else {
	dargs[1].value = (XtArgVal) label_f;
      }
      dargs[2].value = (XtArgVal) editlist[j];

      dargs[10].value = (XtArgVal) (log.height + 6);

      leave_t[j] = XtCreateManagedWidget("leave", asciiTextWidgetClass, editor
					 ,dargs, XtNumber(dargs));
    }
  }
  /**
   * Mode ==0,1,2ならスケジュールのラベル(最後の奴)を作る
   **/

  if (Mode == 0 || Mode == 1 || Mode == 2) {
    string_e[0] = '\0';
    if (schedules) {
      if (Mode == 2) {
	if (*messages[4]) {
	  sprintf(tmpstring, "\n%s\n\n", messages[4]);
	} else {
	  sprintf(tmpstring, "\n%s\n\n", omr.alert_l);
	}
      } else {
	if (*messages[1]) {
	  sprintf(tmpstring, "\n%s\n\n", messages[1]);
	} else {
	  sprintf(tmpstring, "\n%s\n\n", omr.open_l);
	}
      }
      strcat(string_e, tmpstring);
      labelargs[6].value = (XtArgVal) list[j - 1];
    } else {
      if (*messages[2]) {
	sprintf(tmpstring, "\n%s\n\n", messages[2]);
      } else {
	sprintf(tmpstring, "\n%s\n\n", omr.open_n);
      }
      strcat(string_e, tmpstring);
      labelargs[6].value = (XtArgVal) label_f;
    }
    labelargs[0].value = (XtArgVal) string_e;
    labelargs[8].value = (XtArgVal) 30;

    label_e = XtCreateManagedWidget("scheduleLabel", labelWidgetClass, local_open
				    ,labelargs, XtNumber(labelargs));
    XtVaGetValues(label_e, XtNwidth, &Label_width, NULL);
  }
  if (Mode == 3 || Mode == 4) {
    ok = XtVaCreateManagedWidget("editorOk", commandWidgetClass, editor, XtNfromVert
				 ,day[j - 1]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNvertDistance, 20
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
    cancel = XtVaCreateManagedWidget("editorCancel", commandWidgetClass, editor
				     ,XtNfromVert, day[j - 1]
				     ,XtNfromHoriz, ok
				     ,XtNhorizDistance, 5
				     ,XtNlabel, "Cancel"
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
				     ,XtNvertDistance, 20
				     ,XtNinternalHeight, FONT_OFFSET, NULL);
    XtAddCallback(ok, XtNcallback, (XtCallbackProc) DestroyEdit, NULL);
    XtAddCallback(cancel, XtNcallback, (XtCallbackProc) Destroy, NULL);

    for (i = 0; i < 7; i++)
      free(messages[i]);

    for (i = 0; i < 10; i++)
      free(sched_list[i]);

    free(string_f);
    free(string_e);
    free(tmpstring);

    return (editor);
  } else {
    ok = XtVaCreateManagedWidget("openOk", commandWidgetClass, local_open, XtNfromVert
				 ,label_e
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNvertDistance, 20
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
    XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

    if (Mode == 2) {
      dismiss = XtVaCreateManagedWidget("openDismiss", commandWidgetClass, local_open
					,XtNfromVert, label_e
					,XtNfromHoriz, ok
					,XtNhorizDistance, 5
					,XtNlabel, "Dismiss"
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
					,XtNvertDistance, 20
				     ,XtNinternalHeight, FONT_OFFSET, NULL);
      XtAddCallback(dismiss, XtNcallback, (XtCallbackProc) Dismiss, NULL);
    }
    /**
     * とりあえずスケジュールの時間をチェックし、時間の過ぎたものは色を変える
     **/

    ChangeColorPastSched();

    for (i = 0; i < 7; i++)
      free(messages[i]);

    for (i = 0; i < 10; i++)
      free(sched_list[i]);

    free(string_f);
    free(string_e);
    free(tmpstring);

    return (local_open);
  }
}

static int WriteSchedule(struct tm tm_now)
{
  /**
   * return 0 if success to write file ,otherwise return 1
   **/

  FILE *outputfile;
  String dbuf, ebuf, lbuf;
  char *dtmp, *etmp;
  Arg darg[2], earg[2], larg[2];
  int i, n, l;
  time_t tval = 0;
  char filename[128], tday[3], month[4], year[4];

  /**
   * 日付を取得する。7/20ならdate="0720"になる。
   **/

  tval = mktime(&tm_now);

  strftime(tday, sizeof(tday), "%d", localtime(&tval));
  strftime(month, sizeof(month), "%m", localtime(&tval));
  strftime(year, sizeof(year), "%G", localtime(&tval));
  filename[0] = '\0';


  if (omr.xcalendar) {
    setlocale(LC_TIME, "C");
    strftime(month, sizeof(month), "%b", localtime(&tval));
    sprintf(filename, "%s/%sxc%d%s%d", getenv("HOME"),
	    omr.sched_dir, atoi(tday), month, atoi(year));
  } else {
    sprintf(filename, "%s/%sxhs%s%s", getenv("HOME"), omr.sched_dir, month, tday);
  }


  i = 0;
  n = 0;

  if ((outputfile = fopen(filename, "w")) == NULL) {
    return 1;
  }
  dtmp = malloc(BUFSIZ);
  etmp = malloc(BUFSIZ);
  memset(dtmp, 0, BUFSIZ);
  memset(etmp, 0, BUFSIZ);

  XtSetArg(darg[n], XtNstring, &dbuf);
  XtSetArg(earg[n], XtNstring, &ebuf);
  XtSetArg(larg[n], XtNstring, &lbuf);
  n++;

  for (i = 0; i < MAX_SCHED_NUM; i++) {
    XtGetValues(editlist[i], earg, n);
    XtGetValues(day[i], darg, n);
    XtGetValues(leave_t[i], larg, n);

    if (strlen(dbuf) > BUFSIZ)
      dtmp = realloc(dtmp, strlen(dbuf) + 1);
    if (strlen(ebuf) > BUFSIZ)
      etmp = realloc(etmp, strlen(dbuf) + 1);
    ChangeReturn(dbuf, dtmp);
    ChangeReturn(ebuf, etmp);


    if (dtmp[0] != '\0' && etmp[0] != '\0' && strlen(dtmp) == 4) {
      l = atoi(lbuf);
      l = (l > 0 && l < 60 * 24) ? l : omr.leave_t;
      fprintf(outputfile, "%s %d %s\n", dtmp, l, etmp);
    }
  }

  fclose(outputfile);
  free(dtmp);
  free(etmp);
  return 0;
}

static void ChangeReturn(String val, char *ret)
{
  /**
   * 不用意に(?)入った改行を除く
   **/
  char *tmp;
  int i;

  tmp = malloc(strlen(val) + 1);
  memset(tmp, 0, strlen(val) + 1);
  strcpy(tmp, val);
  ret[0] = '\0';
  i = 0;

  while (tmp[i] != '\0') {
    if (tmp[i] != '\n')
      strncat(ret, tmp + i, 1);
    i++;
  }
  strcat(ret, "\0");
  free(tmp);
}

unsigned long GetColor(Display * d, char *c)
{
  Colormap cmap;
  XColor c0, c1;

  cmap = DefaultColormap(d, 0);
  XAllocNamedColor(d, cmap, c, &c1, &c0);
  return (c1.pixel);
}

static int CheckIsSchedPast(int i, Schedule * sc)
{
  /**
   * 指定されたスケジュールと現在時刻の差を分単位で返す。
   * 既に過ぎていれば負の値が返る。
   * 引数にはschedule[i]のiを指定する。0 <= i < MAX_SCHED_NUM でなければ0を返す。
   **/
  time_t now, sched;
  struct tm *tmp;

  if (i >= MAX_SCHED_NUM || i < 0)
    return (0);
  time(&sched);
  tmp = localtime(&sched);
  tmp->tm_min = atoi((sc + i)->min);
  tmp->tm_hour = atoi((sc + i)->hour);
  tmp->tm_mday = atoi(omr.day);
  tmp->tm_mon = atoi(omr.month) - 1;
  sched = mktime(tmp);
  time(&now);

  return ((int) (difftime(sched, now) / 60));
}

static int ChangeColorPastSched()
{
  /**
   * 既に時刻を過ぎたスケジュールや「もうすぐ」なスケジュールの色を変える。
   * 「もうすぐ」なスケジュールの数を返す。
   **/

  int i, j, k;

  i = Schedule_num;

  k = 0;
  for (j = 0; j < i; j++) {
    if (CheckIsSchedPast(j, schedule) < 0) {
      XtVaSetValues(list[j], XtNforeground, GetColor(XtDisplay(top), omr.past_c), NULL);
    } else if (CheckIsSchedPast(j, schedule) <= schedule[j].leave) {
      XtVaSetValues(list[j], XtNforeground, GetColor(XtDisplay(top), omr.alert_c), NULL);
      k++;
    } else {
      XtVaSetValues(list[j], XtNforeground, GetColor(XtDisplay(top), omr.normal_c), NULL);
    }
  }
  return (k);
}


static void ParseConfigFile(int now, char *ret_value)
{
  /**
   * 起動のあいさつを探す。第2引数ret_valueに第1引数の時刻のあいさつが返る。
   * 対応するあいさつがなければNULLが返る。
   **/

  FILE *inputfile;
  char *tmp1, *tmp2, *tmp3;
  int first_h, last_h, return_size,i;

#ifdef EXT_FILTER
  char command[128];
#endif

  tmp1 = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  tmp3 = malloc(BUFSIZ);

  *ret_value = '\0';
  return_size = 0;

  first_h = last_h = 0;

  if ((inputfile = fopen(omr.cfg_file, "r")) != NULL) {

#ifdef EXT_FILTER
    sprintf(command, "%s %s", omr.ext_filter, omr.cfg_file);
    inputfile = popen(command, "r");
#endif

    while (fgets(tmp1, BUFSIZ, inputfile) != NULL) {

      /**
       * もし # で始まっていたらコメントとみなし、その後1行を無視する。
       * 空行も同様。
       **/

      if (tmp1[0] != '#' && tmp1[0] != '\0' && tmp1[0] != '\n') {
	sscanf(tmp1, "%s %s", tmp2, tmp3);

	for (i = 0;i < strlen(tmp1);i++)
	  if (isspace(tmp1[i]))
	    break;
	
	strcpy(tmp3,tmp1 + i + 1);

	if (strchr(tmp2, '-') == NULL) {
	  if (atoi(tmp2) >= 0 && atoi(tmp2) < 24 && atoi(tmp2) == now) {
	    return_size += strlen(tmp3);
	    if (return_size > BUFSIZ) {
#ifdef EXT_FILTER
	      pclose(inputfile);
#else
	      fclose(inputfile);
#endif
	      free(tmp1);
	      free(tmp2);
	      free(tmp3);
	      return;
	    }
	    strcat(ret_value, tmp3);
	  }
	} else {
	  first_h = (strchr(tmp2, '-') == tmp2) ? 0 : atoi(tmp2);
	  last_h = (tmp2[strlen(tmp2) - 1] == '-') ?
	    23 : atoi(strchr(tmp2, '-') + 1);
	  if (first_h >= 0 && first_h < 24 && last_h >= 0 && last_h < 24
	      && first_h <= now && now <= last_h) {
	    return_size += strlen(tmp3);
	    if (return_size > BUFSIZ) {

#ifdef EXT_FILTER
	      pclose(inputfile);
#else
	      fclose(inputfile);
#endif
	      free(tmp1);
	      free(tmp2);
	      free(tmp3);
	      return;
	    }
	    strcat(ret_value, tmp3);
	  }
	}
      }
    }

#ifdef EXT_FILTER
    pclose(inputfile);
#else
    fclose(inputfile);
#endif
  }
  if (omr.ext_hello) {
    strcat(ret_value, "\n\n");
    return_size += 2;
    inputfile = popen(omr.ext_hello, "r");

    while (fgets(tmp1, BUFSIZ, inputfile)) {
      return_size += strlen(tmp1);
      if (return_size > BUFSIZ)
	break;
      strcat(ret_value, tmp1);
    }

    pclose(inputfile);
  }
  free(tmp1);
  free(tmp2);
  free(tmp3);
  return;
}

static void ScheduleSort()
{
  int (*comp) ();

  comp = ScheduleComp;
  qsort(schedule, Schedule_num, sizeof(Schedule), comp);
}


static int ScheduleComp(Schedule * a, Schedule * b)
{
  if (atoi(a->hour) > atoi(b->hour))
    return 1;
  if (atoi(a->hour) < atoi(b->hour))
    return -1;
  if (atoi(a->min) > atoi(b->min))
    return 1;
  return -1;
}

void CheckTimeForSchedule(XtPointer cl, XtIntervalId * id)
{
  time_t now;
  struct tm *tmp;
  int i, check = 1, pid, status;

  if (LeaveWindowID) {
    XtRemoveTimeOut(LeaveWindowID);
    LeaveWindowID = 0;
  }
  time(&now);
  tmp = localtime(&now);

  if (tmp->tm_hour == 0 && tmp->tm_min == 0) {
    /**
     * 日付が変った(00:00)なら予定を新たに読み直す
     **/
    XtDestroyWidget(XtParent(openwin));
    openwin = CreateEditorWindow(cl, 0, *tmp);
  }
  if (tmp->tm_min == 0 && omr.chime ) {
    /**
     * 毎時0分のチャイム。OpenWinを開く
     **/
    XtDestroyWidget(XtParent(openwin));
    openwin = CreateEditorWindow(cl, 0, *tmp);
    OpenPopup();
  }
  if (ChangeColorPastSched()) {
    /**
     * 1つでも「もうすぐ」なスケジュールがあったら通知ウインドをポップアップ
     **/
    XtDestroyWidget(XtParent(openwin));
    openwin = CreateEditorWindow(cl, 2, *tmp);

    /**
     * 「もうすぐ」なスケジュールのうち,1つでもチェックされていない or
     * １分前のものがあるかチェック
     **/

    for (i = 0; i < past_index + 1; i++)
      check &= (int) schedule[i].is_checked;

    if (check == 0) {
      OpenPopup();
    }
  }
  LeaveWindowID = XtAppAddTimeOut(XtWidgetToApplicationContext(XtParent(top))
			    ,(60 - tmp->tm_sec) * 1000, CheckTimeForSchedule
				  ,cl);
}

static void OpenPopup(){
  XtVaSetValues(xhisho, XtNanimType, MAIL, NULL);
  XtVaSetValues(openwin, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(openwin), XtGrabNone);
  OpenWindowShown = 1;

  if (omr.sound_f && UseSound) {
    SoundPlay(omr.sound_f);
  }
}
