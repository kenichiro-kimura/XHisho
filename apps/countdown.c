#include "countdown.h"

int main(){
  FILE *inputfile;
  time_t now,schedule;
  double diff;
  int do_print,y,d,h,m,s;
  struct tm *tm_sched;
  char filename[128],day[3],year[5],month[3],tmp1[BUFSIZ],tmp2[BUFSIZ],tmp3[BUFSIZ];
  char hour[3],min[3],sec[3];
  char *string_index;

#ifdef EXT_FILTER
  char command[128];
#endif

  time(&now);
  tm_sched = localtime(&now);
  do_print = 0;

  sprintf(filename,"%s/%s",getenv("HOME"),CD_FILE);

  if((inputfile = fopen(filename,"r")) == NULL){
    fprintf(stderr,"can't open data file:%s\n",filename);
    exit(1);
  }

#ifdef EXT_FILTER
  fclose(inputfile);
  sprintf(command,"%s %s",FILTER_COMMAND,filename);
  inputfile = popen(command,"r");
#endif

  while(fgets(tmp1,BUFSIZ,inputfile) != NULL){
    if(*tmp1 != '#' && *tmp1 != '\0' && *tmp1 != '\n' && *tmp1 != ' '){

      if(tmp1[strlen(tmp1) - 1] == '\n')
	tmp1[strlen(tmp1) - 1] = '\0';

      sscanf(tmp1,"%s %s",tmp2,tmp3);
      string_index = strstr(tmp1,tmp3);

      switch(strlen(tmp2)){
      case 8:
	/* MMddhhmm */
	strncpy(month,tmp2,2);
	month[2] = '\0';
	strncpy(day,tmp2 + 2,2);
	day[2] = '\0';
	strncpy(hour,tmp2 + 4,2);
	hour[2] = '\0';
	strncpy(min,tmp2 + 6,2);
	min[2] = '\0';

	if(atoi(month) > 0 && atoi(month) < 13 &&
	   atoi(day) > 0 && atoi(day) < 32 &&
	   atoi(hour) >= 0 && atoi(hour) < 24 &&
 	   atoi(hour) >= 0 && atoi(hour) < 60 ){
	  tm_sched->tm_mon = atoi(month) - 1;
	  tm_sched->tm_mday = atoi(day);
	  tm_sched->tm_hour = atoi(hour);
	  tm_sched->tm_min = atoi(min);
	  tm_sched->tm_sec = 0;
	  do_print = 1;
	} else {
	  do_print = 0;
	}
	break;
      case 10:
	/* MMddhhmmss */
	strncpy(month,tmp2,2);
	month[2] = '\0';
	strncpy(day,tmp2 + 2,2);
	day[2] = '\0';
	strncpy(hour,tmp2 + 4,2);
	hour[2] = '\0';
	strncpy(min,tmp2 + 6,2);
	min[2] = '\0';
	strncpy(sec,tmp2 + 8,2);
	sec[2] = '\0';

	if(atoi(month) > 0 && atoi(month) < 13 &&
	   atoi(day)  > 0  && atoi(day) < 32 &&
	   atoi(hour) >= 0 && atoi(hour) < 24 &&
 	   atoi(hour) >= 0 && atoi(hour) < 60 &&
	   atoi(sec)  >= 0 && atoi(sec)  < 60){
	  tm_sched->tm_mon = atoi(month) - 1;
	  tm_sched->tm_mday = atoi(day);
	  tm_sched->tm_hour = atoi(hour);
	  tm_sched->tm_min = atoi(min);
	  tm_sched->tm_sec = atoi(sec);
	  do_print = 1;
	} else {
	  do_print = 0;
	}
	break;
      case 12:
	/* yyyyMMddhhmm */
	strncpy(year,tmp2,4);
	year[4] = '\0';
	strncpy(month,tmp2 + 4,2);
	month[2] = '\0';
	strncpy(day,tmp2 + 6,2);
	day[2] = '\0';
	strncpy(hour,tmp2 + 8,2);
	hour[2] = '\0';
	strncpy(min,tmp2 + 10,2);
	min[2] = '\0';

	if(atoi(month) > 0 && atoi(month) < 13 &&
	   atoi(day) > 0 && atoi(day) < 32 &&
	   atoi(hour) >= 0 && atoi(hour) < 24 &&
 	   atoi(hour) >= 0 && atoi(hour) < 60 ){
	  tm_sched->tm_year = atoi(year) - 1900;
	  tm_sched->tm_mon = atoi(month) - 1;
	  tm_sched->tm_mday = atoi(day);
	  tm_sched->tm_hour = atoi(hour);
	  tm_sched->tm_min = atoi(min);
	  tm_sched->tm_sec = 0;
	  do_print = 1;
	} else {
	  do_print = 0;
	}
	break;
      case 14:
	/* yyyyMMddhhmm */
	strncpy(year,tmp2,4);
	year[4] = '\0';
	strncpy(month,tmp2 + 4,2);
	month[2] = '\0';
	strncpy(day,tmp2 + 6,2);
	day[2] = '\0';
	strncpy(hour,tmp2 + 8,2);
	hour[2] = '\0';
	strncpy(min,tmp2 + 10,2);
	min[2] = '\0';
	strncpy(sec,tmp2 + 12,2);
	sec[2] = '\0';

	if(atoi(month) > 0 && atoi(month) < 13 &&
	   atoi(day) > 0 && atoi(day) < 32 &&
	   atoi(hour) >= 0 && atoi(hour) < 24 &&
 	   atoi(min) >= 0 && atoi(min) < 60 &&
	   atoi(sec) >= 0 && atoi(sec) < 60){
	  tm_sched->tm_year = atoi(year) - 1900;
	  tm_sched->tm_mon = atoi(month) - 1;
	  tm_sched->tm_mday = atoi(day);
	  tm_sched->tm_hour = atoi(hour);
	  tm_sched->tm_min = atoi(min);
	  tm_sched->tm_sec = atoi(sec);
	  do_print = 1;
	} else {
	  do_print = 0;
	}
	break;
      default:
	fprintf(stderr,"bad time format:%d:%s\n",strlen(tmp2),tmp1);
      }

      if(do_print){
	schedule = mktime(tm_sched);
	diff = difftime(schedule,now);

	if(diff > 0){
	  y = diff / (60 * 60 * 24 * 365);
	  diff -= y * 365 * 60 * 60 * 24;
	  d = diff / (60 * 60 * 24);
	  diff -= d * 60 * 60 * 24;
	  h = diff / (60 * 60);
	  diff -= h * 60 * 60;
	  m  = diff / 60;
	  diff -= m * 60;
	  s = diff;
	  
	  printf("あと");
	  if(y) printf("%d年",y);
	  if(d) printf("%d日",d);
	  if(h) printf("%d時間",h);
	  if(m) printf("%d分",m);
	  if(s) printf("%d秒",s);
	  printf("で%sです\n",string_index);
	}
      }
      do_print = 0;
    }
  }

#ifdef EXT_FILTER
  pclose(inputfile);
#else
  fclose(inputfile);
#endif

  return 0;
}

    
