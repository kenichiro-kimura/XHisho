#ifndef _PETNAME_H
#define _PETNAME_H

typedef struct _PetnameList{
  char *petname;
  char *mail_address;
  struct _PetnameList* next;
} PetnameList;

#endif
