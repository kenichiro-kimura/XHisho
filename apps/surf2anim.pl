#!/usr/local/bin/perl

@files = `find ./ -type f -name "surface*.png" -print`;
chomp(@files);

$max = 0;

foreach(@files){
    if($_ =~ /(.*)(surface)([0-9]*)(.png)/){
	$max = $3 if($3 >= $max);
    }
}

print "XHisho Animation File\n";

for($i = 0; $i <= $max;$i++){
    $filename = "surface" . $i . ".png";
    if( -f $filename){
	print $filename . "\n";
    } else {
	print "GOTO 1\n";
    }
}

