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

static void Destroy(Widget,XEvent *, String *, unsigned int *);
static void CommandInit();
static void CheckOption(Widget, int *, XtInputId *);
static int Option_exit(Display *);
static char* or2string(char*);
static void SakuraParser(char*);
static char * nstrncpy(char*,const char*,size_t);
static messageStack* messageStack_new(char*);
static messageStack* messageStack_pop(messageStack**);
static void messageStack_push(messageStack**,char*);
static void ChangeBadKanjiCode(char *);
static void ClearMessage(Widget);
static void InsertMessage(Widget,char*);

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
};

static void Destroy(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  if(OptionTimeoutId){
    XtRemoveTimeOut(OptionTimeoutId);
    OptionTimeoutId = 0;
  }
  XtPopdown(top);
#ifdef USE_UNYUU
  XtPopdown(utop);
#endif
}


Widget CreateOptionWindow(Widget w){
  Widget ok,cancel;
  static XtPopdownIDRec pdrec;
  XtTranslations trans_table;

  static char defaultTranslations[] = "<Btn1Down> : Destroy()\n\
                                       <Btn2Down> : Destroy()\n\
                                       <Btn3Down>: Destroy()";

  pdrec.shell_widget = top;
  pdrec.enable_widget = w;

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

#ifdef USE_UNYUU
  utop = XtVaCreatePopupShell("OptionWindow", transientShellWidgetClass
			     ,w,NULL);
  ulocal_option = XtVaCreateManagedWidget("option", msgwinWidgetClass, utop
					  ,XtNxoff,opr.width + 100
					  ,NULL);
  ulabel = XtVaCreateManagedWidget("optionLabel", asciiTextWidgetClass
				  ,ulocal_option
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

#endif

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

  /*
  ok = XtVaCreateManagedWidget("optionOk", commandWidgetClass, local_option
			       ,XtNfromVert, label
			       ,XtNhorizDistance, POINT_WIDTH + LABEL_OFFSET
			       ,XtNlabel, "OK"
			       ,XtNvertDistance, 20 
			       ,XtNleft, XtChainLeft, XtNright, XtChainLeft
			       ,XtNinternalHeight, FONT_OFFSET, NULL);
  XtAddCallback(ok, XtNcallback, (XtCallbackProc) Destroy, NULL);

  cancel = XtVaCreateManagedWidget("optionCancel", commandWidgetClass
				   , local_option
				   ,XtNfromVert, label
				   ,XtNfromHoriz, ok
				   ,XtNhorizDistance,10
				   ,XtNlabel, "Cancel"
				   ,XtNvertDistance, 20 
				   ,XtNleft, XtChainLeft, XtNright, XtChainLeft
				   ,XtNinternalHeight, FONT_OFFSET, NULL);
  XtAddCallback(cancel, XtNcallback, (XtCallbackProc) Destroy, NULL);
  */
  CommandInit();
  return(local_option);
}

static void CommandInit()
{
  char *command;
  struct stat Ystat;

  command = (char*)malloc(strlen(opr.o_command) + 1);
  if(strchr(opr.o_command,' ') != NULL){
    strncpy(command,opr.o_command
	    ,strlen(opr.o_command) - strlen(strchr(opr.o_command,' ')));
    command[strlen(opr.o_command) - strlen(strchr(opr.o_command,' '))] = '\0';
  } else {
    strcpy(command,opr.o_command);
  }
  if(strlen(opr.o_command) < 1) return;
  if (stat(command, &Ystat) == -1) {
    fprintf(stderr, "no such option command, \"%s\"\n", command);
    exit(1);
  }

  if (virgine) {
    option_fd = popen(opr.o_command,"r");
    virgine = 0;
  }

  OptionId = XtAppAddInput(XtWidgetToApplicationContext(top),
			   fileno(option_fd), (XtPointer) XtInputReadMask,
			   (XtInputCallbackProc) CheckOption, NULL);
  XSetIOErrorHandler(Option_exit);	/** child process ¤Î youbin ¤ò»¦¤¹ **/
  free(command);
}

static void CheckOption(Widget w, int *fid, XtInputId * id)
{
  static unsigned char *message_buffer;
  static unsigned char *_buffer;
  static unsigned char *buffer;
  static int x = 0;
  int len;
  int message_buffer_size = BUFSIZ * 20;
  unsigned char* chr_ptr;
  unsigned char* next_ptr;
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

  if(x == 0){
    message_buffer = (char *)malloc(message_buffer_size);
    _buffer = (char *)malloc(message_buffer_size);
    buffer = (char *)malloc(message_buffer_size);
  }
  x = 1;

  memset(message_buffer,'\0',message_buffer_size);
  memset(_buffer,'\0',message_buffer_size);
  memset(buffer,'\0',message_buffer_size);

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

  _buffer[len] = '\0';


  if(is_end){
    ClearMessage(label);
#ifdef USE_UNYUU
    ClearMessage(ulabel);
#endif

    is_end = 0;
  }

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

  /* here is script decoder .. */

#ifdef DEBUG
  printf("#%s\n",buffer);
#endif

  while((chr_ptr = strstr(buffer,"\\n")) != NULL){
    strcpy(_buffer,chr_ptr +2);
    sprintf(chr_ptr,"\n%s",_buffer);
  }

  /* surface changer */

  while((chr_ptr = strstr(buffer,"(Surface:")) != NULL){
    if(*(chr_ptr - 2) == 'a')
      sakura = 1;
    else 
      sakura = 0;

    next_ptr = chr_ptr + strlen("(Surface:");
    while(isdigit((unsigned char)(*next_ptr))) next_ptr++;
    strncpy(str_num,chr_ptr + strlen("(Surface:")
	    ,next_ptr - chr_ptr - strlen("(Surface:"));
    if(sakura)
      cg_num = atoi(str_num);
    else 
      u_cg_num = atoi(str_num) + 10;
    strcpy(_buffer,next_ptr + 1);
    strcpy(strstr(buffer,"(Surface:"),_buffer);
  }

  chr_ptr = buffer;
  ChangeBadKanjiCode(buffer);

  if(next_ptr = strstr(chr_ptr,"\\e")){
    is_end = 1;
    *next_ptr = '\0';
  }

  XtVaGetValues(label, XtNfontSet, &fset, XtNwidth,&width,NULL);
  XmbTextExtents(fset, "a", 1, &ink, &log);


  SakuraParser(chr_ptr);

  max_len = width / log.width - 2;
  chr_length = strlen(chr_ptr);

  memset(message_buffer,'\0',message_buffer_size);
  len = 0;

  /*
    insert '\n' into lineend
    (with check EUC kanji code)
  */


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
#ifdef DEBUG
  printf("*%s\n",message_buffer);
#endif


  /*
    insert message into window
  */

  InsertMessage(label,message_buffer);
#ifdef USE_UNYUU
  InsertMessage(ulabel,message_buffer);
#endif

  if(sakura && cg_num != -1)
    XtVaSetValues(xhisho,XtNforceCG,True,XtNcgNumber,cg_num,NULL);

  if(is_end && opr.timeout > 0){
    if(OptionTimeoutId){
      XtRemoveTimeOut(OptionTimeoutId);
      OptionTimeoutId = 0;
    }
    OptionTimeoutId = XtAppAddTimeOut(XtWidgetToApplicationContext(local_option)
				      , opr.timeout * 1000
				      , (XtTimerCallbackProc) Destroy
				      , NULL);
  }
}

static int Option_exit(Display * disp)
{
  /**
   * kill all the children
   **/
  kill(0, SIGTERM);
  return 0;
}

void SakuraParser(char* in)
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
    strcat(tmp_buffer,or2string(back_ptr));
    strcat(tmp_buffer,next_ptr+1);
    strcpy(in_buffer,tmp_buffer);
    chr_ptr = strchr(in_buffer,'(');
  }

  strcpy(in,in_buffer);
  free(tmp_string);
  free(tmp_buffer);
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

  if((chr_ptr = strchr(in_ptr,'|')) == NULL)
    return out;

  top = NULL;

  while((chr_ptr = strchr(in_ptr,'|')) != NULL){
    memset(buffer,'\0',strlen(in) + 1);
    nstrncpy(buffer,in_ptr,chr_ptr - in_ptr);
    in_ptr = chr_ptr + 1;
    messageStack_push(&top,buffer);
    num_of_item++;
  }

  pos = rand() % num_of_item;
  do{
    if((item = messageStack_pop(&top)) == NULL) break;

    if(pos-- == 0)
      strcpy(out,item->message);

    free(item->message);
    free(item);
  } while(top);

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

static void InsertMessage(Widget w,char* message_buffer)
{
  XawTextPosition current,last;
  XawTextBlock textblock;

  current = XawTextGetInsertionPoint(w);
  last = XawTextSourceScan (XawTextGetSource (w),(XawTextPosition) 0,
			    XawstAll, XawsdRight, 1, TRUE);

  textblock.firstPos = 0;
  textblock.length = strlen(message_buffer);
  textblock.ptr = message_buffer;
  textblock.format = FMT8BIT;
  XtVaSetValues(w,XtNeditType,XawtextEdit,NULL);
  XawTextReplace(w,last,last,&textblock);
  XtVaSetValues(w,XtNeditType,XawtextRead,NULL);

  if (current == last)
    XawTextSetInsertionPoint(w
			     , last + textblock.length);
  if(strlen(message_buffer) > 0)
     XtPopup(XtParent(XtParent(w)), XtGrabNone);
}

