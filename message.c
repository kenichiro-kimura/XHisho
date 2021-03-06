#define _MESSAGE_GLOBAL
#include "globaldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>

static int RcHash(const char *);

static const char RcName[][256] = {"newmail", "nomail", "open1", "open2",
				   "open3", "alert1"  ,"alert2", "alertformat",
				   "schedule", "menul", "menu0",
				   "menu1", "menu2", "menu3", "menu4", 
				   "menu5", "calendar","resource", 
				   "messagearg","view"};

static char *RcData[NUM_OF_ARRAY(RcName)];

static int RcHash(const char *name)
{
  /**
   * 与えられた文字列がRcNameの何番目かを返す。無ければ-1。
   **/
  int i = 0;

  for (i = 0; i < NUM_OF_ARRAY(RcName); i++) {
    if (!strcmp(RcName[i], name))
      return i;
  }

  return -1;
}


int ReadRcfile(char *filename)
{
  /**
   * Messageファイルを読み、RcDataに登録。成功で0,失敗で-1。
   **/

  FILE *infile;
  char *tmp, *tmp2, *tmp3;
  int i;
#ifdef EXT_FILTER
  char pcommand[128];
#endif

  for (i = 0; i < NUM_OF_ARRAY(RcName); i++) {
    RcData[i] = (char*)malloc(1);
    *RcData[i] = '\0';
  }

#ifdef EXT_FILTER

  sprintf(pcommand, "%s %s", FilterCommand, filename);
  if ((infile = popen(pcommand, "r")) == NULL) {
    fprintf(stderr, "no filter command:%s\n", pcommand);
    return -1;
  }
#else
  if ((infile = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "no such Message file:%s\n", filename);
    return -1;
  }
#endif

  tmp = (char*)malloc(BUFSIZ);
  tmp2 = (char*)malloc(BUFSIZ);
  tmp3 = (char*)malloc(BUFSIZ);

  while (fgets(tmp, BUFSIZ, infile) != NULL) {
    sscanf(tmp, "%s %s", tmp2, tmp3);
    for(i = 0; i < strlen(tmp2);i++)
      if(isspace((unsigned char)tmp[i]))
	break;

    strcpy(tmp3,tmp + i + 1);

    i = RcHash(tmp2);
    if (strlen(tmp3) != 0 && i != -1) {
      Escape2Return(tmp3);
      RcData[i] = strdup(tmp3);
    }
  }

  free(tmp);
  free(tmp2);
  free(tmp3);

#ifdef EXT_FILTER
  pclose(infile);
#else
  fclose(infile);
#endif

  return 0;
}

void ReadRcdata(const char *rc_name, char *ret_value, int size)
{
  /**
   * RcDataからrc_nameで与えられたエントリを探し、ret_valueに返す。
   * ret_valueの最大長をsizeに指定。
   **/
  int i;

  if(ret_value == NULL) return;
  *ret_value = '\0';
  if ((i = RcHash(rc_name)) != -1) {
    strcpy(ret_value, RcData[i]);

    if (ret_value[strlen(ret_value) - 1] == '\n')
      ret_value[strlen(ret_value) - 1] = '\0';
  } 
}

void Escape2Return(char *ret_value)
{
  /**
   * "\n"を'\n'に変える。
   **/
  char *tmp;
  int i;

  if(ret_value == NULL) return;
  tmp = strdup(ret_value);
  *ret_value = '\0';

  for (i = 0; tmp[i] != '\0'; i++) {
    if (!strncmp(tmp + i, "\\n", 2)) {
      strcat(ret_value, "\n");
      i++;
    } else {
      strncat(ret_value, tmp + i, 1);
    }
  }
  free(tmp);
}
