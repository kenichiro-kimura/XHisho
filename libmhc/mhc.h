#ifndef _MHC_H
#define _MHC_H
/*
 * MHC�ǡ����ɤ߽Ф��饤�֥�� libmhc �إå��ե�����
 *
 * (C) 2000, Ken'ichirou Kimura <kimura@db.is.kyushu-u.ac.jp>
 *
 * �ؿ�����
 *
 * ���ȥ꡼�෿I/O�δؿ��ȻȤ����Υ��᡼����Ʊ����
 * readdir()�դ˹�碌�Ƥ��롣
 *
 *
 * MHCD* openmhc(const char *home_dir, const char *year_month);
 *   home_dir/year_month ��MHC�ǡ������ɤ߹�������򤹤롣�㤨��
 *
 *   mhcd_ptr = openmhc("/home/kimura/Mail/schedule/","2000/10");
 *
 *   ������˼��Ԥ�����NULL���֤���
 *
 *
 * mhcent* readmhc(MHCD*);
 *   ����ȥ��缡�ɤ߽Ф����Ǹ�ޤ��ɤ����,����ȥ꤬�ʤ����NULL���֤롣
 *
 *
 * void seekmhc(MHCD*,int loc);
 *   �����ɤ२��ȥ�ξ���locʬ����ȥ��ʤ�롣loc < 0�ʤ���롣
 *   
 *
 * void rewindmhc(MHCD*);
 *   �����ɤ२��ȥ�ξ��������֤��᤹��
 *
 *
 * int  closemhc(MHCD*);
 *   ��λ�������꥽������������롣������0,���Ԥ�1���֤���
 *
 *
 * int  isschedule(const mhcent*,int year,int month,int day);
 *   ���Υ���ȥ꤬year/month/day�Υ������塼�뤫�ɤ��������å���
 *   �������塼��Ǥ���ʤ��1���֤����㤦�ʤ�0���֤���
 *
 *
 * int iscategory(const mhcent*,const char*);
 *   Ϳ����ʸ����X-SC-Category: �����äƤ��뤫������å���
 *   ʸ�����X-SC-Category: Ʊ��,���ڡ������ڤ��ʣ�����ҤǤ��롣
 *   �㤨��X-SC-Category: �� "work private" �ǡ������å�����ʸ����Ȥ���
 *   "work public"��1���֤롣
 *
 *
 * MHC* OpenMHC(const char* home_dir, int year, int month);
 *   home_dir/year/month �˲ä���intersect���ɤ߹��ࡣ
 *   ����ˡ�day���Ȥ��ݻ����롣�ɤ߹��ߤ˼��Ԥ�����NULL���֤���
 *
 * mhcent* ReadMHC(MHC*);
 *   ����ȥ��缡�ɤࡣ�Ǹ�ޤ��ɤ����������ȥ꤬�ʤ����NULL���֤���
 *  
 *
 * void SetMHC(MHC*,int day);
 *   MHC* ��day�˹�碌�롣����ʹߡ�ReadMHC()���ɤޤ��Τ�year/month/day
 *   �Υ���ȥꡣ������֤Ǥ�year/month/1�˹�碌�Ƥ��롣
 *
 *
 * void SeekMHC(MHC*,int);
 *   MHC* ��ʤ�롣
 *
 *
 * void RewindMHC(MHC*);
 *   MHC* �������֤��᤹��
 *
 *
 * int CloseMHC(MHC*);
 *   ��λ������
 *
 *
 * char* GetSubject(const mhcent*);
 *   X-SC-Subject: ����Ф���
 *
 *
 * int GetAlarm(const mhcent*);
 *   X-SC-Alarm: ���ͤ�ʬñ�̤��֤���
 *
 *
 * $HOME/Mail/schedule/ �ˤ��롢2000/10/1�Υ���ȥ����Ф��ˤ�
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
 * �ʤɤȤ��롣
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
