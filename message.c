#define MAX_MESSAGE_NUM 18

#include "globaldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int RcHash(const char*);
void Escape2Return(char*);
int ReadRcfile(char*);
void ReadRcdata(const char*,char*,int size);

static const char RcName[][256] = {"newmail","nomail","open1","open2","open3","alert1"
				   ,"alert2","alertformat","schedule","menul","menu0"
				   ,"menu1","menu2","menu3","menu4","menu5","calendar"
				   ,"resource"};

static char* RcData[MAX_MESSAGE_NUM];

static int RcHash(const char* name){
  /*
   * $BM?$($i$l$?J8;zNs$,(BRcName$B$N2?HVL\$+$rJV$9!#L5$1$l$P(B-1$B!#(B
   */
  int i = 0;

  for(i = 0; i < MAX_MESSAGE_NUM;i++){
    if(!strcmp(RcName[i],name)) return i;
  }

  return -1;
}


int ReadRcfile(char* filename){
  /*
   * Message$B%U%!%$%k$rFI$_!"(BRcData$B$KEPO?!#@.8y$G(B0,$B<:GT$G(B-$B#1!#(B
   */

  FILE* infile;
  char *tmp,*tmp2,*tmp3;
  char* index;
  int i;

  for(i = 0; i < MAX_MESSAGE_NUM;i++){
    RcData[i] = malloc(1);
    *RcData[i] = '\0';
  }

  if((infile = fopen(filename,"r")) == NULL){
    fprintf(stderr,"no such Message file:%s\n",filename);
    return -1;
  }

  tmp = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  tmp3 = malloc(BUFSIZ);

  while(fgets(tmp,BUFSIZ,infile) != NULL){
    sscanf(tmp,"%s %s",tmp2,tmp3);
    index = strstr(tmp,tmp3);
    i = RcHash(tmp2);
    if(index != NULL && i != -1){
      Escape2Return(index);
      RcData[i] = realloc(RcData[i],strlen(index) + 1);
      memset(RcData[i],0,strlen(index) + 1);
      strcpy(RcData[i],index);
    }
  }

  free(tmp);
  free(tmp2);
  free(tmp3);

  fclose(infile);

  return 0;
}

void ReadRcdata(const char* rc_name,char* ret_value,int size){
  /*
   * RcData$B$+$i(Brc_name$B$GM?$($i$l$?%(%s%H%j$rC5$7!"(Bret_value$B$KJV$9!#(B
   * ret_value$B$N:GBgD9$r(Bsize$B$K;XDj!#(B
   */
  int i;

  memset(ret_value,0,size);
  if((i = RcHash(rc_name)) != -1){
    strncpy(ret_value,RcData[i],MIN(size,strlen(RcData[i])));
  }
  if(ret_value[strlen(ret_value) -1] == '\n')
    ret_value[strlen(ret_value) -1] = '\0';

}

void Escape2Return(char* ret_value){
  /*
   * "\n"$B$r(B'\n'$B$KJQ$($k!#(B
   */
  char *tmp;
  int i;

  tmp = malloc(strlen(ret_value));
  strcpy(tmp,ret_value);
  *ret_value = '\0';

  for(i = 0; tmp[i] != '\0';i++){
    if(!strncmp(tmp + i,"\\n",2)){
      strcat(ret_value,"\n");
      i++;
    } else {
      strncat(ret_value,tmp + i,1);
    }
  }
  free(tmp);
}
       
       
       
