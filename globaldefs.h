#ifndef _GLOBALDEFS_H
#define _GLOBALDEFS_H

/*
 * global macro and type defines
 *
 * HASH_KEY の 値は適当 ^^; 
 */

#define XHISHO_VERSION "1.10"
#define CODE_NAME "Lime Release 1"
#define MIN(a,b) ((a) > (b) ? (b) :(a))
#define MAX(a,b) ((a) > (b) ? (a) :(b))
#define NUM_OF_ARRAY(a) (sizeof(a) / sizeof(a[0]))
#define HASH_KEY 253 

typedef enum _Method{POP_AUTH,APOP_AUTH,RPOP_AUTH}AuthMethod;
typedef enum _Biff{POP,APOP,YOUBIN,LOCAL}BiffMethod;

#endif
