#include "petname.h"
#include "globaldefs.h"
#include "mail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Petnameのリストの先頭を保持するポインタの配列。簡単なHashをかける。
 **/
static PetnameList *Petname[HASH_KEY];

extern String FilterCommand;


static int StrHash(char *);
static PetnameList *Petname_new(char *, char *);
static void Petname_delete(PetnameList *);
#ifdef ADDRESSBOOK
static void ReadAddrBook();
#endif

void ReadPetname(char *);
void SearchPetname(char *, char *);


/**
 * PetnameList オブジェクトのコンストラクタに相当する関数
 **/

static PetnameList *Petname_new(char *pname, char *addr)
{
  PetnameList *pname_ptr;

  pname_ptr = (PetnameList *) malloc(sizeof(PetnameList));
  memset(pname_ptr, 0, sizeof(PetnameList));
  pname_ptr->next = NULL;

  pname_ptr->petname = malloc(strlen(pname) + 1);
  memset(pname_ptr->petname, 0, strlen(pname) + 1);
  pname_ptr->mail_address = malloc(strlen(addr) + 1);
  memset(pname_ptr->mail_address, 0, strlen(addr) + 1);

  strcpy(pname_ptr->petname, pname);
  strcpy(pname_ptr->mail_address, addr);

  return pname_ptr;
}


/**
 * PetnameList オブジェクトのデストラクタに相当する関数
 **/

static void Petname_delete(PetnameList * ptr)
{
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

  if (ret > HASH_KEY)
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
  int i, in_quote, j;
  PetnameList *address_list, *pname_ptr, *ptr;
#ifdef EXT_FILTER
  char pcommand[128];
#endif

  sprintf(fname, "%s/.im/Addrbook", getenv("HOME"));

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

  buffer = malloc(BUFSIZ);
  pname = malloc(BUFSIZ);
  addr = malloc(BUFSIZ);
  address_list = pname_ptr = NULL;

  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    memset(pname, 0, BUFSIZ);
    j = 0;
    address_list = NULL;

    if (buffer[0] == ';')
      goto End;

    /**
     * short name を読み飛ばす
     **/
    for (i = 0; i < strlen(buffer); i++) {
      if (isspace(buffer[i]))
	break;
      if (buffer[i] == '#')
	goto End;
    }

    /**
     * 展開規則の定義は読み飛ばす
     **/

    if (buffer[i++] == ':')
      goto End;

    /**
     * アドレスを読む。複数ある場合もあるので、リストで保持する。
     **/

    for (j = 0; i < strlen(buffer); i++) {
      if (buffer[i] == ',') {
	/**
	 * 1つのアドレスの読み込みが終わったので、リストに登録
	 **/
	addr[j] = '\0';

	if (strlen(addr) > 1) {
	  pname_ptr = Petname_new("dummy", addr);

	  if (address_list != NULL) {
	    pname_ptr->next = address_list;
	  }
	  address_list = pname_ptr;
	}
	i++;
	j = 0;
	while (isspace(buffer[i])) {
	  i++;
	  if (buffer[i] == '#')
	    goto End;
	}
      }
      if (isspace(buffer[i]))
	break;
      if (buffer[i] == '#')
	goto End;
      addr[j++] = buffer[i];
    }

    addr[j] = '\0';

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
     * Petname の読み込み。" で囲まれた文字列中の空白文字はPetnameの1
     * 部。" で囲まれていない空白文字はPetnameの区切り文字。
     *
     * int in_quote = 1 (現在呼んでいる文字は " で囲まれた文字列の一部
     *                   である)
     **/

    for (i++, j = in_quote = 0; i < strlen(buffer), j < BUFSIZ; i++) {
      if (buffer[i] == '"') {
	if (in_quote == 1)
	  break;
	in_quote = 1;
      } else {
	if (in_quote == 0 && isspace(buffer[i]))
	  break;
	if (buffer[i] == '#')
	  break;
	if (buffer[i] != '"') {
	  pname[j] = buffer[i];
	  j++;
	}
      }
    }

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
  }

End:
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

  unsigned char *who, *tmp, *tmp2, *buffer;
  FILE *pfp;
  PetnameList *pname_ptr;
  int i, hashed;
#ifdef EXT_FILTER
  char pcommand[128];
#endif				/** EXT_FILTER **/

  who = malloc(BUFSIZ);
  tmp = malloc(BUFSIZ);
  tmp2 = malloc(BUFSIZ);
  buffer = malloc(BUFSIZ);

  for (i = 0; i < HASH_KEY; i++) {
    Petname[i] = NULL;
  }
  memset(tmp2, 0, BUFSIZ);

  if ((pfp = fopen(petname_f, "r")) == NULL) {
    fprintf(stderr, "no petname file:%s\n", petname_f);
    return;
  }
#ifdef EXT_FILTER

  fclose(pfp);
  sprintf(pcommand, "%s %s", FilterCommand, petname_f);
  if ((pfp = popen(pcommand, "r")) == NULL) {	/** reopen as pipe **/
    fprintf(stderr, "no filter command:%s\n", pcommand);
    return;
  }
#endif				/** EXT_FILTER **/

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
