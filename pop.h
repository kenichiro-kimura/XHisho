#ifndef _POP_H
#define _POP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>

enum {
  /*
   * define POP error messages 
   */
  OK = 0,
  ERR = 1,
  BUFFER_IS_TOO_SMALL  = 2,
  NO_PASSWORD_FILE = 3,
  INVALID_PERMISSION = 4
};

typedef struct _UserData{
  /*
   * user data for POP Authorization
   */
  char name[128]; 
  char pass[256];
  char server[256];
} UserData;

#endif
