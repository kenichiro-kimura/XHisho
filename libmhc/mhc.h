#ifndef _MHC_H
#define _MHC_H

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
