#include "globaldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Intrinsic.h>

static int RcHash(const char *);
void Escape2Return(char *);
int ReadRcfile(char *);
void ReadRcdata(const char *, char *, int size);

static const char RcName[][256] = {"newmail", "nomail", "open1", "open2", "open3", "alert1"
  ,"alert2", "alertformat", "schedule", "menul", "menu0"
  ,"menu1", "menu2", "menu3", "menu4", "menu5", "calendar"
,"resource", "messagearg"};

static char *RcData[NUM_OF_ARRAY(RcName)];

  extern String FilterCommand;

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
   * Messageファイルを読み、RcDataに登録。成功で0,失敗で-１。
   **/

  FILE *infile;
  char *tmp, *tmp2, *tmp3;
  char *index;
  int i;
#ifdef EXT_FILTER
  char pcommand[128];
#endif

  for (i = 0; i < NUM_OF_ARRAY(RcName); i++) {
    RcData[i] = malloc(1);
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

  tmp = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  tmp3 = malloc(BUFSIZ);

  while (fgets(tmp, BUFSIZ, infile) != NULL) {
    sscanf(tmp, "%s %s", tmp2, tmp3);
    index = strstr(tmp, tmp3);
    i = RcHash(tmp2);
    if (index != NULL && i != -1) {
      Escape2Return(index);
      RcData[i] = realloc(RcData[i], strlen(index) + 1);
      memset(RcData[i], 0, strlen(index) + 1);
      strcpy(RcData[i], index);
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

  memset(ret_value, 0, size);
  if ((i = RcHash(rc_name)) != -1) {
    strncpy(ret_value, RcData[i], MIN(size, strlen(RcData[i])));
  }
  if (ret_value[strlen(ret_value) - 1] == '\n')
    ret_value[strlen(ret_value) - 1] = '\0';

}

void Escape2Return(char *ret_value)
{
  /**
   * "\n"を'\n'に変える。
   **/
  char *tmp;
  int i;

  tmp = malloc(strlen(ret_value));
  strcpy(tmp, ret_value);
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
