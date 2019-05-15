%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define YYSTYPE char *
 
void yyerror(char *s) {
      fprintf (stderr, "%s\n", s);
    }
 
int yywrap() {
        return 1;
} 

int read_num_count = 0;
int write_num_count = 0;

int main() {
        yyparse();
} 

%}

%token VAR BEGIN_PROGRAM END_CYCLE END_PROGRAM READ READLN WRITE WRITELN INTEGER SHORT UNSIGNED_SHORT UNSIGNED_INT LONG_LONG UNSIGNED_LONG_LONG BOOL CYCLE_IF CYCLE_THEN CYCLE_ELSE CYCLE_FOR CYCLE_TO CYCLE_DOWNTO CYCLE_DO CYCLE_WHILE CYCLE_REPEAT CYCLE_UNTIL LOGIC_AND LOGIC_OR LOGIC_XOR VALUE_TRUE VALUE_FALSE COLON EQUAL UNEQUAL MORE LESS COMMA SEMICOLON PLUS MINUS MUL DIV MOD LPAREN RPAREN VALUE NUMBER ENDOFFILE
%start input
%%
input:
	program ENDOFFILE {
		char start_line[41] = "#include <stdio.h>\n#include <stdbool.h>\n";
		printf("%s\n%s\n", start_line, $1);
		exit(0);
	}
	;
program:
	variable_descryption BEGIN_PROGRAM program_code END_PROGRAM {
		char s[strlen($1)+strlen($3)+25];
		if (strlen($1)==0) {
			sprintf(s, "%sint main() {\n%s%s\n}", $1, $3, "return 0;");
		} else {
			sprintf(s, "%s\nint main() {\n%s%s\n}", $1, $3, "return 0;");
		}
		$$=strdup(s);
	}
	;
variable_descryption:
	VAR descryption {
		$$=strdup($2);
	}
	| {
		$$=strdup("");
	}
	;
descryption:
	variables COLON variable_type SEMICOLON descryption {
		char s[strlen($1)+strlen($3)+strlen($5)+3];
		sprintf(s, "%s %s;\n%s", $3, $1, $5);
		$$=strdup(s);
	}
	| {
		$$=strdup("");
	}
	;
variable_type:
	INTEGER {
		$$=strdup("int");
	}
	|
	SHORT {
		$$=strdup("short");
	}
	|
	LONG_LONG {
		$$=strdup("long long");
	}	
	|
	BOOL{
		$$=strdup("bool");
	}
	|
	UNSIGNED_SHORT {
		$$=strdup("unsigned short");
	}
	|
	UNSIGNED_INT {
		$$=strdup("unsigned int");
	}
	|
	UNSIGNED_LONG_LONG {
		$$=strdup("unsigned long long");
	}
	;
variables:
	VALUE {
		$$=strdup($1);
	}
	|
	VALUE COMMA variables {
		char s[strlen($1)+strlen($3)+2];
		sprintf(s, "%s, %s", $1, $3);
		$$=strdup(s);
	}
	| {
		$$=strdup("");
	}
	;
program_code:
	command program_code {
		char s[strlen($1)+strlen($2)];
		sprintf(s, "%s\n%s", $1, $2);
		$$=strdup(s);
	}
	| {
		$$ = strdup("");
	}
	;
command:
	VALUE COLON EQUAL continue_action SEMICOLON {
		char s[strlen($1)+strlen($4)+5];
		sprintf(s, "%s = %s;", $1, $4);
		$$=strdup(s);
	}
	|
	read_symbol LPAREN read_line RPAREN SEMICOLON {
		char scnf_line[read_num_count * 2 + 2];
		for (int i = 0; i < read_num_count * 2; i++) {
			scnf_line[i] = '\0';
		}
		for (int i = 0; i < read_num_count; i++) {
			sprintf(scnf_line, "%s%s", scnf_line, "%d");
		}
		read_num_count = 0;
		if (strlen($1) == 1) sprintf(scnf_line, "%s%s", scnf_line, "\\n");
		char s[strlen($3)+strlen(scnf_line)+15];
		sprintf(s, "scanf(\"%s\", %s);", scnf_line, $3);
		$$=strdup(s);
	}
	|
	write_symbol LPAREN write_line RPAREN SEMICOLON {
		char prnt_line[write_num_count * 2 + 2];
		for (int i = 0; i < write_num_count * 2; i++) {
			prnt_line[i] = '\0';
		}
		for (int i = 0; i < write_num_count; i++) {
			sprintf(prnt_line, "%s%s", prnt_line, "%d");
		}
		write_num_count = 0;
		char s[strlen($3)+strlen(prnt_line)+20];
		if (strlen($3) == 0) {
			sprintf(s, "printf(\"\");");
		} else if (strlen($1) == 1) {
			sprintf(s, "printf(\"%s\\n\", %s);", prnt_line, $3);
		} else {
			sprintf(s, "printf(\"%s\", %s);", prnt_line, $3);
		}
		$$=strdup(s);
	}
	|
	if_realization {
		$$=strdup($1);
	}
	|
	CYCLE_FOR VALUE COLON EQUAL continue_action to_downto continue_action CYCLE_DO BEGIN_PROGRAM program_code END_CYCLE SEMICOLON {
		char s[strlen($2)*3+strlen($5)+strlen($7)+strlen($10)+25];
		char sign[3];
		char step[3];
		for (int i = 0; i < 3; i++) {
			sign[i] = '\0';
			step[i] = '\0';
		}
		if (strlen($6) == 0) {
			sprintf(sign, "%s", "<=");
			sprintf(step, "%s", "++");
		} else {
			sprintf(sign, "%s", ">=");
			sprintf(step, "%s", "--");
		}
		sprintf(s, "for (%s = %s; %s %s %s; %s%s) {\n%s}", $2, $5, $2, sign, $7, $2, step, $10);
		$$=strdup(s);
	}
	|
	CYCLE_WHILE condition CYCLE_DO BEGIN_PROGRAM program_code END_CYCLE SEMICOLON {
		char s[strlen($2)+strlen($5)+15];
		sprintf(s, "while (%s) {\n%s}", $2, $5);
		$$=strdup(s);
	}
	|
	CYCLE_REPEAT program_code CYCLE_UNTIL condition SEMICOLON {
		char s[strlen($2)+strlen($4)+25];
		sprintf(s, "do {\n%s} while (!(%s));", $2, $4);
		$$=strdup(s);
	}
	;
to_downto:
	CYCLE_TO {
		$$=strdup("");
	}
	|
	CYCLE_DOWNTO {
		$$=strdup("_");
	}
	;
if_realization:
	CYCLE_IF condition CYCLE_THEN BEGIN_PROGRAM program_code END_CYCLE SEMICOLON {
		char s[strlen($2)+strlen($5)+15];
		sprintf(s, "if (%s) {\n%s}", $2, $5);
		$$=strdup(s);
	}
	|
	CYCLE_IF condition CYCLE_THEN BEGIN_PROGRAM program_code END_CYCLE CYCLE_ELSE else_if {
		char s[strlen($2)+strlen($5)+strlen($8)+25];
		sprintf(s, "if (%s) {\n%s} else %s", $2, $5, $8);
		$$=strdup(s);
	}
	;
else_if:
	BEGIN_PROGRAM program_code END_CYCLE SEMICOLON {
		char s[strlen($2)+4];
		sprintf(s, "{\n%s}", $2);
		$$=strdup(s);
	}
	|
	if_realization {
		$$=strdup($1);
	}
	;
condition:
	inside_condition logic_op condition {
		char s[strlen($1)+strlen($2)+strlen($3)+2];
		sprintf(s, "%s %s %s", $1, $2, $3);
		$$=strdup(s);
	}
	|
	inside_condition {
		$$=strdup($1);
	}
	;
inside_condition:
	equality {
		$$=strdup($1);
	}
	|
	LPAREN condition RPAREN {
	char s[strlen($2)+2];
		sprintf(s, "(%s)", $2);
		$$=strdup(s);
	}
	;
logic_op:
	LOGIC_AND {
		$$=strdup("&&");
	}
	|
	LOGIC_OR {
		$$=strdup("||");
	}
	|
	LOGIC_XOR {
		$$=strdup("^");
	}
	;
equality:
	continue_action compare_symbol continue_action {
		char s[strlen($1)+strlen($2)+strlen($3)+2];
		sprintf(s, "%s %s %s", $1, $2, $3);
		$$=strdup(s);
	}
	;
compare_symbol:
	EQUAL {
		$$=strdup("==");
	}
	|
	UNEQUAL {
		$$=strdup("!=");
	}
	|
	MORE {
		$$=strdup(">");
	}
	|
	LESS {
		$$=strdup("<");
	}
	;
read_symbol:
	READ {
		$$=strdup("");
	}
	|
	READLN {
		$$=strdup("_");
	}
	;
write_symbol:
	WRITE {
		$$=strdup("");
	}
	|
	WRITELN {
		$$=strdup("_");
	}
	;
continue_action:
	paren_num action continue_action {
		char s[strlen($1)+strlen($2)+strlen($3)+2];
		sprintf(s, "%s %s %s", $1, $2, $3);
		$$=strdup(s);
	}
	| 
	paren_num {
		$$=strdup($1);
	}
	;
paren_num: 
	negate_number {
		$$=strdup($1);
	}
	|
	negate_value {
		$$=strdup($1);
	}
	|
	VALUE_TRUE {
		$$=strdup("true");
	}
	|
	VALUE_FALSE {
		$$=strdup("false");
	}
	|
	LPAREN continue_action RPAREN {
		char s[strlen($2)+2];
		sprintf(s, "(%s)", $2);
		$$=strdup(s);
	}
	;
read_line:
	VALUE {
		read_num_count++;
		char s[strlen($1)+1];
		sprintf(s, "&%s", $1);
		$$=strdup(s);
	}
	|
	VALUE COMMA read_line {
		read_num_count++;
		char s[strlen($1)+strlen($3)+2];
		sprintf(s, "&%s, %s", $1, $3);
		$$=strdup(s);
	}
	| {
		$$=strdup("");
	}
	;
write_line:
	continue_action {
		write_num_count++;
		$$=strdup($1);
	}
	| 
	continue_action COMMA write_line {
		write_num_count++;
		char s[strlen($1)+strlen($3)+2];
		sprintf(s, "%s, %s", $1, $3);
		$$=strdup(s);
	}
	|{ 
		$$=strdup("");
	}
	;
negate_value:
	VALUE {
		$$=strdup($1);
	}
	|
	MINUS VALUE {
		char s[strlen($2)+1];
		sprintf(s, "-%s", $2);
		$$=strdup(s);
	}
	;
negate_number:
	NUMBER {
		$$=strdup($1);
	}
	|
	MINUS NUMBER {
		char s[strlen($2)+1];
		sprintf(s, "-%s", $2);
		$$=strdup(s);
	}
	;
action:
	PLUS {
		$$=strdup("+");
	}
	|
	MINUS {
		$$=strdup("-");
	}
	|
	MUL {
		$$=strdup("*");
	}
	|
	DIV {
		$$=strdup("/");
	}
	|
	MOD {
		$$=strdup("%");
	}
	;
%%