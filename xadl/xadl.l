%{
#include "y.tab.h"
extern int lines;
%}
%%
[ \t]+ /* no action */
[\n] {lines++;}
"loop" {yylval.val = (char*)strdup(yytext);return LP;}
"display" {yylval.val = (char*)strdup(yytext);return DP;}
"goto" {yylval.val = (char*)strdup(yytext);return GT;}
"mail" {yylval.val = (char*)strdup(yytext);return ML;}
"schedule" {yylval.val = (char*)strdup(yytext);return SC;}
[0-9]+ {yylval.val = (char*)strdup(yytext);return NUMBER;}
[[:alnum:]\./][[:alnum:]\._/]* {yylval.val = (char*)strdup(yytext);return LABEL;}
#.+ {}
.  return *yytext;

