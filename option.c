#define _OPTION_GLOBAL
#include "globaldefs.h"
#include "Msgwin.h"
#include "option.h"
#include <ctype.h>
#include <signal.h>

#ifndef ENABLE_EUC_HANKAKU_KANA
#define ENABLE_EUC_HANKAKU_KANA 0
#endif

static Widget top,label,local_option;
static Widget utop,ulabel,ulocal_option;
static XtInputId OptionId;
static XtIntervalId OptionTimeoutId = 0;
static XtIntervalId MessageWaitId = 0;
static messageBuffer mbuf;

#ifdef USE_KAWARI
static XtIntervalId KAWARITimeoutId = 0;
#endif


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
static char* ChangeBadKanjiCode(char *);
static void ClearMessage(Widget);
static void InsertMessage(XtPointer,XtIntervalId*);
static void AddBuffer(messageBuffer*,char*);
static void GetBuffer(messageBuffer*,char*);
static void HeadOfBuffer(messageBuffer*,char*);
static void _GetBuffer(messageBuffer*,char*,int);
static char* SJIS2EUC(char*);

#ifdef USE_KAWARI
static void GetMessageFromKawari(Widget, int *, XtInputId *);
extern char* RandomMessage(char*);
#endif

static void SakuraParser(char*); /* only for reference */


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
  XtPopdown(utop);
  /*
  ClearMessage(label);
  ClearMessage(ulabel);
  */
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

    virgine = 0;
  }

  top = XtVaCreatePopupShell("OptionWindow", transientShellWidgetClass
			     ,w,NULL);
  XtGetApplicationResources(top, &opr, resources, XtNumber(resources), NULL, 0);
  local_option = XtVaCreateManagedWidget("option", msgwinWidgetClass, top
  				 ,NULL);

  if(opr.m_wait < 1) opr.m_wait = 1;

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

  if(strlen(opr.o_command) > 0)
    CommandInit();

#ifdef USE_KAWARI
  GetMessageFromKawari(local_option,NULL,NULL);
#endif

  MessageWaitId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				  , opr.m_wait * 10
				  , (XtTimerCallbackProc) InsertMessage
				  , NULL);
  return(local_option);
}

static void CommandInit()
{
  static int virgine = 1;

  if (virgine) {
    if ((option_fd = popen(opr.o_command,"r")) == NULL){
      fprintf(stderr, "no such option command, \"%s\"\n", opr.o_command);
      return ;
    }
    virgine = 0;
  }

  OptionId = XtAppAddInput(XtWidgetToApplicationContext(top),
			   fileno(option_fd), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckOption, NULL);
  XSetIOErrorHandler(Option_exit);	/** child process¤ò»¦¤¹ **/
}

static void CheckOption(Widget w, int *fid, XtInputId * id)
{
  static unsigned char *_buffer;
  static int x = 0;
  int len;
  int message_buffer_size = BUFSIZ * 20;
  unsigned char* message_ptr;

  if(OptionTimeoutId){
    XtRemoveTimeOut(OptionTimeoutId);
    OptionTimeoutId = 0;
  }

  if(x == 0){
    _buffer = (char *)malloc(message_buffer_size);
  }
  x = 1;

  memset(_buffer,'\0',message_buffer_size);

  if ((len = read(*fid,_buffer,message_buffer_size)) == 0) {
    XtRemoveInput(OptionId);
    OptionId = 0;
    fprintf(stderr, "option command died!\n");
    return;
  } else if (len == -1) {
    fprintf(stderr, "Can't read from option command!\n");
    return;
  }

  if(len == 0) return;
  _buffer[len] = '\0';

  //  ClearMessage(label);
  //  ClearMessage(ulabel);

  message_ptr = SJIS2EUC(_buffer);
  strncpy(_buffer,message_ptr,MIN(message_buffer_size -1,strlen(message_ptr)));
  free(message_ptr);
  message_ptr = ChangeBadKanjiCode(_buffer);
  strncpy(_buffer,message_ptr,MIN(message_buffer_size -1,strlen(message_ptr)));
  free(message_ptr);

#ifdef DEBUG
  printf("#%s\n",_buffer);
#endif

  ORParser(_buffer);

  AddBuffer(&mbuf,_buffer);

}

#ifdef USE_KAWARI
static void GetMessageFromKawari(Widget w, int * i, XtInputId * id){
  static unsigned char *_buffer;
  static int x = 0;
  int len;
  int message_buffer_size = BUFSIZ * 20;
  unsigned char* message_ptr;

  if(OptionTimeoutId){
    XtRemoveTimeOut(OptionTimeoutId);
    OptionTimeoutId = 0;
  }

  if(KAWARITimeoutId){
    XtRemoveTimeOut(KAWARITimeoutId);
    KAWARITimeoutId = 0;
  }

  if(x == 0){
    _buffer = (char *)malloc(message_buffer_size);
  }
  x = 1;

  memset(_buffer,'\0',message_buffer_size);

  message_ptr = RandomMessage(opr.kawari_dir);
  strncpy(_buffer,message_ptr,MIN(message_buffer_size -1,strlen(message_ptr)));
  free(message_ptr);

  message_ptr = SJIS2EUC(_buffer);
  strncpy(_buffer,message_ptr,MIN(message_buffer_size -1,strlen(message_ptr)));
  free(message_ptr);

  message_ptr = ChangeBadKanjiCode(_buffer);
  strncpy(_buffer,message_ptr,MIN(message_buffer_size -1,strlen(message_ptr)));
  free(message_ptr);

#ifdef DEBUG
  printf("#%s\n",buffer);
#endif

  ORParser(_buffer);

  AddBuffer(&mbuf,_buffer);

  if(opr.k_wait == 0)
    opr.k_wait = 60;

  KAWARITimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				    , opr.k_wait * 1000
				    , (XtTimerCallbackProc) GetMessageFromKawari
				    , NULL
				    );
}
#endif

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
   *  parse random scripts
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
   *  translate OR-string to string
   *  '(aa|bb)' -> 'aa' in 50%, 'bb' in 50%
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
   * strncpy, but not '\n'.
   * original code is in FreeBSD's library.
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

static char* ChangeBadKanjiCode(char *source)
{
  /*
   * change some bad EUC kanji code to '#'
   * (because of outputs of Sakura is SJIS kanji code,
   * some of them include bad kanji codes... sigh.)
   */

  int i;
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
      
      /* change undefined kanji code and X0201 Kana to 2-byte '#' */
      
      
      if ((first_byte >= 0xa9 && first_byte <= 0xaf) ||
	  (first_byte >= 0xf5)){
	/* this is undefined kanji code */
	first_byte = 0xa1;
	second_byte = 0xf4;
      }
      
      if(first_byte == 0x8e){
	if(second_byte >= 0xa1 && second_byte <= 0xdf){
	  /* this is hankaku-kana */
	  if(!ENABLE_EUC_HANKAKU_KANA){
	    first_byte = 0xa1;
	    second_byte = 0xf4;
	  }
	} else {
	/* this is undefined kanji code */
	  first_byte = 0xa1;
	  second_byte = 0xf4;
	}
      }
      
      sprintf(tmp, "%c%c", first_byte, second_byte);
      
      if (first_byte == 0x8f) {
	/* this is hojo-kanji */
	if (++i > strlen(source)) {
	  /* lack EUC kanji code's 3rd byte ? */
	  perror("broken kanji code exist");
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
  
  return result;
}

static void ClearMessage(Widget w)
{
  /*
   * clear message window
   */
  static XawTextPosition current,last;
  static XawTextBlock textblock;
  
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

static void InsertMessage(XtPointer cl,XtIntervalId* id)
{
  /*
   * if there is any string in message buffer,
   * get a string from buffer and insert into window
   *
   * a string is one of follows:
   *  1. a command like "\u" , "\w10" , etc.
   *  2. a 1-byte letter, mainly alphabet.
   *  3. a 2-byte letter, mainly Japanese letter.
   */
  static XawTextPosition current,last;
  static XawTextBlock textblock;
  Widget w;
  unsigned char chr_ptr[BUFSIZ];
  unsigned char buffer[BUFSIZ + 1];
  int cg_num;
  static int dest_win = SAKURA;
  XFontSet fset;
  XRectangle ink, log;
  int max_len;
  Dimension width;

  static int pos = 0;
  int is_display = 0;
  static int is_end = 0;

  HeadOfBuffer(&mbuf,chr_ptr);

  if(*chr_ptr){
    GetBuffer(&mbuf,chr_ptr);
    switch(*chr_ptr){
    case '\\':
      switch(*(chr_ptr + 1)){
      case 'h':
	dest_win = SAKURA;
	pos = 0;
	break;
      case 'u':
	dest_win = UNYUU;
	pos = 0;
	break;
      case 'n':
	is_display = 2;
	pos = 0;
	break;
      case 'e':
	is_end = 1;
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
      case 's':
	cg_num = atoi(chr_ptr + 2);
	if (dest_win == UNYUU) cg_num += 10;
	XtVaSetValues(xhisho,XtNforceCG,True
		      ,(dest_win == SAKURA)? XtNcgNumber:XtNucgNumber
		      ,cg_num
		      ,NULL);
      }
      break;
    default:
      is_display = 1;
    }

    if(is_display){
      w = (dest_win == SAKURA)? label:ulabel;
      if(is_end && *chr_ptr != '\n'){
	ClearMessage(label);
	ClearMessage(ulabel);
	is_end = 0;
      }

      XtVaGetValues(w, XtNfontSet, &fset, XtNwidth,&width,NULL);
      XmbTextExtents(fset, chr_ptr, strlen(chr_ptr), &ink, &log);
      max_len = width - 2;

      if(!IsPopped(XtParent(w)))
	XtPopup(XtParent(XtParent(w)), XtGrabNone);

      last = XawTextSourceScan (XawTextGetSource (w),(XawTextPosition) 0,
				XawstAll, XawsdRight, 1, TRUE);

      if(*chr_ptr == '\n') pos = 0;

      if(is_display == 2){
	strcpy(buffer,"\n");
      } else {
	if((pos += log.width) > max_len){
	  pos = log.width;
	  sprintf(buffer,"\n%s",chr_ptr);
	} else {
	  strcpy(buffer,chr_ptr);
	}
      }
	
      textblock.firstPos = 0;
      textblock.length = strlen(buffer);
      textblock.ptr = buffer;
      textblock.format = FMT8BIT;
      XtVaSetValues(w,XtNeditType,XawtextEdit,NULL);
      XawTextReplace(w,last,last,&textblock);
      XtVaSetValues(w,XtNeditType,XawtextRead,NULL);
      XFlush(XtDisplay(XtParent(w)));
      XawTextSetInsertionPoint(w , last + textblock.length);
    }
  }

  if(MessageWaitId){
    XtRemoveTimeOut(MessageWaitId);
    MessageWaitId = 0;
  }

  MessageWaitId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				  , opr.m_wait * 10
				  , (XtTimerCallbackProc) InsertMessage
				  , NULL);
}

static void AddBuffer(messageBuffer* buffer,char* message)
{
  /*
   * add message int message buffer.
   * if buffer is too small to add message,
   *  make larger buffer automatically.
   */

  size_t newsize;
  char* b;

  newsize = strlen(buffer->buffer) + strlen(message) + 1;

  if(newsize > buffer->size){
    b = strdup(buffer->buffer);
    buffer->buffer = (char*)realloc(buffer->buffer,newsize);
    strcpy(buffer->buffer,b);
    free(b);
    buffer->size = newsize;
  }

  strcat(buffer->buffer,message);
}

static void _GetBuffer(messageBuffer* buffer,char* ret,int mode)
{
  /*
   * do not invoke this function. this function should be
   * invoked by GetBuffer() or HeadOfBuffer().
   */
  unsigned char first_byte;
  int is_wbyte = 0;
  unsigned char str_num[128];
  int wait;
  unsigned char* c_ptr;
  int skip_return = 0;
  int i;

  ret[0] = first_byte = *(buffer->buffer);

  if((ret[0] = first_byte = *(buffer->buffer)) == '\0'){
    return;
  }

  if ((first_byte >= 0xa1 && first_byte <= 0xfe) ||
      (first_byte == 0x8e) || (first_byte == 0x8f) || first_byte == '\\'){
    is_wbyte = 1;
    ret[1] = *(buffer->buffer + 1);
    ret[2] = '\0';

    if(first_byte == '\\'){
      c_ptr = buffer->buffer + 1;
      switch(*c_ptr){
      case 'w':
      case 's':
      case 'j':
      case 'b':
      case 'i':
	c_ptr = buffer->buffer + 2;
	while(isdigit(*c_ptr) || *c_ptr == '\n'){
	  /*
	   * if "\w10HOGE" -> "\w1\n0HOGE" ,
	   *  this function return "\w10" 
	   *     and rest of message should be "\nHOGE".
	   */
	  if(*c_ptr == '\n'){
	    skip_return++;
	    strcpy(c_ptr,c_ptr + 1);
	  } else {
	    c_ptr++;
	    is_wbyte++;
	  }
	}
	strncpy(str_num,buffer->buffer + 2
		,is_wbyte);

	sprintf(ret + 2,"%d",atoi(str_num));
	
	for(i = 0; i < skip_return;i++)
	  *(--c_ptr) = '\n';
	break;
      default:
	break;
      }
    }
  } else {
    ret[1] = '\0';
    ret[2] = '\0';
  }

  if(mode)
    strcpy(buffer->buffer,buffer->buffer + 1 + is_wbyte - skip_return);
  
  return;
}
  
static void HeadOfBuffer(messageBuffer* buffer,char* ret)
{
  /*
   * get string at head of buffer.
   * (buffer does not modify)
   */
  return _GetBuffer(buffer,ret,0);
}

static void GetBuffer(messageBuffer* buffer,char* ret)
{
  /*
   * get string at head of buffer.
   * (string is removed from buffer)
   */
  return _GetBuffer(buffer,ret,1);
}

static void SakuraParser(char* in_ptr)
{
  /*
   *    *Warning* 
   *  this function is obsolete (for reference only)
   *
   *
   * Sakura Script parser
   *
   * parsed script -> works(sleep,change cg,etc) is done
   *  in InsertMessage()
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

static char* SJIS2EUC(char* in) {
  /*
   * change SJIS to EUC. original is in xakane by NAO.
   */

  unsigned char c1;
  unsigned char c2;
  unsigned char* in_ptr;
  unsigned char first_byte;
  unsigned char* out_ptr;
  unsigned char* ret;

  ret = (unsigned char*)malloc(strlen(in) * 2);
  in_ptr = in;
  out_ptr = ret;

  while(*in_ptr != '\0'){
    first_byte = *in_ptr;
    if(first_byte < 0x80){
      *out_ptr++ = *in_ptr++;
    } else if(first_byte >= 0xa1 && first_byte <= 0xdf){
      /*
       * this is hankaku-kana. map to EUC hanaku-kana
       */
      *out_ptr++ = 0x8e;
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

  return ret;
}
