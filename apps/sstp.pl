#!/usr/local/bin/perl
use Socket;

my ($host, $port, $sockaddr);
$| = 1;
$mesg = 'Hello world.';
$port = 11000;

$sockaddr = pack_sockaddr_in($port, INADDR_ANY);
$proto = getprotobyname('tcp'); 

socket(SOCKET, PF_INET, SOCK_STREAM, $proto)
  || die "socket error.\n";

setsockopt(SOCKET, SOL_SOCKET, SO_REUSEADDR, 1)
  || die "setsockopt error.\n";
bind(SOCKET, $sockaddr) || die "bind error.\n";

listen(SOCKET, SOMAXCONN) || die "listen error.\n";

while (1) {
  accept(CLIENT, SOCKET);

  while(<CLIENT> ){
     last if ($_ eq "\r\n");
     $in = $_;
     $in =~ s/\r//g;
     if($in =~ /Script: /){
       print $';
     }
  }
  print CLIENT "200 OK\r\n";
  close(CLIENT);
}
