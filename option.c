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
static XtInputId OptionId;
static int virgine = 1;

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
};

static void Destroy(Widget w, XEvent * event, String * params, unsigned int *num_params)
{
  XtPopdown(top);
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
				  /*
				  ,XtNresize,XawtextResizeWidth
				  ,XtNscrollHorizontal,XawtextScrollWhenNeeded
				  ,XtNscrollVertical,XawtextScrollAlways
				  */
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

  command = (char*)malloc(256);

  if(strlen(opr.o_command) < 1) return;
  if (stat(opr.o_command, &Ystat) == -1) {
    fprintf(stderr, "no such option command, \"%s\"\n", opr.o_command);
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
  char *message_buffer;
  char *_buffer;
  char *buffer;
  static int x = 0;
  int len;
  char* chr_ptr;
  char* next_ptr;
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
  char str_num[128];

#ifdef EXT_FILTER
  char command[128];
  char t_filename[BUFSIZ];
  char d_buffer[BUFSIZ * 3];
  FILE* t_file;
  FILE* in;
#endif			

  message_buffer = (char *)malloc(BUFSIZ * 20);
  if(message_buffer == NULL) return;
  _buffer = (char *)malloc(BUFSIZ * 5);
  if(_buffer == NULL){
    free(message_buffer);
    return;
  }
  buffer = (char *)malloc(BUFSIZ * 5);
  if(buffer == NULL){
    free(_buffer);
    free(message_buffer);
    return;
  }

  memset(message_buffer,'\0',BUFSIZ * 20);
  memset(_buffer,'\0',BUFSIZ * 5);
  memset(buffer,'\0',BUFSIZ * 5);

  if ((len = read(*fid,_buffer,BUFSIZ * 5)) == 0) {
    XtRemoveInput(OptionId);
    OptionId = 0;
    fprintf(stderr, "option command died!\n");
    XtDestroyWidget(optionwin);
    optionwin = CreateOptionWindow(XtParent(xhisho));
    free(buffer);
    free(_buffer);
    free(message_buffer);
    return;
  } else if (len == -1) {
    fprintf(stderr, "Can't read from option command!\n");
  }

  _buffer[len] = '\0';


  x = 1;

  if(is_end){
    last = XawTextSourceScan (XawTextGetSource (label),(XawTextPosition) 0,
			      XawstAll, XawsdRight, 1, TRUE);
    //    last = XawTextGetInsertionPoint(XawTextGetSource(label));
    textblock.ptr = "";
    textblock.firstPos = 0;
    textblock.length = 0;
    textblock.format = FMT8BIT;

    XtVaSetValues(label,XtNeditType,XawtextEdit,NULL);
    XawTextReplace (label, 0, last, &textblock);
    XtVaSetValues(label,XtNeditType,XawtextRead,NULL);

    XawTextSetInsertionPoint(label,0);
    is_end = 0;
  }

#ifdef EXT_FILTER
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
    buffer[0] = '\0';
    while((fgets(d_buffer, BUFSIZ * 3, in)) != NULL){
      strcat(buffer,d_buffer);
    }
    pclose(in);
    unlink(t_filename);
  }
#else
  strcpy(buffer, _buffer);
#endif

  /* here is script decoder .. */

  if(strstr(buffer,"sakura"))
    sakura = 1;
  else 
    sakura = 0;

  if((chr_ptr = strchr(buffer,'(')) != NULL){
    if(!strncasecmp(chr_ptr + 1,"Surface:",strlen("Surface:"))){
      chr_ptr += strlen("(Surface:");
      next_ptr = chr_ptr;
      while(isdigit((unsigned char)(*next_ptr))) next_ptr++;
      strncpy(str_num,chr_ptr,next_ptr - chr_ptr);

      cg_num = atoi(str_num);
      chr_ptr = strchr(buffer,')') + 1;
      strcpy(_buffer,chr_ptr);
      strcpy(strchr(buffer,'('),_buffer);
    }
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

  max_len = width / log.width - 1;
  chr_length = strlen(chr_ptr);

  for(dword = pos = 0;pos < chr_length;pos++){
    if(((signed char)chr_ptr[pos]) < 0) dword ++;
    if(chr_ptr[pos] == '\n'){
      if(pos > 0){
	strncat(message_buffer,chr_ptr, pos);
	chr_ptr += pos;
	chr_length -= pos;
	chr_length--;
      } else{
	chr_ptr++;
	chr_length--;
      strcat(message_buffer,"\n");
      }
      pos = -1;
      dword = 0;
    } else if(pos >= max_len && dword % 2 == 0){
      strncat(message_buffer,chr_ptr, pos - 1);
      strcat(message_buffer,"\n");
      chr_ptr += pos;
      chr_ptr--;
      chr_length -= pos;
      pos = -1;
      dword = 0;
    }
  }
  strcat(message_buffer,chr_ptr);

  current = XawTextGetInsertionPoint(label);
  last = XawTextSourceScan (XawTextGetSource (label),(XawTextPosition) 0,
			    XawstAll, XawsdRight, 1, TRUE);

  textblock.firstPos = 0;
  textblock.length = strlen(message_buffer);
  textblock.ptr = message_buffer;
  textblock.format = FMT8BIT;
  XtVaSetValues(label,XtNeditType,XawtextEdit,NULL);
  XawTextReplace(label,last,last,&textblock);
  XtVaSetValues(label,XtNeditType,XawtextRead,NULL);
  if (current == last)
    XawTextSetInsertionPoint(label
			     , last + textblock.length);

  XtPopup(XtParent(local_option), XtGrabNone);
  if(sakura && cg_num != -1)
    XtVaSetValues(xhisho,XtNforceCG,True,XtNcgNumber,cg_num,NULL);

  free(buffer);
  free(_buffer);
  free(message_buffer);
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
		return 1;
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
