XHisho*international: TRUE
XHisho*thickness: 5
XHisho*frameMode: 0
XHisho*messageFile: XHISHODIR/Messages

XHisho*FontSet: -*-fixed-medium-r-normal--16-*
XHisho*cgFile: XHISHODIR/hisho.bmp
XHisho*extFilter: nkf -e -m

!!
!!resources for biff
!!

XHisho*petnameFile: XHISHODIR/Petname
XHisho.MailAlert.youbinServer: localhost
XHisho.MailAlert.youbinCommand: /usr/local/bin/youbin
XHisho.MailAlert.popServer: localhost

!!
!!resources for clock
!!

XHisho*clockFormat: %s:%s:%s
XHisho*clockArg:hMs
!XHisho.XHisho.FontSet: -*-fixed-medium-r-normal--16-*

!!
!!resources for Scheduler
!!

XHisho.OpenMessage.configFile: XHISHODIR/greeting.cfg

XHisho.OpenMessage.messageArg: ehm
XHisho.OpenMessage.pastSchedColor: LightGrey
XHisho.OpenMessage.alertSchedColor: red
XHisho.OpenMessage.normalSchedColor: black
XHisho.OpenMessage.leaveTime: 5
XHisho.OpenMessage.scheduleDir: .Schedule/
!XHisho.OpenMessage.xcalendarCompatible: False

!!
!!resources for Schedule editor
!!

XHisho.OpenMessage.scheduleSeparator: ,

!!
!!resources for Schedule
!!

XHisho.Menu.itemHead: <>

!!
!!resources for calender
!!

XHisho.Calendar.prevButton: Next
XHisho.Calendar.nextButton: Prev
XHisho.Calendar.existColor: Grey

!!
!!resources for Preference editor
!!

XHisho.ResEdit.resEditLabel0: mail window timeout(sec.)
XHisho.ResEdit.resEditLabel1: mail check interval(sec.)
XHisho.ResEdit.resEditLabel2: message columns
XHisho.ResEdit.resEditLabel3: message lines
XHisho.ResEdit.resEditLabel4: focuswin interval(0.1 sec.)

!!
!! resources for focusWin
!!

!XHisho*focusYoff: 50
!XHisho*justify: right

!!
!! resources for sound
!!

XHisho.OpenMessage.scheduleAlertSound: XHISHODIR/schedule.wav
XHisho.MailAlert.newMailSound: XHISHODIR/newmail.wav
!XHisho*extSoundCommand: cat %s > /dev/audio

!!
!! resources for option window
!!

XHisho*optionTimeout: 5
XHisho*optionWidth:300
XHisho*optionHeight:200
XHisho*UOptionWindow*optionWidth:300
XHisho*UOptionWindow*optionHeight:150

XHisho*option*cursor: left_ptr
!XHisho*option*scrollVertical: whenNeeded
XHisho*kawariDir: XHISHODIR
!XHisho*option*FontSet: -*-fixed-medium-r-normal--12-*

!!
!!About XHisho...
!!

XHisho.About.aboutLabel:\
\              XHisho Ver.%s\n\
\n\
Copyright(c) 1998-2001  Ken'ichirou Kimura\n\
\n\
\n\
XHisho is your private secretary on X Window System.\n\
It works as biff,scheduler,and so on.
