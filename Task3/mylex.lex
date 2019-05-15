%{
#define YYSTYPE char *
#include <stdio.h>
#include <string.h>
#include "mybison.tab.h"
%}

%%
var					return VAR;
begin				return BEGIN_PROGRAM;
end\.				return END_PROGRAM;
end					return END_CYCLE;
read				return READ;
readln				return READLN;
write				return WRITE;
writeln				return WRITELN;
integer|longint		return INTEGER;
smallint			return SHORT;
int64				return LONG_LONG;
boolean				return BOOL;
word				return UNSIGNED_SHORT;
longword|cardinal 	return UNSIGNED_INT;
uint64				return UNSIGNED_LONG_LONG;
if					return CYCLE_IF;
then				return CYCLE_THEN;
else				return CYCLE_ELSE;
for					return CYCLE_FOR;
to					return CYCLE_TO;
downto				return CYCLE_DOWNTO;
do					return CYCLE_DO;
while 				return CYCLE_WHILE;
repeat				return CYCLE_REPEAT;
until				return CYCLE_UNTIL;
and					return LOGIC_AND;
or					return LOGIC_OR;
xor					return LOGIC_XOR;
true				return VALUE_TRUE;
false				return VALUE_FALSE;
:					return COLON;
=					return EQUAL;
\<>					return UNEQUAL;
>					return MORE;
\< 					return LESS;
,        			return COMMA;
\;        	 		return SEMICOLON;
\+					return PLUS;
-					return MINUS;
\*					return MUL;
div					return DIV;
mod					return MOD;
\(        			return LPAREN;
\)         			return RPAREN;
[a-z][a-z0-9]*  	yylval=strdup(yytext); return VALUE;
[0-9]*				yylval=strdup(yytext); return NUMBER;
[ \t\r\n]			;
<<EOF>>         	return ENDOFFILE;
%%