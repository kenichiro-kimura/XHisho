#define _ANIM_GLOBAL
#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


int LoadAnim(ImageInfo *i_info)
{
  int num_of_images,i,loop;
  unsigned int secs;
  int type;
  char buffer[BUFSIZ],t_filename[BUFSIZ],filename[BUFSIZ],path[BUFSIZ],*p_ptr;
  FILE *fp;
  struct stat S_stat;
  
  if(NULL == (fp = fopen(i_info->filename,"r"))){
    return -1;
  }

  num_of_images = 0;
  type = 0;

  /**
   * ファイルのpathを取得する
   **/

  p_ptr = strrchr(i_info->filename,'/');
  if(p_ptr){
    strncpy(path,i_info->filename, p_ptr - i_info->filename + 1);
  } else {
    strcpy(path,"./");
  }

  if(fgets(buffer,BUFSIZ,fp) == NULL ){
    fclose(fp);
    return -1;
  }
  if(strncmp(buffer,"XHisho Animation File",21)){
    fclose(fp);
    return -1;
  }

  /**
   * コマンドの個数の取得
   **/
  while(fgets(buffer,BUFSIZ,fp) != NULL){
    if(buffer[0] == '#') continue;
    num_of_images++;
  }
  rewind(fp);
  fgets(buffer,BUFSIZ,fp);


  free(i_info->image);
  i_info->image = (AnimImage*)malloc(sizeof(AnimImage) * num_of_images);

  if(!i_info->image) return -1;
  
  i = 0;
  while(fgets(buffer,BUFSIZ,fp) != NULL && i < num_of_images){
    if(buffer[0] == '#') continue;

    loop = -1;
    secs = -1;
    sscanf(buffer,"%s %d %d",t_filename,&secs,&loop);

    /**
     * GOTO及びラベルはファイルの存在チェックを飛ばす
     **/

    if(!strcmp(t_filename,"GOTO")){
      strcpy(filename,t_filename);
    } else if(!strcmp(t_filename,"MAIL")){
      strcpy(filename,t_filename);
      type = MAIL;
    } else if(!strcmp(t_filename,"SCHEDULE")){
      strcpy(filename,t_filename);
      type = SCHEDULE;
    } else {
      if(stat(t_filename, &S_stat) != 0){
	sprintf(filename,"%s%s",path,t_filename);
      } else {
	strcpy(filename,t_filename);
      }
    }

    i_info->image[i].filename = malloc(strlen(filename) + 1);
    if(!(i_info->image[i].filename)) return -1;
    strcpy(i_info->image[i].filename,filename);
    i_info->image[i].secs = secs;

    /**
     * ラベルの次の行をテーブルに登録しておく
     **/

    if(type){
      i_info->anim_number[type] = i;
      type = USUAL;
    } 

    /**
     * GOTOの時はループの回数をセットし、カウンタをクリアする
     **/

    if(!strcmp(filename,"GOTO")){
      i_info->image[i].loop_b = loop;
      i_info->image[i].loop_c = 0;
    }
    i++;
  }

  i_info->num_of_images = num_of_images;
  i_info->loaded_images = 0;
  i_info->anim = 1;
  i_info->anim_number[0] = 0;
  fclose(fp);
  return 0;
}
