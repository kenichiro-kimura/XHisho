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
				   "messagearg"};

static char *RcData[NUM_OF_ARRAY(RcName)];

static int RcHash(const char *name)
{
  /**
   * Ϳ����줿ʸ����RcName�β����ܤ����֤���̵�����-1��
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
   * Message�ե�������ɤߡ�RcData����Ͽ��������0,���Ԥ�-����
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
  memset(tmp,0,BUFSIZ);
  memset(tmp2,0,BUFSIZ);
  memset(tmp3,0,BUFSIZ);

  while (fgets(tmp, BUFSIZ, infile) != NULL) {
    sscanf(tmp, "%s %s", tmp2, tmp3);
    for(i = 0; i < strlen(tmp2);i++)
      if(isspace(tmp[i]))
	break;

    strcpy(tmp3,tmp + i + 1);

    i = RcHash(tmp2);
    if (strlen(tmp3) != 0 && i != -1) {
      Escape2Return(tmp3);
      RcData[i] = strdup(tmp3);
    }
    memset(tmp,0,BUFSIZ);
    memset(tmp2,0,BUFSIZ);
    memset(tmp3,0,BUFSIZ);
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
   * RcData����rc_name��Ϳ����줿����ȥ��õ����ret_value���֤���
   * ret_value�κ���Ĺ��size�˻��ꡣ
   **/
  int i;

  if(ret_value == NULL) return;
  memset(ret_value, 0, size);
  if ((i = RcHash(rc_name)) != -1) {
    strncpy(ret_value, RcData[i], MIN(size, strlen(RcData[i])));

    if (ret_value[strlen(ret_value) - 1] == '\n')
      ret_value[strlen(ret_value) - 1] = '\0';
  } 
}

void Escape2Return(char *ret_value)
{
  /**
   * "\n"��'\n'���Ѥ��롣
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
