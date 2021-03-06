#ifndef _OPTION_H
#define _OPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

#include <X11/Xaw/Command.h>

#define XtNoptionCommand "optionCommand"
#define XtCOptionCommand "OptionCommand"
#define XtNoptionWidth "optionWidth"
#define XtCOptionWidth "OptionWidth"
#define XtNoptionHeight "optionHeight"
#define XtCOptionHeight "OptionHeight"
#define XtNoptionTimeout "optionTimeout"
#define XtCOptionTimeout "OptionTimeout"
#define XtNuXOffset "uXOffset"
#define XtCUXOffset "UXOffset"
#define XtNuYOffset "uYOffset"
#define XtCUYOffset "UYOffset"
#define XtNmessageWait "messageWait"
#define XtCMessageWait "MessageWait"
#define XtNkawariWait "kawariWait"
#define XtCKawariWait "KawariWait"
#define XtNkawariDir "kawariDir"
#define XtCKawariDir "KawariDir"
#define XtNsakuraName "sakuraName"
#define XtNsakuraName2 "sakuraName2"
#define XtCSakuraName "SakuraName"
#define XtNkeroName "keroName"
#define XtCKeroName "KeroName"
#define XtNuserName "userName"
#define XtCUserName "UserName"



typedef struct {
  String o_command;
  Dimension width;
  Dimension height;
  int uxoff;
  int uyoff;
  int timeout;
  int m_wait;
  int k_wait;
  String kawari_dir;
  String s_name;
  String sn_name;
  String k_name;
  String u_name;
} OptionRes;

typedef struct _messagestack{
  struct _messagestack* next;
  char* message;
} messageStack;

enum {
  SAKURA = 0,
  UNYUU = 1
};

typedef struct _messagebuffer{
  unsigned char* buffer;
  size_t size;
} messageBuffer;

typedef struct _ct
{
  unsigned int jis;
  unsigned int ucode;
} CodeTable;

#endif
