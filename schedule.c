#define _SCHEDULE_GLOBAL
#include "globaldefs.h"
#include "openwin.h"
#include <ctype.h>
#include <sys/stat.h>
#include <X11/Xlocale.h>

static HolidayList *Hlist[12];

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
  int i, j;
  char filename[128], day[3], month[4], tdate[5], week[2], year[4];
  unsigned char *tmp1, *tmp2, *tmp3, *tmp4, *leave;
  unsigned char *string_index;

#ifdef EXT_FILTER
  char command[128];
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

  if (!(WeeklyCheck & 2)) {


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
	    schedule[i].leave = omr.leave_t;
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
    if (!WeeklyCheck) {
      free(tmp1);
      free(tmp2);
      free(tmp3);
      free(tmp4);
      free(leave);
      return (i);
    }
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

      if (!atoi(leave)) {
	sscanf(tmp1, "%s %s", tmp2, tmp3);
	schedule[i].leave = omr.leave_t;
      } else {
	schedule[i].leave = atoi(leave);
      }

      if (atoi(week) == atoi(tmp4)) {
	strncpy(tdate, tmp2, sizeof(tdate));
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
	for (j++; j < strlen(tmp1); j++)
	  if (isspace(tmp1[j]))
	    break;

	string_index = tmp1 + j + 1;

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

	if (!strcmp(tmp2, "%include")) {
	  if (*tmp3 == '<' && strchr(tmp3, '>') != NULL) {
	    sprintf(includefile, "%s/%s/", getenv("HOME"), omr.sched_dir);
	    strcpy(tmp4, strtok(strchr(tmp3, '<') + 1, ">"));
	    strcat(includefile, tmp4);
	  } else {
	    strcpy(includefile, tmp3);
	  }
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
   * 春分の日、秋分の日は計算して求める。故にYearが引数で必要。
   * Yearは4桁の西暦(1999など)で与える。
   **/

  HolidayList *tlist;
  int day;

  tlist = Hlist[Month];

  for (tlist = Hlist[Month]; tlist != NULL; tlist = tlist->next)
    if (tlist->day == Day)
      return 1;

  /**
   * 春分の日、秋分の日の計算
   **/

  switch (Month) {
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
