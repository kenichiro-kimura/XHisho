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
  {
    XtNoptionWidth,
    XtCOptionWidth,
    XtRDimension,
    sizeof(Dimension),
    XtOffsetOf(OptionRes, width),
    XtRImmediate,
    (XtPointer)300
  },    
  {
    XtNoptionHeight,
    XtCOptionHeight,
    XtRDimension,
    sizeof(Dimension),
    XtOffsetOf(OptionRes, height),
    XtRImmediate,
    (XtPointer)200
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

  label = XtVaCreateManagedWidget("optionLabel", asciiTextWidgetClass
				  ,local_option
				  ,XtNvertDistance,10
				  ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				  ,XtNborderWidth,0
				  ,XtNwidth,opr.width
				  ,XtNheight,opr.height
				  ,XtNresize,False
				  ,XtNdisplayCaret,False
				  ,XtNscrollHorizontal,XawtextScrollWhenNeeded
				  ,XtNscrollVertical,XawtextScrollWhenNeeded
				  ,NULL);

  printf("%d x %d\n",opr.width,opr.height);

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
  if (stat(opr.o_command, &Ystat) == -1) {
    fprintf(stderr, "no such option command, \"%s\"\n", opr.o_command);
    exit(1);
  }

  if (virgine) {
    option_fd = popen(opr.o_command,"r");
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
  static char message_buffer[BUFSIZ];
  static char buffer[BUFSIZ * 10];
  static int x = 0;
  int len;
  char* chr_ptr;
  char* next_ptr;
  int is_end;
  XFontSet fset;
  XRectangle ink, log;


  len = read(*fid,buffer,BUFSIZ);
  buffer[len] = '\0';
  if(x == 0)
    memset(message_buffer,'\0',BUFSIZ * 10);

  is_end = 0;
  x = 1;


  /* here is script decoder .. */

  chr_ptr = buffer;
  if(len >= strlen("Surface:0")){
    if(!strncasecmp(buffer,"Surface:",strlen("Surface:"))){
      int cg_num;
      char str_num[128];

      next_ptr = chr_ptr = buffer + strlen("Surface:");
      while(isdigit((unsigned char)(*next_ptr))) next_ptr++;
      strncpy(str_num,chr_ptr,next_ptr - chr_ptr);

      cg_num = atoi(str_num);
      XtVaSetValues(xhisho,XtNforceCG,True,XtNcgNumber,cg_num,NULL);
      chr_ptr = next_ptr;
    }
  }

  if(next_ptr = strstr(chr_ptr,"\\e")){
    is_end = 1;
    *next_ptr = '\0';
  }

  /**
   * fontの大きさを取得し、表示領域の大きさを決める
   **/

  XtVaGetValues(label, XtNfontSet, &fset, NULL);
  XmbTextExtents(fset, "a", 1, &ink, &log);
  /*
  width = log.width;
  height = log.height;
  height += 20;
  */

  strcat(message_buffer,chr_ptr);

  XtVaSetValues(label,XtNstring,message_buffer
		,XtNjustify,XtJustifyLeft
		,XtNborderWidth,0
		,NULL);

  XtPopup(XtParent(local_option), XtGrabNone);

  if(is_end)
    message_buffer[0] = '\0';
}

static int Option_exit(Display * disp)
{
  /**
   * kill all the children
   **/
  kill(0, SIGTERM);
  return 0;
}
