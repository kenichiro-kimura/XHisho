XHisho*international: TRUE
XHisho*thickness: 5
XHisho*frameMode: 0
XHisho*messageFile: IXHISHODIR/Messages

XHisho*FontSet: -*-fixed-medium-r-normal--16-*
XHisho*cgFile: XHISHODIR/akari.bmp
XHisho*extFilter: nkf -e -m

!!
!!biff関係のリソース
!!

XHisho*petnameFile: IXHISHODIR/Petname.jp
XHisho.MailAlert.youbinServer: localhost
XHisho.MailAlert.youbinCommand: /usr/local/bin/youbin
XHisho.MailAlert.popServer: localhost

!!
!!時計のリソース
!!

XHisho.*clockFormat: %s時%s分%s秒
XHisho*clockArg:hMs
!XHisho.XHisho.FontSet: -*-fixed-medium-r-normal--16-*

!!
!!予定表示のリソース
!!

XHisho.OpenMessage.configFile: IXHISHODIR/aisatu.cfg
XHisho.OpenMessage.messageArg: hme
XHisho.OpenMessage.pastSchedColor: LightGrey
XHisho.OpenMessage.alertSchedColor: red
XHisho.OpenMessage.normalSchedColor: black
XHisho.OpenMessage.leaveTime: 5
XHisho.OpenMessage.scheduleDir: .Schedule/
!XHisho.OpenMessage.xcalendarCompatible: False

!!
!!スケジュールエディットのリソース
!!

XHisho.OpenMessage.scheduleSeparator: から
XHisho*inputMethod: kinput2

!!
!!メニューのリソース
!!

XHisho.Menu.itemHead: ○

!!
!!カレンダーのリソース
!!

XHisho.Calendar.prevButton: 先月
XHisho.Calendar.nextButton: 来月
XHisho.Calendar.existColor: Grey
XHisho.Calendar*SU.label: 日
XHisho.Calendar*MO.label: 月
XHisho.Calendar*TU.label: 火
XHisho.Calendar*WE.label: 水
XHisho.Calendar*TH.label: 木
XHisho.Calendar*FR.label: 金
XHisho.Calendar*SA.label: 土

!!
!!パラメータ変更のリソース
!!

XHisho.ResEdit.resEditLabel0: タイムアウト(秒)
XHisho.ResEdit.resEditLabel1: インターバル(秒)
XHisho.ResEdit.resEditLabel2: メッセージの長さ
XHisho.ResEdit.resEditLabel3: メッセージの行数
XHisho.ResEdit.resEditLabel4: focuswinインターバル(0.1秒)

!!
!! focusWinのリソース
!!

!XHisho*focusYoff: 50
!XHisho*justify: right

!!
!! サウンドのリソース
!!

XHisho.OpenMessage.scheduleAlertSound: XHISHODIR/schedule.wav
XHisho.MailAlert.newMailSound: XHISHODIR/newmail.wav
!XHisho*extSoundCommand: cat %s > /dev/audio

!!
!! 任意用ウインドウのリソース    
!!

XHisho*optionTimeout: 5
XHisho*optionWidth:300
XHisho*optionHeight:200

!!
!!About XAkari...
!!

XHisho.About.aboutLabel:\
\                 XAkari Ver.%s \n\
\n\
Copyright(c) 1998-2001  木村健一郎<Ken'ichirou Kimura>\n\
\n\
\n\
XAkariはあなたのデスクトップに常駐するあかりちゃんです。\n\
メールの到着や予定の時間を優しく教えてくれます :-)

