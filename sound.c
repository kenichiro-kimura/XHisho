/**
 * waveplay - Programmed by Y.Sonoda (Mar. 1998),modified for XHisho by K.Kimura
 **/

#define _SOUND_GLOBAL
#include "globaldefs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <X11/Intrinsic.h>

#if defined (__FreeBSD__)
#include <machine/soundcard.h>
#define NORMAL_SOUND_PLAY
#elif defined (linux)
#include <linux/soundcard.h>
#define NORMAL_SOUND_PLAY
#endif

#include "sound.h"

#ifndef DEFAULT_DSP
#define DEFAULT_DSP "/dev/dsp"
#endif

#ifndef DEFAULT_BUFFERSIZE
#define DEFAULT_BUFFERSIZE 2048
#endif

/**
 *  プロトタイプ
 **/
static int _SoundPlay(const char *);
static int ExtSoundCommand(const char *);

#ifdef NORMAL_SOUND_PLAY
static int readWaveFile(int fd, PWAVEFORMAT pwavefmt, u_int * datasize);
static int openDSP(const char *devname, PWAVEFORMAT pwf);
static int playWave(int data_fd, u_int datasize, int dsp_fd);

static size_t bsize = DEFAULT_BUFFERSIZE;
static char *dsp_fname = DEFAULT_DSP;

static int _SoundPlay(const char *filename)
{
  int in_fd;
  int out_fd;
  WAVEFORMAT wf;
  u_int datasize;
  int rc;

  if (SoundCommand) {
    return ExtSoundCommand(filename);
  }
  if (filename[0] == '\0')
    return 0;

  if ((in_fd = open(filename, O_RDONLY)) == -1) {
    fprintf(stderr, "%s - ", filename);
    perror("open");
    return in_fd;
  }
  if (readWaveFile(in_fd, &wf, &datasize)) {
    close(in_fd);
    return -1;
  }
  if ((out_fd = openDSP(dsp_fname, &wf)) == -1) {
    perror("openDSP");
    close(in_fd);
    return -1;
  }
  rc = playWave(in_fd, datasize, out_fd);

  close(in_fd);
  close(out_fd);
  return rc;
}

static int readWaveFile(int fd, PWAVEFORMAT pwavefmt, u_int * datasize)
{
  int header = 0;
  int size = 0;
  char *buff;

  *datasize = 0;
  read(fd, (char *) &header, sizeof(int));
  if (header != H_RIFF) {
    fprintf(stderr, "Error: Not RIFF file.\n");
    return 1;
  }
  read(fd, (char *) &size, sizeof(int));
  read(fd, (char *) &header, sizeof(int));
  if (header != H_WAVE) {
    fprintf(stderr, "Error: Not WAVE file.\n");
    return 2;
  }
  while (read(fd, (char *) &header, sizeof(int)) == sizeof(int)) {
    read(fd, (char *) &size, sizeof(int));

    if (header == H_FMT) {
      if ((size_t) size < sizeof(WAVEFORMAT)) {
	fprintf(stderr, "Error: Illegal header!\n");
	return 3;
      }
      buff = (char*)malloc((size_t) size);
      read(fd, buff, size);
      memcpy((void *) pwavefmt, (void *) buff, sizeof(WAVEFORMAT));
      free(buff);
      if (pwavefmt->wFormatTag != 1) {
	fprintf(stderr, "Error: Unsupported format.\n");
	return 4;
      }
    } else if (header == H_DATA) {
      /**
       * ファイルポインタがdataチャンクに到達したら関数を終了
       **/
      *datasize = (u_int) size;
      return 0;
    } else {
      /**
       * 余計なチャンクは読み飛ばす
       **/
      lseek(fd, size, SEEK_CUR);
    }

  }

  fprintf(stderr, "Error: data chunk not found.\n");
  return 10;
}

static int openDSP(const char *devname, PWAVEFORMAT pwf)
{
  int fd;
  int status;
  int arg;

  if ((fd = open(devname, O_WRONLY)) == -1)
    return fd;

  /**
   * チャンネル(STEREO or MONAURAL)を設定
   **/
  arg = (int) (pwf->nChannels);
  status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, (char *) &arg);
  if (status == -1 || arg != (int) (pwf->nChannels)) {
    close(fd);
    return -1;
  }
  /**
   * サンプリングレートを設定
   **/
  arg = (int) (pwf->nSamplesPerSec);
  status = ioctl(fd, SOUND_PCM_WRITE_RATE, (char *) &arg);
  if (status == -1) {
    close(fd);
    return -1;
  }
#ifdef DEBUG
  printf("DSP - Sampling rate: %d\n", arg);
#endif

  /**
   * 量子化ビット数(8 or 16Bit)を設定
   **/
  arg = (int) (pwf->wBitsPerSample);
  status = ioctl(fd, SOUND_PCM_WRITE_BITS, (char *) &arg);
  if (status == -1 || arg != (int) (pwf->wBitsPerSample)) {
    close(fd);
    return -1;
  }
  return fd;
}

static int playWave(int data_fd, u_int datasize, int dsp_fd)
{
  register int i, nr, nw, off;
  static char *buf;
  int tr, rd;

  if ((buf = (char *) malloc(bsize)) == NULL) {
    fprintf(stderr, "Error: playWave - malloc failed.\n");
    return -1;
  }
  tr = datasize / bsize;
  rd = datasize % bsize;

  for (i = 0; i < tr + 1; i++) {
    if (i == tr) {
      if (rd == 0)
	break;

      if ((nr = read(data_fd, buf, rd)) <= 0)
	break;
    } else {
      if ((nr = read(data_fd, buf, bsize)) <= 0)
	break;
    }

    for (off = 0; nr; nr -= nw, off += nw) {
      if ((nw = write(dsp_fd, buf + off, nr)) < 0) {
	fprintf(stderr, "Error: playWave - write data\n");
	free(buf);
	return -1;
      }
    }
  }

  free(buf);
  return 0;
}

#else
static int _SoundPlay(const char *filename)
{
  char command[BUFSIZ];

  if (SoundCommand) {
    return ExtSoundCommand(filename);
  }
  return 1;
}

#endif

int SoundPlay(const char *filename)
{
  /**
   * 音を鳴らす実体は_SoudPlay()。ここではforkするだけ
   **/

  int pid,status;

  if((pid = fork()) == 0){
    if(fork() == 0){
      _SoundPlay(filename);
      exit(0);
    } else {
      exit(0);
    }
  } else {
    while(wait(&status) != pid);
  }
    
  return 0;
}

static int ExtSoundCommand(const char *filename)
{
  char command[BUFSIZ];

  sprintf(command, SoundCommand, filename);
  return system(command);
}
