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
   * HolidayList�Υ��󥹥ȥ饯����memset�ˤ��0���ꥢ��ǰ�Τ��ᡣ���֤󤤤��
   * day,name�˰�����Ϳ������Τ򥻥åȤ���next��NULL�ˤ��롣
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
   * ���������դ�������롣7/20�ʤ�date="0720"�ˤʤ롣
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
	 * �⤷ # �ǻϤޤäƤ����饳���ȤȤߤʤ������θ�1�Ԥ�̵�뤹�롣
	 * ���Ԥ�Ʊ�͡�
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
           * ���ϻ����leave���ɤ����Ф���
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
   * Weekly data �μ���
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
     * �⤷ # �ǻϤޤäƤ����饳���ȤȤߤʤ������θ�1�Ԥ�̵�뤹�롣
     * ���Ԥ�Ʊ�͡�
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
         * ���ϻ����leave���ɤ����Ф���
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
   * ���������դ�������롣���������ʤ�date="0720"�ˤʤ롣
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
   * xhs.holiday���ɤ߹����,�����ǡ�����List����Ͽ���롣
   * Hlist[�� - 1]��list���ꡢ���դ���Ͽ���Ƥ�����
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
   * �ºݤ˥ե�������ɤ߹���ǡ��ꥹ�Ȥ���Ͽ���롣������ʬΥ�����Τ�
   * %include hogehoge �Ȥ���������������ᡣ
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
       * �⤷ # �ǻϤޤäƤ����饳���ȤȤߤʤ������θ�1�Ԥ�̵�뤹�롣
       * ���Ԥ�Ʊ�͡�
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
   * �ꥹ�Ȥ򸡺����ơ����������٤ߤǤ����1���֤����㤨��0��
   * Month + 1 �� Day �� ��Ĵ�٤롣
   *
   * ��ʬ��������ʬ�����Ϸ׻����Ƶ��롣�Τ�Year��������ɬ�ס�
   * Year��4�������(1999�ʤ�)��Ϳ���롣
   **/

  HolidayList *tlist;
  int day;

  tlist = Hlist[Month];

  for (tlist = Hlist[Month]; tlist != NULL; tlist = tlist->next)
    if (tlist->day == Day)
      return 1;

  /**
   * ��ʬ��������ʬ�����η׻�
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
