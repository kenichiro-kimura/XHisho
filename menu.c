#define _MENU_GLOBAL
#include "globaldefs.h"
#include "Msgwin.h"
#include "menu.h"

static Widget top, item[MENU_NUM + 1], head[MENU_NUM + 1], ok;
static char Menu[MENU_NUM][256];
static MenuRes mres;
static const char ResName[][128] = {"menul", "menu0", "menu1", "menu2"
,"menu3", "menu4", "menu5"};

static void Destroy(Widget, caddr_t, caddr_t);

static struct {
  /**
   * Menuで選んだものに対応するWindowをPopupする関数の配列
   **/
void (*WindowPopup)(Widget, XEvent *, String *, unsigned int *);

} PopupFunctions[] = {
  {
    OpeningWindowPopup
  },
  {
    ScheduleWindowPopup
  },
  {
    CalendarWindowPopup
  },
  {
    ResEditWindowPopup
  },
  {
    AboutWindowPopup
  },
  {
    Quit
  }
};

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
}

static void SubWindowPopup(Widget w, caddr_t client_data, caddr_t call_data)
{
  int menu_pos;
  menu_pos = (int) client_data;
  XtPopdown(XtParent(XtParent(w)));

  if(menu_pos > 0 && menu_pos <= MENU_NUM){
    PopupFunctions[menu_pos - 1].WindowPopup(top,NULL,NULL,NULL);
  }
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
    {XtNinternational,TRUE},
  };

  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  for (i = 0; i < NUM_OF_ARRAY(ResName); i++) {
    messages[i] = (char*)malloc(BUFSIZ);
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
				    ,XtNinternational,TRUE
				    ,XtNinternalHeight, FONT_OFFSET, NULL);


  for (i = 1; i < MENU_NUM + 1; i++) {
    head[i] = XtVaCreateManagedWidget("menuHead", labelWidgetClass, menu, XtNfromVert
				      ,item[i - 1]
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				      ,XtNlabel, mres.head
				,XtNleft, XtChainLeft, XtNright, XtChainLeft
				      ,XtNborderWidth, 0
				      ,XtNinternational,TRUE
				      ,XtNinternalHeight, FONT_OFFSET, NULL);

    item[i] = XtVaCreateManagedWidget("menuItem", commandWidgetClass, menu, XtNfromVert
				      ,item[i - 1]
				      ,XtNhorizDistance, 2
				      ,XtNlabel, Menu[i - 1]
				      ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				      ,XtNborderWidth, 0
				      ,XtNinternational,TRUE
				      ,XtNfromHoriz, head[i]
				      ,XtNinternalHeight, FONT_OFFSET, NULL);
    if (i == 1) {
      XtVaSetValues(item[i], XtNvertDistance, 20, NULL);
      XtVaSetValues(head[i], XtNvertDistance, 20, NULL);
    } else {
      XtVaSetValues(item[i], XtNvertDistance, 2, NULL);
      XtVaSetValues(head[i], XtNvertDistance, 2, NULL);
    }

    XtAddCallback(item[i], XtNcallback,
		  (XtCallbackProc) SubWindowPopup, (XtPointer)i);
  }
  
  /*
  XtAddCallback(item[1], XtNcallback, (XtCallbackProc) OpeningWindowPopup, NULL);
  XtAddCallback(item[2], XtNcallback, (XtCallbackProc) TodayScheduleWindowPopup, NULL);
  XtAddCallback(item[3], XtNcallback, (XtCallbackProc) CalendarWindowPopup, NULL);
  XtAddCallback(item[4], XtNcallback, (XtCallbackProc) ResEditWindowPopup, NULL);
  XtAddCallback(item[5], XtNcallback, (XtCallbackProc) AboutWindowPopup, NULL);
  XtAddCallback(item[6], XtNcallback, (XtCallbackProc) Quit, NULL);
  */

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
