#ifndef _OPTION_H
#define _OPTION_H

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

#include <X11/Xaw/Command.h>

#define XtNoptionCommand "optionCommand"
#define XtCOptionCommand "OptionCommand"

typedef struct {
  String o_command;
} OptionRes;


#endif
