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

  /**
   * $B%U%!%$%k$N(Bpath$B$r<hF@$9$k(B
   **/

  p_ptr = strrchr(i_info->filename,'/');
  if(p_ptr){
    strncpy(path,i_info->filename, p_ptr - i_info->filename + 1);
    path[p_ptr - i_info->filename + 1] = '\0';
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
   * $B%3%^%s%I$N8D?t$N<hF@(B
   **/

  num_of_images = 0;
  type = 0;
  Loaded_files = NULL;
  
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
     * GOTO$B5Z$S%i%Y%k$O%U%!%$%k$NB8:_%A%'%C%/$rHt$P$9(B
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
     * $B%i%Y%k$N<!$N9T$r%F!<%V%k$KEPO?$7$F$*$/(B
     **/

    if(type){
      i_info->anim_number[type] = i;
      type = USUAL;
    } 

    /**
     * GOTO$B$N;~$O%k!<%W$N2s?t$r%;%C%H$7!"%+%&%s%?$r%/%j%"$9$k(B
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

void Add_files(char *name,int i)
{
  files *tmp;

  tmp = (files*)malloc(sizeof(files));
  tmp->filename = malloc(strlen(name) + 1);
  strcpy(tmp->filename,name);
  tmp->number = i;

  tmp->next = Loaded_files;
  Loaded_files = tmp;
}

int Search_files(char *name)
{
  files *f_p;

  for(f_p = Loaded_files;f_p;){
    if(!strcmp(f_p->filename,name)) return f_p->number;

    f_p = f_p->next;
  }

  return -1;
}
