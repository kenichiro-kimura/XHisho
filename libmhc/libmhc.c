#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include "mhc.h"

static MHCD* MHCDNew(mhcent*);
static void MHCDDelete(MHCD*);
static mhcent* EntryNew();
static void EntryDelete(mhcent*);
static mhcent* ReadEntry(FILE*);
static MHC* MHCNew();
static inline entrylist* EntrylistNew(mhcent*);
static inline int AddEntry(MHC*,mhcent*,int);
static inline int CheckSC_Day(char*,int,int,int);
static inline int CheckSC_Duration(char*,int,int,int);
static inline int CheckSC_Cond(char*,int,int,int);
static inline int CheckMonth(char*,int);
static inline int CheckOrdinal(char*,int,int,int);
static inline int CheckWeek(char*,int,int,int);
static inline int CheckDay(char* ,int);
static inline int datecmp(char*,char*);

static _MHCD* _MHCDNew(mhcent* item)
{
  _MHCD* mhc_ptr;

  mhc_ptr = (_MHCD*)malloc(sizeof(_MHCD));
  if(!mhc_ptr) return NULL;
  mhc_ptr->prev = NULL;
  mhc_ptr->next = NULL;
  mhc_ptr->item = item;

  return mhc_ptr;
}

static MHCD* MHCDNew(mhcent* item){
  MHCD* mhc_ptr;

  mhc_ptr = (MHCD*)malloc(sizeof(MHCD));
  if(!mhc_ptr) return NULL;

  *mhc_ptr = _MHCDNew(item);
  if(!(*mhc_ptr)) return NULL;

  return mhc_ptr;
}

static void MHCDDelete(MHCD* mhc_ptr)
{
  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;

  if((*mhc_ptr)->next != NULL)
    MHCDDelete(&((*mhc_ptr)->next));

  if((*mhc_ptr)->item != NULL)
    EntryDelete((*mhc_ptr)->item);

  free(*mhc_ptr);
}

static mhcent* EntryNew()
{
  mhcent* mhcent_ptr;
  int i;

  mhcent_ptr = (mhcent*)malloc(sizeof(mhcent));
  if(!mhcent_ptr)
    return NULL;

  for(i = 0; i < NUM_OF_ARRAY(mhcent_ptr->Entry);i++)
    mhcent_ptr->Entry[i] = NULL;
  mhcent_ptr->filename = NULL;

  return mhcent_ptr;
}
      
static void EntryDelete(mhcent* item)
{
  int i;

  if(item == NULL)
    return;
  for(i = 0; i < NUM_OF_ARRAY(item->Entry);i++){
    if(item->Entry[i] != NULL)
      free(item->Entry[i]);
  }
  if(item->filename != NULL)
    free(item->filename);
  free(item);
}

static mhcent* ReadEntry(FILE* fp)
{
  mhcent* mhcent_ptr;
  char* buffer;
  char* b_buffer;
  char* chr_ptr;
  int tag,b_tag;
  int day_length;
  int time_length;
  int duration_length;
  int cond_length;
  int alarm_length;
  int subject_length;
  int buffer_length;

  buffer = (char*)malloc(BUFSIZ);
  if(!buffer) return NULL;

  b_buffer = (char*)malloc(BUFSIZ);
  if(!b_buffer) {
    free(buffer);
    return NULL;
  }

  mhcent_ptr = EntryNew();

  if(!mhcent_ptr){
    free(buffer);
    free(b_buffer);
    return NULL;
  }

  b_tag = tag = NO_TAG;
  day_length = strlen("X-SC-Day:");
  time_length = strlen("X-SC-Time:");
  duration_length = strlen("X-SC-Duration:");
  cond_length = strlen("X-SC-Cond:");
  alarm_length = strlen("X-SC-Alarm:");
  subject_length = strlen("X-SC-Subject:");

  while(fgets(buffer,BUFSIZ - 1,fp) != NULL){
    tag = NO_TAG;
    if(buffer[0] == '\n') break;

    buffer_length = strlen(buffer);
    if(buffer_length < 1) continue;
    if(buffer[0] != 'X' && buffer[0] != ' ') continue;

    if(buffer[buffer_length - 1] == '\n'){
      buffer[buffer_length - 1] = '\0';
      buffer_length--;
      if(buffer_length < 1) continue;
    }

    if(isspace((unsigned char)buffer[buffer_length - 1]))
      buffer[buffer_length - 1] = '\0';

    if(!strncasecmp("X-SC-Day:",buffer,day_length)){
      tag = X_SC_Day;
    } else if(!strncasecmp("X-SC-Time:",buffer,time_length)){
      tag = X_SC_Time;        
    } else if(!strncasecmp("X-SC-Duration:",buffer,duration_length)){
      tag = X_SC_Duration;        
    } else if(!strncasecmp("X-SC-Cond:",buffer,cond_length)){
      tag = X_SC_Cond;        
    } else if(!strncasecmp("X-SC-Alarm:",buffer,alarm_length)){
      tag = X_SC_Alarm;
    } else if(!strncasecmp("X-SC-Subject:",buffer,subject_length)){
      tag = X_SC_Subject;
    } else if(buffer[0] == ' '){
      /*
       * in this case, this line should be append before line.
       */
      tag = CONTINUED_LINE;
    } else {
      b_tag = NO_TAG;
    }

    if(tag != NO_TAG){
      chr_ptr = buffer;

      if(tag != CONTINUED_LINE){
	b_tag = tag;
	for(;*chr_ptr != '\0';chr_ptr++)
	  if(*chr_ptr == ':') break;
      }
      for(chr_ptr++;*chr_ptr != '\0';chr_ptr++)
	if(!isspace((unsigned char)*chr_ptr)) break;

      if(tag == CONTINUED_LINE && b_tag != NO_TAG ){
	strcpy(b_buffer,mhcent_ptr->Entry[b_tag]);
	free(mhcent_ptr->Entry[b_tag]);
	mhcent_ptr->Entry[b_tag] = 
	  (char*)malloc(strlen(b_buffer) + strlen(chr_ptr) + 1);
	strcpy(mhcent_ptr->Entry[b_tag],b_buffer);
	strcat(mhcent_ptr->Entry[b_tag],chr_ptr);
      } else if(tag != CONTINUED_LINE){
	mhcent_ptr->Entry[tag] = strdup(chr_ptr);
      /*
	mhcent_ptr->Entry[tag] = (char*)malloc(strlen(chr_ptr) + 1);
	strcpy(mhcent_ptr->Entry[tag],chr_ptr);
      */
      }
    }
  }

  free(buffer);
  free(b_buffer);

  return mhcent_ptr;
}

static MHC* MHCNew()
{
  MHC* mhc_ptr;
  int i;

  mhc_ptr = (MHC*)malloc(sizeof(MHC));
  if(!mhc_ptr) return NULL;
  memset(mhc_ptr,0,sizeof(MHC));

  *mhc_ptr = (_MHC*)malloc(sizeof(_MHC));
  if(!(*mhc_ptr)) return NULL;
  for(i = 0 ; i <= 31;i++)
    (*mhc_ptr)->table[i] = NULL;

  (*mhc_ptr)->ptr = NULL;
  (*mhc_ptr)->mhcd_ptr = NULL;
  (*mhc_ptr)->intersect_ptr = NULL;

  return mhc_ptr;
}

static inline entrylist* EntrylistNew(mhcent* item)
{
  entrylist* newlist;
  int i;

  newlist = (entrylist*)malloc(sizeof(entrylist));
  if(!newlist) return NULL;

  newlist->prev = NULL;
  newlist->next = NULL;
  newlist->item = EntryNew();
  if(newlist->item != NULL){
    for(i = 0; i < NUM_OF_ARRAY(item->Entry);i++){
      if(item->Entry[i] != NULL){
	newlist->item->Entry[i] = strdup(item->Entry[i]);
	/*
	  newlist->item->Entry[i] = (char*)malloc(strlen(item->Entry[i]) + 1);
	  strcpy(newlist->item->Entry[i],item->Entry[i]);
	*/
      }
    }
    if(item->filename != NULL){
      newlist->item->filename = strdup(item->filename);
      /*
	newlist->item->filename = (char*)malloc(strlen(item->filename) + 1);
	strcpy(newlist->item->filename, item->filename);
      */
    }
  }

  return newlist;
}

static inline int AddEntry(MHC* mhc_ptr,mhcent* ent_ptr,int day)
{
  entrylist* newlist;

  if(day < 1 || day > 31) return 1;

  newlist = EntrylistNew(ent_ptr);
  if(!newlist) return 1;

  if(!(*mhc_ptr)->table[day]){
    (*mhc_ptr)->table[day]= newlist;
  } else {
    newlist->next = (*mhc_ptr)->table[day];
    ((*mhc_ptr)->table[day])->prev = newlist;
    (*mhc_ptr)->table[day] = newlist;
  }

  return 0;
}
      
static void EntrylistDelete(entrylist* list_ptr)
{
  if(!list_ptr) return;

  if(list_ptr->next != NULL)
    EntrylistDelete(list_ptr->next);
  if(list_ptr->item != NULL)
    EntryDelete(list_ptr->item);
  free(list_ptr);
}

static inline int CheckSC_Day(char* sc_day,int year,int month,int day)
{
  char* chr_ptr;
  char* yearmonth;
  char* pivot_ptr;
  int sign,isDay;
  int yearmonth_length;
  int pivot_length;
  int sc_length;

  isDay = -1;

  if(sc_day == NULL) return -1;
  if(*sc_day == '\0') return -1;

  pivot_length = strlen("00");
  pivot_ptr = (char*)malloc(pivot_length + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,pivot_length + 1);

  yearmonth = (char*)malloc(strlen("000000") + 1);

  if(!yearmonth){
    free(pivot_ptr);
    return -1;
  }
  sprintf(yearmonth,"%.4d%.2d",year,month);

  chr_ptr = sc_day;

  yearmonth_length = strlen("000000");
  sc_length = strlen(sc_day);

  while(chr_ptr < sc_day + sc_length){

    sign = 0;
    if(*chr_ptr == '!'){
      sign = 1;
      chr_ptr++;
    }

    if(strncasecmp(chr_ptr,yearmonth,yearmonth_length) == 0){
      chr_ptr += yearmonth_length;
      strncpy(pivot_ptr,chr_ptr,pivot_length);
      pivot_ptr[pivot_length] = '\0';
      if(day == atoi(pivot_ptr)){
	isDay = 1;
	break;
      }
      chr_ptr += pivot_length;
    } else {
      chr_ptr += pivot_length;
      chr_ptr += yearmonth_length;
    }

    if(chr_ptr >= sc_day + sc_length) break;

    while(isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  switch(isDay){
  case 0:
    if(sign == 1)
      isDay = 1;
    break;
  case 1:
    if(sign == 1)
      isDay = 0;
    break;
  }

  free(yearmonth);
  free(pivot_ptr);
  return isDay;

}

static inline int CheckSC_Duration(char* sc_duration,int year,int month,int day)
{
  char* duration_begin;
  char* duration_end;
  char* yearmonthday;
  char* chr_ptr;
  int isDuration = -1;
  int length;
  int yearmonthday_length;
  int sc_length;

  yearmonthday_length = strlen("00000000");
  if(!sc_duration || strlen(sc_duration) < yearmonthday_length) return -1;

  duration_begin = (char*)malloc(yearmonthday_length + 1);
  if(!duration_begin)
    return -1;

  
  duration_end = (char*)malloc(yearmonthday_length + 1);
  if(!duration_end){
    free(duration_begin);
    return -1;
  }


  yearmonthday = (char*)malloc(yearmonthday_length + 1);
  if(!yearmonthday){
    free(duration_begin);
    free(duration_end);
    return -1;
  }
  sprintf(yearmonthday,"%.4d%.2d%.2d",year,month,day);
  memset(duration_begin,0,yearmonthday_length + 1);
  memset(duration_end,0,yearmonthday_length + 1);

  chr_ptr = sc_duration;
  sc_length = strlen(sc_duration);

  while(chr_ptr < sc_duration + sc_length){
    while(isspace((unsigned char)*chr_ptr) && *chr_ptr != '\0');

    length = 0;
    while(chr_ptr < sc_duration + sc_length
	  && *chr_ptr != '-' && length < yearmonthday_length){
      *(duration_begin + length) = *chr_ptr++;
      length++;
    }
    chr_ptr++;
    length = 0;
    while(chr_ptr < sc_duration + sc_length
	  && length < yearmonthday_length){
      *(duration_end + length) = *chr_ptr++;
      length++;
    }
    chr_ptr++;

    isDuration = 0;

    if(datecmp(duration_begin,yearmonthday) <= 0 
       && datecmp(yearmonthday,duration_end) <= 0 ){
      isDuration = 1;
      break;
    }
  }

  free(yearmonthday);
  free(duration_end);
  free(duration_begin);

  return isDuration;

}

static inline int CheckMonth(char* sc_cond,int month)
{
  char CondMonth[][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul"
      	      ,"Aug","Sep","Oct","Nov","Dec"};
  char* chr_ptr;
  char* pivot_ptr;
  int ret_value = -1;
  int tag = 0;
  int i,length;
  int pivot_length;
  int sc_length;

  if(!sc_cond) return -1;
  pivot_length = strlen("0000");
  pivot_ptr = (char*)malloc(pivot_length + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,pivot_length + 1);
  
  chr_ptr = sc_cond;
  sc_length = strlen(sc_cond);
  while(chr_ptr < sc_cond + sc_length){
    length = 0;
    while(!isspace((unsigned char)*chr_ptr) && length < pivot_length)
      *(pivot_ptr + length++) = *chr_ptr++;

    for(i = 0; i < 12;i++){
      if(strcasecmp(CondMonth[i],pivot_ptr) == 0){
	ret_value = 0;
	if(i == month - 1)
	  tag = 1;
      }
    }

    while(chr_ptr < sc_cond + sc_length && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static inline int CheckOrdinal(char* sc_cond,int year,int month,int day)
{
  const int wdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  char Ordinal[][4] = {"1st","2nd","3rd","4th","5th"};
  char* chr_ptr;
  char* pivot_ptr;
  time_t sched;
  struct tm *tm_sched;
  int uru_adjust,tOrdinal;
  int isLast = 0;
  int ret_value = -1;
  int tag = 0;
  int i,length;
  int pivot_length;
  int sc_length;

  if(sc_cond == NULL) return -1;
  if(*sc_cond == '\0') return -1;

  pivot_length = strlen("0000");
  pivot_ptr = (char*)malloc(pivot_length + 1);

  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,pivot_length + 1);

  time(&sched);
  tm_sched = localtime(&sched);
  tm_sched->tm_mday = day;
  tm_sched->tm_mon = month - 1;
  tm_sched->tm_year = year - 1900;
  sched = mktime(tm_sched);
  tOrdinal = (day -1) / 7;

  uru_adjust = (year % 4 == 0 && month == 2) ? 1 : 0;
  if(day + 7 > wdays[month -1] + uru_adjust) isLast = 1;

  chr_ptr = sc_cond;
  sc_length = strlen(sc_cond);
  while(chr_ptr < sc_cond + sc_length){
    length = 0;
    while(chr_ptr < sc_cond + sc_length 
	  && !isspace((unsigned char)*chr_ptr) && length < pivot_length)
      *(pivot_ptr + length++) = *chr_ptr++;
    *(pivot_ptr + length) = '\0';

    for(i = 0; i < 4;i++){
      if(strcasecmp(Ordinal[i],pivot_ptr) == 0){
	ret_value = 0;
	if(i == tOrdinal)
	  tag = 1;
      }
    }

    if(strcasecmp(pivot_ptr,"Last") == 0){
      ret_value = 0;
      if(isLast)
	tag = 1;
    }

    while(chr_ptr < sc_cond + sc_length 
	  && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static inline int CheckWeek(char* sc_cond,int year,int month,int day)
{
  char CondWeek[][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

  char* chr_ptr;
  char* pivot_ptr;
  time_t sched;
  struct tm *tm_sched;
  int ret_value = -1;
  int tag = 0;
  int i,length;
  int pivot_length;
  int sc_length;
  int condweek_length;

  if(!sc_cond) return -1;

  pivot_length = strlen("0000");
  pivot_ptr = (char*)malloc(pivot_length + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,pivot_length + 1);

  time(&sched);
  tm_sched = localtime(&sched);
  tm_sched->tm_mday = day;
  tm_sched->tm_mon = month - 1;
  tm_sched->tm_year = year - 1900;
  sched = mktime(tm_sched);

  chr_ptr = sc_cond;
  sc_length = strlen(sc_cond);
  condweek_length = strlen("Mon");
  while(chr_ptr < sc_cond + sc_length){
    length = 0;
    while(chr_ptr < sc_cond + sc_length 
	  && !isspace((unsigned char)*chr_ptr) && length < pivot_length)
      *(pivot_ptr + length++) = *chr_ptr++;

    for(i = 0; i < 7;i++){
      if(strncasecmp(CondWeek[i],pivot_ptr,condweek_length) == 0){
	ret_value = 0;
	if(i == tm_sched->tm_wday)
	  tag = 1;
      }
    }

    while(chr_ptr < sc_cond + sc_length && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static inline int CheckDay(char* sc_cond,int day)
{
  char* chr_ptr;
  char* pivot_ptr;
  int ret_value = -1;
  int tag = 0;
  int length;
  int pivot_length;
  int sc_length;

  if(!sc_cond) return -1;
  sc_length = strlen(sc_cond);

  pivot_length = strlen("0000");
  pivot_ptr = (char*)malloc(sc_length + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,pivot_length + 1);

  chr_ptr = sc_cond;

  while(chr_ptr < sc_cond + sc_length){
    length = 0;
    while(chr_ptr < sc_cond + sc_length 
	  && !isspace((unsigned char)*chr_ptr) && length < pivot_length)
      *(pivot_ptr + length++) = *chr_ptr++;

    if(atoi(pivot_ptr) > 0 && atoi(pivot_ptr) < 32 && strlen(pivot_ptr) == 2){
      ret_value = 0;
      if(atoi(pivot_ptr) == day)
	tag = 1;
    }

    while(chr_ptr < sc_cond + sc_length && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static inline int CheckSC_Cond(char* sc_cond,int year,int month,int day)
{
  int isOrdinal,isCondDay,isCondMonth,isCondWeek;
  int isCond;

  if(!sc_cond) return -1;

  isCond = 0;
  
  isCondMonth = CheckMonth(sc_cond,month);
  isOrdinal = CheckOrdinal(sc_cond,year,month,day);
  isCondWeek = CheckWeek(sc_cond,year,month,day);
  isCondDay = CheckDay(sc_cond,day);

  switch(isCondMonth){
  case 1:
    /*
     * month is defined and correspond
     */
    if(isCondDay == 1){
      /*
       * correspond to day
       */
      return 1;
    }
    if(isOrdinal == 1 && isCondWeek == 1){
      /*
       * correspond to week and ordinal
       */
      return 1;
    }
    if(isOrdinal == -1 && isCondWeek == 1){
      /*
       * ordinal is not defined and coreespond to week
       */
      return 1;
    }
    if(isCondDay == isOrdinal == isCondWeek == -1){
      /*
       * correspond to month, others are not defined.
       */
      return 1;
    }

    /*
     * correspond to month, but other conditions don't correspond
     */
    return 0;
    break;
  case 0:
    /*
     * month is defined and don't correspond
     */

    return 0;
    break;
  case -1:
    /*
     * month is not defined
     */
    if(isCondDay == 1){
      /*
       * correspond to day
       */
      return 1;
    }
    if(isOrdinal == 1 && isCondWeek == 1){
      /*
       * correspond to week and ordinal
       */
      return 1;
    }
    if(isOrdinal == -1 && isCondWeek == 1){
      /*
       * ordinal is not defined and correspond to week
       */
      return 1;
    }
    return 0;
  }
  
  return -1;
}
  
static inline int datecmp(char* a,char* b)
{
  time_t a_t,b_t;
  struct tm *tm_x;
  char year[5],month[3],day[3];
  int diff;
  int yearmonthday_length;

  yearmonthday_length = strlen("20001020");
  if(strlen(a) != yearmonthday_length) return 0;
  if(strlen(b) != yearmonthday_length) return 0;

  time(&a_t);
  strncpy(year,a,4);
  year[4] = '\0';
  strncpy(month,a + 4,2);
  month[2] = '\0';
  strncpy(day,a + 6,2);
  day[2] = '\0';

  tm_x = localtime(&a_t);
  tm_x->tm_mday = atoi(day);
  tm_x->tm_mon = atoi(month) - 1;
  tm_x->tm_year = atoi(year) - 1900;
  a_t = mktime(tm_x);

  strncpy(year,b,4);
  strncpy(month,b + 4,2);
  strncpy(day,b + 6,2);
  year[4] = '\0';
  month[2] = '\0';
  day[2] = '\0';

  time(&b_t);
  tm_x = localtime(&b_t);
  tm_x->tm_mday = atoi(day);
  tm_x->tm_mon = atoi(month) - 1;
  tm_x->tm_year = atoi(year) - 1900;
  b_t = mktime(tm_x);
 
  if(difftime(a_t, b_t) > 0){
    diff = 1;
  } else if(difftime(a_t, b_t) == 0){
    diff = 0;
  } else {
    diff = -1;
  }

  return diff;
}

/**
 * =================================================================
 *                                                                  
 *                 public methods                                   
 *                                                                  
 * =================================================================
 **/


MHCD* openmhc(const char* home_dir, const char* year_month)
{
  /*
   * home_dir   : your MHC home directory. 
   *              for example,"/home/someone/Mail/schedule/".
   *              '~' is *not* expanded automatically.
   * year_month : "yyyymm"
   *
   * open MHC data in "/home/someone/Mail/schedule/yyyymm/"
   */
		    
  int i;
  DIR* dirp;
  FILE* fp;
  struct dirent *dp;
  char* dirname;
  char* filename;
  mhcent* entry = NULL;
  MHCD* mhc_ptr = NULL;
  _MHCD* tmp_ptr = NULL;
  int home_length;
  int yearmonth_length;

  filename = NULL;
  home_length = strlen(home_dir);
  yearmonth_length = strlen(year_month);
  i = (home_dir[home_length - 1] == '/')? 1:2;
  i += ((year_month[yearmonth_length - 1] == '/')? 0:1);
  dirname = (char*)malloc(home_length + yearmonth_length + i);
  if(!dirname) return NULL;

  strcpy(dirname,home_dir);
  if(home_dir[home_length - 1] != '/')
    strcat(dirname,"/");
  strcat(dirname,year_month);
  if(year_month[yearmonth_length - 1] != '/')
    strcat(dirname,"/");

  filename = (char*)malloc(BUFSIZ);
  if(!filename){
    free(dirname);
    return NULL;
  }
  dirp = opendir(dirname);
  if(dirp == NULL){
    free(dirname);
    return NULL;
  }

  while ((dp = readdir(dirp)) != NULL){
    int filename_length,is_mhcfile,dname_length;

    if(*(dp->d_name) == '.') continue;

    filename_length = 0;
    is_mhcfile = 1;
    dname_length = strlen(dp->d_name);
    while(filename_length < dname_length){
      if(!isdigit((unsigned char)(dp->d_name[filename_length]))){
	is_mhcfile = 0;
	break;
      }
      filename_length++;
    }
    
    if(!is_mhcfile) continue;


    strcpy(filename,dirname);
    strcat(filename,dp->d_name);

    fp = fopen(filename,"r");
    if(fp != NULL){
      entry = ReadEntry(fp);
      if(entry){
	entry->filename = strdup(filename);
	/*
	  entry->filename = (char*)malloc(strlen(filename) + 1);
	  strcpy(entry->filename,filename);
	*/

	if(mhc_ptr == NULL){
	  mhc_ptr = MHCDNew(entry);
	  tmp_ptr = *mhc_ptr;
	} else {
	  tmp_ptr->next = _MHCDNew(entry);
	  tmp_ptr->next->prev = tmp_ptr;
	  tmp_ptr = tmp_ptr->next;
	}

      }
      fclose(fp);
    }
  }
  closedir(dirp);

  if(tmp_ptr != NULL){
    tmp_ptr->next = _MHCDNew(NULL);
    tmp_ptr->next->prev = tmp_ptr;
    tmp_ptr = tmp_ptr->next;
  }

  free(dirname);
  free(filename);
  return mhc_ptr;
}  

mhcent* readmhc(MHCD* mhc_ptr)
{
  /*
   *  return MHC entry pointed by mhc_ptr
   */

  mhcent* ent_ptr;

  if(!mhc_ptr) return NULL;

  if(*mhc_ptr){
    ent_ptr = (*mhc_ptr)->item;
      *mhc_ptr = (*mhc_ptr)->next;
    return ent_ptr;
  }
  return NULL;
}

int closemhc(MHCD* mhc_ptr)
{
  /*
   * close MHC handler
   *
   * if succeed, return 0. otherwise, return 1.
   */
  if(!mhc_ptr) return 1;
  if(!*mhc_ptr) return 1;
  rewindmhc(mhc_ptr);
  MHCDDelete(mhc_ptr);

  free(mhc_ptr);
  return 0;
}

void rewindmhc(MHCD* mhc_ptr)
{
  /*
   * reset MHC handler 
   */
  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;

  while((*mhc_ptr)->prev != NULL)
    *mhc_ptr = (*mhc_ptr)->prev;
}

void seekmhc(MHCD* mhc_ptr,int locate)
{
  /*
   * set MHC handler to location pointed by 'int locate'.
   */
  int forward;

  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;
  
  forward = (locate > 0)? 1:0;

  while(locate && 
	(((*mhc_ptr)->prev && !forward) || ((*mhc_ptr)->next && forward))){
    *mhc_ptr = forward? (*mhc_ptr)->next:(*mhc_ptr)->prev;
    locate = forward? locate - 1:locate + 1;
  }
}

int isschedule(const mhcent* ent_ptr,int year,int month,int day)
{
  /*
   * check whether the MHC entry pointed by ent_ptr is a schedule of
   * year/month/day .
   *
   * If so, return 0. otherwise return -1.
   */
  int isDay,isCond,isDuration;

  isDay = isCond = isDuration = -1;

  if(!ent_ptr) return -1;

  /*
   * Check X-SC-Day
   */

  isDay = CheckSC_Day(ent_ptr->Entry[X_SC_Day],year,month,day);

  /*
   * Check X-SC-Duration 
   */

  isDuration = CheckSC_Duration(ent_ptr->Entry[X_SC_Duration],year,month,day);

  /*
   * Check X-SC-Cond
   */

  isCond = CheckSC_Cond(ent_ptr->Entry[X_SC_Cond],year,month,day);

  /*
   * is** 
   *      = -1: not defined
   *      =  0: not correspond
   *      =  1: correspond
   *
   * Exceptionally, isDay
   *      = -1: not defined about year/month/day
   *      =  0: defined, but not correspond (defined with !)
   *      =  1: defined and correspond
   */    

  switch(isDay){
  case -1:
    if(isCond == -1) return 0;
    if(isCond == 1 && isDuration != 0) return 1;
    return 0;
  case 0:
    return 0;
  case 1:
    if(isDuration != 0) return 1;
    return 0;
  }
  return -1;
}
    
MHC* OpenMHC(const char* home_dir, int year,int month)
{
  /*
   * home_dir   : your MHC home directory. 
   *              for example,"/home/someone/Mail/schedule/".
   *              '~' is *not* extended automatically.
   * year_month : "yyyymm"
   *
   * open MHC data in "/home/someone/Mail/schedule/yyyy/mm/" and
   *                  "/home/someone/Mail/schedule/intersect/".
   */

  MHC* mhc_ptr;
  MHCD* mhcd_ptr;
  mhcent* ent_ptr;
  int day,i;
  char* year_month;

  mhc_ptr = MHCNew();
  if(!mhc_ptr) return NULL;

  year_month = (char*)malloc(strlen("0000/00") + 1);
  if(!year_month) return NULL;
  sprintf(year_month,"%.4d/%.2d",year,month);

  (*mhc_ptr)->mhcd_ptr = openmhc(home_dir,year_month);
  (*mhc_ptr)->intersect_ptr = openmhc(home_dir,"intersect");

  for(i = 0; i < 2;i++){
    mhcd_ptr = i ? (*mhc_ptr)->mhcd_ptr:(*mhc_ptr)->intersect_ptr;
    while((ent_ptr = readmhc(mhcd_ptr)) != NULL){
      for(day = 1; day <= 31;day++){
	if(isschedule(ent_ptr,year,month,day))
	  AddEntry(mhc_ptr,ent_ptr,day);
      }
    }
  }

  free(year_month);
  (*mhc_ptr)->ptr = (*mhc_ptr)->table[0];

  return mhc_ptr;
}

void SetMHC(MHC* mhc_ptr,int day)
{
  /*
   * set MHC handler to yyyy/mm/day .
   *
   * mhc_ptr is already opened by OpenMHC() for yyyy/mm .
   */

  if(!mhc_ptr || day < 1 || day > 31) return;
  (*mhc_ptr)->ptr = (*mhc_ptr)->table[day];
}

mhcent* ReadMHC(MHC* mhc_ptr){
  /*
   * read MHC entry pointed by mhc_ptr
   */

  mhcent* ent_ptr;

  if(!mhc_ptr) return NULL;
  if(!(*mhc_ptr)) return NULL;

  if((*mhc_ptr)->ptr){
    ent_ptr = (*mhc_ptr)->ptr->item;
    if(ent_ptr)
      (*mhc_ptr)->ptr = (*mhc_ptr)->ptr->next;

    return ent_ptr;
  } else {
    return NULL;
  }
}

void RewindMHC(MHC* mhc_ptr)
{
  /*
   * reset MHC handler
   */

  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;
  if(!(*mhc_ptr)->ptr) return;

  while((*mhc_ptr)->ptr->prev)
    (*mhc_ptr)->ptr = (*mhc_ptr)->ptr->prev;
}

void SeekMHC(MHC* mhc_ptr,int locate)
{
  /*
   * set MHC handler to location pointed by 'int locate'.
   */

  int forward;

  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;
  if(!(*mhc_ptr)->ptr) return;
  
  forward = (locate > 0)? 1:0;

  while(locate && 
	(((*mhc_ptr)->ptr->prev && !forward) 
	 || ((*mhc_ptr)->ptr->next && forward))){
    (*mhc_ptr)->ptr = forward? (*mhc_ptr)->ptr->next:(*mhc_ptr)->ptr->prev;
    locate = forward? locate - 1:locate + 1;
  }
}
    
int CloseMHC(MHC* mhc_ptr)
{
  /*
   * close MHC handler.
   * If succeed, return 0. otherwise, return 1.
   */

  int i,rv;

  if(!mhc_ptr) return 1;
  if(!(*mhc_ptr)) return 1;

  for(i = 0; i < NUM_OF_ARRAY((*mhc_ptr)->table);i++)
    EntrylistDelete((*mhc_ptr)->table[i]);

  rv = closemhc((*mhc_ptr)->mhcd_ptr);
  rv = closemhc((*mhc_ptr)->intersect_ptr);
  free(*mhc_ptr);
  free(mhc_ptr);

  return rv;
}

char* GetSubject(const mhcent* ent_ptr)
{
  /*
   * return X-SC-Subject: 
   */
  return ent_ptr->Entry[X_SC_Subject];
}

int GetAlarm(const mhcent* ent_ptr)
{
  /*
   * return X-SC-Alarm by minutes.
   */
  char* chr_ptr;
  char* tmp;
  int length,ret_value;

  if(!ent_ptr) return -1;

  if(!ent_ptr->Entry[X_SC_Alarm]) return 0;

  tmp = (char*)malloc(strlen("00") + 1);
  if(!tmp) return -1;

  chr_ptr = ent_ptr->Entry[X_SC_Alarm];
  length = ret_value = 0;
  while(chr_ptr && !isspace((unsigned char)*chr_ptr) && length < 2)
    *(tmp + length++) = *chr_ptr++;

  chr_ptr++;

  if(strcasecmp(chr_ptr,"minute") == 0){
    ret_value = atoi(tmp);
  } else if (strcasecmp(chr_ptr,"hour") == 0){
    ret_value = atoi(tmp) * 60;
  } else if (strcasecmp(chr_ptr,"day") == 0){
    ret_value = atoi(tmp) * 60 * 24;
  }

  free(tmp);
  return ret_value;
}
  

