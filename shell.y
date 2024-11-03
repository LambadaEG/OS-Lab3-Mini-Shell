%union {
    char *string_val;
}

%token <string_val> WORD
%token PIPE
%token GREAT NEWLINE APPEND_COMBINE APPEND LESS
%token AMPERSAND 
%token APPEND_OVERWRITE




%{
extern "C"
{
    int yylex();
    void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%% 

goal:    
    commands
    ;

commands: 
    command
    | commands command 
    //| commands pipe_command
    ;

command: 
    simple_command
    ;

simple_command:    
    command_and_args redirect_sequence background_opt NEWLINE {
        printf("   Yacc: Execute command\n");
        Command::_currentCommand.execute();
    }
    |    
    command_and_args redirect_sequence background_opt PIPE {
        printf("   Yacc: Execute command pipe\n");
        //Command::_currentCommand.execute();
    }
    | NEWLINE 
    | error NEWLINE { yyerrok; }
    ;

command_and_args:
    command_word arg_list {
        Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
    }
    ;

arg_list:
    arg_list argument
    | /* can be empty */
    ;

argument:
    WORD {
        printf("   Yacc: insert argument \"%s\"\n", $1);
        Command::_currentSimpleCommand->insertArgument($1);
    }
    ;

command_word:
    WORD {
        printf("   Yacc: insert command \"%s\"\n", $1);
        Command::_currentSimpleCommand = new SimpleCommand();
        Command::_currentSimpleCommand->insertArgument($1);
    }
    ;

//pipe_command:
//    simple_command PIPE simple_command {
   //     // Create and setup piping for the commands
     //   printf("   Yacc: Piped command\n");
       // Command::_currentCommand.insertSimpleCommand(Command::_currentSimpleCommand);
        //Command::_currentCommand.executeWithPipe();
    //}
    //;

redirect_sequence:
    redirect_input redirect_sequence
    | redirect_output redirect_sequence
    | append_output redirect_sequence
    | append_combine redirect_sequence
    | append_overwrite redirect_sequence
    | /* empty */
    ;

redirect_input:
    LESS WORD {
        printf("   Yacc: redirect input from \"%s\"\n", $2);
        Command::_currentCommand._inputFile = $2;
    }
    ;

redirect_output:
    GREAT WORD {
        printf("   Yacc: redirect output to \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._appendOutput = false;
    }
    ;

append_output:
    APPEND WORD {
        printf("   Yacc: append output to \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._appendOutput = true;
    }
    ;

append_overwrite:
    APPEND_OVERWRITE WORD {
        printf("   Yacc: overwrite and redirect and output to \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._errFile = $2;
    }
    ;

append_combine:
    APPEND_COMBINE WORD {
        printf("   Yacc: append and redirect output to \"%s\"\n", $2);
        Command::_currentCommand._outFile = $2;
        Command::_currentCommand._errFile = $2;
        Command::_currentCommand._appendOutput = true;
    }
    ;

background_opt:
    AMPERSAND {
        printf("   Yacc: Running command in background\n");
        Command::_currentCommand._background = true;
    }
    | /* can be empty */ 
    ;


%% 

void yyerror(const char * s)
{
    fprintf(stderr, "%s\n", s);
}

#if 0
int main()
{
    yyparse();
}
#endif
