#include "Msgwin.h"
#include "menu.h"
#include "globaldefs.h"

static Widget top, menu, item[MENU_NUM + 1], head[MENU_NUM + 1], ok;
static char Menu[MENU_NUM][256];
static MenuRes mres;
static const char ResName[][128] = {"menul", "menu0", "menu1", "menu2"
,"menu3", "menu4", "menu5"};

extern int WindowMode, MenuWindowShown, OpenWindowShown, AboutWindowShown;
extern Widget openwin, calendarwin, about, resedit;

static void Destroy(Widget, caddr_t, caddr_t);
static void Quit(Widget, caddr_t, caddr_t);
static void CalendarWindowPopup(Widget, caddr_t, caddr_t);
static void OpeningWindowPopup(Widget, caddr_t, caddr_t);
static void TodayScheduleWindowPopup(Widget, caddr_t, caddr_t);
static void AboutWindowPopup(Widget, caddr_t, caddr_t);
static void ResEditWindowPopup(Widget, caddr_t, caddr_t);
Widget CreateMenuWindow(Widget);
extern Widget CreateEditorWindow(Widget, int, struct tm);
extern Widget CreateCalendarWindow(Widget, int, struct tm);
extern void ReadRcdata(const char *, char *, int);

static XtResource resources[] = {
    {
	XtNmenuLabel,
	XtCMenuLabel,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, label),
	XtRImmediate,
	(XtPointer) MENU_LABEL
    },
    {
	XtNmenuItem0,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu0),
	XtRImmediate,
	(XtPointer) MENU0
    },
    {
	XtNmenuItem1,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu1),
	XtRImmediate,
	(XtPointer) MENU1
    },
    {
	XtNmenuItem2,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu2),
	XtRImmediate,
	(XtPointer) MENU2
    },
    {
	XtNmenuItem3,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu3),
	XtRImmediate,
	(XtPointer) MENU3
    },
    {
	XtNmenuItem4,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu4),
	XtRImmediate,
	(XtPointer) MENU4
    },
    {
	XtNmenuItem5,
	XtCMenuItem,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, menu5),
	XtRImmediate,
	(XtPointer) MENU5
    },
    {
	XtNitemHead,
	XtCItemHead,
	XtRString,
	sizeof(String),
	XtOffsetOf(MenuRes, head),
	XtRImmediate,
	(XtPointer) HEAD
    },
};


static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
}

static void Quit(Widget w, caddr_t client_data, caddr_t call_data)
{
    exit(0);
}

static void CalendarWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
    time_t now;
    struct tm *tm_now;

    XtDestroyWidget(XtParent(calendarwin));

    time(&now);
    tm_now = localtime(&now);

    calendarwin = CreateCalendarWindow(XtParent(top), tm_now->tm_mon, *tm_now);
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
    XtPopup(XtParent(calendarwin), XtGrabNone);
}

static void OpeningWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
    time_t now;
    struct tm *tm_now;

    XtDestroyWidget(XtParent(openwin));

    time(&now);
    tm_now = localtime(&now);

    openwin = CreateEditorWindow(XtParent(top), 0, *tm_now);
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
    OpenWindowShown = 1;
    XtPopup(XtParent(openwin), XtGrabNone);
}

static void TodayScheduleWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
    time_t now;
    struct tm *tm_now;

    XtDestroyWidget(XtParent(openwin));

    time(&now);
    tm_now = localtime(&now);

    openwin = CreateEditorWindow(XtParent(top), 1, *tm_now);
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
    XtPopup(XtParent(openwin), XtGrabNone);
    OpenWindowShown = 1;
}

static void AboutWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
    XtPopup(XtParent(about), XtGrabNone);
    AboutWindowShown = 1;
}

static void ResEditWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
    XtPopdown(XtParent(XtParent(w)));
    MenuWindowShown = 0;
    XtPopup(XtParent(resedit), XtGrabNone);
}

Widget CreateMenuWindow(Widget w)
{
    static XtPopdownIDRec pdrec;
    int i;
    char *messages[NUM_OF_ARRAY(ResName)];

    static Arg menuargs[] = {
	{XtNwindowMode, 0},
	{XtNlabel, (XtArgVal) ""},
	{XtNwidth, 10},
	{XtNx, 100},
    };

    /**
     * Popdown処理のための準備 
     **/

    pdrec.shell_widget = top;
    pdrec.enable_widget = w;

    for (i = 0; i < NUM_OF_ARRAY(ResName); i++) {
	messages[i] = malloc(BUFSIZ);
	ReadRcdata(ResName[i], messages[i], BUFSIZ);
    }

    /**
     * toplevel Widgetの生成 
     **/

    top = XtCreatePopupShell("Menu", transientShellWidgetClass
			     ,w, menuargs, XtNumber(menuargs));

    XtGetApplicationResources(top, &mres, resources, XtNumber(resources), NULL, 0);

    if (*messages[1]) {
	strcpy(Menu[0], messages[1]);
    } else {
	strcpy(Menu[0], mres.menu0);
    }
    if (*messages[2]) {
	strcpy(Menu[1], messages[2]);
    } else {
	strcpy(Menu[1], mres.menu1);
    }
    if (*messages[3]) {
	strcpy(Menu[2], messages[3]);
    } else {
	strcpy(Menu[2], mres.menu2);
    }
    if (*messages[4]) {
	strcpy(Menu[3], messages[4]);
    } else {
	strcpy(Menu[3], mres.menu3);
    }
    if (*messages[5]) {
	strcpy(Menu[4], messages[5]);
    } else {
	strcpy(Menu[4], mres.menu4);
    }
    if (*messages[6]) {
	strcpy(Menu[5], messages[6]);
    } else {
	strcpy(Menu[5], mres.menu5);
    }

    menu = XtCreateManagedWidget("menu", msgwinWidgetClass, top
				 ,menuargs, XtNumber(menuargs));

    item[0] = XtVaCreateManagedWidget("menuLabel", labelWidgetClass, menu, XtNfromVert
				      ,NULL
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			,XtNlabel, (*messages[0]) ? messages[0] : mres.label
				      ,XtNborderWidth, 0
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
				      ,XtNvertDistance, 20
				      ,XtNinternalHeight, FONT_OFFSET, NULL);


    for (i = 1; i < MENU_NUM + 1; i++) {
	head[i] = XtVaCreateManagedWidget("menuHead", labelWidgetClass, menu, XtNfromVert
					  ,item[i - 1]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
					  ,XtNlabel, mres.head
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
					  ,XtNborderWidth, 0
				     ,XtNinternalHeight, FONT_OFFSET, NULL);

	item[i] = XtVaCreateManagedWidget("menuItem", commandWidgetClass, menu, XtNfromVert
					  ,item[i - 1]
					  ,XtNhorizDistance, 2
					  ,XtNlabel, Menu[i - 1]
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
					  ,XtNborderWidth, 0
					  ,XtNfromHoriz, head[i]
				     ,XtNinternalHeight, FONT_OFFSET, NULL);
	if (i == 1) {
	    XtVaSetValues(item[i], XtNvertDistance, 20, NULL);
	    XtVaSetValues(head[i], XtNvertDistance, 20, NULL);
	} else {
	    XtVaSetValues(item[i], XtNvertDistance, 2, NULL);
	    XtVaSetValues(head[i], XtNvertDistance, 2, NULL);
	}
    }

    XtAddCallback(item[1], XtNcallback, (XtCallbackProc) OpeningWindowPopup, NULL);
    XtAddCallback(item[2], XtNcallback, (XtCallbackProc) TodayScheduleWindowPopup, NULL);
    XtAddCallback(item[3], XtNcallback, (XtCallbackProc) CalendarWindowPopup, NULL);
    XtAddCallback(item[4], XtNcallback, (XtCallbackProc) ResEditWindowPopup, NULL);
    XtAddCallback(item[5], XtNcallback, (XtCallbackProc) AboutWindowPopup, NULL);
    XtAddCallback(item[6], XtNcallback, (XtCallbackProc) Quit, NULL);

    ok = XtVaCreateManagedWidget("menuOk", commandWidgetClass, menu, XtNfromVert
				 ,item[MENU_NUM]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNvertDistance, 20
				 ,XtNinternalHeight, FONT_OFFSET, NULL);

    XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

    for (i = 0; i < 7; i++)
	free(messages[i]);

    return (menu);
}
