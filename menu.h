#ifndef _MENU_H
#define _MENU_H


#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xmu/Editres.h>
#include <X11/Xlocale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define MENU_NUM 6
#define XtNmenuLabel "menuLabel"
#define XtCMenuLabel "MenuLabel"
#define XtNmenuItem0 "menuItem0"
#define XtNmenuItem1 "menuItem1"
#define XtNmenuItem2 "menuItem2"
#define XtNmenuItem3 "menuItem3"
#define XtNmenuItem4 "menuItem4"
#define XtNmenuItem5 "menuItem5"
#define XtCMenuItem "MenunuItem"
#define XtNitemHead "itemHead"
#define XtCItemHead "ItemHead"

#ifdef JP
#define MENU_LABEL "何か御用でしょうか"
#define MENU0 "起動のあいさつが見たい"
#define MENU1 "今日の予定が見たい"
#define MENU2 "予定を編集したい"
#define MENU3 "設定を変更したい"
#define MENU4 "XHishoについて知りたい"
#define MENU5 "終わる"
#define HEAD "●"
#else
#define MENU_LABEL "What's happen?"

#define MENU0 "Show me opening message."
#define MENU1 "Show me today's schedules."
#define MENU2 "Edit schedules."
#define MENU3 "Edit Preferences."
#define MENU4 "about XHisho."
#define MENU5 "Quit."
#define HEAD "* "
#endif
typedef struct {
  String label;
  String menu0;
  String menu1;
  String menu2;
  String menu3;
  String menu4;
  String menu5;
  String head;
} MenuRes;

#endif



