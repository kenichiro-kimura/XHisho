#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/xpm.h>
#include <X11/Xmu/Editres.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <X11/Xaw/Label.h>

#include "XHisho.h"
#include "Msgwin.h"
#include "globaldefs.h"
#include "config.h"

/* local variable */

static Widget toplevel;
static int IsSet = 0; /* 1回目のExposeでだけ子Widgetを作る */

Widget mail,openwin,xhisho,about,editwin,calendarwin,menu,nomail,resedit; 
int MailWindowShown,OpenWindowShown,MenuWindowShown,AboutWindowShown;
BiffMethod Biff = LOCAL;
int UseSound = 1;
String FilterCommand;

/* external variable */

extern int isMailChecked;/* in mail.c */

/* local function */

static void Wait(Widget,XEvent*,String*,unsigned int*); 
static void Quit(Widget,XEvent*,String*,unsigned int*);
static void OpenWindowPopup(Widget,XEvent*,String*,unsigned int*);
static void AboutWindowPopup(Widget,XEvent*,String*,unsigned int*);
static void MenuWindowPopup(Widget,XEvent*,String*,unsigned int*);
static void CalendarWindowPopup(Widget,XEvent*,String*,unsigned int*);
static void CheckMailNow(Widget,XEvent*,String*,unsigned int*);
static void PrintUsage(int,char**);
void CloseEditWindow();

/* external function */

extern Widget CreateMailAlert(Widget,int);   /* in mail.c  */
extern int CheckMail(XtPointer, XtIntervalId*);
extern int CheckPOP3(XtPointer, XtIntervalId*);

#ifdef PETNAME
extern void ReadPetname(char*); /* in petname.c */
#endif
extern Widget CreateAboutWindow(Widget);   /* in about.c  */
extern Widget CreateEditorWindow(Widget,int,struct tm);   /* in editor.c  */
extern int ChangeColorPastSched(); 
extern void CheckTimeForSchedule();
extern Widget CreateCalendarWindow(Widget,int,struct tm);   /* in calendar.c  */
extern Widget CreateMenuWindow(Widget);   /* in menu.c  */
extern Widget CreateResEditWindow(Widget);   /* in ResEdit.c  */
extern void WritePrefFile();
extern int ReadRcfile(char*);


/* action table */

static XtActionsRec actionTable[]={
  {"Quit",Quit},
  {"OpenWindowPopup",OpenWindowPopup},
  {"AboutWindowPopup",AboutWindowPopup},
  {"MenuWindowPopup",MenuWindowPopup},
  {"CalendarWindowPopup",CalendarWindowPopup},
  {"CheckMailNow",CheckMailNow},
  {"Expose",Wait},
};

/* command line options */

static XrmOptionDescRec options[] = {
  {"-cgfile",    "*cgFile",   XrmoptionSepArg, NULL},
  {"-timeout",  "*m_timeout",  XrmoptionSepArg, NULL},
  {"-mailcheck",  "*m_check",  XrmoptionSepArg, NULL},
  {"-noclock","*drawClock",XrmoptionNoArg,"False"},
  {"-focus","*focusWin",XrmoptionNoArg,"True"},
  {"-justify","*justify",XrmoptionSepArg,NULL},
  {"-ypos","*focusYoff",XrmoptionSepArg,NULL},
  {"-shape","*isShape",XrmoptionNoArg,"True"},
  {"-message","*messageFile",XrmoptionSepArg,NULL},
  {"-scheddir","*scheduleDir",XrmoptionSepArg,NULL},
  {"-chime","*zeroChime",XrmoptionNoArg,"True"},
#ifdef EXT_FILTER
  {"-filter",  "*extFilter",  XrmoptionSepArg, NULL},
#endif
  {"-soundcmd",  "*extSoundCommand",  XrmoptionSepArg, NULL},
  {"-yserver",  "*youbinServer",  XrmoptionSepArg, NULL},
  {"-pserver",  "*popServer",  XrmoptionSepArg, NULL},
};

static void Wait(Widget w,XEvent* e,String* s,unsigned int* i){
  time_t now;
  struct tm *tm_now;


  time(&now);
  tm_now = localtime(&now);

  if(!IsSet){

    resedit = CreateResEditWindow(toplevel);

    about = CreateAboutWindow(toplevel);

    /* Mail Windowの生成 */

    nomail = CreateMailAlert(toplevel,1);
    mail = CreateMailAlert(toplevel,0);

    /* OpenMessage Windowを生成し、Opening messageを表示する */
    
    openwin = CreateEditorWindow(toplevel,0,*tm_now);

    calendarwin = CreateCalendarWindow(toplevel,tm_now->tm_mon,*tm_now);

    editwin = CreateEditorWindow(toplevel,3,*tm_now);

    menu = CreateMenuWindow(toplevel);

    /* openwin のポップアップ */
    
    XtPopup(XtParent(openwin),XtGrabNone);
    OpenWindowShown = 1;

    /* 起動時のスケジュール時間のチェック */
    
    CheckTimeForSchedule((XtPointer)toplevel,(XtIntervalId)NULL);
    
    IsSet = 1;
  }
}

static void Quit(Widget w,XEvent *event,String *params,unsigned int *num_params){
  Widget top;

  top = w;
  while(XtParent(top) != NULL)
    top = XtParent(top);

  WritePrefFile();
  XCloseDisplay(XtDisplay(top)); /* child process の youbin を殺すため */
  exit(0);
}

static void CheckMailNow(Widget w,XEvent *event,String *params,unsigned int *num_params){
  int ret_value = 0;

  if(MailWindowShown){
    MailWindowShown = 0;
    XtPopdown(XtParent(mail));
    XtPopdown(XtParent(nomail));
    return;
  }

  isMailChecked = 1;

  if(Biff == POP || Biff == APOP)
    ret_value = CheckPOP3((XtPointer)(mail),(XtIntervalId)NULL);
  else 
    ret_value = CheckMail((XtPointer)(mail),(XtIntervalId)NULL);

  if(ret_value == 0){
    MailWindowShown = 1;
    XtVaSetValues(nomail,XtNwindowMode,0,NULL);
    XtPopup(XtParent(nomail),XtGrabNone);
  }
}


static void OpenWindowPopup(Widget w,XEvent *event,String *params,unsigned int *num_params){
  time_t now;
  struct tm *tm_now;

  if(OpenWindowShown){
    OpenWindowShown = 0;
    XtPopdown(XtParent(openwin));
    return;
  }

  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(openwin));
  openwin = CreateEditorWindow(XtParent(w),1,*tm_now);

  /* Opening message WindowのPopup */
  OpenWindowShown = 1;
  XtVaSetValues(openwin,XtNwindowMode,0,NULL);
  XtPopup(XtParent(openwin),XtGrabNone);
}

static void AboutWindowPopup(Widget w,XEvent *event,String *params,unsigned int *num_params){

  if(AboutWindowShown){
    XtPopdown(XtParent(about));
    AboutWindowShown = 0;
    return;
  }

  /* About WindowのPopup */
  XtVaSetValues(about,XtNwindowMode,0,NULL);
  XtPopup(XtParent(about),XtGrabNone);
  AboutWindowShown = 1;
}

static void MenuWindowPopup(Widget w,XEvent *event,String *params,unsigned int *num_params){

  if(MenuWindowShown){
    MenuWindowShown = 0;
    XtPopdown(XtParent(menu));
    return;
  }

  /* Menu WindowのPopup */
  XtVaSetValues(menu,XtNwindowMode,0,NULL);
  XtPopup(XtParent(menu),XtGrabNone);
  MenuWindowShown = 1;
}

static void CalendarWindowPopup(Widget w,XEvent *event,String *params,unsigned int *num_params){
  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(calendarwin));
  calendarwin = CreateCalendarWindow(toplevel,tm_now->tm_mon,*tm_now);
  /* Calendar WindowのPopup */
  XtVaSetValues(calendarwin,XtNwindowMode,0,NULL);
  XtPopup(XtParent(calendarwin),XtGrabNone);
}

void CloseEditWindow(){
  /* 予定をeditした直後に変更をleave機能に反映する */

  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(openwin));
  openwin = CreateEditorWindow(toplevel,0,*tm_now);
  CheckTimeForSchedule((XtPointer)toplevel,(XtIntervalId)NULL);
}

int main(int argc,char** argv){

  /*  actionTableとTranslationsの設定。main windowはBtn1Down,Btn3Down。 */

  static char defaultTranslations[] = "Shift<Btn2Down> : AboutWindowPopup()\n\
                                            <Btn2Down> : OpenWindowPopup()\n\
                                       Shift<Btn3Down> : Quit()\n\
                                       <Btn1Down> : MenuWindowPopup()\n\
                                       <Btn3Down> : CheckMailNow()\n\
                                       <Expose>: Expose()";

  static Arg args[] = {
    {XtNtranslations,(XtArgVal)NULL},
    {XtNlabel,(XtArgVal)""},
  };

  String rcfile,petname_f;

  /*  Localeをセットする */


  XtSetLanguageProc(NULL, NULL, NULL); 

  /*  toplevel widgetを生成  */

  toplevel = XtInitialize(argv[0],"XHisho",options,XtNumber(options),&argc,argv);
  XtAddEventHandler(toplevel, 0, True, _XEditResCheckMessages, NULL); 

  PrintUsage(argc,argv);

  XtVaSetValues(toplevel,XtNwidth,100,XtNheight,100,NULL);

  /*  main windowを生成。class名はxhisho。actionも追加。 */

  args[0].value = (XtArgVal)XtParseTranslationTable(defaultTranslations);
  xhisho = XtCreateManagedWidget("xhisho",xHishoWidgetClass,toplevel
				 ,args,XtNumber(args));

  /* rcfileとfilter commandとpetname fileをセット */

  XtVaGetValues(xhisho,XtNmessageFile,&rcfile,NULL);
  XtVaGetValues(xhisho,XtNextFilter,&FilterCommand,NULL);
  XtVaGetValues(xhisho,XtNpetnameFile,&petname_f,NULL);

  /* rcfileとpetnameを読む */
  ReadRcfile(rcfile);
#ifdef PETNAME
  ReadPetname(petname_f);
#endif


  /* Action のセット */
  XtAddActions(actionTable,XtNumber(actionTable));

  /*  WidgetのRealize */

  XtRealizeWidget(toplevel);

  /* Event Loop */

  XtMainLoop();

  return(-1);
}

static void PrintUsage(int argc,char** argv){
  static char* usages = 
    "  optins:\n"
    "     -version                    : print xhisho's version\n"
    "     -coption                    : show compile option\n"
#ifdef WITH_XPM
    "     -cgfile [file_name]         : cg file name(XPM or BMP)\n"
#else
    "     -cgfile [file_name]         : cg file name(BMP)\n"
#endif
    "     -timeout [n]                : mail window timeout(second)\n"
    "     -mailcheck [n]              : mail check interval(second)\n"
    "     -noclock                    : don't draw clock\n"
    "     -focus                      : use Focuswin module\n"
    "     -justify [left/center/rignt]: set Focuswin justify\n"
    "     -ypos [n]                   : Y-position offset for Focuswin\n"
    "     -shape                      : use transparent BMP's background\n"
    "     -message [file_name]        : message file name\n"
    "     -scheddir [dir_name]        : schedule dir name\n"
    "     -chime                      : use zero-hour chime\n"
#ifdef EXT_FILTER
    "     -filter command             : external filter command\n"
#endif
    "     -soundcmd command           : external sound command\n"
    "     -yserver [server_name]      : youbin server name\n"
    "     -pserver [server_name]      : POP3 server name\n"
    "\n";

  static char* compile_option = 
    "Compile option:\n"
#ifdef WITH_XPM
    "    Use XPM file\n"
#endif
#ifdef EXT_FILTER
    "    Use external filter command\n"
#endif
#ifdef PETNAME
    "    Use Petname for From: and Subject:\n"
#endif
#ifdef ADDRESSBOOK
    "    Use Mew's address book for Petname\n"
#endif
    "\n\n";
    
  int i,print_usage,j,print_coption;

  print_usage = 1;
  j = print_coption = 0;

  for(i = 1;i < argc;i++){
    if(!strcmp(argv[i],"-version")){
      print_usage = 0;
      j++;
    } else if(!strcmp(argv[i],"-coption")){
      print_coption = 1;
      j++;
    } else if(!strcmp(argv[i],"-youbin")){
      Biff = YOUBIN;
    } else if(!strcmp(argv[i],"-pop")){
      Biff = POP;
    } else if(!strcmp(argv[i],"-apop")){
      Biff = APOP;
    } else if(!strcmp(argv[i],"-nosound")){
      UseSound = 0;
    } else if(*argv[i] == '-'){
      j++;
      printf("unknown option:%s\n",argv[i]);
    }
  }
  
   
  if(j){
    printf("\n");
    fprintf(stderr,"XHisho version:%s[%s]\n\n",XHISHO_VERSION,CODE_NAME);

    if(print_coption)
      fprintf(stderr,"%s",compile_option);
    if(print_usage)
      fprintf(stderr,"Usage:%s [options] \n%s",argv[0],usages);
    exit(1);
  }

}


