#ifndef _MAIL_H
#define _MAIL_H

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

#define XtNm_timeout "m_timeout"
#define XtCTimeinterval "Timeinterval"
#define XtNm_check "m_check"
#define XtNmailbox "mailbox"
#define XtCMailbox "Mailbox"
#define XtNmailLabel "mailLabel"
#define XtCMailLabel "MailLabel"
#define XtNnoMailLabel "noMailLabel"
#define XtCNoMailLabel "NoMailLabel"
#define XtNmaxLines "maxLines"
#define XtNfromMaxLen "fromMaxLen"
#define XtCMaxLines "MaxLines"
#define XtCFromMaxLen "FromMaxLen"
#define XtNnewMailSound "newMailSound"
#define XtCNewMailSound "NewMailSound"
#define XtNyoubinServer "youbinServer"
#define XtCYoubinServer "YoubinServer"
#define XtNyoubinCommand "youbinCommand"
#define XtCYoubinCommand "YoubinCommand"
#define XtNpopServer "popServer"
#define XtCPopServer "PopServer"

#define MAILBOX "/var/mail/"
#define CFGFILE "aisatu.cfg"
#define MAIL_LINES 2
#define FROM_MAXLEN 50
#define SOUND_F "newmail.wav"
#define Y_SERVER "localhost"
#define Y_COMMAND "/usr/local/bin/youbin"

#define MAILLABEL "You have new mail."
#define NOLABEL "You have no new mail."

typedef struct {
  int m_timeout;
  int m_check;
  String mailbox;
  String mail_l;
  String no_l;
  int mail_lines;
  int from_maxlen;
  String sound_f;
  String y_server;
  String y_command;
  String p_server;
}   MailAlertRes;

#endif
