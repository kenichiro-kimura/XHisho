#define _OPTION_GLOBAL
#include "globaldefs.h"
#include "mail.h"
#include "petname.h"
#include "Msgwin.h"
#include "ResEdit.h"
#include "option.h"
#include <ctype.h>
#include <signal.h>

static Widget top,label,local_option;
#ifdef USE_UNYUU
static Widget utop,ulabel,ulocal_option;
#endif
static XtInputId OptionId;
static int virgine = 1;
static XtIntervalId OptionTimeoutId = 0;
#ifdef USE_KAWARI
static XtIntervalId KAWARITimeoutId = 0;
#endif
static messageBuffer mbuf,mdest;

static void Destroy(Widget,XEvent *, String *, unsigned int *);
static void CommandInit();
static void CheckOption(Widget, int *, XtInputId *);
static int Option_exit(Display *);
static char* or2string(char*);
static void ORParser(char*);
static char* nstrncpy(char*,const char*,size_t);
static messageStack* messageStack_new(char*);
static messageStack* messageStack_pop(messageStack**);
static void messageStack_push(messageStack**,char*);
static void ChangeBadKanjiCode(char *);
static void ClearMessage(Widget);
static void InsertMessage(Widget,char*,int);
static void _InsertMessage(XtPointer,XtIntervalId*);
static void InsertReturn(char*,char*,int,int);
static void RemoveStr(char*,char*);
static void AddBuffer(messageBuffer*,char*);
static void GetBuffer(messageBuffer*,char*);
static void HeadOfBuffer(messageBuffer*,char*);
static void _GetBuffer(messageBuffer*,char*,int);
static void SakuraParser(char*);
extern char* RandomMessage(char*);
static void SJIS2EUC(char*);

static XtActionsRec actionTable[] = {
  {"Destroy", Destroy},
};

static XtResource resources[] = {
  {
    XtNoptionCommand,
    XtCOptionCommand,
    XtRString,
    sizeof(String),
    XtOffsetOf(OptionRes, o_command),
    XtRImmediate,
    (XtPointer) ""
  },
  {
    XtNoptionWidth,
    XtCOptionWidth,
    XtRDimension,
    sizeof(Dimension),
    XtOffsetOf(OptionRes, width),
    XtRImmediate,
    (XtPointer)300
  },    
  {
    XtNoptionHeight,
    XtCOptionHeight,
    XtRDimension,
    sizeof(Dimension),
    XtOffsetOf(OptionRes, height),
    XtRImmediate,
    (XtPointer)200
  },    
  {
    XtNoptionTimeout,
    XtCOptionTimeout,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OptionRes, timeout),
    XtRImmediate,
    (XtPointer)5
  },    
  {
    XtNuXOffset,
    XtCUXOffset,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OptionRes, uxoff),
    XtRImmediate,
    (XtPointer)300
  },    
  {
    XtNuYOffset,
    XtCUYOffset,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OptionRes, uyoff),
    XtRImmediate,
    (XtPointer)200
  },    
  {
    XtNmessageWait,
    XtCMessageWait,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OptionRes, m_wait),
    XtRImmediate,
    (XtPointer)0
  },    
  {
    XtNkawariWait,
    XtCKawariWait,
    XtRInt,
    sizeof(int),
    XtOffsetOf(OptionRes, k_wait),
    XtRImmediate,
    (XtPointer)60
  },    
  {
    XtNkawariDir,
    XtCKawariDir,
    XtRString,
    sizeof(String),
    XtOffsetOf(OptionRes, kawari_dir),
    XtRImmediate,
    (XtPointer)"xhisho"
  },    
};

static void Destroy(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  if(OptionTimeoutId){
    XtRemoveTimeOut(OptionTimeoutId);
    OptionTimeoutId = 0;
  }
  XtPopdown(top);
  ClearMessage(label);
#ifdef USE_UNYUU
  XtPopdown(utop);
  ClearMessage(ulabel);
#endif
}


Widget CreateOptionWindow(Widget w){
  static XtPopdownIDRec pdrec;
  XtTranslations trans_table;
  static int virgine = 1;

  static char defaultTranslations[] = "<Btn1Down> : Destroy()\n\
                                       <Btn2Down> : Destroy()\n\
                                       <Btn3Down>: Destroy()";

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

  if(virgine){
    mbuf.buffer = (unsigned char*)malloc(BUFSIZ * 10);
    mbuf.size = BUFSIZ * 10;
    *mbuf.buffer = '\0';

    mdest.buffer = (unsigned char*)malloc(BUFSIZ);
    mdest.size = BUFSIZ;
    *mdest.buffer = '\0';

    virgine = 0;
  }

  top = XtVaCreatePopupShell("OptionWindow", transientShellWidgetClass
			     ,w,NULL);
  XtGetApplicationResources(top, &opr, resources, XtNumber(resources), NULL, 0);
  local_option = XtVaCreateManagedWidget("option", msgwinWidgetClass, top
  				 ,NULL);

  label = XtVaCreateManagedWidget("optionLabel", asciiTextWidgetClass
				  ,local_option
				  ,XtNvertDistance,10
				  ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				  ,XtNvertDistance, 30
				  ,XtNborderWidth,0
				  ,XtNwidth,opr.width
				  ,XtNheight,opr.height
				  ,XtNleft,XtChainLeft
				  ,XtNright,XtChainRight
				  ,XtNdisplayCaret,False
				  ,XtNsensitive,True
				  ,XtNjustify,XtJustifyLeft
				  ,XtNautoFill,True
				  ,NULL);

  XtAppAddActions(XtWidgetToApplicationContext(label)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(label,trans_table);

  XtAppAddActions(XtWidgetToApplicationContext(top)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(top,trans_table);

  XtAppAddActions(XtWidgetToApplicationContext(local_option)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(local_option,trans_table);

#ifdef USE_UNYUU
  utop = XtVaCreatePopupShell("UOptionWindow", transientShellWidgetClass
			     ,w,NULL);
  XtGetApplicationResources(utop, &uopr, resources, XtNumber(resources), NULL, 0);
  ulocal_option = XtVaCreateManagedWidget("option", msgwinWidgetClass, utop
					  ,XtNxoff,uopr.uxoff
					  ,XtNyoff,uopr.uyoff
					  ,NULL);
  ulabel = XtVaCreateManagedWidget("optionLabel", asciiTextWidgetClass
				   ,ulocal_option
				   ,XtNvertDistance,10
				   ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
				   ,XtNvertDistance, 30
				   ,XtNborderWidth,0
				   ,XtNwidth,uopr.width
				   ,XtNheight,uopr.height
				   ,XtNleft,XtChainLeft
				   ,XtNright,XtChainRight
				   ,XtNdisplayCaret,False
				   ,XtNsensitive,True
				   ,XtNjustify,XtJustifyLeft
				   ,XtNautoFill,True
				   ,NULL);

  XtAppAddActions(XtWidgetToApplicationContext(ulabel)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(ulabel,trans_table);

  XtAppAddActions(XtWidgetToApplicationContext(utop)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(utop,trans_table);

  XtAppAddActions(XtWidgetToApplicationContext(ulocal_option)
		  ,actionTable, XtNumber(actionTable));
  trans_table = XtParseTranslationTable(defaultTranslations);
  XtOverrideTranslations(ulocal_option,trans_table);

#endif

  CommandInit();

  if(opr.m_wait)
    XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				      , opr.m_wait * 10
				      , (XtTimerCallbackProc) _InsertMessage
				      , NULL);
  return(local_option);
}

static void CommandInit()
{
#ifdef USE_KAWARI
  CheckOption(top,NULL,NULL);
#else
  if(strlen(opr.o_command) < 1) return;

  if (virgine) {
    if ((option_fd = popen(opr.o_command,"r")) == NULL){
      fprintf(stderr, "no such option command, \"%s\"\n", opr.o_command);
      exit(1);
    }
    virgine = 0;
  }

  OptionId = XtAppAddInput(XtWidgetToApplicationContext(top),
			   fileno(option_fd), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckOption, NULL);
  XSetIOErrorHandler(Option_exit);	/** child process¤ò»¦¤¹ **/
#endif
}

static void CheckOption(Widget w, int *fid, XtInputId * id)
{
  static unsigned char *message_buffer;
#ifdef USE_UNYUU
  static unsigned char *umessage_buffer;
  static int last_message;
#endif
  static unsigned char *_buffer;
  static unsigned char *buffer;
  static int x = 0;
  int len;
  int message_buffer_size = BUFSIZ * 20;
  unsigned char* chr_ptr;
  unsigned char* next_ptr;
  unsigned char* message_ptr;
  unsigned char* c_ptr;
  static int is_end = 0;
  XFontSet fset;
  XRectangle ink, log;
  int max_len;
  Dimension width;
  int sakura;
  int chr_length,dword,pos,mpos;
  XawTextPosition current,last;
  XawTextBlock textblock;
  int cg_num = -1;
  int u_cg_num = -1;
  unsigned char str_num[128];

#ifdef EXT_FILTER
  char command[128];
  char t_filename[BUFSIZ];
  char d_buffer[BUFSIZ * 5];
  FILE* t_file;
  FILE* in;
#endif			

  if(OptionTimeoutId){
    XtRemoveTimeOut(OptionTimeoutId);
    OptionTimeoutId = 0;
  }

  if(x == 0){
    message_buffer = (char *)malloc(message_buffer_size);
#ifdef USE_UNYUU
    umessage_buffer = (char *)malloc(message_buffer_size);
#endif
    _buffer = (char *)malloc(message_buffer_size);
    buffer = (char *)malloc(message_buffer_size);
  }
  x = 1;

  memset(message_buffer,'\0',message_buffer_size);
#ifdef USE_UNYUU
  memset(umessage_buffer,'\0',message_buffer_size);
#endif
  memset(_buffer,'\0',message_buffer_size);
  memset(buffer,'\0',message_buffer_size);

#ifndef USE_KAWARI
  if ((len = read(*fid,_buffer,message_buffer_size)) == 0) {
    XtRemoveInput(OptionId);
    OptionId = 0;
    fprintf(stderr, "option command died!\n");
    XtDestroyWidget(optionwin);
    optionwin = CreateOptionWindow(XtParent(xhisho));
    return;
  } else if (len == -1) {
    fprintf(stderr, "Can't read from option command!\n");
  }

  if(len == 0) return;
  _buffer[len] = '\0';
#endif

  if(is_end && opr.m_wait == 0){
    ClearMessage(label);
#ifdef USE_UNYUU
    ClearMessage(ulabel);
#endif
    is_end = 0;
  }

#ifdef USE_KAWARI
  if(KAWARITimeoutId){
    XtRemoveTimeOut(KAWARITimeoutId);
    KAWARITimeoutId = 0;
  }
  ClearMessage(label);
#ifdef USE_UNYUU
  ClearMessage(ulabel);
#endif
  message_ptr = RandomMessage(opr.kawari_dir);
  strcpy(_buffer,message_ptr);
  free(message_ptr);
#endif

  SJIS2EUC(_buffer);
  strcpy(buffer, _buffer);

#if 0
#ifdef EXT_FILTER
  len = 0;
  strcpy(t_filename, tempnam(Tmp_dir, "xhtmp"));
  if ((t_file = fopen(t_filename, "w")) == NULL) {
    fprintf(stderr, "can't open temporary file,%s\n", t_filename);
  } else {
    fprintf(t_file, "%s", _buffer);
    fclose(t_file);

    sprintf(command, "%s %s", FilterCommand, t_filename);
    if ((in = popen(command, "r")) == NULL) {
      fprintf(stderr, "no such filter command:%s\n", command);
      exit(1);
    }

    while((fgets(d_buffer, BUFSIZ * 5, in)) != NULL){
      strcat(buffer,d_buffer);
      if((len += strlen(d_buffer)) >= message_buffer_size) break;
    }
    pclose(in);
    unlink(t_filename);
  }
#else
  strcpy(buffer, _buffer);
#endif
#endif

  /* here is script decoder .. */

#ifdef DEBUG
  printf("#%s\n",buffer);
#endif

  while((chr_ptr = strstr(buffer,"\\n")) != NULL){
    strcpy(_buffer,chr_ptr +2);
    sprintf(chr_ptr,"\n%s",_buffer);
  }

  ChangeBadKanjiCode(buffer);
#ifdef USE_KAWARI
  SakuraParser(buffer);
#endif

  XtVaGetValues(label, XtNfontSet, &fset, XtNwidth,&width,NULL);
  XmbTextExtents(fset, "a", 1, &ink, &log);
  max_len = width / log.width - 2;


  chr_ptr = buffer;
  next_ptr = strtok(buffer,"\n");
  while(*chr_ptr){
    if(next_ptr == NULL){
#ifdef USE_UNYUU
      if(strstr(chr_ptr,"sakura:")){
	strcpy(message_buffer,chr_ptr);
	message_ptr = message_buffer;
	last_message = SAKURA;
      } else if(strstr(chr_ptr,"unyuu:")){
	strcpy(message_buffer,chr_ptr);
	message_ptr = message_buffer;
	last_message = UNYUU;
      } else {
	if(last_message == SAKURA){
	  strcpy(message_buffer,chr_ptr);
	  message_ptr = message_buffer;
	} else {
	  strcpy(umessage_buffer,chr_ptr);
	  message_ptr = umessage_buffer;
	}
      }
#else
      strcpy(message_buffer,chr_ptr);
      message_ptr = message_buffer;      
#endif
    } else {
#ifdef USE_UNYUU
      if(strstr(chr_ptr,"sakura:")){
	strcpy(message_buffer,chr_ptr);
	message_ptr = message_buffer;
	last_message = SAKURA;
      } else if(strstr(chr_ptr,"unyuu:")){
	strcpy(message_buffer,chr_ptr);
	message_ptr = message_buffer;
	last_message = UNYUU;
      } else {
	if(last_message == SAKURA){
	  strncpy(message_buffer,chr_ptr,next_ptr - chr_ptr);
	  message_ptr = message_buffer;
	} else {
	  strncpy(umessage_buffer,chr_ptr,next_ptr - chr_ptr);
	  message_ptr = umessage_buffer;
	}
      }
#else
      strncpy(message_buffer,chr_ptr,next_ptr - chr_ptr);
      message_ptr = message_buffer;
#endif
      message_ptr[next_ptr - chr_ptr] = '\0';
    }
    
    /* surface changer */

    while((chr_ptr = strstr(message_ptr,"(Surface:")) != NULL){
      if(*(chr_ptr - 2) == 'a')
	sakura = 1;
      else 
	sakura = 0;

      c_ptr = chr_ptr + strlen("(Surface:");
      while(isdigit((unsigned char)(*c_ptr))) c_ptr++;
      strncpy(str_num,chr_ptr + strlen("(Surface:")
	      ,c_ptr - chr_ptr - strlen("(Surface:"));
      if(sakura)
	cg_num = atoi(str_num);
      else 
	u_cg_num = atoi(str_num) + 10;

      if(opr.m_wait)
	sprintf(_buffer,"\\%d%s",atoi(str_num),c_ptr + 1);
      else
	strcpy(_buffer,c_ptr + 1);
      strcpy(strstr(message_ptr,"(Surface:"),_buffer);
    }

    if(c_ptr = strstr(message_ptr,"\\e")){
      is_end = 1;
      if(opr.m_wait == 0){
	*c_ptr = '\n';
	*(c_ptr + 1) = '\0';
      } else {
	*(c_ptr + 2) = '\0';
      }
    }

    ORParser(message_ptr);
    strcpy(_buffer,message_ptr);
#ifdef USE_UNYUU
    if(last_message == SAKURA)
      RemoveStr(_buffer,"sakura:");
    else
      RemoveStr(_buffer,"unyuu:");
#endif
    *message_ptr = '\0';
    InsertReturn(message_ptr,_buffer,max_len,message_buffer_size);

    if((cg_num != -1 || u_cg_num != -1) && opr.m_wait == 0)
      XtVaSetValues(xhisho,XtNforceCG,True,XtNcgNumber,cg_num
		    ,XtNucgNumber,u_cg_num
		    ,NULL);
    cg_num = u_cg_num = -1;
#ifdef USE_UNYUU
    if(last_message == SAKURA)
      InsertMessage(label,message_ptr,SAKURA);
    else 
      InsertMessage(ulabel,message_ptr,UNYUU);
#else
    InsertMessage(label,message_ptr,SAKURA);
#endif

    if((chr_ptr = next_ptr) == NULL) break;
    next_ptr = strtok(NULL,"\n");
    XFlush(XtDisplay(label));
  }

  if(is_end && opr.timeout > 0 && opr.m_wait == 0){
    OptionTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				      , opr.timeout * 1000
				      , (XtTimerCallbackProc) Destroy
				      , NULL);
  }

#ifdef USE_KAWARI
  if(opr.k_wait == 0)
    opr.k_wait = 60;

  KAWARITimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				    , opr.k_wait * 1000
				    , (XtTimerCallbackProc) CheckOption
				    , NULL
				    );
#endif
}

static int Option_exit(Display * disp)
{
  /**
   * kill all the children
   **/
  kill(0, SIGTERM);
  return 0;
}

void ORParser(char* in)
{
  /*
    parse random scripts
  */

  char* in_buffer;
  char* tmp_buffer;
  char* tmp_string;
  char* out;
  char* chr_ptr;
  char* next_ptr;
  char* or_ptr;
  int len;

  len = strlen(in) + 1;
  in_buffer = (char*)malloc(len);
  tmp_buffer = (char*)malloc(len);
  tmp_string = (char*)malloc(len);
  memset(in_buffer,'\0',len);

  strcpy(in_buffer,in);

  chr_ptr = strchr(in_buffer,'(');
  while(chr_ptr != NULL && (next_ptr = strchr(in_buffer,')'))!= NULL){
    char* back_ptr;
    memset(tmp_buffer,'\0',len);
    memset(tmp_string,'\0',len);
    if(chr_ptr > next_ptr) break;
    strncpy(tmp_buffer,in_buffer,chr_ptr - in_buffer);
    strncpy(tmp_string,in_buffer,next_ptr - in_buffer);
    back_ptr = strrchr(tmp_string,'(');
    strncat(tmp_buffer,strchr(tmp_string,'(')
	    ,back_ptr - strchr(tmp_string,'('));
    or_ptr = or2string(back_ptr);
    strcat(tmp_buffer,or_ptr);
    free(or_ptr);
    strcat(tmp_buffer,next_ptr+1);
    strcpy(in_buffer,tmp_buffer);
    chr_ptr = strchr(in_buffer,'(');
  }

  strcpy(in,in_buffer);
  free(tmp_string);
  free(tmp_buffer);
  free(in_buffer);
}
      
static char* or2string(char* in)
{
  /*
    translate OR-string to string
    '(aa|bb)' -> 'aa' in 50%, 'bb' in 50%
  */

  char* out;
  char* buffer;
  char* chr_ptr;
  char* in_ptr;
  int num_of_item;
  int pos;
  messageStack* top;
  messageStack* item;

  out = (char*)malloc(strlen(in) + 1);
  buffer = (char*)malloc(strlen(in) + 1);
  memset(out,'\0',strlen(in) + 1);
  num_of_item = pos = 0;

  if((chr_ptr = strchr(in,'(')) != NULL)
    in_ptr = chr_ptr + 1;
  else 
    in_ptr = in;

  if((chr_ptr = strchr(in_ptr,'|')) == NULL){
    free(buffer);
    return out;
  }

  top = NULL;

  while((chr_ptr = strchr(in_ptr,'|')) != NULL){
    memset(buffer,'\0',strlen(in) + 1);
    nstrncpy(buffer,in_ptr,chr_ptr - in_ptr);
    in_ptr = chr_ptr + 1;
    messageStack_push(&top,buffer);
    num_of_item++;
  }

  messageStack_push(&top,in_ptr);
  num_of_item++;
  
  pos = rand() % num_of_item;
  do{
    if((item = messageStack_pop(&top)) == NULL) break;

    if(pos == 0)
      strcpy(out,item->message);

    free(item->message);
    free(item);
    pos--;
  } while(top);

  free(buffer);
  return out;
}

static char* nstrncpy(char* dst, const char* src, size_t n){
  /*
    strncpy, but not '\n'
  */

  if (n != 0) {
    register char *d = dst;
    register const char *s = src;

    do {
      if(*s == '\n') s++;
      if ((*d++ = *s++) == 0) {
	/* NUL pad the remaining n-1 bytes */
	while (--n != 0)
	  *d++ = 0;
	break;
      }
    } while (--n != 0);
  }
  return (dst);
}

static messageStack* messageStack_new(char* s)
{
  messageStack* r;

  r = (messageStack*)malloc(sizeof(messageStack));
  r->next = NULL;
  r->message = strdup(s);
  return r;
}
  
static messageStack* messageStack_pop(messageStack** t)
{
  messageStack* r;

  if(t == NULL) return NULL;
  if(*t == NULL) return NULL;

  r = *t;
  *t = r->next;
  return r;
}

static void messageStack_push(messageStack** t,char* s)
{
  messageStack* n;

  n = messageStack_new(s);
  n->next = *t;
  *t = n;
}

static void ChangeBadKanjiCode(char *source)
{
  /*
    change some bad EUC kanji code to '#'
    (because of outputs of Sakura is SJIS kanji code,
     some of them include bad kanji codes... sigh.)
  */
    int i,chnum = 0;
    unsigned char tmp[4];
    unsigned char first_byte, second_byte;
    char *result;

    result = (char*)malloc(strlen(source) * 2);
    if(result == NULL) return;
    memset(result,'\0',strlen(source) * 2);

    for (i = 0; i < strlen(source); i++) {
	/* first byte */
	first_byte = *(source + i);

	if ((first_byte >= 0xa1 && first_byte <= 0xfe) ||
	    (first_byte == 0x8e) ||
	    (first_byte == 0x8f)) {

	    /* this is EUC kanji code.... */

	    if (++i > strlen(source)) {
		/* lack EUC kanji code's 2nd byte ? */
		perror("broken kanji code exist");
		break;
	    }
	    second_byte = *(source + i);

	    /* change undefined kanji code and X0201 Kana to '#' */

	    if ((first_byte >= 0xa9 && first_byte <= 0xaf) ||
		(first_byte >= 0xf5) ||
		(first_byte == 0x8e)) {
	      first_byte = 0xa1;
	      second_byte = 0xf4;
	      chnum++;
	    }
	    sprintf(tmp, "%c%c", first_byte, second_byte);

	    if (first_byte == 0x8f) {
		/* this is hojo-kanji */
		if (++i > strlen(source)) {
		    /* lack EUC kanji code's 3rd byte ? */
		  break;
		}
		sprintf(tmp, "%c%c%c", first_byte, second_byte, *(source + i));
	    }
	} else {
	    /* this is perhaps ASCII code... */
	    sprintf(tmp, "%c", first_byte);
	}
	strcat(result, tmp);
    }

    strcpy(source,result);
    free(result);
    return;
}

static void ClearMessage(Widget w)
{
  XawTextPosition current,last;
  XawTextBlock textblock;

  last = XawTextSourceScan (XawTextGetSource (w),(XawTextPosition) 0,
			    XawstAll, XawsdRight, 1, TRUE);
  textblock.ptr = "";
  textblock.firstPos = 0;
  textblock.length = 0;
  textblock.format = FMT8BIT;

  XtVaSetValues(w,XtNeditType,XawtextEdit,NULL);
  XawTextReplace (w, 0, last, &textblock);
  XtVaSetValues(w,XtNeditType,XawtextRead,NULL);
}

static void InsertMessage(Widget w,char* message_buffer,int mode)
{
  XawTextPosition current,last;
  XawTextBlock textblock;
  char* chr_ptr;

  if(strlen(message_buffer) > 0){
    if(!IsPopped(XtParent(w)))
      XtPopup(XtParent(XtParent(w)), XtGrabNone);
  } else {
    return;
  }
  XtVaSetValues(XtParent(w),XtNwindowMode,-1,NULL);
  XFlush(XtDisplay(XtParent(w)));

  current = XawTextGetInsertionPoint(w);
  if(opr.m_wait){
    AddBuffer(&mbuf,message_buffer);
    AddBuffer(&mbuf,"$");
    if(mode == SAKURA)
      AddBuffer(&mdest,"s");
    else
      AddBuffer(&mdest,"u");
  } else {
    last = XawTextSourceScan (XawTextGetSource (w),(XawTextPosition) 0,
			      XawstAll, XawsdRight, 1, TRUE);
    textblock.firstPos = 0;
    textblock.length = strlen(message_buffer);
    textblock.ptr = message_buffer;
    textblock.format = FMT8BIT;
    XtVaSetValues(w,XtNeditType,XawtextEdit,NULL);
    XawTextReplace(w,last,last,&textblock);
    XtVaSetValues(w,XtNeditType,XawtextRead,NULL);
    XFlush(XtDisplay(XtParent(w)));
    XawTextSetInsertionPoint(w , last + textblock.length);
  }
}

static void _InsertMessage(XtPointer cl,XtIntervalId* id)
{
  XawTextPosition current,last;
  XawTextBlock textblock;
  Widget w;
  unsigned char chr_ptr[BUFSIZ];
  char dest[BUFSIZ];
  int cg_num;

  HeadOfBuffer(&mbuf,chr_ptr);
  HeadOfBuffer(&mdest,dest);

  if(*chr_ptr && *dest){
    GetBuffer(&mbuf,chr_ptr);
    switch(*chr_ptr){
    case '$':
      GetBuffer(&mdest,dest);
      break;
    case '\\':
      switch(*(chr_ptr + 1)){
      case 'e':
	if(opr.timeout > 0){
	  if(OptionTimeoutId){
	    XtRemoveTimeOut(OptionTimeoutId);
	    OptionTimeoutId = 0;
	  }
	  OptionTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
					    , opr.timeout * 1000
					    , (XtTimerCallbackProc) Destroy
					    , NULL);
	}
	break;
      case 'w':
	cg_num = atoi(chr_ptr + 2);
	usleep(cg_num * 50 * 1000);
	break;
      default:
	cg_num = atoi(chr_ptr + 1);
	if (*dest == 'u') cg_num += 10;
	XtVaSetValues(xhisho,XtNforceCG,True
		      ,(*dest == 's')? XtNcgNumber:XtNucgNumber
		      ,cg_num
		      ,NULL);
      }
      break;
    default:
      w = (*dest == 's')? label:ulabel;
      last = XawTextSourceScan (XawTextGetSource (w),(XawTextPosition) 0,
				XawstAll, XawsdRight, 1, TRUE);
      textblock.firstPos = 0;
      if ((*chr_ptr >= 0xa1 && *chr_ptr <= 0xfe) ||
	  (*chr_ptr == 0x8e) || (*chr_ptr == 0x8f))
	textblock.length = 2;
      else 
	textblock.length = 1;
      textblock.ptr = chr_ptr;
      textblock.format = FMT8BIT;
      XtVaSetValues(w,XtNeditType,XawtextEdit,NULL);
      XawTextReplace(w,last,last,&textblock);
      XtVaSetValues(w,XtNeditType,XawtextRead,NULL);
      XFlush(XtDisplay(XtParent(w)));
      XawTextSetInsertionPoint(w , last + textblock.length);
    }
  }

  XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
		  , opr.m_wait * 10
		  , (XtTimerCallbackProc) _InsertMessage
		  , NULL);
}

static void InsertReturn(char* message_buffer,char* chr_ptr,int max_len,int message_buffer_size)
{
  /*
    insert '\n' into lineend
    (with check EUC kanji code)
  */

  int pos;
  int chr_length;
  int len = 0;

  chr_length = strlen(chr_ptr);
  for(pos = 0;pos < chr_length;pos++){
    unsigned char first_byte;

    first_byte = chr_ptr[pos];
    if ((first_byte >= 0xa1 && first_byte <= 0xfe) ||
	(first_byte == 0x8e) || (first_byte == 0x8f))
      pos += 2;

    if(chr_ptr[pos] == '\n'){
      if(pos > 0){
	strncat(message_buffer,chr_ptr, pos);
	if((len += pos) >= message_buffer_size) break;
	chr_ptr += pos;
	chr_length -= pos;
	chr_length--;
      } else{
	chr_ptr++;
	chr_length--;
	strcat(message_buffer,"\n");
	if((len ++) >= message_buffer_size) break;
      }
      pos = -1;
    } else if(pos >= max_len){
      strncat(message_buffer,chr_ptr, pos);
      if((len += pos) >= message_buffer_size) break;
      strcat(message_buffer,"\n");
      if((len ++) >= message_buffer_size) break;
      chr_ptr += pos;
      chr_length -= pos;
      pos = -1;
    } else {
      if ((first_byte >= 0xa1 && first_byte <= 0xfe) ||
	  (first_byte == 0x8e) || (first_byte == 0x8f))
	pos--;
    }
  }

  strncat(message_buffer,chr_ptr,MIN(message_buffer_size,strlen(chr_ptr)));
  if(message_buffer[strlen(message_buffer) - 1] != '\n'){
    if(strlen(message_buffer) >= message_buffer_size){
      message_buffer[strlen(message_buffer) - 1] = '\n';
    } else {
      strcat(message_buffer,"\n");
    }
  }
}

static void RemoveStr(char* message_buffer,char* str)
{
  char* chr_ptr;

  while((chr_ptr = strstr(message_buffer,str)) != NULL)
    strcpy(chr_ptr,chr_ptr + strlen(str));
}

static void AddBuffer(messageBuffer* buffer,char* message)
{
  size_t newsize;
  char* b;

  newsize = strlen(buffer->buffer) + strlen(message) + 1;

  if(newsize > buffer->size){
    b = strdup(buffer->buffer);
    buffer->buffer = (char*)realloc(buffer->buffer,newsize);
    strcpy(buffer->buffer,b);
    free(b);
    buffer->size = newsize;
    printf("%d\n",newsize);
  }

  strcat(buffer->buffer,message);
}

static void _GetBuffer(messageBuffer* buffer,char* ret,int mode)
{
  unsigned char first_byte;
  int is_wbyte = 0;
  unsigned char str_num[128];
  int wait;
  unsigned char* c_ptr;

  ret[0] = first_byte = *(buffer->buffer);

  if((ret[0] = first_byte = *(buffer->buffer)) == '\0'){
    return;
  }

  if ((first_byte >= 0xa1 && first_byte <= 0xfe) ||
      (first_byte == 0x8e) || (first_byte == 0x8f) || first_byte == '\\'){
    is_wbyte = 1;
    ret[1] = *(buffer->buffer + 1);
    ret[2] = '\0';

    if(strncmp(ret,"\\w",strlen("\\w")) == 0){
      c_ptr = buffer->buffer + 2;
      while(isdigit(*c_ptr)){
	c_ptr++;
	is_wbyte++;
      }
      strncpy(str_num,buffer->buffer + 2
	      ,is_wbyte);
      sprintf(ret,"\\w%d",atoi(str_num));
    }
  } else {
    ret[1] = '\0';
    ret[2] = '\0';
  }

  if(mode)
    strcpy(buffer->buffer,buffer->buffer + 1 + is_wbyte);

  return;
}
  
static void HeadOfBuffer(messageBuffer* buffer,char* ret)
{
  return _GetBuffer(buffer,ret,0);
}

static void GetBuffer(messageBuffer* buffer,char* ret)
{
  return _GetBuffer(buffer,ret,1);
}

static void SakuraParser(char* in_ptr)
{
  /*
   * Sakura Script parser
   *
   * parsed script -> works(sleep,change cg,etc) is done
   *  in _InsertMessage()
   */

  messageBuffer kbuf;
  unsigned char* buffer;
  unsigned char* chr_ptr;
  int num;
  unsigned char* c_ptr;
  unsigned char str_num[128];

  if(in_ptr == NULL) return;
  
  kbuf.buffer = (unsigned char*)malloc(BUFSIZ * 10);
  kbuf.size = BUFSIZ * 10;
  *kbuf.buffer = '\0';

  buffer = (unsigned char*)malloc(strlen(in_ptr));

  for(chr_ptr = in_ptr;*chr_ptr;chr_ptr++){

    if(*chr_ptr == '\\'){
      chr_ptr++;
      if(*chr_ptr == '\0') goto END;
      switch(*chr_ptr) {
      case 'h':
	strcpy(buffer,"\nsakura:");
	AddBuffer(&kbuf,buffer);
	break;
      case 'u':
	strcpy(buffer,"\nunyuu:");
	AddBuffer(&kbuf,buffer);
	break;
      case 'n':
	strcpy(buffer,"\n");
	AddBuffer(&kbuf,buffer);
	break;
      case 'e':
	strcpy(buffer,"\\e\n");
	AddBuffer(&kbuf,buffer);
	goto END;
	break;
      case 't':
	break;
      case 'c':
	break;
      case 'x':
	break;
      case '-':
	break;
      case 'z':
	break;
      case '*':
	break;
      case 'a':
	break;
      case 'v':
	break;
      case 'w':
	chr_ptr++;
	if(*chr_ptr  == '\0') goto END;
	c_ptr = chr_ptr;
	while(isdigit(*c_ptr)) c_ptr++;
	if(opr.m_wait){
	  strncpy(str_num,chr_ptr
		  ,c_ptr - chr_ptr);
	  sprintf(buffer,"\\w%d",atoi(str_num));
	  AddBuffer(&kbuf,buffer);
	}
	chr_ptr = c_ptr - 1;
	break;
      case 's':
	chr_ptr++;
	if(*chr_ptr == '\0') goto END;
	c_ptr = chr_ptr;
	while(isdigit(*c_ptr)) c_ptr++;
	strncpy(str_num,chr_ptr
		,c_ptr - chr_ptr);
	sprintf(buffer,"(Surface:%d)",atoi(str_num));
	AddBuffer(&kbuf,buffer);
	chr_ptr = c_ptr - 1;
	break;
      }
    } else {
      strncpy(buffer,chr_ptr,1);
      buffer[1] = '\0';
      AddBuffer(&kbuf,buffer);
    }
  }

 END:
  strcpy(in_ptr,kbuf.buffer);
  free(buffer);
  free(kbuf.buffer);
  return ;
}

static void SJIS2EUC(char* in) {
  unsigned char* out;
  unsigned char c1;
  unsigned char c2;
  unsigned char* in_ptr;
  unsigned char* out_ptr;
  unsigned char first_byte;

  out = (unsigned char*)malloc(strlen(in) + 1);
  in_ptr = in;
  out_ptr = out;

  while(*in_ptr != '\0'){
    first_byte = *in_ptr;
    if(first_byte < 0x80){
      *out_ptr++ = *in_ptr++;
    } else {
      c1 = first_byte;
      in_ptr++;
      if(*in_ptr == '\0') break;
      c2 = (*in_ptr);

      if( c2 < 0x9f ) {
	if( c1 < 0xa0 ) {
	  c1 -= 0x81;
	  c1 *= 2;
	  c1 += 0xa1;
	} else {
	  c1 -= 0xe0;
	  c1 *= 2;
	  c1 += 0xdf;
	}
	if( c2 > 0x7f ) { --c2; }
	c2 += 0x61;
      } else {
	if( c1 < 0xa0 ) {
	  c1 -= 0x81;
	  c1 *= 2;
	  c1 += 0xa2;
	} else {
	  c1 -= 0xe0;
	  c1 *= 2;
	  c1 += 0xe0;
	}
	c2 += 2;
      }

      *out_ptr++ = c1;
      *out_ptr++ = c2;
      in_ptr++;
    }
  }
  *out_ptr = '\0';
  strcpy(in,out);
  free(out);
}
