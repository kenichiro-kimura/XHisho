#ifndef _ABOUT_H
#define _ABOUT_H

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/Atoms.h>
#include <X11/extensions/shape.h>
#include <X11/Xlocale.h>
#include <X11/Shell.h>
#include <X11/Xmu/Editres.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>

#define XtNaboutLabel "aboutLabel"
#define XtCAboutLabel "AboutLabel"

typedef struct {
  String about_s;
}   AboutRes;

#endif
