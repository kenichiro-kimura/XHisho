#ifndef _CALENDAR_H
#define _CALENDAR_H

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xmu/Editres.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define XtNcalendarLabel "calendarLabel"
#define XtCCalendarLabel "CalendarLabel"
#define XtNprevButton "prevButton"
#define XtNnextButton "nextButton"
#define XtCButton "Button"
#define XtNexistColor "existColor"
#define XtCExistColor "ExistColor"

#define EX_COLOR "Blue"

#ifdef JP
#define CAL_LABEL "%d��Υ��������Ǥ�"
#define PREV_BUTTON "���"
#define NEXT_BUTTON "���"
#else
#define CAL_LABEL "Calendar for %d th month."
#define PREV_BUTTON "Prev"
#define NEXT_BUTTON "Next"
#endif
typedef struct {
  String label;
  String prev;
  String next;
  String color;
} CalendarRes;

#endif

