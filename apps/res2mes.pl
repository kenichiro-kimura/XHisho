#!/usr/local/bin/perl
#
# $B5l%P!<%8%g%s$N%j%=!<%9$rFI$_9~$_!"$=$3$+$i%a%C%;!<%8%G!<%?$N$_$rH4$-(B
# $B=P$7$F(BMessage$B%U%!%$%k$K=q$/!#$=$l0J30$N%j%=!<%9@_Dj$O(BXHisho.new$B$K=q(B
# $B$-=P$9!#(B
#



$old_file = "XHisho";
$new_file = "XHisho.new";
$message_file = "Messages";

open(IN,"$old_file") or die("Can't open:$old_file\n");

open(OUT,">$new_file") or die("Can't open:$new_file\n");
open(MES,">$message_file") or die("Can't open:$message_file\n");

print OUT "XHisho.messageFile:\n";

while(<IN>){
    if($_ =~ /mailLabel:/){
	printf(MES "newmail%s", $');
    } elsif($_ =~ /noMailLabel:/){
	printf(MES "nomail%s", $');
    } elsif($_ =~ /openMessageF:/){
	printf(MES "open1%s", $');
    } elsif($_ =~ /openMessageL:/){
	printf(MES "open2%s",$');
    } elsif($_ =~ /openMessageN:/){
	printf(MES "open3%s",$');
    } elsif($_ =~ /alertMessageF:/){
	printf(MES "alert1%s",$');
    } elsif($_ =~ /alertMessageL:/){
	printf(MES "alert2%s",$');
    } elsif($_ =~ /messageFormat:/){
	printf(MES "alertformat%s",$');
    } elsif($_ =~ /scheduleEditMessage:/){
	printf(MES "schedule%s",$');
    } elsif($_ =~ /menuLabel:/){
	printf(MES "menul%s",$');
    } elsif($_ =~ /menuItem0:/){
	printf(MES "menu0%s",$');
    } elsif($_ =~ /menuItem1:/){
	printf(MES "menu1%s",$');
    } elsif($_ =~ /menuItem2:/){
	printf(MES "menu2%s",$');
    } elsif($_ =~ /menuItem3:/){
	printf(MES "menu3%s",$');
    } elsif($_ =~ /menuItem4:/){
	printf(MES "menu4%s",$');
    } elsif($_ =~ /menuItem5:/){
	printf(MES "menu5%s",$');
    } elsif($_ =~ /calendarLabel:/){
	printf(MES "calendar%s",$');
    } elsif($_ =~ /resEditLabel:/){
	printf(MES "resource%s",$');
    } elsif($_ =~/weeklyEditMessage/){
	#
    } else {
	print OUT $_ ;
    }
}

close(IN);
close(OUT);
close(MES);


