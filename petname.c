#define _PETNAME_GLOBAL
#include "globaldefs.h"
#include "petname.h"
#include "mail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Petnameのリストの先頭を保持するポインタの配列。簡単なHashをかける。
 **/
static PetnameList *Petname[HASH_KEY];

static int StrHash(char *);
static PetnameList *Petname_new(char *, char *);
static void Petname_delete(PetnameList *);
#ifdef ADDRESSBOOK
static void ReadAddrBook();
#endif

/**
 * PetnameList オブジェクトのコンストラクタに相当する関数
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
 * PetnameList オブジェクトのデストラクタに相当する関数
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
   * 文字列を簡単なHashにかける。0 〜 HASH_KEY の間の整数を返す。計算
   * は適当なので、Hashの散り具合いは期待しないように(笑)。それでも線
   * 形に全ての要素を調べるよりはましだと思う。
   * 試しに私のPetnameファイル(248エントリ)でHASH_KEY=253でやってみた
   * ら、最大衝突数が4で、157のHash値に散ったのでそれなりに動くんじゃ
   * ないかと思います。Petnameのエントリが少ないときは
   * HASH_KEY(globaldefs.hで定義)を減らすとメモリが節約できます。
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
   * Petnameをアドレスブックから読み、Hashリストで保持する。mailアドレスを
   * StrHash()にかけ、テーブルの該当ヶ所にエントリを挿入する。Hash値が
   * ぶつかったテーブルの先はリストのふりをしたスタック(もしくはキュー)。
   * 取り出さないからどちらと言ってもいいが、要するにリスト型の先頭に
   * 要素を追加していっている。
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
     * short name を読み飛ばす
     **/
    ch_ptr = buffer;
    while(ch_ptr != NULL && *ch_ptr != '\0'
	  &&!isspace((unsigned char)*ch_ptr) && *ch_ptr != ':')
      ch_ptr++;

    if(ch_ptr == NULL || *ch_ptr == '\0')
      continue;

    /**
     * 展開規則の定義は読み飛ばす
     **/

    if(*ch_ptr == ':') continue;

    ch_ptr++;
    if(ch_ptr == NULL || *ch_ptr == '\0')
      continue;

    /**
     * アドレスを読む。複数ある場合もあるので、リストで保持する。
     **/

    dst_ptr = addr;
    while(strlen(ch_ptr) > 0){
      if(isspace((unsigned char)*ch_ptr)) break;
      if(*ch_ptr == ','){
	/**
	 * 1つのアドレスの読み込みが終わったので、リストに登録
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
     * 最後に読み込んだアドレスをリストに登録
     **/

    if (strlen(addr) > 1) {
      pname_ptr = Petname_new("dummy", addr);

      if (address_list != NULL) {
	pname_ptr->next = address_list;
      }
      address_list = pname_ptr;
    }

    /**
     * Petname の読み込み。"" で囲まれた文字列中の空白文字はPetnameの1
     * 部。"" で囲まれていない空白文字はPetnameの区切り文字。
     *
     * int in_quote = 1 (現在呼んでいる文字は "" で囲まれた文字列の一部
     *                   である)
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
     * Petname の登録。リストからアドレスを順に取り出し、登録する。
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
   * 終了処理。通常の終了以外にコメント行以下の作業中止(行頭の ";",途
   * 中の "#")がある。Petnameの途中で "#" が出たら、そこまでをPetname
   * として登録する。
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
   * Petnameをファイルから読み、Hashリストで保持する。mailアドレスを
   * StrHash()にかけ、テーブルの該当ヶ所にエントリを挿入する。Hash値が
   * ぶつかったテーブルの先はリストのふりをしたスタック(もしくはキュー)。
   * 取り出さないからどちらと言ってもいいが、要するにリスト型の先頭に
   * 要素を追加していっている。
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
   * リストを検索して該当するPetnameが無いか探す。もしあったらret_valueに
   * 「From:Petname」の形で入れて返す。無ければret_valueはそのまま。
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
