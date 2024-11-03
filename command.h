#ifndef command_h
#define command_h

// Command Data Structure
struct SimpleCommand {
    // Available space for arguments currently preallocated
    int _numberOfAvailableArguments;

    // Number of arguments
    int _numberOfArguments;
    char **_arguments;

    SimpleCommand();
    void insertArgument(char *argument);
};

struct Command {
    int _numberOfAvailableSimpleCommands; // Space allocated for simple commands
    int _numberOfSimpleCommands;           // Count of simple commands
    SimpleCommand **_simpleCommands;       // Array of simple commands
    char *_outFile;                        // Output redirection file
    char *_inputFile;                      // Input redirection file
    char *_errFile;                        // Error redirection file
    int _background;                       // Background execution flag
    int _append;                           // Append mode flag (1 if append mode, 0 otherwise)
    int _combineErrOut;                    // Flag for combined output and error redirection
    bool _appendOutput;                    // Flag to indicate if output should be appended (true/false)

    void prompt();                         // Display shell prompt
    void print();                          // Print command details
    void execute();                        // Execute the command
    void executeWithPipe();                // Execute commands with pipe
    void executeSimpleCommand(SimpleCommand *simpleCommand); // Execute a simple command
    void clear();                          // Clear command data

    Command();
    void insertSimpleCommand(SimpleCommand *simpleCommand); // Insert a simple command

    static Command _currentCommand;       // Current command instance
    static SimpleCommand *_currentSimpleCommand; // Current simple command instance
};

#endif
