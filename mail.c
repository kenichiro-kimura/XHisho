#define _MAIL_GLOBAL
#include "globaldefs.h"
#include "mail.h"
#include "petname.h"
#include "Msgwin.h"
#include "ResEdit.h"
#include <ctype.h>
#include <signal.h>

/**
 * local variable
 **/

#define TIMEOUT_INTERVAL (mar.m_timeout * 1000)
static Widget top[2], local_mail[2], from, label;
static char m_filename[256];
static XtIntervalId MailTimeoutId,MailCheckId;
static int MailCheckInterval;
static const char ResName[][256] = {"newmail", "nomail"};

static XtInputId YoubinId;
static int virgine = 1;
static int isMailChecked;

/**
 * isMailChecked =
 *                 0 .. checked
 *                 1 .. not yet
 *                 2 .. timeout closed(not checked)
 **/


/**
 * function definition
 **/

static void Destroy(Widget, caddr_t, caddr_t);
static int isMail(int*,int);
static void SetPrefVal(int, float);

static void CheckYoubin(Widget, int *, XtInputId *);
static void YoubinInit();
static void GetFromandSubject(char *, char *);
static int Youbin_exit(Display *);
static void AddMailTimeout();
static void RemoveMailTimeout();

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
    (XtPointer) MAIL_SOUND_F
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

  if (!Mode) {
    isMailChecked = 0;
  }

  XtPopdown(top[Mode]);
}

static void TimerCheck(XtPointer cl, XtIntervalId * id)
{
  int i = 0;

  if ((i = IsPopped(mail)) || IsPopped(nomail)) {
      isMailChecked = 2;
      if (i)
	XtPopdown(top[0]);
      else
	XtPopdown(top[1]);
  }
  AddMailTimeout();
}


static int isMail(int* OldSize,int NewSize)
{
  if (NewSize == 0) {
    *OldSize = NewSize;		/** inc されたか メールが来ていないか **/
    return 0;
  } else if (NewSize > *OldSize) {	/** 新しいメールが来た **/
    *OldSize = NewSize;
    return 1;
  } else if (NewSize == *OldSize) {	/** 新しいメールは来ていないが、incしてない **/
    return 2;
  }
  *OldSize = NewSize;		/** NewSize < OldSizeである -> incしている。**/
  return 0;
}

int IsMailChecked(int x)
{
  switch(x){
  case -1:
    AddMailTimeout();
    isMailChecked = 2;
    break;
  case 0:
  case 1:
  case 2:
    isMailChecked = x;
    break;
  default:
    isMailChecked = 1;
  }

  AddMailTimeout();
  return isMailChecked;
}

static int CheckMailTimer(XtPointer cl, XtIntervalId * id)
{
  int num_of_mail;

  num_of_mail = CheckMail(1);

  if(MailCheckId){
    XtRemoveTimeOut(MailCheckId);
    MailCheckId = 0;
  }
    
  MailCheckId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_mail[0])
				,MailCheckInterval
				, (XtTimerCallbackProc) CheckMailTimer
				, (XtPointer) local_mail[0]);
  if(num_of_mail == 0){
    if(HaveSchedule){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
    }
  }

  return (ExistMailNum = num_of_mail);
}

int CheckMail(int mode)
{
  int i;
  char *buf;
  static int OldSize = 0;
  int NewSize = 0;
  struct stat MailStat;
  int num_of_mail = 0;
  FILE* fp;
  char* tmp;
  char* message;

  NewSize = (stat(m_filename, &MailStat) == 0) ? MailStat.st_size : 0;

  buf = (char*)malloc(BUFSIZ);
  tmp = (char*)malloc(BUFSIZ);
  message = (char*)malloc(BUFSIZ);

  if((fp = fopen(m_filename,"r")) != NULL){
    while(fgets(tmp,BUFSIZ,fp) != NULL){
      if(!strncmp(tmp,"From ",strlen("From ")))
	num_of_mail++;
    }
    fclose(fp);
  }

  i = isMail(&OldSize,NewSize);
  ReadRcdata("newmail",tmp,BUFSIZ);
  sprintf(message,tmp,num_of_mail);

  switch (i) {
  case 0:
    break;
  case 1:
    if (!IsPopped(mail) && mode) {
      isMailChecked = 1;
      GetFromandSubject(m_filename, buf);
      XtVaSetValues(from, XtNlabel, buf, NULL);
      XtVaSetValues(label, XtNlabel, message, NULL);
      MailPopup(0,mode);
    }
    break;
  case 2:
    switch (isMailChecked) {
    case 1:
      if (!IsPopped(mail) && mode) {
	XtVaSetValues(label, XtNlabel, message, NULL);
	/*	MailPopup(0,mode);*/
	MailPopup(0,0);
      }
      break;
    case 2:
      isMailChecked = 1;
      break;
    }

    break;
  }

  free(buf);
  free(tmp);
  free(message);

  if(num_of_mail == 0){
    if(HaveSchedule){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
    }
  }

  if(mode == 2)
    RemoveMailTimeout();

  return (ExistMailNum = num_of_mail);
}

static int CheckPOP3Timer(XtPointer cl, XtIntervalId * id)
{
  int num_of_mail;

  num_of_mail = CheckPOP3(2);

  if (MailCheckInterval < 60 * 1000)
    MailCheckInterval = 60 * 1000;

  if(MailCheckId){
    XtRemoveTimeOut(MailCheckId);
    MailCheckId = 0;
  }

  MailCheckId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_mail[0])
				,MailCheckInterval
				, (XtTimerCallbackProc) CheckPOP3Timer
				, (XtPointer) local_mail[0]);
  if(num_of_mail == 0){
    if(HaveSchedule){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
    }
  }

  return (ExistMailNum = num_of_mail);
}

int CheckPOP3(int mode)
{
  int ret_value = 0;
  char *buf;
  char *message;
  char *tmp;

  buf = (char*)malloc(BUFSIZ);
  message = (char*)malloc(BUFSIZ);
  tmp = (char*)malloc(BUFSIZ);

  switch (Biff) {
  case APOP:
    ret_value = pop3(APOP_AUTH, mar.p_server, buf);
    break;
  case IMAP:
    ret_value = pop3(IMAP_AUTH, mar.p_server, buf);
    break;
  case POP:
    ret_value = pop3(POP_AUTH, mar.p_server, buf);
    break;
  default:
  }

  ReadRcdata("newmail",tmp,BUFSIZ);
  if(*tmp == '\0')
    sprintf(message,mar.mail_l,ret_value);
  else
    sprintf(message,tmp,ret_value);

  if (ret_value > 0 && mode) {
    XtVaSetValues(from, XtNlabel, buf, NULL);
    XtVaSetValues(label, XtNlabel, message, NULL);
    MailPopup(0,mode);
  }

  free(buf);
  free(message);
  free(tmp);

  if(ret_value == 0){
    if(HaveSchedule){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
    }
  }

  if(mode == 2)
    RemoveMailTimeout();

  return (ExistMailNum = ret_value);
}

static int CheckYoubinNowTimer(XtPointer cl, XtIntervalId * id){
  int ret_value;

  ret_value = CheckYoubinNow(0);
  if(ret_value < 0) ret_value = 0;

  if(MailCheckId){
    XtRemoveTimeOut(MailCheckId);
    MailCheckId = 0;
  }
    
  MailCheckId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_mail[0])
				,MailCheckInterval
				, (XtTimerCallbackProc) CheckYoubinNowTimer
				, (XtPointer) local_mail[0]);

  /*  printf("r:%d,S:%d,E:%d\n",ret_value,HaveSchedule,ExistMailNum);*/

  if(ret_value == 0){
    if(HaveSchedule){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
    }
  } else {
    if(HaveSchedule && ExistMailNum == ret_value){
      XtVaSetValues(xhisho, XtNanimType, SCHEDULE, NULL);
    } else {
      XtVaSetValues(xhisho, XtNanimType, MAIL, NULL);
    }
  }


  return (ExistMailNum = ret_value);
}
  
int CheckYoubinNow(int mode){
  /*
   * mode = 0: no popup(for timer check)
   *      = 1: popup with sound(for youbin check)
   *      = 2: popup with no sound(for right click checker)
   */
  int num_of_mail = 0;
  FILE *fp;
  char* tmp;
  char* message;

  tmp = (char*)malloc(BUFSIZ);
  message = (char*)malloc(BUFSIZ);

  if((fp = fopen(YoubinFile,"r")) != NULL){
    while(fgets(tmp,BUFSIZ,fp) != NULL)
      num_of_mail++;

    fclose(fp);
  } else {
    ExistMailNum = 0;
    return -1;
  }

  ReadRcdata("newmail",tmp,BUFSIZ);
  if(*tmp == '\0')
    sprintf(message,mar.mail_l,num_of_mail);
  else
    sprintf(message,tmp,num_of_mail);

  if(num_of_mail > 0 && mode){
    XtVaSetValues(label, XtNlabel, message, NULL);
    MailPopup(0,mode);
  }

  free(tmp);
  free(message);

  if(mode == 2)
    RemoveMailTimeout();

  return num_of_mail;
}

Widget CreateMailAlert(Widget w, int Mode)
{

  /**
   * Mode = 0 ... 普通の Mail Alert
   * Mode = 1 ... No Mail
   **/

  Widget ok;
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
    messages[i] = (char*)malloc(BUFSIZ);
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

  local_mail[Mode] = XtCreateManagedWidget("mail", msgwinWidgetClass, top[Mode],
				     mailargs, XtNumber(mailargs));

  /**
   * label Widget
   **/

  label = XtCreateManagedWidget("mailLabel", labelWidgetClass, local_mail[Mode]
				,labelargs, XtNumber(labelargs));

  /**
   * from Widget
   **/

  if (Mode) {
    ok = XtVaCreateManagedWidget("mailOk", commandWidgetClass, local_mail[Mode]
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
    fromargs[0].value = (XtArgVal) " ";

    from = XtCreateManagedWidget("mailFrom", labelWidgetClass, local_mail[Mode]
				 ,fromargs, XtNumber(fromargs));


    /**
     * ok Widget
     **/

    ok = XtVaCreateManagedWidget("mailOk", commandWidgetClass, local_mail[Mode], XtNfromVert, from
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				 ,XtNlabel, "OK"
				 ,XtNvertDistance, 20
				 ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				 ,XtNinternalHeight, FONT_OFFSET, NULL);
    XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, (XtPointer) Mode);
    AddMailTimeout();
  }


  if (Mode == 0) {
    /**
     * 起動時のメールチェック と mail checkをtimer eventに追加
     **/

    switch (Biff) {
    case YOUBIN:
      YoubinInit();
      CheckYoubinNowTimer((XtPointer) (w), (XtIntervalId) NULL);
      XSetIOErrorHandler(Youbin_exit);	/** child process の youbin を殺す **/
      break;
    case POP:
    case APOP:
    case IMAP:
      CheckPOP3Timer((XtPointer) (w), (XtIntervalId) NULL);
      break;
    default:
      CheckMailTimer((XtPointer) (w), (XtIntervalId) NULL);
    }
  }
  for (i = 0; i < 2; i++)
    free(messages[i]);

  return (local_mail[Mode]);
}

static void GetFromandSubject(char *m_file, char *From)
{
  FILE *fp;
  unsigned char *tmp1, *tmp2, *buf, *head1, *head2;
  int i = 0, length;
  int isheader = 0;
  int filehead = 1;

#ifdef EXT_FILTER
  char command[128];		/** フィルタコマンド。mailとpetname用 **/
#endif				/** EXT_FILTER **/

#ifdef PETNAME
  char *who, *pname;
#endif				/** PETNAME **/

  *From = '\0';
  tmp2 = (char*)malloc(BUFSIZ);
  tmp1 = (char*)malloc(BUFSIZ);

  buf = (char*)malloc(BUFSIZ);
  head1 = (char*)malloc(BUFSIZ);
  head2 = (char*)malloc(BUFSIZ);

#ifdef PETNAME
  who = (char*)malloc(BUFSIZ);
  pname = (char*)malloc(BUFSIZ);
#endif

#ifdef EXT_FILTER
  sprintf(command, "%s %s", FilterCommand, m_file);
  fp = popen(command, "r");
#else
  if ((fp = fopen(m_file, "r")) == NULL) {
    return;
  }
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
	tmp2[mar.from_maxlen - (mar.from_maxlen % 2)] = '\0';

	length = MIN(strlen(tmp2), mar.from_maxlen);
	tmp2[length - 1] = '\n';
	tmp2[length] = '\0';

#ifdef PETNAME
	if (!strncmp(buf, "From:", 5)) {
	  strcpy(tmp1, buf);

	  strcpy(who, buf + 6);

	  if (strchr(who, '@')) {
	    strcpy(pname, who);
	  } else {
	    if (!strchr(buf, '<'))
	      fgets(buf, BUFSIZ, fp);
	    if (strchr(buf, '<') && strchr(buf, '>')){
	      strncpy(pname, strchr(buf, '<') + 1,
		      strchr(buf, '>') - strchr(buf, '<') - 1);
	      pname[strchr(buf, '>') - strchr(buf, '<') - 1] = '\0';
	    } else {
	      pname[0] = '\0';
	    }
	  }
	  SearchPetname(tmp1, pname);
	  strncat(From, tmp1, MIN(strlen(tmp1), mar.from_maxlen));
	} else {
	  strncat(From, tmp2,MIN(strlen(tmp2), mar.from_maxlen));
	}
#else
	strncat(From, tmp2, MIN(strlen(tmp2), mar.from_maxlen));
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
  free(who);
  free(pname);
#endif
}

static void YoubinInit()
{
  char *command;
  struct stat Ystat;
  int youbin_pfp[2];

  command = (char*)malloc(256);

  if (stat(mar.y_command, &Ystat) == -1) {
    fprintf(stderr, "no such youbin command, \"%s\"\n", mar.y_command);
    exit(1);
  }

  sprintf(YoubinFile, "/tmp/xhtmp%s-%d/xhyoubin", getenv("USER"),getpid());

  if (virgine) {
    if(pipe(youbin_pfp)){
      perror("youbin pipe");
      exit(1);
    }
    if((youbin_pid = fork()) < 0){
      perror("youbin fork");
      exit(1);
    }
    if(youbin_pid == 0){
      close(1);
      dup(youbin_pfp[1]);
      close(youbin_pfp[1]);
      execl(mar.y_command,"youbin","-b","-s",mar.y_server,NULL);
      exit(1);
    }
    close(youbin_pfp[1]);
    virgine = 0;
  }
  if ((youbin_fd = fdopen(youbin_pfp[0], "r")) == NULL) {
    perror("youbin fdopen");
    exit(1);
  }
 
  YoubinId = XtAppAddInput(XtWidgetToApplicationContext(top[0]),
			   fileno(youbin_fd), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckYoubin, NULL);
  free(command);
}

static void CheckYoubin(Widget w, int *fid, XtInputId * id)
{
  int len;
  char *buf,*tmp, *message;
  char *tmp1;

  char *cp, *q, *From, *tmp2,*ch_ptr;
  int mail_size;
  static int old_mail_size = 0;
  long date;
  int i = 0, j;
  int isFrom = 0, isSubject = 0;
  int isHeader;
  int broken_from;

#ifdef PETNAME
  unsigned char *from_who, *who, *pname, *next_ptr, *left_ptr, *right_ptr;
#endif				/** PETNAME **/
#ifdef EXT_FILTER
  FILE *in, *t_file;
  char command[256], t_filename[128];
#endif

  From = (char*)malloc(BUFSIZ * 5);
  *From = '\0';
  tmp2 = (char*)malloc(BUFSIZ);
  buf = (char*)malloc(BUFSIZ);
  tmp = (char*)malloc(BUFSIZ);
  message = (char*)malloc(BUFSIZ);

#ifdef PETNAME
  from_who = (char*)malloc(BUFSIZ);
  who = (char*)malloc(BUFSIZ);
  pname = (char*)malloc(BUFSIZ);
#endif				/** PETNAME **/


  if ((len = read(*fid, buf, BUFSIZ)) == 0) {
    fprintf(stderr, "Youbin died!\n");
  } else if (len == -1) {
    fprintf(stderr, "Can't read from Youbin!\n");
  }
  buf[MIN(len, BUFSIZ) - 1] = '\0';

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

  isHeader = 1;
  while (tmp1 != NULL && i < mar.mail_lines && isHeader) {
    if(*tmp1 == '\n'){
      isHeader = 0;
      continue;
    }
    broken_from = 0;
    ch_ptr = tmp1;
    while(*ch_ptr != '\0' && 
	  (isdigit((unsigned char)*ch_ptr) 
	   || isspace((unsigned char)*ch_ptr))){
      ch_ptr++;
      tmp1++;
    }
    if (!strncmp(ch_ptr, "From:", 5) || !strncmp(ch_ptr, "Subject:", 8)) {
      *tmp2 = '\0';

#ifdef PETNAME
      if (!strncmp(ch_ptr, "From:", 5) && !isFrom) {
	next_ptr = ch_ptr;
	for (j = 0; j < strlen(ch_ptr) - strlen("From:") - 1; j++)
	  if (!isspace((unsigned char)*(ch_ptr + j + strlen("From:"))))
	    break;

	strcpy(who, ch_ptr + j + strlen("From:"));

	if (strchr(who, '@') != NULL && strchr(who,'<') == NULL){
	  strcpy(pname, who);
	} else {
	  if (strchr(ch_ptr, '<') == NULL)
	    next_ptr = strtok(NULL, "\n");
	  if(next_ptr != NULL){
	    left_ptr = strchr(next_ptr, '<');
	    right_ptr = strchr(next_ptr, '>');
	    if (left_ptr != NULL && right_ptr != NULL){
	      strncpy(pname, left_ptr + 1,
		      MIN(right_ptr - left_ptr - 1,BUFSIZ- 1));
	      pname[MIN(right_ptr - left_ptr - 1,BUFSIZ- 1)] = '\0';
	    } else {
	      *pname = '\0';
	      broken_from = 1;
	    }
	  } else {
	    *pname = '\0';
	  }
	}
	SearchPetname(ch_ptr, pname);
      }
#endif

#ifdef EXT_FILTER
      strcpy(t_filename, tempnam(Tmp_dir, "xhtmp"));
      if ((t_file = fopen(t_filename, "w")) == NULL) {
	fprintf(stderr, "can't open temporary file,%s\n", t_filename);
      } else {
	fprintf(t_file, "%s\n", ch_ptr);
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
	unlink(t_filename);
      }
#else
      strcpy(tmp2, ch_ptr);
#endif

      if((!strncmp(tmp2, "Subject:", 8) && !isSubject) || 
	 (!strncmp(tmp2, "From:", 5) && !isFrom)){
	if(*tmp2 == 'S') isSubject = 1;
	if(*tmp2 == 'F') isFrom = 1;
	strncat(From, tmp2, MIN(mar.from_maxlen - 1, strlen(tmp2)));
	if(From[strlen(From) - 1] != '\n')
	  strcat(From,"\n");
	i++;
      }
    }
    if(broken_from)
      tmp1 = next_ptr;
    else
      tmp1 = strtok(NULL, "\n");
  }

  XtVaSetValues(from, XtNlabel, From, NULL);
  i = CheckYoubinNow(0);
  if(i < 1) i = 1;

  ExistMailNum = i;

  ReadRcdata("newmail",tmp,BUFSIZ);
  if(*tmp == '\0')
    sprintf(message,mar.mail_l,i);
  else
    sprintf(message,tmp,i);

  if(*From != '\0'){
    XtVaSetValues(xhisho, XtNanimType, MAIL, NULL);
    XtVaSetValues(label, XtNlabel, message, NULL);
    MailPopup(0,1);
  }

End:
  /**
   * 共通終了処理
   **/
  free(From);
  free(tmp2);
  free(buf);
  free(tmp);
  free(message);
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
  unlink(YoubinFile);
  return 0;
}

void MailPopup(int mode,int sound){
  if(mode != 0 && mode != 1) return;

  XtVaSetValues(local_mail[mode], XtNwindowMode, 0, NULL);
  if(mode){
    if(!HaveSchedule)
      XtVaSetValues(xhisho, XtNanimType, USUAL, NULL);
  } else {
    XtVaSetValues(xhisho, XtNanimType, MAIL, NULL);
  }
  AddMailTimeout();

  XtPopup(XtParent(local_mail[mode]), XtGrabNone);
  if (mar.sound_f  && UseSound && mode == 0 && sound == 1) {
    SoundPlay(mar.sound_f);
  }
}

static void AddMailTimeout(){
  if(MailTimeoutId){
    XtRemoveTimeOut(MailTimeoutId);
  }
  if(top[0] != NULL)
    MailTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(top[0])
				    ,TIMEOUT_INTERVAL, TimerCheck
				    , (XtPointer) top[0]);
}

static void RemoveMailTimeout(){
  if(MailTimeoutId){
    XtRemoveTimeOut(MailTimeoutId);
    MailTimeoutId = 0;
  }
}
