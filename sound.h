/*
  header file for waveplay.
  Copyright(c) 1998 Yoshihide Sonoda (ysonoda@dontaku.csce.kyushu-u.ac.jp)
*/

#ifndef _WAVE_FMT_H_
#define _WAVE_FMT_H_

#include <sys/types.h>

#define H_RIFF (*(int *)"RIFF")
#define H_WAVE (*(int *)"WAVE")
#define H_DATA (*(int *)"data")
#define H_FMT  (*(int *)"fmt ")

/* 構造体定義 (ref. MS-Windows mmsystem.h) */
typedef struct tWAVEFORMAT{
  u_short wFormatTag;
  u_short nChannels;
  u_long  nSamplesPerSec;
  u_long  nAvgBytesPerSec;
  u_short nBlockAlign;
  u_short wBitsPerSample;
} WAVEFORMAT, *PWAVEFORMAT;

typedef struct tWAVEFORMATEX{
  u_short wFormatTag;
  u_short nChannels;
  u_long  nSamplesPerSec;
  u_long  nAvgBytesPerSec;
  u_short nBlockAlign;
  u_short wBitsPerSample;
  u_short cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX;

#endif /* _WAVE_FMT_H_ */
