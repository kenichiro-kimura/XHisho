#include "petname.h"
#include "globaldefs.h"
#include "mail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Petname$B$N%j%9%H$N@hF,$rJ];}$9$k%]%$%s%?$NG[Ns!#4JC1$J(BHash$B$r$+$1$k!#(B
 */
static PetnameList* Petname[HASH_KEY];

extern String FilterCommand;


static int StrHash(char*);
static PetnameList* Petname_new(char*,char*);
static void Petname_delete(PetnameList*);
#ifdef ADDRESSBOOK
static void ReadAddrBook();
#endif

void ReadPetname(char*);
void SearchPetname(char*,char*);


/*
 * PetnameList $B%*%V%8%'%/%H$N%3%s%9%H%i%/%?$KAjEv$9$k4X?t(B
 */

static PetnameList* Petname_new(char *pname,char *addr){
  PetnameList* pname_ptr;

  pname_ptr = (PetnameList*)malloc(sizeof(PetnameList));
  memset(pname_ptr,0,sizeof(PetnameList));
  pname_ptr->next = NULL;

  pname_ptr->petname = malloc(strlen(pname) + 1);
  memset(pname_ptr->petname,0,strlen(pname) + 1);
  pname_ptr->mail_address = malloc(strlen(addr) + 1);
  memset(pname_ptr->mail_address,0,strlen(addr) + 1);

  strcpy(pname_ptr->petname,pname);
  strcpy(pname_ptr->mail_address,addr);

  return pname_ptr;
}
  

/*
 * PetnameList $B%*%V%8%'%/%H$N%G%9%H%i%/%?$KAjEv$9$k4X?t(B
 */

static void Petname_delete(PetnameList* ptr){
  free(ptr->mail_address);
  free(ptr->petname);
  free(ptr);
}

static int StrHash(char* string){
  /*
   * $BJ8;zNs$r4JC1$J(BHash$B$K$+$1$k!#(B0 $B!A(B HASH_KEY $B$N4V$N@0?t$rJV$9!#7W;;(B
   * $B$OE,Ev$J$N$G!"(BHash$B$N;6$j6q9g$$$O4|BT$7$J$$$h$&$K(B($B>P(B)$B!#$=$l$G$b@~(B
   * $B7A$KA4$F$NMWAG$rD4$Y$k$h$j$O$^$7$@$H;W$&!#(B
   * $B;n$7$K;d$N(BPetname$B%U%!%$%k(B(248$B%(%s%H%j(B)$B$G(BHASH_KEY=253$B$G$d$C$F$_$?(B
   * $B$i!":GBg>WFM?t$,(B4$B$G!"(B157$B$N(BHash$BCM$K;6$C$?$N$G$=$l$J$j$KF0$/$s$8$c(B
   * $B$J$$$+$H;W$$$^$9!#(BPetname$B$N%(%s%H%j$,>/$J$$$H$-$O(B
   * HASH_KEY(globaldefs.h$B$GDj5A(B)$B$r8:$i$9$H%a%b%j$,@aLs$G$-$^$9!#(B
   */

  int ret;
  char* ptr;

  ret = 0;
  
  for(ptr = string;*ptr;ptr++){
    ret += (int)(*ptr);
    ret %= HASH_KEY;
  }

  if(ret > HASH_KEY) ret %= HASH_KEY;
  return abs(ret);
}

#ifdef ADDRESSBOOK
static void ReadAddrBook(){
  /*
   * support for mew's Addrbook :-)
   *
   * Petname$B$r%"%I%l%9%V%C%/$+$iFI$_!"(BHash$B%j%9%H$GJ];}$9$k!#(Bmail$B%"%I%l%9$r(B
   * StrHash()$B$K$+$1!"%F!<%V%k$N3:Ev%v=j$K%(%s%H%j$rA^F~$9$k!#(BHash$BCM$,(B
   * $B$V$D$+$C$?%F!<%V%k$N@h$O%j%9%H$N$U$j$r$7$?%9%?%C%/(B($B$b$7$/$O%-%e!<(B)$B!#(B
   * $B<h$j=P$5$J$$$+$i$I$A$i$H8@$C$F$b$$$$$,!"MW$9$k$K%j%9%H7?$N@hF,$K(B
   * $BMWAG$rDI2C$7$F$$$C$F$$$k!#(B
   */

  FILE *fp;
  char *buffer,*pname,*addr;
  char fname[128];
  int i,in_quote,j;
  PetnameList *address_list,*pname_ptr,*ptr;
#ifdef EXT_FILTER
  char pcommand[128];
#endif

  sprintf(fname,"%s/.im/Addrbook",getenv("HOME"));

#ifdef EXT_FILTER
  
  sprintf(pcommand,"%s %s",FilterCommand,fname);
  if((fp = popen(pcommand,"r")) == NULL){
    fprintf(stderr,"no filter command:%s\n",pcommand);
    return;
  }
#else
  if((fp = fopen(fname,"r")) == NULL){
    fprintf(stderr,"no addrbook:%s\n",fname);
    return;
  }
#endif

  buffer = malloc(BUFSIZ);
  pname = malloc(BUFSIZ);
  addr = malloc(BUFSIZ);
  address_list = pname_ptr = NULL;

  while(fgets(buffer,BUFSIZ,fp) != NULL){
    memset(pname,0,BUFSIZ);
    j = 0;
    address_list = NULL;

    if(buffer[0] == ';') goto End;

    /*
     * short name $B$rFI$_Ht$P$9(B
     */
    for(i = 0; i < strlen(buffer);i++){
      if(isspace(buffer[i])) break;
      if(buffer[i] == '#') goto End;
    }

    /*
     * $BE83+5,B'$NDj5A$OFI$_Ht$P$9(B
     */

    if(buffer[i++] == ':') goto End;

    /*
     * $B%"%I%l%9$rFI$`!#J#?t$"$k>l9g$b$"$k$N$G!"%j%9%H$GJ];}$9$k!#(B
     */

    for(j = 0;i < strlen(buffer);i++){
      if(buffer[i] == ','){
	/*
	 * 1$B$D$N%"%I%l%9$NFI$_9~$_$,=*$o$C$?$N$G!"%j%9%H$KEPO?(B
	 */
	addr[j] = '\0';

	if(strlen(addr) > 1){
	  pname_ptr = Petname_new("dummy",addr);

	  if(address_list != NULL){
	    pname_ptr->next = address_list;
	  }

	  address_list = pname_ptr;
	}
	i++;
	j = 0;
	while(isspace(buffer[i])){
	  i++;
	  if(buffer[i] == '#') goto End;
	}
      }
      if(isspace(buffer[i])) break;
      if(buffer[i] == '#') goto End;
      addr[j++] = buffer[i];
    }

    addr[j] = '\0';

    /*
     * $B:G8e$KFI$_9~$s$@%"%I%l%9$r%j%9%H$KEPO?(B
     */

    if(strlen(addr) > 1){
      pname_ptr = Petname_new("dummy",addr);

      if(address_list != NULL){
	pname_ptr->next = address_list;
      }

      address_list = pname_ptr;
    }

    /*
     * Petname $B$NFI$_9~$_!#(B" $B$G0O$^$l$?J8;zNsCf$N6uGrJ8;z$O(BPetname$B$N(B1
     * $BIt!#(B" $B$G0O$^$l$F$$$J$$6uGrJ8;z$O(BPetname$B$N6h@Z$jJ8;z!#(B
     *
     * int in_quote = 1 ($B8=:_8F$s$G$$$kJ8;z$O(B " $B$G0O$^$l$?J8;zNs$N0lIt(B
     *                   $B$G$"$k(B)
     */

    for(i++,j = in_quote = 0;i < strlen(buffer);i++){
      if(buffer[i] == '"'){
	if(in_quote) break;
	in_quote = 1;
      } else {
	if(!in_quote && isspace(buffer[i])) break;
	if(buffer[i] == '#') break;
	if(buffer[i] != '"') pname[j++] = buffer[i];
      }
    }

    /*
     * Petname $B$NEPO?!#%j%9%H$+$i%"%I%l%9$r=g$K<h$j=P$7!"EPO?$9$k!#(B
     */

    ptr = address_list;
    while(ptr && strlen(pname) > 1){
      i = StrHash(ptr->mail_address);

      pname_ptr = Petname_new(pname,ptr->mail_address);

      if(Petname[i] != NULL){
	pname_ptr->next = Petname[i];
      }
      Petname[i] = pname_ptr;

      pname_ptr = ptr->next;
      Petname_delete(ptr);
      ptr = pname_ptr;
    }
  }

  End:
  /*
   * $B=*N;=hM}!#DL>o$N=*N;0J30$K%3%a%s%H9T0J2<$N:n6HCf;_(B($B9TF,$N(B ";",$BES(B
   * $BCf$N(B "#")$B$,$"$k!#(BPetname$B$NESCf$G(B "#" $B$,=P$?$i!"$=$3$^$G$r(BPetname
   * $B$H$7$FEPO?$9$k!#(B
   */

#ifdef EXT_FILTER
  pclose(fp);
#else
  fclose(fp);
#endif
  free(buffer);
  free(pname);
  free(addr);
}
#endif

void ReadPetname(char* petname_f){
  /*
   * Petname$B$r%U%!%$%k$+$iFI$_!"(BHash$B%j%9%H$GJ];}$9$k!#(Bmail$B%"%I%l%9$r(B
   * StrHash()$B$K$+$1!"%F!<%V%k$N3:Ev%v=j$K%(%s%H%j$rA^F~$9$k!#(BHash$BCM$,(B
   * $B$V$D$+$C$?%F!<%V%k$N@h$O%j%9%H$N$U$j$r$7$?%9%?%C%/(B($B$b$7$/$O%-%e!<(B)$B!#(B
   * $B<h$j=P$5$J$$$+$i$I$A$i$H8@$C$F$b$$$$$,!"MW$9$k$K%j%9%H7?$N@hF,$K(B
   * $BMWAG$rDI2C$7$F$$$C$F$$$k!#(B
   */

  char *who,*tmp,*tmp2,*buffer;
  FILE *pfp;
  PetnameList* pname_ptr;
  int i,hashed;
#ifdef EXT_FILTER
  char pcommand[128];
#endif /* EXT_FILTER */

  who = malloc(BUFSIZ);
  tmp = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  buffer = malloc(BUFSIZ);

  for(i = 0; i < HASH_KEY;i++){
    Petname[i] = NULL;
  }
  memset(tmp2,0,BUFSIZ);
  
  if((pfp = fopen(petname_f,"r")) == NULL){
    fprintf(stderr,"no petname file:%s\n",petname_f);
    return;
  }
	    
#ifdef EXT_FILTER
	    
  fclose(pfp);
  sprintf(pcommand,"%s %s",FilterCommand,petname_f);
  if((pfp = popen(pcommand,"r")) == NULL){  /* reopen as pipe */
    fprintf(stderr,"no filter command:%s\n",pcommand);
    return;
  }
	    
#endif /* EXT_FILTER */
	    
  while(fgets(buffer,BUFSIZ,pfp) !=NULL){
    sscanf(buffer,"%s %s",who,tmp);
    if(strchr(tmp,'\"')){
      strcpy(tmp2,strtok(strchr(tmp,'\"') + 1, "\""));
      hashed = StrHash(who);

      pname_ptr = Petname_new(tmp2,who);

      if(Petname[hashed] != NULL){
	pname_ptr->next = Petname[hashed];
      }
      Petname[hashed] = pname_ptr;
    }
  }
  
#ifdef EXT_FILTER
  pclose(pfp);
#else
  fclose(pfp);
#endif /* EXT_FILTER */
  free(buffer);
  free(tmp);
  free(tmp2);
  free(who);

#ifdef ADDRESSBOOK
  ReadAddrBook();
#endif
}

void SearchPetname(char* ret_value,char* pname){
  /* 
   * $B%j%9%H$r8!:w$7$F3:Ev$9$k(BPetname$B$,L5$$$+C5$9!#$b$7$"$C$?$i(Bret_value$B$K(B
   * $B!V(BFrom:Petname$B!W$N7A$GF~$l$FJV$9!#L5$1$l$P(Bret_value$B$O$=$N$^$^!#(B
   */

  PetnameList* plist;
  int hashed;

  hashed = StrHash(pname);
  plist = Petname[hashed];

  while(plist){
    if(!strcmp(plist->mail_address,pname)){
      sprintf(ret_value,"From: %s\n",plist->petname);
      return;
    }
    plist = plist->next;
  }

  return;
}



