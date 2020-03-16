//defining the built-in functions
#define CD "cd"
#define CLR "clr"
#define DR "dir"
#define ENVIRON "environ"
#define ECHO "echo"
#define PATH "path"
#define HELP "help"
#define PAUSE "pause"
#define QUIT "quit"

//global variables
char path[50][50];
char pwd[100];
char shellpath[100];
int pathsize = 1;

/*
  This struct holds the parsed command line.
  out, out1, in, and pipe:
    These variables hold the location (in args) of the argument
    corresponding to the respective command.
    For example, if the input is cmd cmdarg1 > outfile:
      args = {cmd, cmdarg1, outfile}
    int i = 0;                //  out = 2;
    pipe holds the location of the second command
*/
typedef struct{
  int builtin;            //1 if the command is built-in
  char args[50][50];      //hold the arguments for a command
  int argcount;
  int out;                // >
  int out1;               // >>
  int in;                 // <
  int pipe;               //amount of pipes
  int backexec;           //1 if the command should be executed in the background
}command;

void error();
void initstruct(command *);
void parse(char *, command *);
void handlebuiltin(command);
void cd(command);
void dir(command);
void environ(command);
void changepath(command);
void echo(command);
void help(command);
void pauseshell(command);
void handleother(command);
void handlepipe(char args[50][50], int, int);
char * checkpath(char args[50][50], int);
