/**
 * 大域関数および変数、マクロ、定数、構造体の定義
 **/

#ifndef _GLOBALDEFS_H
#define _GLOBALDEFS_H
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Xmu/Editres.h>
#include "config.h"
#include "Msgwin.h"
#include "XHisho.h"
#include "ResEdit.h"
#include "mail.h"
#include "openwin.h"
#include "about.h"

#ifdef _RESEDIT_GLOBAL 
#define RESEDIT_GLOBAL 
#else
#define RESEDIT_GLOBAL extern
#endif

#ifdef _ABOUT_GLOBAL 
#define ABOUT_GLOBAL
#else
#define ABOUT_GLOBAL extern
#endif

#ifdef _CALENDAR_GLOBAL 
#define CALENDAR_GLOBAL 
#else
#define CALENDAR_GLOBAL extern
#endif

#ifdef _EDITOR_GLOBAL 
#define EDITOR_GLOBAL 
#else
#define EDITOR_GLOBAL extern
#endif

#ifdef _MAIL_GLOBAL 
#define MAIL_GLOBAL 
#else
#define MAIL_GLOBAL extern
#endif

#ifdef _MAIN_GLOBAL 
#define MAIN_GLOBAL 
#else
#define MAIN_GLOBAL extern
#endif

#ifdef _MENU_GLOBAL 
#define MENU_GLOBAL 
#else	 
#define MENU_GLOBAL extern
#endif

#ifdef _MESSAGE_GLOBAL
#define MESSAGE_GLOBAL
#else
#define MESSAGE_GLOBAL extern
#endif

#ifdef _PETNAME_GLOBAL
#define PETNAME_GLOBAL
#else
#define PETNAME_GLOBAL extern
#endif

#ifdef _POP_GLOBAL
#define POP_GLOBAL
#else
#define POP_GLOBAL extern
#endif

#ifdef _SCHEDULE_GLOBAL
#define SCHEDULE_GLOBAL
#else
#define SCHEDULE_GLOBAL extern
#endif

#ifdef _SOUND_GLOBAL
#define SOUND_GLOBAL
#else
#define SOUND_GLOBAL extern
#endif

/**
 * global macro and type defines
 * 
 * HASH_KEY の 値は適当 ^^;
 **/

#define XHISHO_VERSION "1.20"
#define CODE_NAME "Lime Release 2"
#define MIN(a,b) ((a) > (b) ? (b) :(a))
#define MAX(a,b) ((a) > (b) ? (a) :(b))
#define NUM_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))
#define HASH_KEY 253

typedef enum _Method {
  POP_AUTH, APOP_AUTH, RPOP_AUTH
}   AuthMethod;

typedef enum _Biff {
  POP, APOP, YOUBIN, LOCAL
}   BiffMethod;

typedef struct _hlist {
  int day;
  char *name;
  struct _hlist *next;
}   HolidayList;


/**
 * global function define 
 **/

RESEDIT_GLOBAL Widget CreateResEditWindow(Widget);
RESEDIT_GLOBAL void ReadPrefFile();
RESEDIT_GLOBAL void WritePrefFile();
RESEDIT_GLOBAL void ChangeBar(Widget, caddr_t, int);
RESEDIT_GLOBAL void MoveBar(int i, float p);

ABOUT_GLOBAL Widget CreateAboutWindow(Widget);

CALENDAR_GLOBAL Widget CreateCalendarWindow(Widget, int, struct tm);

EDITOR_GLOBAL void CheckTimeForSchedule();
EDITOR_GLOBAL Widget CreateEditorWindow(Widget, int, struct tm);
EDITOR_GLOBAL unsigned long GetColor(Display *, char *);

MAIL_GLOBAL Widget CreateMailAlert(Widget, int);
MAIL_GLOBAL int CheckMail(XtPointer, XtIntervalId *);
MAIL_GLOBAL int CheckPOP3(XtPointer, XtIntervalId *);

MAIN_GLOBAL void CloseEditWindow();

MENU_GLOBAL Widget CreateMenuWindow(Widget);

MESSAGE_GLOBAL void Escape2Return(char *);
MESSAGE_GLOBAL int ReadRcfile(char *);
MESSAGE_GLOBAL void ReadRcdata(const char *, char *, int size);

#ifdef PETNAME
PETNAME_GLOBAL void ReadPetname(char *);
PETNAME_GLOBAL void SearchPetname(char *, char *);
#endif

POP_GLOBAL int pop3(AuthMethod, char *, char *);

SCHEDULE_GLOBAL int CheckSchedule(OpenMessageRes *, Schedule *, int, struct tm);
SCHEDULE_GLOBAL int ExistSchedule(int, int);
SCHEDULE_GLOBAL int ExistHoliday(int, int, int);
SCHEDULE_GLOBAL void ReadHoliday();

SOUND_GLOBAL int SoundPlay(const char *filename);
SOUND_GLOBAL int ExtSoundCommand(const char *filename);


/**
 * global variables define
 **/

MAIN_GLOBAL int isMailChecked;

/**
 * isMailChecked =
 *                 0 .. checked
 *                 1 .. not yet
 *                 2 .. timeout closed(not checked)
 **/

MAIN_GLOBAL MailAlertRes mar;
MAIN_GLOBAL BiffMethod Biff;
MAIN_GLOBAL int UseSound;
MAIN_GLOBAL String FilterCommand, SoundCommand;
MAIN_GLOBAL Widget mail, openwin, xhisho, about, editwin, calendarwin, menu
                  ,nomail, resedit;
MAIN_GLOBAL int MailWindowShown, OpenWindowShown, MenuWindowShown
              , AboutWindowShown, CalendarWindowShown;
MAIN_GLOBAL ResEditRes rer;
MAIN_GLOBAL OpenMessageRes omr;

#endif
