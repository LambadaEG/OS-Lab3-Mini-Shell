#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "command.h"
#include <time.h>

void insertlog(int pid) {
    int logFile = open("child_log.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (logFile == -1) {
        perror("open log file");
        return;
    }

    // Get the current time
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    
    // Format the timestamp
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    // Create the log entry with the timestamp and PID
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "[%s] Child process with PID %d terminated\n", timestamp, pid);
    
    // Write the log entry to the file
    write(logFile, logEntry, strlen(logEntry));
    close(logFile);
}

void handle_sigchld(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Write the log entry
            insertlog(pid);
    }

    // Close the log file

}

SimpleCommand::SimpleCommand() {
    _numberOfAvailableArguments = 5;
    _numberOfArguments = 0;
    _arguments = (char **) malloc(_numberOfAvailableArguments * sizeof(char *));
}

void SimpleCommand::insertArgument(char *argument) {
    if (_numberOfAvailableArguments == _numberOfArguments + 1) {
        _numberOfAvailableArguments *= 2;
        _arguments = (char **) realloc(_arguments, _numberOfAvailableArguments * sizeof(char *));
    }

    _arguments[_numberOfArguments] = argument;
    _arguments[_numberOfArguments + 1] = NULL;
    _numberOfArguments++;
}

Command::Command() {
    signal(SIGINT, SIG_IGN);
    signal(SIGCHLD, handle_sigchld);
    _numberOfAvailableSimpleCommands = 1;
    _simpleCommands = (SimpleCommand **) malloc(_numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    
    _numberOfSimpleCommands = 0;
    _outFile = nullptr;
    _inputFile = nullptr;
    _errFile = nullptr;
    _background = 0;
    _combineErrOut = 0; // Initialize this flag
    _appendOutput = 0;   // Initialize append output flag
}

void Command::insertSimpleCommand(SimpleCommand *simpleCommand) {
    if (_numberOfAvailableSimpleCommands == _numberOfSimpleCommands) {
        _numberOfAvailableSimpleCommands *= 2;
        _simpleCommands = (SimpleCommand **) realloc(_simpleCommands, _numberOfAvailableSimpleCommands * sizeof(SimpleCommand *));
    }

    _simpleCommands[_numberOfSimpleCommands] = simpleCommand;
    _numberOfSimpleCommands++;
}

void Command::clear() {
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
            free(_simpleCommands[i]->_arguments[j]);
        }
        free(_simpleCommands[i]->_arguments);
        free(_simpleCommands[i]);
    }

    if (_outFile) {
        free(_outFile);
    }
    if (_inputFile) {
        free(_inputFile);
    }

    _numberOfSimpleCommands = 0;
    _outFile = nullptr;
    _inputFile = nullptr;
    _errFile = nullptr;
    _background = 0;
    _combineErrOut = 0; // Reset flag
    _appendOutput = 0;   // Reset append flag
}

void Command::print() {
    printf("\n\n");
    printf("              COMMAND TABLE                \n");
    printf("\n");
    printf("  #   Simple Commands\n");
    printf("  --- ----------------------------------------------------------\n");
    
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        printf("  %-3d ", i);
        for (int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++) {
            printf("\"%s\" \t", _simpleCommands[i]->_arguments[j]);
        }
        printf("\n");
    }

    printf("\n\n");
    printf("  Output       Input        Error        Background\n");
    printf("  ------------ ------------ ------------ ------------\n");
    printf("  %-12s %-12s %-12s %-12s\n", 
           _outFile ? _outFile : "default",
           _inputFile ? _inputFile : "default",
           _errFile ? _errFile : "default",
           _background ? "YES" : "NO");
    printf("\n\n");
}

void Command::execute() {
    if (_numberOfSimpleCommands == 0) {
        prompt();
        return;
    }

    if (!strcasecmp(_simpleCommands[0]->_arguments[0], "exit"))
    {

        printf("Good bye!!\n");
        exit(0);
    }
    if (!strcasecmp(_simpleCommands[0]->_arguments[0], "cd"))
    {
        if(_simpleCommands[0]->_numberOfArguments==1){
            printf("Change directory to HOME\n");
            printf("Home directory: %s\n", getenv("HOME"));
            chdir(getenv("HOME"));
        }
        else if(_simpleCommands[0]->_arguments[1])
            chdir(_simpleCommands[0]->_arguments[1]);

        print();
        clear();
        prompt();
        return;
    }
    

    print();
    if (_numberOfSimpleCommands == 1) {
        executeSimpleCommand(_simpleCommands[0]);
    } else {
        executeWithPipe();
    }
    clear();
    prompt();

}

void Command::executeSimpleCommand(SimpleCommand *simpleCommand) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Handle output redirection
        if (_outFile) {
            int flags = _appendOutput ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
            int fd = open(_outFile, flags, 0666);
            if (fd == -1) {
                perror("open output file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Handle input redirection
        if (_inputFile) {
            int fd = open(_inputFile, O_RDONLY);
            if (fd == -1) {
                perror("open input file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle error output redirection
        if (_errFile) {
            int flags = _appendOutput ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
            int fd = open(_errFile, flags, 0666);
            if (fd == -1) {
                perror("open error file");
                exit(1);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        // Execute the command
        execvp(simpleCommand->_arguments[0], simpleCommand->_arguments);
        perror("execvp");
        exit(1);
    }

    // Parent process
    if (!_background) {
        waitpid(pid, NULL, 0);
        insertlog(pid);
    }
}

void Command::executeWithPipe() {
    int pipefd[2 * (_numberOfSimpleCommands - 1)];

    for (int i = 0; i < _numberOfSimpleCommands - 1; i++) {
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return;
        }

        if (pid == 0) {
            if (i > 0) {
                dup2(pipefd[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < _numberOfSimpleCommands - 1) {
                dup2(pipefd[i * 2 + 1], STDOUT_FILENO);
            }
            else {
                if (_outFile) {
            int flags = _appendOutput ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
            int fd = open(_outFile, flags, 0666);
            if (fd == -1) {
                perror("open output file");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Handle input redirection
        if (_inputFile) {
            int fd = open(_inputFile, O_RDONLY);
            if (fd == -1) {
                perror("open input file");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        // Handle error output redirection
        if (_errFile) {
            int fd = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("open error file");
                exit(1);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        }
            }

            for (int j = 0; j < 2 * (_numberOfSimpleCommands - 1); j++) {
                close(pipefd[j]);
            }

            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("execvp");
            exit(1);
        }
    }

    for (int i = 0; i < 2 * (_numberOfSimpleCommands - 1); i++) {
        close(pipefd[i]);
    }

    for (int i = 0; i < _numberOfSimpleCommands; i++) {
        if (!_background) {
            wait(NULL);
        }
    }
}

void Command::prompt() {
    printf("jimmy shell>");
    fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int main() {
    Command::_currentCommand.prompt();
    yyparse();
    return 0;
}
