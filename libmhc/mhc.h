#ifndef _MHC_H
#define _MHC_H
/*
 * MHCデータ読み出しライブラリ libmhc ヘッダファイル
 *
 * (C) 2000, Ken'ichirou Kimura <kimura@db.is.kyushu-u.ac.jp>
 *
 * 関数一覧
 *
 * ストリーム型I/Oの関数と使い方のイメージは同じ。
 * readdir()辺に合わせてある。
 *
 *
 * MHCD* openmhc(const char *home_dir, const char *year_month);
 *   home_dir/year_month のMHCデータを読み込む準備をする。例えば
 *
 *   mhcd_ptr = openmhc("/home/kimura/Mail/schedule/","2000/10");
 *
 *   初期化に失敗したらNULLを返す。
 *
 *
 * mhcent* readmhc(MHCD*);
 *   エントリを順次読み出す。最後まで読んだか,エントリがなければNULLが返る。
 *
 *
 * void seekmhc(MHCD*,int loc);
 *   次に読むエントリの場所をloc分エントリを進める。loc < 0なら戻る。
 *   
 *
 * void rewindmhc(MHCD*);
 *   次に読むエントリの場所を初期位置に戻す。
 *
 *
 * int  closemhc(MHCD*);
 *   終了処理。リソースを解放する。成功で0,失敗で1を返す。
 *
 *
 * int  isschedule(const mhcent*,int year,int month,int day);
 *   そのエントリがyear/month/dayのスケジュールかどうかチェック。
 *   スケジュールであるならば1を返し、違うなら0を返す。
 *
 *
 * int iscategory(const mhcent*,const char*);
 *   与えた文字列がX-SC-Category: に入っているかをチェック。
 *   文字列はX-SC-Category: 同様,スペース区切りで複数記述できる。
 *   例えばX-SC-Category: が "work private" で、チェックする文字列として
 *   "work public"は1が返る。
 *
 *
 * MHC* OpenMHC(const char* home_dir, int year, int month);
 *   home_dir/year/month に加えてintersectも読み込む。
 *   さらに、dayごとに保持する。読み込みに失敗したらNULLを返す。
 *
 * mhcent* ReadMHC(MHC*);
 *   エントリを順次読む。最後まで読んだか、エントリがなければNULLを返す。
 *  
 *
 * void SetMHC(MHC*,int day);
 *   MHC* をdayに合わせる。これ以降、ReadMHC()で読まれるのはyear/month/day
 *   のエントリ。初期状態ではyear/month/1に合わせてある。
 *
 *
 * void SeekMHC(MHC*,int);
 *   MHC* を進める。
 *
 *
 * void RewindMHC(MHC*);
 *   MHC* を初期位置に戻す。
 *
 *
 * int CloseMHC(MHC*);
 *   終了処理。
 *
 *
 * char* GetSubject(const mhcent*);
 *   X-SC-Subject: を取り出す。
 *
 *
 * int GetAlarm(const mhcent*);
 *   X-SC-Alarm: の値を分単位で返す。
 *
 *
 * $HOME/Mail/schedule/ にある、2000/10/1のエントリを取り出すには
 *
 *   MHC* mhc_ptr;
 *   mhcent* ent_ptr;
 *
 *   mhc_ptr = OpenMHC("$HOME/Mail/schedule/",2000,10);
 *   SetMHC(mhc_ptr,1);
 *
 *   while((ent_ptr = ReadMHC(mhc_ptr)) != NULL){
 *      printf("Subject:%s\n",GetSubject(ent_ptr));
 *   }
 *  
 *   CloseMHC(mhc_ptr);
 *
 * などとする。
 *
 */

#define NUM_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

typedef struct _mhcent{
  char* Entry[7];
  char* filename;
} mhcent;

typedef struct __MHCD{
  struct __MHCD* prev;
  struct __MHCD* next;  
  mhcent* item;
} _MHCD;

typedef _MHCD* MHCD;

typedef struct _entrylist{
  mhcent* item;
  struct _entrylist* prev;
  struct _entrylist* next;
} entrylist;

typedef struct _MHC{
  entrylist* table[32];
  entrylist* ptr;
  MHCD* mhcd_ptr;
  MHCD* intersect_ptr;
} _MHC;

typedef _MHC* MHC;

enum {
  CONTINUED_LINE = -2,
  NO_TAG = -1,
  X_SC_Day = 0,
  X_SC_Time = 1,
  X_SC_Duration = 2,
  X_SC_Cond = 3,
  X_SC_Alarm = 4,
  X_SC_Subject = 5,
  X_SC_Category = 6
};

MHCD* openmhc(const char *, const char *);
mhcent* readmhc(MHCD*);
void seekmhc(MHCD*,int);
void rewindmhc(MHCD*);
int  closemhc(MHCD*);
int  isschedule(const mhcent*,int,int,int);
int  iscategory(const mhcent*,const char*);

MHC* OpenMHC(const char*, int, int);
mhcent* ReadMHC(MHC*);
void SetMHC(MHC*,int);
void SeekMHC(MHC*,int);
void RewindMHC(MHC*);
int CloseMHC(MHC*);

char* GetSubject(const mhcent*);
int GetAlarm(const mhcent*);
char* GetCategory(const mhcent*);

#endif
