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
				  ,XtNborderWidth,1
				  ,XtNwidth,opr.width
				  ,XtNheight,opr.height
				  ,XtNleft,XtChainLeft
				  ,XtNright,XtChainRight
				  ,XtNsensitive,True
				  ,XtNjustify,XtJustifyLeft
				  /*
				  ,XtNresize,XawtextResizeWidth
				  */
				  ,XtNscrollHorizontal,XawtextScrollWhenNeeded
				  ,XtNscrollVertical,XawtextScrollWhenNeeded
				  ,XtNautoFill,True
				  ,XtNeditType,XawtextAppend
				  ,NULL);

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
  static char message_buffer[BUFSIZ * 20];
  char _buffer[BUFSIZ * 5];
  char buffer[BUFSIZ* 5];
  static int x = 0;
  int len;
  char* chr_ptr;
  char* next_ptr;
  int is_end;
  XFontSet fset;
  XRectangle ink, log;
  int max_len;
  Dimension width;
#ifdef EXT_FILTER
  char command[128];
  char t_filename[BUFSIZ];
  char d_buffer[BUFSIZ * 3];
  FILE* t_file;
  FILE* in;
#endif			


  if ((len = read(*fid,_buffer,BUFSIZ * 5)) == 0) {
    fprintf(stderr, "option command died!\n");
  } else if (len == -1) {
    fprintf(stderr, "Can't read from option command!\n");
  }

  _buffer[len] = '\0';

  if(x == 0){
    memset(message_buffer,'\0',BUFSIZ * 20);
  }

  is_end = 0;
  x = 1;

#ifdef EXT_FILTER
  strcpy(t_filename, tempnam(Tmp_dir, "xhtmp"));
  if ((t_file = fopen(t_filename, "w")) == NULL) {
    fprintf(stderr, "can't open temporary file,%s\n", t_filename);
  } else {
    fprintf(t_file, "%s", _buffer);
    fclose(t_file);

    sprintf(command, "%s %s", FilterCommand, t_filename);
    if ((in = popen(command, "r")) == NULL) {
      fprintf(stderr, "no such filter command:%s\n", command);
      exit(1);
    }
    buffer[0] = '\0';
    while((fgets(d_buffer, BUFSIZ * 3, in)) != NULL){
      strcat(buffer,d_buffer);
    }
    pclose(in);
    unlink(t_filename);
  }
#else
  strcpy(buffer, _buffer);
#endif

  /* here is script decoder .. */

  if((chr_ptr = strchr(buffer,'(')) != NULL){
    if(!strncasecmp(chr_ptr + 1,"Surface:",strlen("Surface:"))){
      int cg_num;
      char str_num[128];

      chr_ptr += strlen("(Surface:");
      next_ptr = chr_ptr;
      while(isdigit((unsigned char)(*next_ptr))) next_ptr++;
      strncpy(str_num,chr_ptr,next_ptr - chr_ptr);

      cg_num = atoi(str_num);
      XtVaSetValues(xhisho,XtNforceCG,True,XtNcgNumber,cg_num,NULL);
      chr_ptr = strchr(buffer,')') + 1;
      strcpy(_buffer,chr_ptr);
      strcpy(strchr(buffer,'('),_buffer);
    }
  }
  
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
  max_len = 0;
  chr_ptr = strtok(message_buffer,"\n");
  if(chr_ptr == NULL) max_len = strlen(message_buffer);

  while(chr_ptr){
    next_ptr = strtok(NULL,"\n");
    if(next_ptr == NULL)
      if(max_len < strlen(message_buffer))
	max_len = strlen(message_buffer);
    else 
      if(max_len < strlen(chr_ptr) - strlen(next_ptr))
	max_len = strlen(chr_ptr) - strlen(next_ptr);

    chr_ptr = next_ptr;
  }
  printf("%d\n", max_len * log.width);
  width = max_len * log.width;

  XtVaSetValues(label
		,XtNstring,message_buffer
		,XtNresize,XawtextResizeWidth
		,XtNautoFill,True
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


