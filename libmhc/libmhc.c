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
static entrylist* EntrylistNew(mhcent*);
static int AddEntry(MHC*,mhcent*,int);
static int CheckSC_Day(char*,int,int,int);
static int CheckSC_Duration(char*,int,int,int);
static int CheckSC_Cond(char*,int,int,int);
static int CheckMonth(char*,int);
static int CheckOrdinal(char*,int,int,int);
static int CheckWeek(char*,int,int,int);
static int CheckDay(char* ,int);
static int datecmp(char*,char*);

static MHCD* MHCDNew(mhcent* item){
  MHCD* mhc_ptr;

  mhc_ptr = (MHCD*)malloc(sizeof(MHCD));
  if(!mhc_ptr) return NULL;

  *mhc_ptr = (_MHCD*)malloc(sizeof(_MHCD));
  if(!(*mhc_ptr)) return NULL;

  (*mhc_ptr)->prev = NULL;
  (*mhc_ptr)->next = NULL;
  (*mhc_ptr)->item = item;

  return mhc_ptr;
}

static void MHCDDelete(MHCD* mhc_ptr){
  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;

  if(!((*mhc_ptr)->next))
    MHCDDelete(&((*mhc_ptr)->next));

  if((*mhc_ptr)->item != NULL)
    EntryDelete((*mhc_ptr)->item);

  memset(*mhc_ptr,0,sizeof(MHC));
  free(*mhc_ptr);
}

static mhcent* EntryNew(){
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
      
static void EntryDelete(mhcent* item){
  int i;

  for(i = 0; i < NUM_OF_ARRAY(item->Entry);i++){
    if(!item->Entry[i])
      free(item->Entry[i]);
    if(!item->filename)
      free(item->filename);
  }
}

static mhcent* ReadEntry(FILE* fp){
  mhcent* mhcent_ptr;
  char* buffer;
  char* chr_ptr;
  int tag;

  buffer = (char*)malloc(BUFSIZ);
  if(!buffer) return NULL;

  mhcent_ptr = EntryNew();
  if(!mhcent_ptr){
    free(buffer);
    return NULL;
  }

  while(fgets(buffer,BUFSIZ,fp) != NULL){
    tag = -1;

    if(buffer[strlen(buffer) - 1] == '\n')
      buffer[strlen(buffer) - 1] = '\0';

    if(isspace((unsigned char)buffer[strlen(buffer) - 1]))
      buffer[strlen(buffer) - 1] = '\0';

    if(!strncmp("X-SC-Day:",buffer,strlen("X-SC-Day:"))){
      tag = X_SC_Day;
    } else if(!strncmp("X-SC-Time:",buffer,strlen("X-SC-Time:"))){
      tag = X_SC_Time;        
    } else if(!strncmp("X-SC-Duration:",buffer,strlen("X-SC-Duration:"))){
      tag = X_SC_Duration;        
    } else if(!strncmp("X-SC-Cond:",buffer,strlen("X-SC-Cond:"))){
      tag = X_SC_Cond;        
    } else if(!strncmp("X-SC-Alarm:",buffer,strlen("X-SC-Alarm:"))){
      tag = X_SC_Alarm;
    } else if(!strncmp("X-SC-Subject:",buffer,strlen("X-SC-Subject:"))){
      tag = X_SC_Subject;
    }

    if(tag != -1){
      /*
       * タグを読みとばす
       */
      for(chr_ptr = buffer;chr_ptr;chr_ptr++)
	if(*chr_ptr == ':') break;

      /*
       * タグの後ろのスペースを読み飛ばす
       */
      for(chr_ptr++;chr_ptr;chr_ptr++)
	if(!isspace((unsigned char)*chr_ptr)) break;
    
      mhcent_ptr->Entry[tag] = strdup(chr_ptr);
    }
  }

  free(buffer);
  return mhcent_ptr;
}

static MHC* MHCNew(){
  MHC* mhc_ptr;
  int i;

  mhc_ptr = (MHC*)malloc(sizeof(MHC));
  if(!mhc_ptr) return NULL;
  memset(mhc_ptr,0,sizeof(MHC));

  *mhc_ptr = (_MHC*)malloc(sizeof(_MHC));
  if(!(*mhc_ptr)) return NULL;
  for(i = 0 ; i < 31;i++)
    (*mhc_ptr)->table[i] = NULL;

  (*mhc_ptr)->ptr = NULL;
  (*mhc_ptr)->mhcd_ptr = NULL;
  (*mhc_ptr)->intersect_ptr = NULL;

  return mhc_ptr;
}

static entrylist* EntrylistNew(mhcent* item){
  entrylist* newlist;

  newlist = (entrylist*)malloc(sizeof(entrylist));
  if(!newlist) return NULL;

  newlist->prev = NULL;
  newlist->next = NULL;
  newlist->item = item;

  return newlist;
}

static int AddEntry(MHC* mhc_ptr,mhcent* ent_ptr,int day){
  entrylist* newlist;
  entrylist* list_ptr;

  if(day < 1 || day > 31) return 1;

  newlist = EntrylistNew(ent_ptr);
  if(!newlist) return 1;
  
  if(!(*mhc_ptr)->table[day]){
    (*mhc_ptr)->table[day]= newlist;
  } else {
    list_ptr = (*mhc_ptr)->table[day];

    while(list_ptr->next)
      list_ptr = list_ptr->next;

    list_ptr->next = newlist;
    newlist->prev = list_ptr;
  }

  return 0;
}
      
static void EntrylistDelete(entrylist* list_ptr){
  if(!list_ptr) return;

  if(!list_ptr->next)
    EntrylistDelete(list_ptr->next);

  memset(list_ptr,0,sizeof(MHC));
  free(list_ptr);
}

static int CheckSC_Day(char* sc_day,int year,int month,int day){
  char* chr_ptr;
  char* yearmonth;
  char* pivot_ptr;
  int sig,isDay;
  isDay = -1;

  if(!sc_day || !*sc_day) return -1;

  pivot_ptr = (char*)malloc(strlen("00") + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,strlen("00") + 1);

  yearmonth = (char*)malloc(strlen("000000") + 1);
  if(!yearmonth){
    free(pivot_ptr);
    return -1;
  }
  sprintf(yearmonth,"%.4d%.2d",year,month);

  chr_ptr = sc_day;

  while(chr_ptr < sc_day + strlen(sc_day)){
    sig = 0;
    if(*chr_ptr == '!'){
      sig = 1;
      chr_ptr++;
    }

    if(!strncmp(chr_ptr,yearmonth,strlen(yearmonth))){
      chr_ptr += strlen(yearmonth);
      strncpy(pivot_ptr,chr_ptr,strlen("00"));
      pivot_ptr[2] = '\0';
      if(day == atoi(pivot_ptr)){
	isDay = 1;
      }else{
	isDay = 0;
      }
      chr_ptr += strlen("00");
    } else {
      isDay = 0;
      chr_ptr += strlen("00") + strlen(yearmonth);
    }
    
    switch(isDay){
    case 0:
      if(sig)
	isDay = 1;
      break;
    case 1:
      if(sig)
	isDay = 0;
      break;
    }

    if(chr_ptr >= sc_day + strlen(sc_day)) break;
    if((isDay == 1 && sig == 0) || (isDay == 0 && sig == 1)) break;
    while(isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  return isDay;

}

static int CheckSC_Duration(char* sc_duration,int year,int month,int day){
  char* duration_begin;
  char* duration_end;
  char* yearmonthday;
  char* chr_ptr;
  int isDuration = -1;
  int length;

  if(!sc_duration || !*sc_duration) return -1;

  duration_begin = (char*)malloc(strlen("00000000") + 1);
  if(!duration_begin)
    return -1;

  memset(duration_begin,0,strlen("00000000") + 1);
  
  duration_end = (char*)malloc(strlen("00000000") + 1);
  if(!duration_end){
    free(duration_begin);
    return -1;
  }
  memset(duration_end,0,strlen("00000000") + 1);

  yearmonthday = (char*)malloc(strlen("00000000") + 1);
  if(!yearmonthday){
    free(duration_begin);
    free(duration_end);
    return -1;
  }
  sprintf(yearmonthday,"%.4d%.2d%.2d",year,month,day);

  chr_ptr = sc_duration;

  while(chr_ptr < sc_duration + strlen(sc_duration)){
    while(isspace((unsigned char)*chr_ptr) && *chr_ptr != '\0');

    length = 0;
    while(chr_ptr < sc_duration + strlen(sc_duration)
	  && *chr_ptr != '-' && length < strlen("00000000")){
      *(duration_begin + length) = *chr_ptr++;
      length++;
    }
    chr_ptr++;
    length = 0;
    while(chr_ptr < sc_duration + strlen(sc_duration)
	  && length < strlen("00000000")){
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

  return isDuration;

}

static int CheckMonth(char* sc_cond,int month){
  char CondMonth[][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul"
      	      ,"Aug","Sep","Oct","Nov","Dec"};
  char* chr_ptr;
  char* pivot_ptr;
  int ret_value = -1;
  int tag = 0;
  int i,length;

  if(!sc_cond || !*sc_cond) return -1;
  pivot_ptr = (char*)malloc(strlen("0000") + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,strlen("0000") + 1);

  chr_ptr = sc_cond;
  while(chr_ptr < sc_cond + strlen(sc_cond)){
    length = 0;
    while(!isspace((unsigned char)*chr_ptr) && length < strlen("0000"))
      *(pivot_ptr + length++) = *chr_ptr++;

    for(i = 0; i < 12;i++){
      if(!strcmp(CondMonth[i],pivot_ptr)){
	ret_value = 0;
	if(i == month - 1)
	  tag = 1;
      }
    }

    while(chr_ptr < sc_cond + strlen(sc_cond) && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  if(!pivot_ptr)
    free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static int CheckOrdinal(char* sc_cond,int year,int month,int day){
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

  if(!sc_cond || !*sc_cond) return -1;

  pivot_ptr = (char*)malloc(strlen("0000") + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,strlen("0000") + 1);

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
  while(chr_ptr < sc_cond + strlen(sc_cond)){
    length = 0;
    while(chr_ptr < sc_cond + strlen(sc_cond) && !isspace((unsigned char)*chr_ptr) && length < strlen("0000"))
      *(pivot_ptr + length++) = *chr_ptr++;

    for(i = 0; i < 4;i++){
      if(!strcmp(Ordinal[i],pivot_ptr)){
	ret_value = 0;
	if(i == tOrdinal)
	  tag = 1;
      }
    }

    if(!strcmp(pivot_ptr,"Last") && isLast){
      ret_value = 0;
      tag = 1;
    }

    while(chr_ptr < sc_cond + strlen(sc_cond) && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  if(!pivot_ptr)
    free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static int CheckWeek(char* sc_cond,int year,int month,int day){
  char CondWeek[][4] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

  char* chr_ptr;
  char* pivot_ptr;
  time_t sched;
  struct tm *tm_sched;
  int ret_value = -1;
  int tag = 0;
  int i,length;

  if(!sc_cond || !*sc_cond) return -1;

  pivot_ptr = (char*)malloc(strlen("0000") + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,strlen("0000") + 1);

  time(&sched);
  tm_sched = localtime(&sched);
  tm_sched->tm_mday = day;
  tm_sched->tm_mon = month - 1;
  tm_sched->tm_year = year - 1900;
  sched = mktime(tm_sched);

  chr_ptr = sc_cond;
  while(chr_ptr < sc_cond + strlen(sc_cond)){
    length = 0;
    while(chr_ptr < sc_cond + strlen(sc_cond) && !isspace((unsigned char)*chr_ptr) && length < strlen("0000"))
      *(pivot_ptr + length++) = *chr_ptr++;

    for(i = 0; i < 7;i++){
      if(!strcmp(CondWeek[i],pivot_ptr)){
	ret_value = 0;
	if(i == tm_sched->tm_wday)
	  tag = 1;
      }
    }

    while(chr_ptr < sc_cond + strlen(sc_cond) && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  if(!pivot_ptr)
    free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static int CheckDay(char* sc_cond,int day){
  char* chr_ptr;
  char* pivot_ptr;
  int ret_value = -1;
  int tag = 0;
  int length;

  if(!sc_cond || !*sc_cond) return -1;

  pivot_ptr = (char*)malloc(strlen("0000") + 1);
  if(!pivot_ptr)
    return -1;
  memset(pivot_ptr,0,strlen("0000") + 1);

  chr_ptr = sc_cond;
  while(chr_ptr < sc_cond + strlen(sc_cond)){
    length = 0;
    while(chr_ptr < sc_cond + strlen(sc_cond) && !isspace((unsigned char)*chr_ptr) && length < strlen("0000"))
      *(pivot_ptr + length++) = *chr_ptr++;

    if(atoi(pivot_ptr) > 0 && atoi(pivot_ptr) < 32 && strlen(pivot_ptr) == 2){
      ret_value = 0;
      if(atoi(pivot_ptr) == day)
	tag = 1;
    }

    while(chr_ptr < sc_cond + strlen(sc_cond) && isspace((unsigned char)*chr_ptr))
      chr_ptr++;
  }

  if(!pivot_ptr)
    free(pivot_ptr);

  if(ret_value != -1) return tag;

  return -1;
}

static int CheckSC_Cond(char* sc_cond,int year,int month,int day){
  int isOrdinal,isCondDay,isCondMonth,isCondWeek;
  int isCond;

  if(!sc_cond || !*sc_cond) return -1;

  isCond = 0;
  
  isCondMonth = CheckMonth(sc_cond,month);
  isOrdinal = CheckOrdinal(sc_cond,year,month,day);
  isCondWeek = CheckWeek(sc_cond,year,month,day);
  isCondDay = CheckDay(sc_cond,day);

  switch(isCondMonth){
  case 1:
    /*
     * 月が定義されていて、しかも該当する
     */
    if(isCondDay == 1){
      /*
       * 該当する日である
       */
      return 1;
    }
    if(isOrdinal == 1 && isCondWeek == 1){
      /*
       * 序数と曜日が該当する
       */
      return 1;
    }
    if(isOrdinal == -1 && isCondWeek == 1){
      /*
       * 序数が未定義で曜日が該当する
       */
      return 1;
    }
    if(isCondDay == isOrdinal == isCondWeek == -1){
      /*
       * 月以外全て見定義
       */
      return 1;
    }

    /*
     * 月はあっているが、それ以外の条件があっていない
     */
    return 0;
    break;
  case 0:
    /*
     * 月が定義されていて、しかも該当しない
     */

    return 0;
    break;
  case -1:
    /*
     * 月が未定義
     */
    if(isCondDay == 1){
      /*
       * 該当する日である
       */
      return 1;
    }
    if(isOrdinal == 1 && isCondWeek == 1){
      /*
       * 序数と曜日が該当する
       */
      return 1;
    }
    if(isOrdinal == -1 && isCondWeek == 1){
      /*
       * 序数が未定義で曜日が該当する
       */
      return 1;
    }
    return 0;
  }
  
  return -1;
}
  
static int datecmp(char* a,char* b){
  time_t a_t,b_t;
  struct tm *tm_x;
  char year[5],month[3],day[3];
  int diff;

  if(strlen(a) != strlen("20001020")) return 0;
  if(strlen(b) != strlen("00001020")) return 0;

  time(&a_t);
  strncpy(year,a,4);
  year[4] = '\0';
  strncpy(month,a + 4,2);
  month[4] = '\0';
  strncpy(day,a + 6,2);
  day[4] = '\0';

  tm_x = localtime(&a_t);
  tm_x->tm_mday = atoi(day);
  tm_x->tm_mon = atoi(month) - 1;
  tm_x->tm_year = atoi(year) - 1900;
  a_t = mktime(tm_x);

  strncpy(year,b,4);
  strncpy(month,b + 4,2);
  strncpy(day,b + 6,2);
  year[4] = '\0';
  month[4] = '\0';
  day[4] = '\0';

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


MHCD* openmhc(const char* home_dir, const char* year_month){
  int i;
  DIR* dirp;
  FILE* fp;
  struct dirent *dp;
  char* dirname;
  char* filename;
  mhcent* entry = NULL;
  MHCD* mhc_ptr = NULL;
  MHCD* tmp_ptr;

  filename = NULL;
  i = (home_dir[strlen(home_dir) - 1] == '/')? 1:2;
  i = (year_month[strlen(year_month) - 1] == '/')? i:i+1;
  dirname = (char*)malloc(strlen(home_dir) + strlen(year_month) + i);
  if(!dirname) return NULL;

  strcpy(dirname,home_dir);
  if(home_dir[strlen(home_dir) - 1] != '/')
    strcat(dirname,"/");
  strcat(dirname,year_month);
  if(year_month[strlen(year_month) - 1] != '/')
    strcat(dirname,"/");

  dirp = opendir(dirname);
  if(dirp == NULL){
    free(dirname);
    return NULL;
  }

  while ((dp = readdir(dirp)) != NULL){
    if(!strncmp(dp->d_name,".",1)) continue;

    filename = filename?
      (char*)malloc(strlen(dirname) + strlen(dp->d_name) + 1):
      (char*)realloc(filename,strlen(dirname) + strlen(dp->d_name) + 1);

    if(!filename){
      free(dirname);
      return NULL;
    }

    strcpy(filename,dirname);
    strcat(filename,dp->d_name);
    fp = fopen(filename,"r");

    if(fp){
      entry = ReadEntry(fp);
      if(entry){
	entry->filename = strdup(filename);

	if(!mhc_ptr){
	  mhc_ptr = (MHCD*)malloc(sizeof(MHCD));
	  tmp_ptr = (MHCD*)malloc(sizeof(MHCD));
	  *mhc_ptr = *tmp_ptr = *MHCDNew(entry);
	} else {
	  (*tmp_ptr)->next = *MHCDNew(entry);
	  (*tmp_ptr)->next->prev = *tmp_ptr;
	  *tmp_ptr = (*tmp_ptr)->next;
	}
      }
      fclose(fp);
    }
  }
  closedir(dirp);

  free(dirname);
  free(filename);
  return mhc_ptr;
}  

mhcent* readmhc(MHCD* mhc_ptr){
  mhcent* ent_ptr;

  if(!mhc_ptr) return NULL;

  if(*mhc_ptr){
    ent_ptr = (*mhc_ptr)->item;
    if(ent_ptr)
      *mhc_ptr = (*mhc_ptr)->next;

    return ent_ptr;
  } else {
    return NULL;
  }
}

int closemhc(MHCD* mhc_ptr){
  if(!mhc_ptr) return 1;
  rewindmhc(mhc_ptr);
  MHCDDelete(mhc_ptr);

  free(*mhc_ptr);
  (*mhc_ptr) = NULL;
  return 0;
}

void rewindmhc(MHCD* mhc_ptr){
  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;

  while((*mhc_ptr)->prev)
    *mhc_ptr = (*mhc_ptr)->prev;
}

void seekmhc(MHCD* mhc_ptr,int locate){
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

int isschedule(const mhcent* ent_ptr,int year,int month,int day){
  int isDay,isCond,isDuration;

  isDay = isCond = isDuration = -1;

  if(!ent_ptr) return -1;

  /*
   * まず X-SC-Day のチェック 
   */

  isDay = CheckSC_Day(ent_ptr->Entry[X_SC_Day],year,month,day);

  /*
   * X-SC-Duration のチェック
   */

  isDuration = CheckSC_Duration(ent_ptr->Entry[X_SC_Duration],year,month,day);

  /*
   * X-SC-Cond のチェック
   */

  isCond = CheckSC_Cond(ent_ptr->Entry[X_SC_Cond],year,month,day);

  /*
   * is** 
   *      = -1: 無指定
   *      =  0: 該当しない
   *      +  1: 該当する
   */    

  switch(isDay){
  case -1:
    if(isCond == 1 && isDuration != 0) return 1;
    return 0;
  case 0:
    return 0;
  case 1:
    if(isCond != 0 && isDuration != 0) return 1;
    return 0;
  }

  return -1;
}
    
MHC* OpenMHC(const char* home_dir, int year,int month){
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

  return mhc_ptr;
}

void SetMHC(MHC* mhc_ptr,int day){
  if(!mhc_ptr || day < 1 || day > 31) return;
  (*mhc_ptr)->ptr = (*mhc_ptr)->table[day];
}

mhcent* ReadMHC(MHC* mhc_ptr){
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

void RewindMHC(MHC* mhc_ptr){
  if(!mhc_ptr) return;
  if(!*mhc_ptr) return;
  if(!(*mhc_ptr)->ptr) return;

  while((*mhc_ptr)->ptr->prev)
    (*mhc_ptr)->ptr = (*mhc_ptr)->ptr->prev;
}

void SeekMHC(MHC* mhc_ptr,int locate){
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
    
int CloseMHC(MHC* mhc_ptr){
  int i,rv;

  if(!mhc_ptr) return 1;
  if(!(*mhc_ptr)) return 1;

  for(i = 0; i < NUM_OF_ARRAY((*mhc_ptr)->table);i++){
    EntrylistDelete((*mhc_ptr)->table[i]);
    (*mhc_ptr)->table[i] = NULL;
  }

  rv = closemhc((*mhc_ptr)->mhcd_ptr);
  free(*mhc_ptr);
  (*mhc_ptr) = NULL;

  return rv;
}

char* GetSubject(const mhcent* ent_ptr){
  return ent_ptr->Entry[X_SC_Subject];
}

int GetAlarm(const mhcent* ent_ptr){
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

  if(!strcmp(chr_ptr,"minute")){
    ret_value = atoi(tmp);
  } else if (!strcmp(chr_ptr,"hour")){
    ret_value = atoi(tmp) * 60;
  } else if (!strcmp(chr_ptr,"day")){
    ret_value = atoi(tmp) * 60 * 24;
  }

  free(tmp);
  return ret_value;
}
  

