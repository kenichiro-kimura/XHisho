#include "globaldefs.h"
#include "mail.h"
#include "pop.h"
#include "md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static char* MD5Digest(unsigned char*);

#ifdef PETNAME
extern void SearchPetname(char*,char*);
#endif

static int ReadPOPMessage(int,char*,int);
static int WritePOPCommand(int,char*);
static int ReadUserFile(UserData*);
static int Auth(int,UserData);
static int ApopAuth(int,UserData);
static int RpopAuth(int,UserData);
static void GetFromandSubject(int,char*);

int pop3(AuthMethod,char*,char*);

extern String FilterCommand;
extern MailAlertRes mar;

static int ReadPOPMessage(int sock,char* buffer,int size){

  /*
    POP3の,サーバからの応答メッセージを読む
  */

  int i,ret_value;
  char *comm_buffer;

  *buffer = '\0';
  comm_buffer = malloc(size + 1);
  memset(comm_buffer,0,size + 1);

  do{
    i = recv(sock,comm_buffer,size,0);
    comm_buffer[i] = '\0';
    if( i + strlen(buffer) >= size ){
      ret_value = BUFFER_IS_TOO_SMALL;
      goto End;
    }
    strcat(buffer,comm_buffer);
  }while(!strstr(comm_buffer,"\n")); 

  if(strncmp(buffer,"+OK",strlen("+ok")) == 0){
    ret_value = OK;
  } else {
    ret_value = ERR;
  }

End:
  free(comm_buffer);
  return ret_value;
}
  
static int WritePOPCommand(int sock,char* command){
  /*
    POP3の,サーバへのコマンド送信
  */

  return write(sock,command,strlen(command));
}

static int ReadUserFile(UserData* user){
  /*
    POP3認証のためのファイルを読み、ユーザ名及びパスワードを取得する。
    パーミッションもチェックし、700じゃなかったら(600でもいいけど)エラー
    を返す。
  */

  FILE* fp;
  char *buffer;
  char filename[256];
  int i,ret_value;
  struct stat sb;
  
  sprintf(filename,"%s/%s",getenv("HOME"),".pop_password");
  if((fp = fopen(filename,"r")) == NULL){
    return NO_PASSWORD_FILE;
  }

    stat(filename,&sb);
  if((sb.st_mode) & (S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH)){
    fprintf(stderr,"check your password file's permission,%s\n",filename);
    return INVALID_PERMISSION;
  }

  *(user->name) = *(user->pass) = '\0';

  buffer = malloc(BUFSIZ);

  while(fgets(buffer,BUFSIZ,fp) != NULL){
    if(!strncmp(buffer,"USER:",5)){
      strcpy(user->name,buffer + 5);
    } else if (!strncmp(buffer,"PASS:",5)){
      strcpy(user->pass,buffer + 5);
    }
  }

  for(i = 0;i < strlen(user->name);i++){
    if(*(user->name + i) == '\n') *(user->name + i) = '\0';
  }

  for(i = 0;i < strlen(user->pass);i++){
    if(*(user->pass + i) == '\n') *(user->pass + i) = '\0';
  }

  if(user->name && user->pass ){
    ret_value = OK;
  } else {
    ret_value = ERR;
  }

  free(buffer);
  return ret_value;
}

int pop3(AuthMethod method,char* server,char* From){
  /*
    POP3の本体
  */

  char *comm_buffer;
  char tmp[128];
  UserData user;
  int ret_value,mailNumber,mailSize;

  /* 
     通信用変数
  */

  int loop,sockdsc;
  struct hostent *hent;
  struct sockaddr_in sockadd;

  ret_value = ReadUserFile(&user);
  comm_buffer = malloc(BUFSIZ);

  switch(ret_value){
  case ERR:
    sprintf(From,"invalid format password file\n");
    ret_value = 0;
    goto POPQUIT;
  case NO_PASSWORD_FILE:
    sprintf(From,"no password file:\n %s/%s",getenv("HOME"),".pop_password");
    ret_value = 0;
    goto POPQUIT;
  case INVALID_PERMISSION:
    sprintf(From,"check your password file's permission:\n %s/%s"
	    ,getenv("HOME"),".pop_password");
    ret_value = 0;
    goto POPQUIT;
  default:
    strcpy(user.server,server);
    break;
  }

  ret_value = 0;

  /*
    通信の準備
  */

 if((sockdsc = socket(PF_INET,SOCK_STREAM,0)) < 0){
    fprintf(stderr,"can't create socket\n");
    ret_value = 0;
    goto POPQUIT;
  }
  if(NULL == (hent = gethostbyname(server))){
    fprintf(stderr,"can't resolv pop3 server:%s\n",server);
    ret_value = 0;
    goto POPQUIT;
  }

  memcpy(&sockadd.sin_addr,hent->h_addr,hent->h_length);
  sockadd.sin_port = htons(110);
  sockadd.sin_family = AF_INET;
  for(loop = 0;0 != connect(sockdsc,(struct sockaddr*)&sockadd,sizeof(sockadd));++loop){
    if(10 < loop){
      fprintf(stderr,"can't connect pop3 server:%s\n",server);
      ret_value = 0;
      goto POPQUIT;
    }
    sleep(1);
  }

  /*
    Authorization
  */

  switch(method){
  case POP_AUTH:
    ret_value = Auth(sockdsc,user);
    break;
  case APOP_AUTH:
    ret_value = ApopAuth(sockdsc,user);
    break;
  case RPOP_AUTH:
    ret_value = RpopAuth(sockdsc,user);
    break;
  }

  if(ret_value == ERR){
    sprintf(From,"Fail POP authorization:\n %s@%s\n",user.name,user.server);
    ret_value =  1;
    write(sockdsc,"QUIT\n",5);
    close(sockdsc);
    goto POPQUIT;
  }


  /*
    スプールの状態の取得
  */

  WritePOPCommand(sockdsc,"STAT\n");

  if(OK == ReadPOPMessage(sockdsc,comm_buffer,BUFSIZ)){
    sscanf(comm_buffer,"%s %d %d",tmp,&mailNumber,&mailSize);

    ret_value = mailNumber;
  } else {
    fprintf(stderr,"Fail Read POP Message:STAT\n%s\n",comm_buffer);
    ret_value = 0;
    write(sockdsc,"QUIT",5);
    close(sockdsc);
    goto POPQUIT;
  }

  if(ret_value > 0){
    From[0] = '\0';
    GetFromandSubject(sockdsc,From);
  }

  write(sockdsc,"QUIT\n",5);
  close(sockdsc);

  /*
    終了処理
  */

POPQUIT:
  free(comm_buffer);
  return(ret_value);
}

static int Auth(int sock,UserData user){
  /*
    普通のユーザ名と平文パスワードを使った認証
  */

  char *comm_buffer;
  int ret_value;
  
  comm_buffer = malloc(BUFSIZ);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){
    ret_value = ERR;
    goto End;
  }


  sprintf(comm_buffer,"%s %s\n","USER",user.name);
  WritePOPCommand(sock,comm_buffer);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){

#ifdef DEBUG
    fprintf(stderr,"%s\n",comm_buffer);
#endif
    ret_value = ERR;
    goto End;
  }

  sprintf(comm_buffer,"%s %s\n","PASS",user.pass);
  WritePOPCommand(sock,comm_buffer);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){
    fprintf(stderr,"Fail POP authorization:%s@%s\n",user.name,user.server);

#ifdef DEBUG
    fprintf(stderr,"%s\n",comm_buffer);
#endif

    ret_value = ERR;
    goto End;
  }

  ret_value = OK;

End:
  free(comm_buffer);
  return ret_value;
}

static int ApopAuth(int sock,UserData user){
  /*
    APOPによる認証
  */

  char *comm_buffer;
  char *apop_message;
  char *digest;
  int i,ret_value;

  comm_buffer = malloc(BUFSIZ);
  apop_message = malloc(BUFSIZ);
  digest = malloc(BUFSIZ);

  *apop_message = '\0';

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){

#ifdef DEBUG
    fprintf(stderr,"%s\n",comm_buffer);
#endif
    ret_value = ERR;
    goto End;
  }


  /*
    Greeting messageを取得する
  */

  for(i = 0;i < strlen(comm_buffer);i++){
    if(*(comm_buffer + i) == '<'){
      do{
	strncat(apop_message,comm_buffer + i,1);
	i++;
      }while(i < strlen(comm_buffer) && *(comm_buffer + i - 1) != '>');
    }
  }

  strcat(apop_message,user.pass);

  sprintf(digest,"APOP %s %s\n",user.name,MD5Digest(apop_message));

  WritePOPCommand(sock,digest);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){
    fprintf(stderr,"Fail APOP authorization:%s@%s\n",user.name,user.server);

#ifdef DEBUG
    fprintf(stderr,"%s\n",comm_buffer);
#endif
    ret_value = ERR;
    goto End;
  }

  ret_value = OK;

End:
  free(comm_buffer);
  free(apop_message);
  free(digest);
  return ret_value;
}

static int RpopAuth(int sock,UserData user){
  /*
    RPOPによる認証。とりあえず書いてるけど使っちゃダメ。たぶん動かない。
  */

  char *comm_buffer;
  int ret_value;

  comm_buffer = malloc(BUFSIZ);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){
    ret_value = ERR;
    goto End;
  }

  sprintf(comm_buffer,"%s %s\n","RPOP",user.name);
  WritePOPCommand(sock,comm_buffer);

  if(ERR == ReadPOPMessage(sock,comm_buffer,BUFSIZ)){
    fprintf(stderr,"Fail POP authorization:%s@%s\n",user.name,user.server);

#ifdef DEBUG
    fprintf(stderr,"%s\n",comm_buffer);
#endif

    ret_value = ERR;
    goto End;
  }

  ret_value = OK;

End:
  free(comm_buffer);
  return ret_value;
}


static char* MD5Digest(unsigned char *s){
  /*
    MD5ダイジェストを計算する。RFC1321のAppendixそのまま。本体はmd5c.c。
  */

  int i;
  MD5_CTX context;
  unsigned char digest[16];
  static char ascii_digest [33];

  MD5Init(&context);
  MD5Update(&context, s, strlen(s));
  MD5Final(digest, &context);
  
  for (i = 0;  i < 16;  i++) 
    sprintf(ascii_digest+2*i, "%02x", digest[i]);
 
  return(ascii_digest);
}


static void GetFromandSubject(int sock,char* buffer){
  unsigned char *buf,*tmp,*tmp2,*tmp3;
  int i = 0,j;
  int length;

#ifdef PETNAME
  char *from,*who,*pname;
#endif /* PETNAME */
  
#ifdef EXT_FILTER
  FILE *in,*t_file;
  char *command,*t_filename,*Tmp_dir;

  command = malloc(BUFSIZ * 3);
  t_filename = malloc(BUFSIZ * 2);
  Tmp_dir = malloc(BUFSIZ);

  sprintf(Tmp_dir,"/tmp/xhtmp%s",getenv("USER"));
  mkdir(Tmp_dir,S_IRWXU);

  t_filename[0] = '\0';
  sprintf(t_filename,"%s",tempnam(Tmp_dir,"xhtmp"));
#endif

  buf = malloc(BUFSIZ * 5);
  memset(buf,0,BUFSIZ * 5);
  tmp = malloc(BUFSIZ * 5);
  tmp3 = malloc(BUFSIZ * 5);
  memset(tmp,0,BUFSIZ * 5);

#ifdef PETNAME
  from = malloc(BUFSIZ);
  who =  malloc(BUFSIZ);
  pname =  malloc(BUFSIZ);
#endif

  WritePOPCommand(sock,"TOP 1 0\n");
  if(ERR == ReadPOPMessage(sock,buf,BUFSIZ)){
    fprintf(stderr,"fail pop command: TOP\n");
    goto End;
  }
  memset(buf,0,BUFSIZ * 5);

  do{
    ReadPOPMessage(sock,buf,BUFSIZ * 5);
  }while((strncmp(buf + strlen(buf) -4, "\n.",2) != 0) &&
	 (!strstr(buf,"From:")));
  tmp2 = strtok(buf,"\n");


  i = 0;

  while(tmp2 && (i < mar.mail_lines)){
    if(!strncmp(tmp2,"From:",5) || !strncmp(tmp2,"Subject:",8)){
      strcpy(tmp3,tmp2);

#ifdef PETNAME
      if(!strncmp(tmp2,"From:",5)){
	sscanf(tmp2,"%s %s",from,who);

	for(j = 0; j < strlen(from);j++)
	  if(isspace(tmp2[j])) break;

	strcpy(who,tmp2 + j);

	if(strchr(who,'@')){
	  strcpy(pname,who);
	} else {
	  if(!strchr(tmp2,'<')) tmp2 = strtok(NULL,"\n");
	  if(strchr(tmp2,'<') && strchr(tmp2,'>'))	  
	    strcpy(pname, strtok(strchr(tmp2,'<') + 1,">"));
	}
	SearchPetname(tmp3,pname);
      }
#endif

#ifdef EXT_FILTER
      if((t_file = fopen(t_filename,"w")) == NULL){
	fprintf(stderr,"can't open temporary file,%s\n",t_filename);
	perror("popen");
	exit(1);
      }
      
      fprintf(t_file,"%s\n",tmp3);
      fclose(t_file);
      
      sprintf(command,"%s %s",FilterCommand,t_filename);
      in = popen(command,"r");

      fgets(tmp,BUFSIZ,in);

      pclose(in);
#else
      strcpy(tmp,tmp3);
#endif

      length = MIN(mar.from_maxlen,strlen(tmp));
      if(!strncmp(tmp2,"Subject:",8)){
	tmp[length - 2] = '\n';
	tmp[length - 1] = '\0';
      } else {
	tmp[length - 1] = '\n';
	tmp[length] = '\0';
      }
      
      strncat(buffer,tmp,mar.from_maxlen);
      memset(tmp,0,BUFSIZ * 5);
      i++;
    }
    tmp2 = strtok(NULL,"\n");
  }

End:
  
  free(buf);
  free(tmp);
  free(tmp3);
#ifdef PETNAME
  free(from);
  free(who);
  free(pname);
#endif
#ifdef EXT_FILTER
  free(command);
  free(t_filename);
  free(Tmp_dir);
#endif

  return;
}  
