#define _PETNAME_GLOBAL
#include "globaldefs.h"
#include "petname.h"
#include "mail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Petname�Υꥹ�Ȥ���Ƭ���ݻ�����ݥ��󥿤����󡣴�ñ��Hash�򤫤��롣
 **/
static PetnameList *Petname[HASH_KEY];

static int StrHash(char *);
static PetnameList *Petname_new(char *, char *);
static void Petname_delete(PetnameList *);
#ifdef ADDRESSBOOK
static void ReadAddrBook();
#endif

/**
 * PetnameList ���֥������ȤΥ��󥹥ȥ饯������������ؿ�
 **/

static PetnameList *Petname_new(char *pname, char *addr)
{
  PetnameList *pname_ptr;

  pname_ptr = (PetnameList *) malloc(sizeof(PetnameList));
  if(pname_ptr == NULL) return NULL;
  pname_ptr->next = NULL;
  /*
  pname_ptr->petname = strdup(pname);
  pname_ptr->mail_address = strdup(addr);
  */
  pname_ptr->petname = malloc(strlen(pname) + 1);
  strcpy(pname_ptr->petname,pname);
  pname_ptr->mail_address = malloc(strlen(addr) + 1);
  strcpy(pname_ptr->mail_address,addr);

  if(pname_ptr->petname[strlen(pname) - 1] == '\n')
    pname_ptr->petname[strlen(pname) - 1] = '\0';

  return pname_ptr;
}


/**
 * PetnameList ���֥������ȤΥǥ��ȥ饯������������ؿ�
 **/

static void Petname_delete(PetnameList * ptr)
{
  if(ptr == NULL) return;
  free(ptr->mail_address);
  free(ptr->petname);
  free(ptr);
}

static int StrHash(char *string)
{
  /**
   * ʸ������ñ��Hash�ˤ����롣0 �� HASH_KEY �δ֤��������֤����׻�
   * ��Ŭ���ʤΤǡ�Hash�λ����礤�ϴ��Ԥ��ʤ��褦��(��)������Ǥ���
   * �������Ƥ����Ǥ�Ĵ�٤���Ϥޤ����Ȼפ���
   * ��˻��Petname�ե�����(248����ȥ�)��HASH_KEY=253�Ǥ�äƤߤ�
   * �顢������Ϳ���4�ǡ�157��Hash�ͤ˻��ä��ΤǤ���ʤ��ư���󤸤�
   * �ʤ����Ȼפ��ޤ���Petname�Υ���ȥ꤬���ʤ��Ȥ���
   * HASH_KEY(globaldefs.h�����)�򸺤餹�ȥ��꤬����Ǥ��ޤ���
   **/

  int ret;
  char *ptr;

  ret = 0;

  for (ptr = string; *ptr; ptr++) {
    ret += (int) (*ptr);
    ret %= HASH_KEY;
  }

  if (abs(ret) > HASH_KEY)
    ret %= HASH_KEY;
  return abs(ret);
}

#ifdef ADDRESSBOOK
static void ReadAddrBook()
{
  /**
   * support for mew's Addrbook :-)
   *
   * Petname�򥢥ɥ쥹�֥å������ɤߡ�Hash�ꥹ�Ȥ��ݻ����롣mail���ɥ쥹��
   * StrHash()�ˤ������ơ��֥�γ�������˥���ȥ���������롣Hash�ͤ�
   * �֤Ĥ��ä��ơ��֥����ϥꥹ�ȤΤդ�򤷤������å�(�⤷���ϥ��塼)��
   * ���Ф��ʤ�����ɤ���ȸ��äƤ⤤�������פ���˥ꥹ�ȷ�����Ƭ��
   * ���Ǥ��ɲä��Ƥ��äƤ��롣
   **/

  FILE *fp;
  unsigned char *buffer, *pname, *addr;
  char fname[128];
  char* ch_ptr;
  char* dst_ptr;
  int i, in_quote, j;
  PetnameList *address_list, *pname_ptr, *ptr;
#ifdef EXT_FILTER
  char pcommand[128];
#endif

  sprintf(fname, "%s/.im/Addrbook", getenv("HOME"));
  address_list = pname_ptr = NULL;

#ifdef EXT_FILTER

  sprintf(pcommand, "%s %s", FilterCommand, fname);
  if ((fp = popen(pcommand, "r")) == NULL) {
    fprintf(stderr, "no filter command:%s\n", pcommand);
    return;
  }
#else
  if ((fp = fopen(fname, "r")) == NULL) {
    fprintf(stderr, "no addrbook:%s\n", fname);
    return;
  }
#endif

  buffer = (char*)malloc(BUFSIZ);
  pname = (char*)malloc(BUFSIZ);
  addr = (char*)malloc(BUFSIZ);

  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    j = 0;
    address_list = NULL;

    if (buffer[0] == ';' || strlen(buffer) < 1)
      continue;

    /**
     * short name ���ɤ����Ф�
     **/
    ch_ptr = buffer;
    while(ch_ptr != NULL && *ch_ptr != '\0'
	  &&!isspace((unsigned char)*ch_ptr) && *ch_ptr != ':')
      ch_ptr++;

    if(ch_ptr == NULL || *ch_ptr == '\0')
      continue;

    /**
     * Ÿ����§��������ɤ����Ф�
     **/

    if(*ch_ptr == ':') continue;

    ch_ptr++;
    if(ch_ptr == NULL || *ch_ptr == '\0')
      continue;

    /**
     * ���ɥ쥹���ɤࡣʣ��������⤢��Τǡ��ꥹ�Ȥ��ݻ����롣
     **/

    dst_ptr = addr;
    while(strlen(ch_ptr) > 0){
      if(isspace((unsigned char)*ch_ptr)) break;
      if(*ch_ptr == ','){
	/**
	 * 1�ĤΥ��ɥ쥹���ɤ߹��ߤ�����ä��Τǡ��ꥹ�Ȥ���Ͽ
	 **/
	*dst_ptr = '\0';

	if (strlen(addr) > 0) {
	  pname_ptr = Petname_new("dummy", addr);
	  if (address_list != NULL) {
	    pname_ptr->next = address_list;
	  }
	  address_list = pname_ptr;
	}
	dst_ptr = addr;
	ch_ptr++;
	while(ch_ptr != NULL && isspace((unsigned char)*ch_ptr))
	  ch_ptr++;
      }
      while(ch_ptr != NULL && isspace((unsigned char)*ch_ptr))
	ch_ptr++;
      if (*ch_ptr == '#'){
	for(ptr = address_list;ptr!= NULL;){
	  pname_ptr = ptr->next;
	  Petname_delete(ptr);
	  ptr = pname_ptr;
	}
	goto W_End;
      }
      *dst_ptr++ = *ch_ptr++;
    }

    *dst_ptr = '\0';

    /**
     * �Ǹ���ɤ߹�������ɥ쥹��ꥹ�Ȥ���Ͽ
     **/

    if (strlen(addr) > 1) {
      pname_ptr = Petname_new("dummy", addr);

      if (address_list != NULL) {
	pname_ptr->next = address_list;
      }
      address_list = pname_ptr;
    }

    /**
     * Petname ���ɤ߹��ߡ�"" �ǰϤޤ줿ʸ������ζ���ʸ����Petname��1
     * ����"" �ǰϤޤ�Ƥ��ʤ�����ʸ����Petname�ζ��ڤ�ʸ����
     *
     * int in_quote = 1 (���߸Ƥ�Ǥ���ʸ���� "" �ǰϤޤ줿ʸ����ΰ���
     *                   �Ǥ���)
     **/

    while(ch_ptr != NULL && isspace((unsigned char)*ch_ptr))
      ch_ptr++;

    for (j = in_quote = 0; ch_ptr!= NULL && j < BUFSIZ; ch_ptr++) {
      if (*ch_ptr == '"') {
	if (in_quote == 1)
	  break;
	in_quote = 1;
      } else {
	if (in_quote == 0 && isspace((unsigned char)*ch_ptr))
	  break;
	if (*ch_ptr == '#')
	  break;
	if (*ch_ptr != '"') {
	  pname[j] = *ch_ptr;
	  j++;
	}
      }
    }

    pname[j] = '\0';

    /**
     * Petname ����Ͽ���ꥹ�Ȥ��饢�ɥ쥹���˼��Ф�����Ͽ���롣
     **/

    ptr = address_list;
    while (ptr && strlen(pname) > 1) {
      i = StrHash(ptr->mail_address);

      pname_ptr = Petname_new(pname, ptr->mail_address);

      if (Petname[i] != NULL) {
	pname_ptr->next = Petname[i];
      }
      Petname[i] = pname_ptr;

      pname_ptr = ptr->next;
      Petname_delete(ptr);
      ptr = pname_ptr;
    }
  W_End:
  }


  /**
   * ��λ�������̾�ν�λ�ʳ��˥����ȹ԰ʲ��κ�����(��Ƭ�� ";",��
   * ��� "#")�����롣Petname������� "#" ���Ф��顢�����ޤǤ�Petname
   * �Ȥ�����Ͽ���롣
   **/

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

void ReadPetname(char *petname_f)
{
  /**
   * Petname��ե����뤫���ɤߡ�Hash�ꥹ�Ȥ��ݻ����롣mail���ɥ쥹��
   * StrHash()�ˤ������ơ��֥�γ�������˥���ȥ���������롣Hash�ͤ�
   * �֤Ĥ��ä��ơ��֥����ϥꥹ�ȤΤդ�򤷤������å�(�⤷���ϥ��塼)��
   * ���Ф��ʤ�����ɤ���ȸ��äƤ⤤�������פ���˥ꥹ�ȷ�����Ƭ��
   * ���Ǥ��ɲä��Ƥ��äƤ��롣
   **/

  char *who, *tmp, *tmp2, *buffer;
  FILE *pfp;
  PetnameList *pname_ptr;
  int i, hashed;
#ifdef EXT_FILTER
  char pcommand[128];
#endif				/** EXT_FILTER **/

  for (i = 0; i < HASH_KEY; i++) {
    Petname[i] = NULL;
  }

#ifdef EXT_FILTER
  sprintf(pcommand, "%s %s", FilterCommand, petname_f);
  if ((pfp = popen(pcommand, "r")) == NULL) {	/** reopen as pipe **/
    fprintf(stderr, "no filter command:%s\n", pcommand);
    return;
  }
#else
  if ((pfp = fopen(petname_f, "r")) == NULL) {
    fprintf(stderr, "no petname file:%s\n", petname_f);
    return;
  }
#endif				/** EXT_FILTER **/

  who = (char*)malloc(BUFSIZ);
  tmp = (char*)malloc(BUFSIZ);
  tmp2 = (char*)malloc(BUFSIZ);
  buffer = (char*)malloc(BUFSIZ);

  while (fgets(buffer, BUFSIZ, pfp) != NULL) {
    sscanf(buffer, "%s %s", who, tmp);
    if (strchr(tmp, '\"')) {
      strcpy(tmp2, strtok(strchr(tmp, '\"') + 1, "\""));
      hashed = StrHash(who);

      pname_ptr = Petname_new(tmp2, who);

      if (Petname[hashed] != NULL) {
	pname_ptr->next = Petname[hashed];
      }
      Petname[hashed] = pname_ptr;
    }
  }

#ifdef EXT_FILTER
  pclose(pfp);
#else
  fclose(pfp);
#endif				/** EXT_FILTER **/
  free(buffer);
  free(tmp);
  free(tmp2);
  free(who);

#ifdef ADDRESSBOOK
  ReadAddrBook();
#endif
}

void SearchPetname(char *ret_value, char *pname)
{
  /**
   * �ꥹ�Ȥ򸡺����Ƴ�������Petname��̵����õ�����⤷���ä���ret_value��
   * ��From:Petname�פη���������֤���̵�����ret_value�Ϥ��Τޤޡ�
   **/

  PetnameList *plist;
  int hashed;

  if(pname[0] == '\0') return;

  hashed = StrHash(pname);
  plist = Petname[hashed];

  while (plist) {
    if (!strcmp(plist->mail_address, pname)) {
      sprintf(ret_value, "From: %s\n", plist->petname);
      return;
    }
    plist = plist->next;
  }

  return;
}
