#ifndef _OPEN_H
#define _OPEN_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xmu/Editres.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>

#define XtNopenMessageF "openMessageF"
#define XtNopenMessageL "openMessageL"
#define XtNopenMessageN "openMessageN"
#define XtNalertMessageF "alertMessageF"
#define XtNalertMessageL "alertMessageL"
#define XtCAlertMessage "AlertMessage"
#define XtNmessageFormat "messageFormat"
#define XtNmessageArg "messageArg"
#define XtCOpenMessage  "Openmessage"
#define XtCMessageFormat  "MessageFormat"
#define XtCMessageArg  "MessageArg"
#define XtNpastSchedColor "pastSchedColor"
#define XtCPastSchedColor "PastSchedColor"
#define XtNalertSchedColor "alertSchedColor"
#define XtCAlertSchedColor "AlertSchedColor"
#define XtNnormalSchedColor "normalSchedColor"
#define XtCNormalSchedColor "NormalSchedColor"
#define XtNconfigFile "configFile"
#define XtCConfigFile "ConfigFile"
#define XtNleaveTime "leaveTime"
#define XtCLeaveTime "LeaveTime"
#define XtNextFilter "extFilter"
#define XtCExtFilter "ExtFilter"
#define XtNscheduleAlertSound "scheduleAlertSound"
#define XtCScheduleAlertSound "ScheduleAlertSound"
#define XtNscheduleEditMessage "scheduleEditMessage"
#define XtCScheduleEditMessage "ScheduleEditMessage"
#define XtNweeklyEditMessage "weeklyEditMessage"
#define XtCWeeklyEditMessage "WeeklyEditMessage"
#define XtNscheduleDir "scheduleDir"
#define XtCScheduleDir "ScheduleDir"
#define XtNscheduleSeparator "scheduleSeparator"
#define XtCScheduleSeparator "ScheduleSeparator"
#define XtNxcalendarCompatible "xcalendarCompatible"
#define XtCXcalendarCompatible "XcalendarCompatible"
#define XtNextHelloCommand "extHelloCommand"
#define XtCExtHelloCommand "ExtHelloCommand"
#define XtNzeroChime "zeroChime"
#define XtCZeroChime "ZeroChime"

#define PAST_C "LightGrey"
#define ALERT_C "red"
#define NORMAL_C "black"
#define MAX_SCHED_NUM 10
#define CFGFILE "aisatu.cfg"
#define LEAVE_TIME 5
#define FILTER "nkf -e -m"
#define SOUND_F "schedule.wav"
#define SCHED_DIR "/.Schedule/"

#define OPENF "Today is %s/%s. Your schedule is"
#define OPENL "."
#define OPENN "no schedule."
#define FORMAT "%s,from %s:%s"
#define ALERTF "Master,now the time for"
#define ALERTL "."
#define EDIT_M "What's your schedule at %s/%s ?"
#define SCHED_SEP ",from "
#define ARG "ehm"


typedef struct {
  char hour[3];
  char min[3];
  int leave;
  int is_checked;
  char ev[BUFSIZ];
}   Schedule;


typedef struct {
  String open_f;
  String open_l;
  String open_n;
  String alert_f;
  String alert_l;
  String message_f;
  String message_arg;
  String past_c;
  String alert_c;
  String normal_c;
  String cfg_file;
  char month[3];
  char day[3];
  int leave_t;
  String ext_filter;
  String sound_f;
  String edit_m;
  String edit_w;
  String sched_dir;
  String sched_sep;
  Boolean xcalendar;
  String ext_hello;
  Boolean chime;
}   OpenMessageRes;

#endif
