#define _MAIN_GLOBAL
#include "globaldefs.h"
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Xmu/Editres.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include <X11/Xaw/Label.h>

#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#include "XHisho.h"
#include "Msgwin.h"
#include "config.h"

/**
 * local variable
 **/

static int IsSet = 0;		/** 1回目のExposeでだけ子Widgetを作る **/
static Widget toplevel;
static int ShowGreet = 1;

#ifdef OPTION
static int UseOption = 1;
#endif

/**
 * local function
 **/

static void Wait(Widget, XEvent *, String *, unsigned int *);
static void CheckMailNow(Widget, XEvent *, String *, unsigned int *);
static void PrintUsage(int, char **);

/**
 * action table
 **/

static XtActionsRec actionTable[] = {
  {"Quit", Quit},
  {"ScheduleWindowPopup", ScheduleWindowPopup},
  {"CalendarWindowPopup", CalendarWindowPopup},
  {"AboutWindowPopup", AboutWindowPopup},
  {"MenuWindowPopup", MenuWindowPopup},
  {"OpeningWindowPopup", OpeningWindowPopup},
  {"CheckMailNow", CheckMailNow},
  {"Expose", Wait},
};

/**
 * command line options
 **/

static XrmOptionDescRec options[] = {
  {"-cgfile", "*cgFile", XrmoptionSepArg, NULL},
  {"-noclock", "*drawClock", XrmoptionNoArg, "False"},
  {"-focus", "*focusWin", XrmoptionNoArg, "True"},
  {"-adjust", "*adjust", XrmoptionNoArg, "True"},
  {"-justify", "*justify", XrmoptionSepArg, NULL},
  {"-ypos", "*focusYoff", XrmoptionSepArg, NULL},
  {"-shape", "*isShape", XrmoptionNoArg, "True"},
  {"-message", "*messageFile", XrmoptionSepArg, NULL},
  {"-scheddir", "*scheduleDir", XrmoptionSepArg, NULL},
  {"-chime", "*zeroChime", XrmoptionNoArg, "True"},
  {"-nochime", "*zeroChime", XrmoptionNoArg, "False"},
#ifdef EXT_FILTER
  {"-filter", "*extFilter", XrmoptionSepArg, NULL},
#endif
  {"-soundcmd", "*extSoundCommand", XrmoptionSepArg, NULL},
  {"-yserver", "*youbinServer", XrmoptionSepArg, NULL},
  {"-pserver", "*popServer", XrmoptionSepArg, NULL},
  {"-xoff", "*xoff", XrmoptionSepArg, NULL},
  {"-yoff", "*yoff", XrmoptionSepArg, NULL},
#ifdef OPTION
  {"-optioncmd", "*optionCommand", XrmoptionSepArg, NULL},
  {"-optionwidth", "*optionWidth", XrmoptionSepArg, "300"},
  {"-optionheight", "*optionHeight", XrmoptionSepArg, "200"},
  {"-optiontimeout", "*optionTimeout", XrmoptionSepArg, "5"},
  {"-wait", "*messageWait", XrmoptionSepArg, "0"},
#endif
  {"-unyuu", "*UseUnyuu", XrmoptionNoArg, "True"},
  {"-uxoff", "*uXOffset", XrmoptionSepArg, "300"},
  {"-uyoff", "*uYOffset", XrmoptionSepArg, "0"},
  {"-uwidth", "*UOptionWindow*optionWidth", XrmoptionSepArg, "300"},
  {"-uheight", "*UOptionWindow*optionHeight", XrmoptionSepArg, "200"},
  {"-ucgoff", "*uCGoff", XrmoptionSepArg, "0"},
#ifdef USE_KAWARI
  {"-kawaridir", "*kawariDir", XrmoptionSepArg, "xhisho"},
  {"-kwait", "*kawariWait", XrmoptionSepArg, "60"},
#endif
};

static void Wait(Widget w, XEvent * e, String * s, unsigned int *i)
{
  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime(&now);

  if (!IsSet) {
    resedit = CreateResEditWindow(toplevel);
    XtVaSetValues(xhisho, XtNfocusWinInterval, (int)rer.Pref[4].param, NULL);

    about = CreateAboutWindow(toplevel);

    /**
     * Mail Windowの生成
     **/

    nomail = CreateMailAlert(toplevel, 1);
    mail = CreateMailAlert(toplevel, 0);

    /**
     * OpenMessage Windowを生成し、Opening messageを表示する
     **/

    /*    editwin = CreateEditorWindow(toplevel, 3, *tm_now);*/
    editwin = NULL;
    tm_now = localtime(&now);
    openwin = CreateEditorWindow(toplevel, 0, *tm_now);
    tm_now = localtime(&now);
    calendarwin = CreateCalendarWindow(toplevel, tm_now->tm_mon, *tm_now);
    menu = CreateMenuWindow(toplevel);
#ifdef OPTION
    if(UseOption)
      optionwin = CreateOptionWindow(toplevel);
#endif

    /**
     * openwin のポップアップ
     **/

    if(ShowGreet){
      XtVaSetValues(openwin, XtNwindowMode, 0, NULL);
      XtPopup(XtParent(openwin), XtGrabNone);
    }

    /**
     * 起動時のスケジュール時間のチェック
     **/

    CheckTimeForSchedule((XtPointer) toplevel, (XtIntervalId) NULL);

    IsSet = 1;
  }
}

void Quit(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  Widget top;
  int i = 1;

  top = w;
  while (XtParent(top) != NULL)
    top = XtParent(top);

  WritePrefFile();
  XCloseDisplay(XtDisplay(top));

  if(Biff == YOUBIN)
    unlink(YoubinFile);

  while((i = rmdir(Tmp_dir)) < 0);

  if(Biff == YOUBIN){
    fclose(youbin_fd);
    kill(youbin_pid,SIGTERM);
  }

#ifdef OPTION
  fclose(option_fd);
  kill(option_pid,SIGTERM);
#endif
  exit(0);
}

static void CheckMailNow(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  int ret_value = 0;
  int i = 0;

  if ((i = IsPopped(mail)) || IsPopped(nomail)) {
    IsMailChecked(-1);
    if(i)
      XtPopdown(XtParent(mail));
    else
      XtPopdown(XtParent(nomail));
    return;
  }
  IsMailChecked(1);

  switch(Biff){
  case POP:
  case APOP:
  case IMAP:
    ret_value = CheckPOP3(2);
    break;
  case YOUBIN:
    ret_value = CheckYoubinNow(2);
    break;
  default:
    ret_value = CheckMail(2);
    break;
  }

  if(ret_value == 0)
    MailPopup(1,0);
}


void ScheduleWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  time_t now;
  struct tm *tm_now;

  if (IsPopped(openwin)) {
    XtPopdown(XtParent(openwin));
    return;
  }
  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(openwin));
  openwin = CreateEditorWindow(XtParent(w), 1, *tm_now);

  /**
   * Opening message WindowのPopup
   **/
  XtVaSetValues(openwin, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(openwin), XtGrabNone);
}

void OpeningWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  time_t now;
  struct tm *tm_now;

  if (IsPopped(openwin)) {
    XtPopdown(XtParent(openwin));
    return;
  }
  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(openwin));
  openwin = CreateEditorWindow(XtParent(w), 0, *tm_now);

  /**
   * Opening message WindowのPopup
   **/
  XtVaSetValues(openwin, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(openwin), XtGrabNone);
}

void AboutWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{

  if (IsPopped(about)) {
    XtPopdown(XtParent(about));
    return;
  }
  /**
   * About WindowのPopup
   **/
  XtVaSetValues(about, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(about), XtGrabNone);
}

void ResEditWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  if (IsPopped(resedit)) {
    XtPopdown(XtParent(resedit));
    return;
  }
  /**
   * Resource editor WindowのPopup
   **/
  XtVaSetValues(resedit, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(resedit), XtGrabNone);
}

void MenuWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{

  if (IsPopped(menu)) {
    XtPopdown(XtParent(menu));
    return;
  }
  /**
   * Menu WindowのPopup
   **/
  XtVaSetValues(menu, XtNwindowMode, 0, NULL);
  XtPopup(XtParent(menu), XtGrabNone);
}

void CalendarWindowPopup(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  time_t now;
  struct tm *tm_now;
  int pid,status;

  if(EditCommand){
    if((pid = fork()) == 0){
      if(fork() == 0){
	execvp(EditCommand,&EditCommand);
	exit(0);
      } else {
	exit(0);
      }
    } else {
      while(wait(&status) != pid);
    }
  } else {
    if(IsPopped(calendarwin)){
      XtPopdown(XtParent(calendarwin));
      return;
    }

    time(&now);
    tm_now = localtime(&now);

    XtDestroyWidget(XtParent(calendarwin));
    calendarwin = CreateCalendarWindow(toplevel, tm_now->tm_mon, *tm_now);
    /**
     * Calendar WindowのPopup
     **/
    XtVaSetValues(calendarwin, XtNwindowMode, 0, NULL);
    XtPopup(XtParent(calendarwin), XtGrabNone);
  }
}

void CloseEditWindow()
{
  /**
   * 予定をeditした直後に変更をleave機能に反映する
   **/

  time_t now;
  struct tm *tm_now;

  time(&now);
  tm_now = localtime(&now);

  XtDestroyWidget(XtParent(openwin));
  openwin = CreateEditorWindow(toplevel, 0, *tm_now);
  CheckTimeForSchedule((XtPointer) toplevel, (XtIntervalId) NULL);
}

int IsPopped(Widget w)
{
  if(!w) return 0;
  if(!XtParent(w)) return 0;
  return ((ShellWidget)XtParent(w))->shell.popped_up;
}
  

int main(int argc, char **argv)
{

  /**
   * actionTableとTranslationsの設定。main windowはBtn1Down,Btn3Down。
   **/

  static char defaultTranslations[] = "Shift<Btn2Down> : AboutWindowPopup()\n\
                                            <Btn2Down> : ScheduleWindowPopup()\n\
                                       Shift<Btn3Down> : Quit()\n\
                                       <Btn1Down> : MenuWindowPopup()\n\
                                       <Btn3Down> : CheckMailNow()\n\
                                       <Expose>: Expose()";

  String rcfile, petname_f;
  XtTranslations trans_table;
  XtAppContext app;

  Biff = LOCAL;
  IsMailChecked(0);
  UseSound = 1;
  ExistMailNum = HaveSchedule = 0;

  if(!IsSet)
    BeforeAnimatonMode = USUAL;

  /**
   *  Localeをセットする
   **/

  XtSetLanguageProc(NULL, NULL, NULL);

  toplevel = XtVaOpenApplication(&app, "XHisho", options, XtNumber(options)
				 , &argc, argv, NULL
				 , sessionShellWidgetClass,NULL);
  /**
   * Usageの表示
   **/

  PrintUsage(argc, argv);

  /**
   * main windowを生成。class名はxhisho。
   **/
  xhisho = XtVaCreateManagedWidget("xhisho",xHishoWidgetClass
				   ,toplevel
				   ,NULL);
  /**
   * rcfileとfilter commandとpetname fileをセット
   **/

  XtVaGetValues(xhisho, XtNmessageFile, &rcfile, NULL);
  XtVaGetValues(xhisho, XtNextFilter, &FilterCommand, NULL);
  XtVaGetValues(xhisho, XtNextSoundCommand, &SoundCommand, NULL);
  XtVaGetValues(xhisho, XtNpetnameFile, &petname_f, NULL);
  XtVaGetValues(xhisho,XtNextEditCommand,&EditCommand,NULL);

  /**
   * rcfileとpetnameを読む
   **/

  ReadRcfile(rcfile);
#ifdef PETNAME
  ReadPetname(petname_f);
#endif

  /**
   * Action のセット
   **/

  XtAddEventHandler(xhisho, 0, True, _XEditResCheckMessages, NULL);

  XtAppAddActions(app,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtAugmentTranslations(xhisho,trans_table);

  /**
   *  WidgetのRealize
   **/

  XtRealizeWidget(toplevel);
  sprintf(Tmp_dir, "/tmp/xhtmp%s-%d", getenv("USER"),getpid());
  mkdir(Tmp_dir, S_IRWXU);

  /**
   * Event Loop
   **/

  XtAppMainLoop(app);

  return (-1);
}

static void PrintUsage(int argc, char **argv)
{
  static char *usages =
  "  optins:\n"
  "     -version                    : print xhisho's version\n"
  "     -coption                    : show compile option\n"
  "     -cgfile [file_name]         : cg file name(BMP"
#ifdef HAVE_LIBXPM
  ",XPM"
#endif
#ifdef HAVE_LIBJPEG
  ",JPEG"
#endif
#ifdef HAVE_LIBPNG
  ",PNG"
#endif
  ")\n"
  "     -pop                        : check POP for biff\n"
  "     -apop                       : check APOP for biff\n"
  "     -imap                       : check IMAP4 for biff\n"
  "     -youbin                     : check youbin for biff\n"
  "     -noclock                    : don't draw clock\n"
  "     -nogreeting                 : don't show initial greeting window\n"
  "     -focus                      : use Focuswin module\n"
  "     -justify [left/center/rignt]: set Focuswin justify\n"
  "     -ypos [n]                   : Y-position offset for Focuswin\n"
  "     -shape                      : use transparent background\n"
  "     -message [file_name]        : message file name\n"
  "     -scheddir [dir_name]        : schedule dir name\n"
  "     -chime                      : use zero-min chime\n"
  "     -nochime                    : not use zero-min chime\n"
#ifdef EXT_FILTER
  "     -filter command             : external filter command\n"
#endif
  "     -soundcmd command           : external sound command\n"
  "     -yserver [server_name]      : youbin server name\n"
  "     -pserver [server_name]      : POP3(or IMAP4) server name\n"
  "     -xoff [n]                   : Popup Window X-offset\n"
  "     -yoff [n]                   : Popup Window Y-offset\n"
#ifdef OPTION
  "     -optioncmd                  : Option Command\n"
  "     -optionwidth                : width of Option window\n"
  "     -optionheight               : height of Option window\n"
  "     -optiontimeout              : Option window timeout\n"
  "     -wait                       : Option message wait\n"
#endif
  "     -unyuu                      : use UNYUU Window \n"
  "     -uxoff                      : UNYUU Window X-offset\n"
  "     -uyoff                      : UNYUU Window Y-offset\n"
  "     -uwidth                     : UNYUU Window Width\n"
  "     -uheight                    : UNYUU Window Height\n"
  "     -ucgoff                     : UNYUU CG X-offset\n"
#ifdef USE_KAWARI
  "     -kwait                      : KAWARI wait\n"
#endif
  "\n";

  static char *compile_option =
  "Compile option:\n"
#ifdef HAVE_LIBXPM
  "    Use XPM file\n"
#endif
#ifdef HAVE_LIBJPEG
  "    Use JPEG file\n"
#endif
#ifdef HAVE_LIBPNG
  "    Use PNG file\n"
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
#ifdef OPTION
  "    Use 'Something except that with option'\n"
#endif
#ifdef LIBMHC
  "    Use MHC's data\n"
#endif
#ifdef USE_SHARED
  "    Use own libraries as shared library\n"
#endif
#ifdef USE_KAWARI
  "    Use KAWARI module for 'Something except that with option'\n"
#endif
  "\n\n";

  int i, print_usage, j, print_coption;

  print_usage = 1;
  j = print_coption = 0;

  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-version")) {
      print_usage = 0;
      j++;
    } else if (!strcmp(argv[i], "-coption")) {
      print_coption = 1;
      print_usage = 0;
      j++;
    } else if (!strcmp(argv[i], "-youbin")) {
      Biff = YOUBIN;
    } else if (!strcmp(argv[i], "-pop")) {
      Biff = POP;
    } else if (!strcmp(argv[i], "-apop")) {
      Biff = APOP;
    } else if (!strcmp(argv[i], "-imap")) {
      Biff = IMAP;
    } else if (!strcmp(argv[i], "-nosound")) {
      UseSound = 0;
    } else if (!strcmp(argv[i], "-nogreeting")) {
      ShowGreet = 0;
#ifdef OPTION
    } else if (!strcmp(argv[i], "-nooption")) {
      UseOption = 0;
#endif
    } else if (*argv[i] == '-') {
      j++;
      printf("unknown option:%s\n", argv[i]);
    }
  }


  if (j) {
    printf("\n");
    fprintf(stderr, "%s version:%s\n\n"
	    , XHISHO_PACKAGE, XHISHO_VERSION);

    if (print_coption)
      fprintf(stderr, "%s", compile_option);
    if (print_usage)
      fprintf(stderr, "Usage:%s [options] \n%s", argv[0], usages);
    exit(1);
  }
}

