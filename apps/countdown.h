#ifndef _COUNTDOWN_H
#define _COUNTDOWN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "../config.h"

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif



#define CD_FILE ".Schedule/xhs.cdown"
#define FILTER_COMMAND "nkf -e"

#endif

