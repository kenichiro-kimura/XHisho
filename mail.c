#include "mail.h"
#include "petname.h"
#include "Msgwin.h"
#include "globaldefs.h"
#include "ResEdit.h"
#include <ctype.h>
#include <signal.h>

/**
 * local variable
 **/

static Widget top[2], mail[2], from;
static int MailCount = 0;
static char m_filename[256];
static int TimeoutInterval = 1 * 1000;
static int MailTimeout;
static XtIntervalId MailTimeoutId;
static int MailCheckInterval;
static const char ResName[][256] = {"newmail", "nomail"};

static XtInputId YoubinId;
static int virgine = 1;
static char Tmp_dir[256];

int isMailChecked = 0;
/**
 * isMailChecked =
 *                 0 .. checked
 *                 1 .. not yet
 *                 2 .. timeout closed(not checked)
 **/
MailAlertRes mar;



/**
 * external variable
 **/

extern ResEditRes rer;		/** in ResEdit.c **/
extern int MailWindowShown;	/** in main.c **/
extern BiffMethod Biff;
extern String FilterCommand;
extern int UseSound;

/**
 * function definition
 **/

static void Destroy(Widget w, caddr_t client_data, caddr_t call_data);
static int isMail();
static void SetPrefVal(int, float);
static int Youbin_exit(Display *);
Widget CreateMailAlert(Widget, int);
int CheckMail(XtPointer, XtIntervalId *);

extern void ChangeBar(Widget, caddr_t, int);	/** in ResEdit.c **/
extern void MoveBar(int, float);
extern void ReadRcdata(const char *, char *, int);

#ifdef PETNAME
extern void SearchPetname(char *, char *);	/** in petname.c **/
#endif

extern int SoundPlay(char *);	/** in sound.c **/

static void CheckYoubin(Widget, int *, XtInputId *);
static void YoubinInit();
static void GetFromandSubject(char *, char *);

extern int pop3(AuthMethod, char *, char *);	/** in pop.c **/
int CheckPOP3(XtPointer, XtIntervalId *);

/**
 * resources
 **/

static XtResource resources[] = {
  {
    XtNm_timeout,
    XtCTimeinterval,
    XtRInt,
    sizeof(int),
    XtOffsetOf(MailAlertRes, m_timeout),
    XtRImmediate,
    (XtPointer) 10
  },
  {
    XtNm_check,
    XtCTimeinterval,
    XtRInt,
    sizeof(int),
    XtOffsetOf(MailAlertRes, m_check),
    XtRImmediate,
    (XtPointer) 5
  },
  {
    XtNmailbox,
    XtCMailbox,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, mailbox),
    XtRImmediate,
    (XtPointer) MAILBOX
  },
  {
    XtNmailLabel,
    XtCMailLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, mail_l),
    XtRImmediate,
    (XtPointer) MAILLABEL
  },
  {
    XtNnoMailLabel,
    XtCNoMailLabel,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, no_l),
    XtRImmediate,
    (XtPointer) NOLABEL
  },
  {
    XtNmaxLines,
    XtCMaxLines,
    XtRInt,
    sizeof(int),
    XtOffsetOf(MailAlertRes, mail_lines),
    XtRImmediate,
    (XtPointer) MAIL_LINES
  },
  {
    XtNfromMaxLen,
    XtCFromMaxLen,
    XtRInt,
    sizeof(int),
    XtOffsetOf(MailAlertRes, from_maxlen),
    XtRImmediate,
    (XtPointer) FROM_MAXLEN
  },
  {
    XtNnewMailSound,
    XtCNewMailSound,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, sound_f),
    XtRImmediate,
    (XtPointer) SOUND_F
  },
  {
    XtNyoubinServer,
    XtCYoubinServer,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, y_server),
    XtRImmediate,
    (XtPointer) Y_SERVER
  },
  {
    XtNyoubinCommand,
    XtCYoubinCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, y_command),
    XtRImmediate,
    (XtPointer) Y_COMMAND
  },
  {
    XtNpopServer,
    XtCPopServer,
    XtRString,
    sizeof(String),
    XtOffsetOf(MailAlertRes, p_server),
    XtRImmediate,
    (XtPointer) Y_SERVER
  },
};

static void SetPrefVal(int i, float p)
{
  float param;

  rer.Pref[i].param = p;
  param = (rer.Pref[i].param - rer.Pref[i].offset) / rer.Pref[i].max;
  ChangeBar(NULL, (caddr_t) i, (int) (XtPointer) (&param));
  MoveBar(i, param);
}

static void Destroy(Widget w, caddr_t client_data, caddr_t call_data)
{
  int Mode = (int) (client_data);

  MailWindowShown = 0;
  if (!Mode) {
    isMailChecked = 0;
    MailCount = 0;
  }
  XtPopdown(top[Mode]);
}

static void TimerCheck(XtPointer cl, XtIntervalId * id)
{
  if (MailWindowShown) {
    MailCount++;
    if (MailCount == MailTimeout) {
      MailCount = 0;
      MailWindowShown = 0;
      isMailChecked = 2;
      XtPopdown(top[0]);
    }
  }
  MailTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(top[0])
			  ,TimeoutInterval, TimerCheck, (XtPointer) top[0]);
}


static int isMail()
{
  static int OldSize;
  int NewSize = 0;
  struct stat MailStat;
  NewSize = (stat(m_filename, &MailStat) == 0) ? MailStat.st_size : 0;

  if (NewSize == 0) {
    OldSize = NewSize;		/** inc されたか メールが来ていないか **/
    return 0;
  } else if (NewSize > OldSize) {	/** 新しいメールが来た **/
    OldSize = NewSize;
    return 1;
  } else if (NewSize == OldSize) {	/** 新しいメールは来ていないが、incしてない **/
    return 2;
  }
  OldSize = NewSize;		/** NewSize < OldSizeである -> incしている。**/
  return 0;
}

int CheckMail(XtPointer cl, XtIntervalId * id)
{
  int i;
#ifndef YOUBIN
  char *buf;

  buf = malloc(mar.from_maxlen * mar.mail_lines + 1);
  memset(buf, 0, mar.from_maxlen * mar.mail_lines + 1);
#endif


  i = isMail();

  switch (i) {
  case 0:
    break;
  case 1:
    if (!MailWindowShown) {
      isMailChecked = 1;
      MailWindowShown = 1;
#ifndef YOUBIN
      GetFromandSubject(m_filename, buf);
      XtVaSetValues(from, XtNlabel, buf, NULL);
#endif
      MailWindowShown = 1;
      XtVaSetValues(mail[0], XtNwindowMode, 0, NULL);
      XtPopup(XtParent(mail[0]), XtGrabNone);

      if (mar.sound_f && UseSound) {
	if (0 == fork()) {
	  SoundPlay(mar.sound_f);
	  exit(0);
	}
      }
      MailWindowShown = 1;
    }
    break;
  case 2:
    switch (isMailChecked) {
    case 1:
      if (!MailWindowShown) {
	MailWindowShown = 1;
	XtVaSetValues(mail[0], XtNwindowMode, 0, NULL);
	XtPopup(XtParent(mail[0]), XtGrabNone);
      }
      break;
    case 2:
      isMailChecked = 1;
      break;
    }

    break;
  }
  XtAppAddTimeOut(XtWidgetToApplicationContext(mail[0])
  ,MailCheckInterval, (XtTimerCallbackProc) CheckMail, (XtPointer) mail[0]);

#ifndef YOUBIN
  free(buf);
#endif

  return i;
}

int CheckPOP3(XtPointer cl, XtIntervalId * id)
{
  int ret_value = 0;
  char *buf;

  buf = malloc(mar.from_maxlen * mar.mail_lines + 1);
  memset(buf, 0, mar.from_maxlen * mar.mail_lines + 1);

  switch (Biff) {
  case APOP:
    ret_value = pop3(APOP_AUTH, mar.p_server, buf);
    break;
  case POP:
    ret_value = pop3(POP_AUTH, mar.p_server, buf);
    break;
  default:
  }

  if (ret_value > 0) {
    if (mar.sound_f && UseSound) {
      if (0 == fork()) {
	SoundPlay(mar.sound_f);
	exit(0);
      }
    }
    MailWindowShown = 1;
    XtVaSetValues(from, XtNlabel, buf, NULL);
    XtVaSetValues(mail[0], XtNwindowMode, 0, NULL);
    XtPopup(XtParent(mail[0]), XtGrabNone);
  }
  free(buf);

  if (MailCheckInterval < 60 * 1000)
    MailCheckInterval = 60 * 1000;

  XtAppAddTimeOut(XtWidgetToApplicationContext(mail[0])
  ,MailCheckInterval, (XtTimerCallbackProc) CheckPOP3, (XtPointer) mail[0]);
  return ret_value;
}


Widget CreateMailAlert(Widget w, int Mode)
{

  /**
   * Mode = 0 ... 普通の Mail Alert
   * Mode = 1 ... No Mail
   **/

  Widget ok, label;
  static XtPopdownIDRec pdrec;
  int i;
  XFontSet fset;
  XRectangle ink, log;
  char *messages[NUM_OF_ARRAY(ResName)];

  static Arg mailargs[] = {
    {XtNwindowMode, 0},
    {XtNlabel, (XtArgVal) ""},
  };

  static Arg labelargs[] = {
    {XtNlabel, (XtArgVal) NULL},
    {XtNheight, 0},
    {XtNinternational, TRUE},
    {XtNborderWidth, 0},
    {XtNleft, XtChainLeft},
    {XtNright, XtChainLeft},
    {XtNfromHoriz, (XtArgVal) NULL},
    {XtNfromVert, (XtArgVal) NULL},
    {XtNhorizDistance, 0},
    {XtNvertDistance, 0}
  };

  static Arg fromargs[] = {
    {XtNlabel, (XtArgVal) NULL},
    {XtNfromVert, (XtArgVal) NULL},
    {XtNinternational, TRUE},
    {XtNinternalHeight, 10},
    {XtNborderWidth, 1},
    {XtNleft, XtChainLeft},
    {XtNright, XtChainLeft},
    {XtNhorizDistance, 0},
    {XtNvertDistance, 20},
    {XtNresize, FALSE},
    {XtNwidth, 0},
    {XtNheight, 0}
  };

  MailWindowShown = 0;
  /**
   * Popdown処理のための準備
   **/

  pdrec.shell_widget = top[Mode];
  pdrec.enable_widget = w;

  /**
   * 全てのWidgetの生成
   **/

  top[Mode] = XtCreatePopupShell("MailAlert", transientShellWidgetClass
				 ,w, mailargs, XtNumber(mailargs));

  XtGetApplicationResources(top[Mode], &mar, resources, XtNumber(resources), NULL, 0);

  if (rer.Pref[0].is_set) {
    mar.m_timeout = (int) rer.Pref[0].param;
  } else {
    SetPrefVal(0, (float) mar.m_timeout);
  }

  if (rer.Pref[1].is_set) {
    mar.m_check = (int) rer.Pref[1].param;
  } else {
    SetPrefVal(1, (float) mar.m_check);
  }

  if (rer.Pref[2].is_set) {
    mar.from_maxlen = (int) rer.Pref[2].param;
  } else {
    SetPrefVal(2, (float) mar.from_maxlen);
  }

  if (rer.Pref[3].is_set) {
    mar.mail_lines = (int) rer.Pref[3].param;
  } else {
    SetPrefVal(3, (float) mar.mail_lines);
  }

  for (i = 0; i < NUM_OF_ARRAY(ResName); i++) {
    messages[i] = malloc(BUFSIZ);
    ReadRcdata(ResName[i], messages[i], BUFSIZ);
  }


  /**
   * メールチェック用のファイル名の取得。
   **/

  sprintf(m_filename, "%s%s", mar.mailbox, getenv("USER"));

  labelargs[0].value = Mode ? (XtArgVal) ((*messages[1] == '\0') ? mar.no_l : messages[1])
    : (XtArgVal) ((*messages[0] == '\0') ? mar.mail_l : messages[0]);
  labelargs[8].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;
  labelargs[9].value = Mode ? (XtArgVal) 30 : (XtArgVal) 0;

  fromargs[7].value = (XtArgVal) POINT_WIDTH + LABEL_OFFSET;

  MailCheckInterval = mar.m_check * 1000;

  MailTimeout = mar.m_timeout;

  mail[Mode] = XtCreateManagedWidget("mail", msgwinWidgetClass, top[Mode],
				     mailargs, XtNumber(mailargs));

  /**
   * label Widget
   **/

  label = XtCreateManagedWidget("mailLabel", labelWidgetClass, mail[Mode]
				,labelargs, XtNumber(labelargs));

  /**
   * from Widget
   **/

  if (Mode) {
    ok = XtVaCreateManagedWidget("mailOk", commandWidgetClass, mail[Mode]
				 ,XtNfromVert, label
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNvertDistance, 20 * (Mode + 1)
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
    XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, (XtPointer) Mode);
  } else {
    /**
     * fontの大きさを取得し、fromの大きさを決める
     **/
    XtVaGetValues(label, XtNfontSet, &fset, NULL);
    XmbTextExtents(fset, "a", 1, &ink, &log);

    fromargs[10].value = log.width * mar.from_maxlen;
    fromargs[11].value = log.height * (mar.mail_lines) + 20;

    /**
     * +20 = InternalHeight * 2
     **/

    fromargs[1].value = (XtArgVal) label;

    from = XtCreateManagedWidget("mailFrom", labelWidgetClass, mail[Mode]
				 ,fromargs, XtNumber(fromargs));


    /**
     * ok Widget
     **/

    ok = XtVaCreateManagedWidget("mailOk", commandWidgetClass, mail[Mode], XtNfromVert, from
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNvertDistance, 20
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
    XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, (XtPointer) Mode);
    MailTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(top[Mode])
		       ,TimeoutInterval, TimerCheck, (XtPointer) top[Mode]);
  }


  if (!Mode) {
    /**
     * 起動時のメールチェック と mail checkをtimer eventに追加
     **/

    switch (Biff) {
    case YOUBIN:
      YoubinInit();
      XSetIOErrorHandler(Youbin_exit);	/** child process の youbin を殺す **/
      break;
    case POP:
    case APOP:
      CheckPOP3((XtPointer) (w), (XtIntervalId) NULL);
    default:
      CheckMail((XtPointer) (w), (XtIntervalId) NULL);
    }
  }
  for (i = 0; i < 2; i++)
    free(messages[i]);

  return (mail[Mode]);
}

static void GetFromandSubject(char *m_file, char *From)
{
  FILE *fp;
  unsigned char *tmp1, *tmp2, *buf, *head1, *head2;
  int i = 0, length, j;
  int isheader = 0;
  int filehead = 1;

#ifdef EXT_FILTER
  char command[128];		/** フィルタコマンド。mailとpetname用 **/
#endif				/** EXT_FILTER **/

#ifdef PETNAME
  char *from_who, *who, *pname;
#endif				/** PETNAME **/

  *From = '\0';
  tmp2 = malloc(mar.from_maxlen + 2);
  tmp1 = malloc(mar.from_maxlen + 1);

  memset(tmp1, 0, mar.from_maxlen + 1);
  memset(tmp2, 0, mar.from_maxlen + 2);
  buf = malloc(BUFSIZ);
  head1 = malloc(BUFSIZ);
  head2 = malloc(BUFSIZ);

#ifdef PETNAME
  from_who = malloc(BUFSIZ);
  who = malloc(BUFSIZ);
  pname = malloc(BUFSIZ);
#endif

  if ((fp = fopen(m_file, "r")) == NULL) {
    return;
  }
#ifdef EXT_FILTER
  fclose(fp);
  sprintf(command, "%s %s", FilterCommand, m_file);
  fp = popen(command, "r");
#endif				/** EXT_FILTER **/

  while (fgets(buf, BUFSIZ, fp) != NULL && i < mar.mail_lines) {
    if (*buf == '\n' && isheader) {
      /**
       * 現在いるところがヘッダで、空行が来たらここから先はヘッダではない
       **/
      isheader = 0;
    } else {
      if (*buf == '\n') {
	/**
	 * 空行だったらこの後はヘッダの可能性あり
	 **/
	if (fgets(buf, BUFSIZ, fp) == NULL)
	  break;
	sscanf(buf, "%s %s", head1, head2);
	if (!strcmp(head1, "From") && head2) {
	  isheader = 1;
	}
      } else if (filehead) {
	/**
	 * ファイルの先頭なら間違いなくヘッダ
	 **/
	isheader = 1;
	filehead = 0;
      }
    }

    if (isheader) {
      if (!strncmp(buf, "From:", 5) || !strncmp(buf, "Subject:", 8)) {
	strncpy(tmp2, buf, mar.from_maxlen - (mar.from_maxlen % 2));

	length = MIN(strlen(tmp2), mar.from_maxlen);
	tmp2[length - 1] = '\n';
	tmp2[length] = '\0';

#ifdef PETNAME
	if (!strncmp(buf, "From:", 5)) {
	  memset(who, 0, BUFSIZ);
	  sscanf(buf, "%s %s", from_who, who);
	  strcpy(tmp1, buf);

	  for (j = 0; j < strlen(from_who); j++)
	    if (isspace(tmp2[j]))
	      break;

	  strcpy(who, tmp2 + j + 1);

	  if (strchr(who, '@')) {
	    strcpy(pname, who);
	  } else {
	    if (!strchr(buf, '<'))
	      fgets(buf, BUFSIZ, fp);
	    if (strchr(buf, '<') && strchr(buf, '>'))
	      strcpy(pname, strtok(strchr(buf, '<') + 1, ">"));
	  }
	  SearchPetname(tmp1, pname);
	  length = MIN(strlen(tmp1), mar.from_maxlen);
	  tmp1[length - 1] = '\n';
	  tmp1[length] = '\0';
	  strcat(From, tmp1);
	} else {
	  strcat(From, tmp2);
	}
#else
	strcat(From, tmp2);
#endif
	i++;
      }
    }
  }
#ifdef EXT_FILTER
  pclose(fp);
#else
  fclose(fp);
#endif				/** EXT_FILTER **/
  free(tmp2);
  free(tmp1);
  free(buf);
  free(head1);
  free(head2);

#ifdef PETNAME
  free(from_who);
  free(who);
  free(pname);
#endif
}

static void YoubinInit()
{
  char *command;
  struct stat Ystat;
  static FILE *pfp;

  command = malloc(256);

  if (stat(mar.y_command, &Ystat) == -1) {
    fprintf(stderr, "no such youbin command, \"%s\"\n", mar.y_command);
    exit(1);
  }
  sprintf(command, "exec %s -b -s %s", mar.y_command, mar.y_server);

  sprintf(Tmp_dir, "/tmp/xhtmp%s", getenv("USER"));

  mkdir(Tmp_dir, S_IRWXU);

  if (virgine) {
    if ((pfp = popen(command, "r")) == NULL) {
      fprintf(stderr, "can't exec youbin\n");
      perror("popen");
      exit(1);
    }
    virgine = 0;
  }
  YoubinId = XtAppAddInput(XtWidgetToApplicationContext(top[0]),
			   fileno(pfp), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckYoubin, NULL);
  free(command);
}

static void CheckYoubin(Widget w, int *fid, XtInputId * id)
{
  int len;
  char *buf, *tmp1;
  char *cp, *q, *From, *tmp2;
  int mail_size, length;
  static int old_mail_size = 0;
  long date;
  int i = 0, j;

#ifdef PETNAME
  unsigned char *from_who, *who, *pname, *next_ptr, *left_ptr, *right_ptr;
#endif				/** PETNAME **/
#ifdef EXT_FILTER
  FILE *in, *t_file;
  char command[256], t_filename[128];
#endif


  From = malloc(mar.from_maxlen * mar.mail_lines + 1);
  memset(From, 0, mar.from_maxlen * mar.mail_lines + 1);

  tmp2 = malloc(mar.from_maxlen + 1);
  memset(tmp2, 0, mar.from_maxlen + 1);

  buf = malloc(BUFSIZ);
  memset(buf, 0, BUFSIZ);

#ifdef PETNAME
  from_who = malloc(BUFSIZ);
  who = malloc(BUFSIZ);
  pname = malloc(BUFSIZ);
#endif				/** PETNAME **/


  if ((len = read(*fid, buf, BUFSIZ)) == 0) {
    fprintf(stderr, "Youbin died!\n");
  } else if (len == -1) {
    fprintf(stderr, "Can;'t read from Youbin!\n");
  }
  buf[MIN(len, BUFSIZ)] = '\0';

  mail_size = (int) strtol(buf, &cp, 10);
  if (*cp != ' ') {
    fprintf(stderr, "Invalid message from youbin\n");
    goto End;
  }
  if (mail_size == old_mail_size)
    goto End;

  old_mail_size = mail_size;
  date = strtol(cp, &q, 10);
  tmp1 = strtok(q + 1, "\n");

  if (tmp1 == NULL)
    goto End;

  while (tmp1 && i < mar.mail_lines) {
    if (!strncmp(tmp1, "From:", 5) || !strncmp(tmp1, "Subject:", 8)) {
      tmp2[0] = '\0';

#ifdef PETNAME
      if (!strncmp(tmp1, "From:", 5)) {
	next_ptr = tmp1;
	sscanf(tmp1, "%s %s", from_who, who);

	for (j = 0; j < strlen(from_who); j++)
	  if (isspace(tmp1[j]))
	    break;

	strcpy(who, tmp1 + j);

	if (strchr(who, '@') != NULL) {
	  strcpy(pname, who);
	} else {
	  if (strchr(tmp1, '<') == NULL)
	    next_ptr = strtok(NULL, "\n");
	  left_ptr = strchr(next_ptr, '<');
	  right_ptr = strchr(next_ptr, '>');
	  if (left_ptr != NULL && right_ptr != NULL)
	    strcpy(pname, strtok(left_ptr + 1, ">"));
	}
	SearchPetname(tmp2, pname);
      }
#endif

#ifdef EXT_FILTER
      t_filename[0] = '\0';

      strcpy(t_filename, tempnam(Tmp_dir, "xhtmp"));
      if ((t_file = fopen(t_filename, "w")) == NULL) {
	fprintf(stderr, "can't open temporary file,%s\n", t_filename);
	exit(1);
      }
      fprintf(t_file, "%s\n", tmp1);
      fclose(t_file);

      sprintf(command, "%s %s", FilterCommand, t_filename);
      if ((in = popen(command, "r")) == NULL) {
	fprintf(stderr, "no such filter command:%s\n", command);
	exit(1);
      }
      if (*tmp2 == '\0') {
	fgets(tmp2, mar.from_maxlen + 1, in);
      }
      pclose(in);
#else
      strcpy(tmp2, tmp1);
#endif

      length = MIN(mar.from_maxlen, strlen(tmp2));
      tmp2[length - 1] = '\n';
      tmp2[length] = '\0';
      strncat(From, tmp2, mar.from_maxlen);
      i++;
    }
    tmp1 = strtok(NULL, "\n");
  }

  XtVaSetValues(from, XtNlabel, From, NULL);
  if (!MailWindowShown) {
    isMailChecked = 1;
    MailWindowShown = 1;

    XtVaSetValues(mail[0], XtNwindowMode, 0, NULL);
    XtPopup(XtParent(mail[0]), XtGrabNone);

    if (mar.sound_f && UseSound) {
      if (0 == fork()) {
	SoundPlay(mar.sound_f);
	exit(0);
      }
    }
  }
End:
  /**
   * 共通終了処理
   **/
  free(From);
  free(tmp2);
  free(buf);
#ifdef PETNAME
  free(from_who);
  free(who);
  free(pname);
#endif

  return;
}

static int Youbin_exit(Display * disp)
{
  /**
   * kill all the children
   **/
  kill(0, SIGTERM);
  return 0;
}
