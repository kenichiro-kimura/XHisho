#define _SCHEDULE_GLOBAL
#include "globaldefs.h"
#include "openwin.h"
#include <ctype.h>
#include <sys/stat.h>
#include <X11/Xlocale.h>
#ifdef LIBMHC
#include "mhc.h"
#endif      

static HolidayList *Hlist[12];

static const int wdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int SafeTimeFormat(Schedule);
static void ReadHolidayFile(char *);
static HolidayList *HolidayList_new(int, char *);


static HolidayList *HolidayList_new(int day, char *name)
{
  /**
   * HolidayListのコンストラクタ。memsetによる0クリアは念のため。たぶんいらん。
   * day,nameに引数で与えたものをセットし、nextをNULLにする。
   **/

  HolidayList *h_ptr;

  h_ptr = (HolidayList *) malloc(sizeof(HolidayList));
  memset(h_ptr, 0, sizeof(HolidayList));

  h_ptr->day = day;
  h_ptr->next = NULL;
  h_ptr->name = malloc(strlen(name) + 1);
  memset(h_ptr->name, 0, strlen(name) + 1);
  strcpy(h_ptr->name, name);

  return h_ptr;
}


int CheckSchedule(OpenMessageRes * l_omr, Schedule * schedule, int WeeklyCheck, struct tm tm_now)
{
  /**
   * return number of schedule readed from the file xhs* and/or xhs.weekly
   *
   *  WeeklyCheck = 0: read from xhs*
   *              = 1: read from both
   *              = 2: read from xhs.weekly
   **/

  FILE *inputfile;
  time_t tval;
  int i, j,is_week,nth_week,uru_adjust;
  char filename[128], day[3], month[4], tdate[5], week[2], year[5];
  unsigned char *tmp1, *tmp2, *tmp3, *tmp4, *leave;
  unsigned char *string_index;

#ifdef EXT_FILTER
  char command[128];
#endif
#ifdef LIBMHC
  MHC* mhc_ptr;
  mhcent* ent_ptr;
  FILE *t_file;
  char *t_filename, *Tmp_dir;
  char home[BUFSIZ];
#endif      


  tmp1 = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  tmp3 = malloc(BUFSIZ);
  tmp4 = malloc(BUFSIZ);
  leave = malloc(BUFSIZ);

  i = 0;

  tval = mktime(&tm_now);

  /**
   * 今日の日付を取得する。7/20ならdate="0720"になる。
   **/

  strftime(day, sizeof(day), "%d", localtime(&tval));
  strftime(month, sizeof(month), "%m", localtime(&tval));
  strftime(week, sizeof(week), "%u", localtime(&tval));
  strftime(year, sizeof(year), "%G", localtime(&tval));

  strncpy(l_omr->month, month, 2);
  strncpy(l_omr->day, day, 2);

  if (l_omr->xcalendar) {
    setlocale(LC_TIME, "C");
    strftime(month, sizeof(month), "%b", localtime(&tval));
    sprintf(filename, "%s/%sxc%d%s%d", getenv("HOME"),
	    l_omr->sched_dir, atoi(day), month, atoi(year));
  } else {
    sprintf(filename, "%s/%sxhs%s%s", getenv("HOME"), l_omr->sched_dir, month, day);
  }

  if ((inputfile = fopen(filename, "r")) != NULL) {

#ifdef EXT_FILTER
    fclose(inputfile);
    sprintf(command, "%s %s", l_omr->ext_filter, filename);
    inputfile = popen(command, "r");
#endif

    while (fgets(tmp1, BUFSIZ - 1, inputfile) != NULL && (i < MAX_SCHED_NUM)) {

      /**
       * もし # で始まっていたらコメントとみなし、その後1行を無視する。
       * 空行も同様。
       **/

      *tmp2 = *tmp3 = *leave = '\0';

      if (tmp1[0] != '#' && tmp1[0] != '\0' && tmp1[0] != '\n' && tmp1[0] != ' ') {
	sscanf(tmp1, "%s %s %s", tmp2, leave, tmp3);

	if (!atoi(leave)) {
	  sscanf(tmp1, "%s %s", tmp2, tmp3);
	  if(!strcmp(leave,"*") || !strcmp(leave,"0")){
	    schedule[i].leave = -1;
	  } else {
	    schedule[i].leave = omr.leave_t;
	  }
	} else {
	  schedule[i].leave = atoi(leave);
	}

	strncpy(tdate, tmp2, sizeof(tdate));
	tdate[4] = '\0';
	strncpy(schedule[i].hour, tdate, sizeof(char) * 2);
	strncpy(schedule[i].min, tdate + 2, sizeof(char) * 2);
	
	/**
	 * 開始時刻とleaveを読み飛ばす。
	 **/

	for (j = 0; j < strlen(tmp1); j++)
	  if (isspace(tmp1[j]))
	    break;
	for (j++; j < strlen(tmp1); j++)
	  if (isspace(tmp1[j]))
	    break;

	string_index = tmp1 + j + 1;
	
	if (string_index) {
	  Escape2Return(string_index);
	  strncpy(schedule[i].ev, string_index, MIN(BUFSIZ, strlen(string_index)));

	  if (schedule[i].ev[MIN(BUFSIZ, strlen(string_index)) - 1] == '\n')
	    schedule[i].ev[MIN(BUFSIZ, strlen(string_index)) - 1] = '\0';
	  if (SafeTimeFormat(schedule[i])) {
	    i++;
	  }
	}
      }
    }
#ifdef EXT_FILTER
    pclose(inputfile);
#else
    fclose(inputfile);
#endif
  }

#ifdef LIBMHC
  if(WeeklyCheck){
    t_filename = malloc(BUFSIZ * 2);
    Tmp_dir = malloc(BUFSIZ);

    sprintf(Tmp_dir, "/tmp/xhtmp%s", getenv("USER"));
    mkdir(Tmp_dir, S_IRWXU);

    t_filename[0] = '\0';
    sprintf(t_filename, "%s", tempnam(Tmp_dir, "xhtmp"));
    sprintf(home,"%s/Mail/schedule/",getenv("HOME"));
    mhc_ptr = OpenMHC(home,atoi(year),atoi(month));
    SetMHC(mhc_ptr,atoi(day));

    while((ent_ptr = ReadMHC(mhc_ptr)) != NULL && i < MAX_SCHED_NUM){
#ifdef EXT_FILTER
      if ((t_file = fopen(t_filename, "w")) == NULL) {
	fprintf(stderr, "can't open temporary file,%s\n", t_filename);
	perror("popen");
	exit(1);
      }
      fprintf(t_file, "%s\n", ent_ptr->Entry[X_SC_Subject]);
      fclose(t_file);

      sprintf(command, "%s %s", FilterCommand, t_filename);
      inputfile = popen(command, "r");

      fgets(tmp1, BUFSIZ, inputfile);

      pclose(inputfile);
      unlink(t_filename);
#else
      strncpy(tmp1, ent_ptr->Entry[X_SC_Subject],MIN(strlen(ent_ptr->Entry[X_SC_Subject],BUFSIZ)));
#endif

      if(ent_ptr->Entry[X_SC_Time] && strlen(ent_ptr->Entry[X_SC_Time]) >= 4){
	strncpy(schedule[i].hour, ent_ptr->Entry[X_SC_Time], sizeof(char) * 2);
	strncpy(schedule[i].min, ent_ptr->Entry[X_SC_Time] + 3, sizeof(char) * 2);
      } else {
	strcpy(schedule[i].hour, "*");
      }

      schedule[i].leave = GetAlarm(ent_ptr);
      if(schedule[i].leave <= 0) schedule[i].leave = 0;
      if(schedule[i].leave > 60 * 24 - 1) schedule[i].leave = 0;

      Escape2Return(tmp1);
      strncpy(schedule[i].ev, tmp1, MIN(BUFSIZ, strlen(tmp1)));
      if (schedule[i].ev[MIN(BUFSIZ, strlen(tmp1)) - 1] == '\n')
	schedule[i].ev[MIN(BUFSIZ, strlen(tmp1)) - 1] = '\0';

      if (SafeTimeFormat(schedule[i])){
	i++;
      }else{
	strcpy(schedule[i].hour, "*");
	i++;
      }
	
    }

    CloseMHC(mhc_ptr);
    free(t_filename);
    free(Tmp_dir);
  }
#endif      
    
  if (!WeeklyCheck) {
    free(tmp1);
    free(tmp2);
    free(tmp3);
    free(tmp4);
    free(leave);
    return (i);
  }

  /**
   * Weekly data の取得
   **/

  filename[0] = '\0';
  sprintf(filename, "%s/%sxhs.weekly", getenv("HOME"), l_omr->sched_dir);

  if ((inputfile = fopen(filename, "r")) == NULL || i >= MAX_SCHED_NUM) {
    free(tmp1);
    free(tmp2);
    free(tmp3);
    free(tmp4);
    free(leave);
    return (i);
  }
#ifdef EXT_FILTER
  sprintf(command, "%s %s", l_omr->ext_filter, filename);
  inputfile = popen(command, "r");
#endif

  while (fgets(tmp1, BUFSIZ, inputfile) != NULL && (i < MAX_SCHED_NUM)) {
    /**
     * もし # で始まっていたらコメントとみなし、その後1行を無視する。
     * 空行も同様。
     **/

    if (tmp1[0] != '#' && tmp1[0] != '\0' && tmp1[0] != '\n') {
      sscanf(tmp1, "%s %s %s %s", tmp4, tmp2, leave, tmp3);

      if (!isdigit(leave[0])) {
	sscanf(tmp1, "%s %s %s", tmp4, tmp2, tmp3);
	for ( j = 0; j < strlen(tmp1);j++)
	  if(isspace(tmp1[j])) break;
	for ( j++; j < strlen(tmp1);j++)
	  if(isspace(tmp1[j])) break;
	schedule[i].leave = omr.leave_t;
      } else {
	for ( j = 0; j < strlen(tmp1);j++)
	  if(isspace(tmp1[j])) break;
	for ( j++; j < strlen(tmp1);j++)
	  if(isspace(tmp1[j])) break;
	for ( j++; j < strlen(tmp1);j++)
	  if(isspace(tmp1[j])) break;
	schedule[i].leave = atoi(leave);
      }

      string_index = tmp1 + j + 1;

      /**
       * 該当日かどうかのチェック
       **/

      is_week = 0;
      nth_week = (atoi(day) -1) / 7;
      uru_adjust = ((atoi(year) % 4) == 0 && atoi(month) == 2) ? 1 : 0;

      if(strlen(tmp4) == 1 && atoi(week) == atoi(tmp4)) is_week = 1;
      if(strlen(tmp4) == 2){
	if(atoi(tmp4 + 1) == atoi(week)){
	  if(tmp4[0] - '0' == nth_week && atoi(tmp4 +1) == atoi(day)) is_week = 1;
	  if(tmp4[0] == '6' && atoi(day) + 7 > wdays[atoi(month) -1] + uru_adjust)
	    is_week = 1;
	}
      }

      if (is_week) {
	strncpy(tdate, tmp2, sizeof(tdate));
	strncpy(schedule[i].hour, tdate, sizeof(char) * 2);
	strncpy(schedule[i].min, tdate + 2, sizeof(char) * 2);


	if (string_index) {
	  Escape2Return(string_index);
	  strncpy(schedule[i].ev, string_index, MIN(BUFSIZ, strlen(string_index)));
	  if (schedule[i].ev[MIN(BUFSIZ, strlen(string_index)) - 1] == '\n')
	    schedule[i].ev[MIN(BUFSIZ, strlen(string_index)) - 1] = '\0';
	  if (SafeTimeFormat(schedule[i]))
	    i++;
	}
      }
    }
  }

#ifdef EXT_FILTER
  pclose(inputfile);
#else
  fclose(inputfile);
#endif

  free(tmp1);
  free(tmp2);
  free(tmp3);
  free(tmp4);
  free(leave);
  return (i);
}


static int SafeTimeFormat(Schedule schedule)
{
  /**
   * return 1 if schedule.[hour,min] is safe time format,else return 0
   **/

  if (isdigit(schedule.hour[0]) &&
      isdigit(schedule.min[0]) &&
      isdigit(schedule.hour[1]) &&
      isdigit(schedule.min[1]) &&
      atoi(schedule.hour) >= 0 &&
      atoi(schedule.hour) <= 23 &&
      atoi(schedule.min) >= 0 &&
      atoi(schedule.min) <= 59) {
    return 1;
  } else {
    if(schedule.hour[0] == '*') return 1;
    return 0;
  }
}

int ExistSchedule(int Month, int Day)
{
  FILE *inputfile;
  int i;
  time_t tval = 0;
  struct tm *tm_now;
  char filename[128], day[3], month[4], year[4], week[2];
  struct stat S_stat;

  i = 0;

  /**
   * 今日の日付を取得する。７／２０ならdate="0720"になる。
   **/
  time(&tval);
  tm_now = localtime(&tval);

  tm_now->tm_mon = Month;
  tm_now->tm_mday = Day;

  strftime(day, sizeof(day), "%d", tm_now);
  strftime(month, sizeof(month), "%m", tm_now);
  strftime(week, sizeof(week), "%u", tm_now);
  strftime(year, sizeof(year), "%G", localtime(&tval));
  filename[0] = '\0';

  if (omr.xcalendar) {
    setlocale(LC_TIME, "C");
    strftime(month, sizeof(month), "%b", localtime(&tval));
    sprintf(filename, "%s/%sxc%d%s%d", getenv("HOME")
	    ,omr.sched_dir, atoi(day), month, atoi(year));
  } else {
    sprintf(filename, "%s/%sxhs%s%s", getenv("HOME"), omr.sched_dir, month, day);
  }

  i = (stat(filename, &S_stat) == 0) ? S_stat.st_size : 0;

  if ((inputfile = fopen(filename, "r")) != NULL && i) {
    fclose(inputfile);
    return (1);
  }
  return (0);
}

void ReadHoliday()
{
  /**
   * xhs.holidayを読み込んで,休日データをListに登録する。
   * Hlist[月 - 1]にlistを作り、日付を登録しておく。
   **/
  char filename[128];
  int i;

  for (i = 0; i < 12; i++)
    Hlist[i] = NULL;

  sprintf(filename, "%s/%s/xhs.holiday", getenv("HOME"), omr.sched_dir);
  ReadHolidayFile(filename);
}

static void ReadHolidayFile(char *filename)
{
  /**
   * 実際にファイルに読み込んで、リストに登録する。ここを分離したのは
   * %include hogehoge という書き方を許すため。
   **/

  FILE *inputfile;
  unsigned char *tmp1, *tmp2, *tmp3, *tmp4, *includefile;
  char tdate[5];
  HolidayList *tlist;
  int m, d;

  tmp1 = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  tmp3 = malloc(BUFSIZ);
  tmp4 = malloc(BUFSIZ);
  includefile = malloc(BUFSIZ);

  if ((inputfile = fopen(filename, "r")) != NULL) {
    while (fgets(tmp1, BUFSIZ - 1, inputfile) != NULL) {

      /**
       * もし # で始まっていたらコメントとみなし、その後1行を無視する。
       * 空行も同様。
       **/
      *tmp2 = *tmp3 = *tmp4 = '\0';

      if (tmp1[0] != '#' && tmp1[0] != '\0' && tmp1[0] != '\n' && tmp1[0] != ' ') {
	sscanf(tmp1, "%s %s", tmp2, tmp3);

	/**
	 * %include行の処理
	 **/
	if (!strcmp(tmp2, "%include")) {

	  /**
	   * もしファイル名が <>で囲まれていたら,sched_dirを展開する
	   **/
	  if (*tmp3 == '<' && strchr(tmp3, '>') != NULL) {
	    sprintf(includefile, "%s/%s/", getenv("HOME"), omr.sched_dir);
	    strcpy(tmp4, strtok(strchr(tmp3, '<') + 1, ">"));
	    strcat(includefile, tmp4);
	  } else {
	    strcpy(includefile, tmp3);
	  }
	  /**
	   * 単に再帰的に読んでるだけなので、includeが入れ子になったらはまる。
	   * Cのプリプロセッサのようにifdefなんかが使えるといいけど、そこまで
	   * やる気はない。
	   **/
	  ReadHolidayFile(includefile);
	}
	strncpy(tdate, tmp2, sizeof(tdate));
	tdate[4] = '\0';
	strncpy(tmp4, tdate, sizeof(char) * 2);
	m = atoi(tmp4) - 1;
	strncpy(tmp4, tdate + 2, sizeof(char) * 2);
	d = atoi(tmp4);

	if (m < 0 || m > 11)
	  continue;

	if (Hlist[m] == NULL) {
	  Hlist[m] = HolidayList_new(d, tmp3);
	} else {
	  tlist = HolidayList_new(d, tmp3);
	  tlist->next = Hlist[m];
	  Hlist[m] = tlist;
	}
      }
    }
  }
  free(tmp1);
  free(tmp2);
  free(tmp3);
  free(tmp4);
  free(includefile);
}

int ExistHoliday(int Year, int Month, int Day)
{
  /**
   * リストを検索して、指定日が休みであれば1を返す。違えば0。
   * Month + 1 月 Day 日 を調べる。
   *
   * 春分の日、秋分の日、成人の日、体育の日は計算して求める。故にYearが引数で必要。
   * Yearは4桁の西暦(1999など)で与える。
   **/

  HolidayList *tlist;
  int day;
  struct tm *tm_now;
  time_t tval;

  time(&tval);
  tm_now = localtime(&tval);
  tm_now->tm_mon = Month;
  tm_now->tm_mday = Day;
  tm_now->tm_year = Year - 1900;
  tval = mktime(tm_now);
  tm_now = localtime(&tval);

  tlist = Hlist[Month];

  for (tlist = Hlist[Month]; tlist != NULL; tlist = tlist->next)
    if (tlist->day == Day)
      return 1;

  /**
   * 春分の日、秋分の日,成人の日,体育の日の計算
   **/

  switch (Month) {
#ifdef I18N
    /**
     * 成人の日と体育の日は日本にしかないからね ^^;
     * # I18NといいつつJapanizeだったといういい見本(苦笑)
     **/
  case 0:
  case 9:
    if(tm_now->tm_wday == 1 && tm_now->tm_mday >= 8 && tm_now->tm_mday <= 14)
      return 1;
    return 0;
#endif
  case 2:
    day = (int) (20.8431 + 0.242194 * (Year - 1980) - (int) ((Year - 1980) / 4));
    if (day == Day)
      return 1;
    return 0;
  case 8:
    day = (int) (23.2488 + 0.242194 * (Year - 1980) - (int) ((Year - 1980) / 4));
    if (day == Day)
      return 1;
    return 0;
  default:
    return 0;
  }

  return 0;
}
