#define _RESEDIT_GLOBAL 

#include "globaldefs.h"
#include "ResEdit.h"
#include "mail.h"

static Widget top, resedit, ok, scrollbar[5], label[5], parameter[5], toplabel,
    cancel;

static int PrefHash(char *);

static XtResource resources[] = {
  {
    XtNresEditLabel,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, label),
    XtRImmediate,
    (XtPointer) TOP_LABEL
  },
  {
    XtNresEditLabel0,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[0].label),
    XtRImmediate,
    (XtPointer) TIMEOUT_LABEL
  },
  {
    XtNresEditLabel1,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[1].label),
    XtRImmediate,
    (XtPointer) INTERVAL_LABEL
  },
  {
    XtNresEditLabel2,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[2].label),
    XtRImmediate,
    (XtPointer) LENGTH_LABEL
  },
  {
    XtNresEditLabel3,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[3].label),
    XtRImmediate,
    (XtPointer) LINES_LABEL
  },
  {
    XtNresEditLabel4,
    XtCResEditLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[4].label),
    XtRImmediate,
    (XtPointer) FOCUS_LABEL
  },
  {
    XtNresEditRes0,
    XtCResEditRes,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[0].name),
    XtRImmediate,
    (XtPointer) TIMEOUT_RES
  },
  {
    XtNresEditRes1,
    XtCResEditRes,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[1].name),
    XtRImmediate,
    (XtPointer) INTERVAL_RES
  },
  {
    XtNresEditRes2,
    XtCResEditRes,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[2].name),
    XtRImmediate,
    (XtPointer) LENGTH_RES
  },
  {
    XtNresEditRes3,
    XtCResEditRes,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[3].name),
    XtRImmediate,
    (XtPointer) LINES_RES
  },
  {
    XtNresEditRes4,
    XtCResEditRes,
    XtRString,
    sizeof(String),
    XtOffsetOf(ResEditRes, Pref[4].name),
    XtRImmediate,
    (XtPointer) FOCUS_RES
  },
  {
    XtNresEditMax0,
    XtCResEditMax,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[0].max),
    XtRString,
    (XtPointer) TIMEOUT_MAX
  },
  {
    XtNresEditMax1,
    XtCResEditMax,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[1].max),
    XtRString,
    (XtPointer) INTERVAL_MAX
  },
  {
    XtNresEditMax2,
    XtCResEditMax,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[2].max),
    XtRString,
    (XtPointer) LENGTH_MAX
  },
  {
    XtNresEditMax3,
    XtCResEditMax,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[3].max),
    XtRString,
    (XtPointer) LINES_MAX
  },
  {
    XtNresEditMax4,
    XtCResEditMax,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[4].max),
    XtRString,
    (XtPointer) FOCUS_MAX
  },
  {
    XtNresEditOffset0,
    XtCResEditOffset,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[0].offset),
    XtRString,
    (XtPointer) TIMEOUT_OFFSET
  },
  {
    XtNresEditOffset1,
    XtCResEditOffset,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[1].offset),
    XtRString,
    (XtPointer) INTERVAL_OFFSET
  },
  {
    XtNresEditOffset2,
    XtCResEditOffset,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[2].offset),
    XtRString,
    (XtPointer) LENGTH_OFFSET
  },
  {
    XtNresEditOffset3,
    XtCResEditOffset,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[3].offset),
    XtRString,
    (XtPointer) LINES_OFFSET
  },
  {
    XtNresEditOffset4,
    XtCResEditOffset,
    XtRFloat,
    sizeof(float),
    XtOffsetOf(ResEditRes, Pref[4].offset),
    XtRString,
    (XtPointer) FOCUS_OFFSET
  },
};


static void Destroy(Widget w, caddr_t cdata, caddr_t p)
{

  if ((int) cdata) {
    ReadPrefFile();
  } else {
    WritePrefFile();
  }

  XtPopdown(XtParent(XtParent(w)));
}

void ChangeBar(Widget w, caddr_t cdata, int p)
{
  char *tmp_string;

  tmp_string = malloc(20);

  sprintf(tmp_string, "%3d", (int) (*(float *) p * rer.Pref[(int) cdata].max
				    + rer.Pref[(int) cdata].offset));
  rer.Pref[(int) cdata].param = *(float *) p *rer.Pref[(int) cdata].max
  +   rer.Pref[(int) cdata].offset;
  XtVaSetValues(parameter[(int) cdata], XtNlabel, XtNewString(tmp_string), NULL);
  free(tmp_string);
}

void MoveBar(int i, float p)
{
  XawScrollbarSetThumb(scrollbar[i], p, 0);
}

Widget CreateResEditWindow(Widget w)
{
  static XtPopdownIDRec pdrec;
  int i;
  Dimension tmp_height, Longest_label;
  char tmp_string[20];
  float t_top, t_shown;
  char *message;

  static Arg resargs[] = {
    {XtNwindowMode, 0},
    {XtNlabel, (XtArgVal) ""},
    {XtNwidth, 10},
    {XtNx, 100}
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
    {XtNinternalWidth, 10},
  };

  Longest_label = 0;
  message = malloc(BUFSIZ);
  ReadRcdata("resource", message, BUFSIZ);

  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  /**
   * toplevel Widgetの生成
   **/

  top = XtCreatePopupShell("ResEdit", transientShellWidgetClass
			   ,w, resargs, XtNumber(resargs));

  XtGetApplicationResources(top, &rer, resources, XtNumber(resources), NULL, 0);

  resedit = XtCreateManagedWidget("resedit", msgwinWidgetClass, top,
				  resargs, XtNumber(resargs));

  ReadPrefFile();

  labelargs[0].value = (XtArgVal) ((*message) ? message : rer.label);
  labelargs[5].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
  labelargs[6].value = (XtArgVal) NULL;
  labelargs[7].value = (XtArgVal) NULL;

  toplabel = XtCreateManagedWidget("toplabel", labelWidgetClass, resedit,
				   labelargs, XtNumber(labelargs));

  for (i = 0; i < MAX_PREF_NUM; i++) {
    labelargs[0].value = (XtArgVal) rer.Pref[i].label;
    labelargs[5].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
    labelargs[6].value = i ? (XtArgVal) label[i - 1] : (XtArgVal) toplabel;
    labelargs[7].value = (XtArgVal) NULL;
    labelargs[8].value = i ? 2 : 20;

    label[i] = XtCreateManagedWidget("label", labelWidgetClass, resedit
				     ,labelargs, XtNumber(labelargs));

    XtVaGetValues(label[i], XtNwidth, &tmp_height, NULL);
    if (tmp_height > Longest_label)
      Longest_label = tmp_height;

    sprintf(tmp_string, "%3d", (int) rer.Pref[i].param);

    labelargs[0].value = (XtArgVal) XtNewString(tmp_string);
    labelargs[5].value = (XtArgVal) 2;
    labelargs[7].value = (XtArgVal) label[i];

    parameter[i] = XtCreateManagedWidget("parameter", labelWidgetClass, resedit
					 ,labelargs, XtNumber(labelargs));

    labelargs[7].value = (XtArgVal) parameter[i];

    XtVaGetValues(parameter[i], XtNheight, &tmp_height, NULL);

    scrollbar[i] = XtVaCreateManagedWidget("scrollbar", scrollbarWidgetClass, resedit,
					 XtNorientation, XtorientHorizontal,
					   XtNfromVert,
			  i ? (XtArgVal) label[i - 1] : (XtArgVal) toplabel,
					   XtNfromHoriz, parameter[i],
					   XtNhorizDistance, 2,
					   XtNvertDistance,
					   i ? 2 : 20,
					   NULL);
    t_top = (rer.Pref[i].param - rer.Pref[i].offset) / rer.Pref[i].max;
    t_shown = 0;

    XawScrollbarSetThumb(scrollbar[i], t_top, 0);

    XtVaSetValues(scrollbar[i], XtNwidth, 100, XtNheight, tmp_height,
		  XtNorientation, XtorientHorizontal,
		  XtNshown, t_shown,
		  NULL);

    XtVaGetValues(scrollbar[i], XtNtopOfThumb, &t_top, XtNshown, &t_shown, NULL);

    XtAddCallback(scrollbar[i], XtNjumpProc, (XtCallbackProc) ChangeBar, (XtPointer) i);
  }

  for (i = 0; i < MAX_PREF_NUM; i++) {
    XtVaSetValues(label[i], XtNwidth, Longest_label, NULL);
  }

  ok = XtVaCreateManagedWidget("reseditOk", commandWidgetClass, resedit, XtNfromVert
			       ,label[4]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			       ,XtNlabel, "OK"
			       ,XtNleft, XtChainLeft, XtNright, XtChainLeft
			       ,XtNvertDistance, 20
			       ,XtNinternalHeight, FONT_OFFSET, NULL);

  cancel = XtVaCreateManagedWidget("reseditOk", commandWidgetClass, resedit, XtNfromVert
				   ,label[4]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				   ,XtNlabel, "Cancel"
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
				   ,XtNvertDistance, 20
				   ,XtNfromHoriz, ok
				   ,XtNhorizDistance, 4
				   ,XtNinternalHeight, FONT_OFFSET, NULL);

  i = 0;
  XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, (XtPointer) i);
  i++;
  XtAddCallback(cancel, XtNcallback, (XtCallbackProc) Destroy, (XtPointer) i);

  free(message);
  return (resedit);
}

int PrefHash(char *name)
{
  int i = 0;

  for (i = 0; i < MAX_PREF_NUM; i++) {
    if (!strcmp(rer.Pref[i].name, name))
      return i;
  }

  return -1;
}

void ReadPrefFile()
{
  char *filename, *buf, *tmp1, *tmp2;
  FILE *fp;
  int i;

  filename = malloc(BUFSIZ);
  buf = malloc(BUFSIZ);
  tmp1 = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);

  for (i = 0; i < MAX_PREF_NUM; i++) {
    rer.Pref[i].is_set = 0;
    if (rer.Pref[i].max > rer.Pref[i].offset) {
      rer.Pref[i].max -= rer.Pref[i].offset;
    } else {
      rer.Pref[i].offset = 0;
    }
  }

  sprintf(filename, "%s", "./.xhishorc");

  if ((fp = fopen(filename, "r")) == NULL) {
    sprintf(filename, "%s%s", getenv("HOME"), "/.xhishorc");
    if ((fp = fopen(filename, "r")) == NULL) {
      rer.Pref[0].param = mar.m_timeout;
      rer.Pref[1].param = mar.m_check;
      rer.Pref[2].param = mar.from_maxlen;
      rer.Pref[3].param = mar.mail_lines;

      free(filename);
      free(buf);
      free(tmp1);
      free(tmp2);

      return;
    }
  }
  while (fgets(buf, BUFSIZ, fp) != NULL) {
    sscanf(buf, "%s %s", tmp1, tmp2);
    i = PrefHash(tmp1);
    if (i != -1) {
      rer.Pref[i].is_set = 1;
      if (atoi(tmp2) > rer.Pref[i].max) {
	rer.Pref[i].param = rer.Pref[i].max;
      } else if (atoi(tmp2) < rer.Pref[i].offset) {
	rer.Pref[i].param = rer.Pref[i].offset;
      } else {
	rer.Pref[i].param = atoi(tmp2);
      }
    }
  }

  free(filename);
  free(buf);
  free(tmp1);
  free(tmp2);

  fclose(fp);
}

void WritePrefFile()
{
  char filename[128];
  FILE *fp;
  int i;

  sprintf(filename, "%s%s", getenv("HOME"), "/.xhishorc");
  if ((fp = fopen(filename, "w")) == NULL)
    return;

  for (i = 0; i < MAX_PREF_NUM; i++) {
    fprintf(fp, "%s %d\n", rer.Pref[i].name, (int) (rer.Pref[i].param));
  }

  mar.m_timeout = (int) rer.Pref[0].param;
  mar.m_check = (int) rer.Pref[1].param;
  mar.from_maxlen = (int) rer.Pref[2].param;
  mar.mail_lines = (int) rer.Pref[3].param;

  fclose(fp);
}
