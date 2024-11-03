
/*
 *
 * CS-413 Spring 98
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include "y.tab.h"

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t]+ 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}
"<" 	{
		return LESS;
 	}
"&>"	{
		return APPEND_OVERWRITE;
	}

"&>>"	{
		return APPEND_COMBINE;
	}	

"|"	{
		return PIPE;
	}
">>" 	{
		return APPEND;
	}	
"&"	{
		return AMPERSAND;
	}

[^ \t\n&><|]+  { 
	/* Support for Arguments - Match any sequence without special characters */
	yylval.string_val = strdup(yytext); return WORD; 
	}



.   { 
		return NOTOKEN; 
	} 

%%

int yywrap() {
    return 1;
}	
