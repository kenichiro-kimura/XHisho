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
 *   mhcd_ptr = openmhc("/home/kimura/Mail/schedule/","2000/10");
 *
 * mhcent* readmhc(MHCD*);
 *   エントリを順次読み出す。
 *
 * void seekmhc(MHCD*,int loc);
 *   次に読むエントリの場所をloc分エントリを進める。loc < 0なら戻る
 *
 * void rewindmhc(MHCD*);
 *   次に読むエントリの場所を初期位置に戻す。
 *
 * int  closemhc(MHCD*);
 *   終了処理。リソースを解放する。
 *
 * int  isschedule(const mhcent*,int year,int month,int day);
 *   そのエントリがyear/month/dayのスケジュールかどうかチェック。
 *   スケジュールであるならば1を返し、違うなら0を返す。
 *
 * MHC* OpenMHC(const char* home_dir, int year, int month);
 *   home_dir/year/month に加えてintersectも読み込む。
 *   さらに、dayごとに保持する。
 *
 * mhcent* ReadMHC(MHC*);
 *   エントリを順次読む。
 *
 * void SetMHC(MHC*,int day);
 *   MHC* をdayに合わせる。これ以降、ReadMHC()で読まれるのはyear/month/day
 *   のエントリ。
 *
 * void SeekMHC(MHC*,int);
 *   MHC* を進める。
 *
 * void RewindMHC(MHC*);
 *   MHC* を初期位置に戻す。
 *
 * int CloseMHC(MHC*);
 *   終了処理。
 *
 * char* GetSubject(const mhcent*);
 *   X-SC-Subject: を取り出す。
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
  char* Entry[6];
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
  X_SC_Day = 0,
  X_SC_Time = 1,
  X_SC_Duration = 2,
  X_SC_Cond = 3,
  X_SC_Alarm = 4,
  X_SC_Subject = 5
};



MHCD* openmhc(const char *, const char *);
mhcent* readmhc(MHCD*);
void seekmhc(MHCD*,int);
void rewindmhc(MHCD*);
int  closemhc(MHCD*);
int  isschedule(const mhcent*,int,int,int);

MHC* OpenMHC(const char*, int, int);
mhcent* ReadMHC(MHC*);
void SetMHC(MHC*,int);
void SeekMHC(MHC*,int);
void RewindMHC(MHC*);
int CloseMHC(MHC*);

char* GetSubject(const mhcent*);
int GetAlarm(const mhcent*);

#endif
