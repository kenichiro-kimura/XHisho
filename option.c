#define _OPTION_GLOBAL
#include "globaldefs.h"
#include "mail.h"
#include "petname.h"
#include "Msgwin.h"
#include "ResEdit.h"
#include "option.h"
#include <ctype.h>
#include <signal.h>

static Widget top,label,local_option;
static XtInputId OptionId;
static int virgine = 1;
static char option_command[] = "/home/show/bin/hoge.pl";

static void Destroy(Widget, caddr_t, caddr_t);
static void CommandInit();
static void CheckOption(Widget, int *, XtInputId *);
static int Option_exit(Display *);

static XtResource resources[] = {
  {
    XtNoptionCommand,
    XtCOptionCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(OptionRes, o_command),
    XtRImmediate,
    (XtPointer) ""
  },
};

static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
  XtPopdown(top);
}


Widget CreateOptionWindow(Widget w){
  Widget ok,cancel;
  static XtPopdownIDRec pdrec;

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  top = XtVaCreatePopupShell("OptionWindow", transientShellWidgetClass
			     ,w,NULL);

  XtGetApplicationResources(top, &opr, resources, XtNumber(resources), NULL, 0);

  local_option = XtVaCreateManagedWidget("option", msgwinWidgetClass, top
					 ,NULL);

  label = XtVaCreateManagedWidget("optionLabel", labelWidgetClass,local_option
				  ,XtNhorizDistance,POINT_WIDTH + LABEL_OFFSET
				  ,XtNvertDistance,0
				  ,XtNlabel, "hogehoge",NULL);

  ok = XtVaCreateManagedWidget("optionOk", commandWidgetClass, local_option
			       ,XtNfromVert, label
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			       ,XtNlabel, "OK"
			       ,XtNvertDistance, 20 
			       ,XtNleft, XtChainLeft, XtNright, XtChainLeft
			       ,XtNinternalHeight, FONT_OFFSET, NULL);
  XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

  cancel = XtVaCreateManagedWidget("optionCancel", commandWidgetClass
				   , local_option
				   ,XtNfromVert, label
				   ,XtNfromHoriz, ok
				   ,XtNhorizDistance,10
				   ,XtNlabel, "Cancel"
				   ,XtNvertDistance, 20 
				   ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				   ,XtNinternalHeight, FONT_OFFSET, NULL);
  XtAddCallback(cancel, XtNcallback, (XtCallbackProc) Destroy, NULL);

  CommandInit();
  return(local_option);
}

static void CommandInit()
{
  char *command;
  struct stat Ystat;

  command = (char*)malloc(256);

  if(strlen(opr.o_command) < 1) return;
  if (stat(option_command, &Ystat) == -1) {
    fprintf(stderr, "no such option command, \"%s\"\n", option_command);
    exit(1);
  }

  if (virgine) {
    option_fd = popen(option_command,"r");
    virgine = 0;
  }

  OptionId = XtAppAddInput(XtWidgetToApplicationContext(top),
			   fileno(option_fd), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckOption, NULL);
  XSetIOErrorHandler(Option_exit);	/** child process の youbin を殺す **/
  free(command);
}

static void CheckOption(Widget w, int *fid, XtInputId * id)
{
  char buffer[BUFSIZ];
  int len;
  len = read(*fid,buffer,BUFSIZ);
  buffer[len] = '\0';
  XtVaSetValues(label,XtNlabel,buffer,NULL);
  XtPopup(XtParent(local_option), XtGrabNone);
}

static int Option_exit(Display * disp)
{
  /**
   * kill all the children
   **/
  kill(0, SIGTERM);
  return 0;
}
