#define _ABOUT_GLOBAL
#include "globaldefs.h"
#include "about.h"
#include "Msgwin.h"

/**
 * local variable
 **/

static Widget top, about, label, ok;
static AboutRes abr;

/**
 * function definition
 **/

static void Destroy(Widget, caddr_t, caddr_t);

/**
 *resources
 **/

static XtResource resources[] = {
  {
    XtNaboutLabel,
    XtCAboutLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(AboutRes, about_s),
    XtRImmediate,
    (XtPointer) ABOUT_S
  },
};


static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
  AboutWindowShown = 0;
  XtPopdown(top);
}

Widget CreateAboutWindow(Widget w)
{
  static XtPopdownIDRec pdrec;
  char* aboutMessage;

  static Arg openargs[] = {
    {XtNwindowMode, 0},
    {XtNlabel, (XtArgVal) ""},
    {XtNwidth, 10},
    {XtNx, 100},
  };

  static Arg labelargs[] = {
    {XtNhorizDistance, 0},
    {XtNlabel, (XtArgVal) NULL},
    {XtNvertDistance, 0},
    {XtNinternational, TRUE},
    {XtNborderWidth, 0},
    {XtNleft, XtChainLeft},
    {XtNright, XtChainLeft},
    {XtNinternalHeight, 20},
    {XtNresize, FALSE},
  };

  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  /**
   * toplevel Widgetの生成
   **/

  top = XtCreatePopupShell("About", transientShellWidgetClass
			   ,w, openargs, XtNumber(openargs));

  XtGetApplicationResources(top, &abr, resources, XtNumber(resources), NULL, 0);

  aboutMessage = malloc(strlen(abr.about_s) + strlen(XHISHO_VERSION) + 1);
  sprintf(aboutMessage,abr.about_s,XHISHO_VERSION);

  labelargs[0].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
  labelargs[1].value = (XtArgVal) aboutMessage;


  about = XtCreateManagedWidget("about", msgwinWidgetClass
				,top, openargs, XtNumber(openargs));

  label = XtCreateManagedWidget("aboutLabel", labelWidgetClass, about
				,labelargs, XtNumber(labelargs));

  ok = XtVaCreateManagedWidget("aboutOk", commandWidgetClass, about, XtNfromVert, label
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			       ,XtNlabel, "OK"
			       ,XtNvertDistance, 20
			       ,XtNleft, XtChainLeft, XtNright, XtChainLeft
			       ,XtNinternalHeight, FONT_OFFSET, NULL);

  XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

  return (about);
}
