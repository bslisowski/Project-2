/*
  Brendan Lisowski
  CIS 3207 - Project 2
  02/26/2020
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

//built-in command functions
//other command function

//defining the built-in functions
#define CD "cd"
#define CLR "clr"
#define DIR "dir"
#define ENVIRON "environ"
#define ECHO "echo"
#define PATH "path"
#define ECHO "echo"
#define HELP "help"
#define PAUSE "pause"
#define QUIT "quit"

//global variables
char path[50][50];
char pwd[100];

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
  char args[50][20];      //hold the arguments for a command
  int argcount;
  int out;                // >
  int out1;               // >>
  int in;                 // <
  int pipe;               // |
  int backexec;           //1 if the command should be executed in the background
}command;

void error();
void initstruct(command *);
void parse(char *, command *);

int main(int argc, char *argv[]){

  //initializing path and pwd
  strcpy(path[0], "/bin");
  strcpy(path[1], "\0");
  getcwd(pwd, 100);

  //defining and initializing the command struct
  command cmd;
  initstruct(&cmd);

  //checking for batchmode
  int batchmode;              //1 if batchmode, 0 for interactive batchmode
  FILE *fp;
  if (argc == 2){
    batchmode = 1;
    fp = fopen(argv[1], "r");
    if (fp == NULL){
      printf("file cannot be opened");
      exit(1);
    }
  }
  else if (argc > 2){
    printf("too many arguments");
    exit(1);
  }
  else batchmode = 0;

  size_t size = 100;
  //allocating memory for a char array to hold the user inputs
  char *line = (char *)malloc(size*sizeof(char));

  while(1){
    printf("myshell> ");

    if (batchmode){
      if (getline(&line, &size, fp) == -1){
        break;
      }
    }
    else getline(&line, &size, stdin);

    parse(line, &cmd);
    printf("argcount = %d\n", cmd.argcount);
    //handle parsed command

    initstruct(&cmd);
  }

  fclose(fp);
  free(line);
}

//function for errors;
void error(){
  char *error_msg = "an error has occured";
  write(STDERR_FILENO, error_msg, strlen(error_msg));
}
//function to set the struct's int variables to 0
void initstruct(command *cmd){
  cmd->builtin = 0;
  cmd->argcount = 0;
  cmd->out = 0;
  cmd->out1 = 0;
  cmd->in = 0;
  cmd->pipe = 0;
  cmd->backexec = 0;
}
/*
  Function for parsing the command line input

  Arguments:
    char *line - holds the command line input to be parsed
  Return:
    void
*/
void parse(char *line, command *cmd){

  const char *delims = " \n\t";
  char *buffer = strtok(line, delims);        //buffer to hold tokens

  //checking if the command is builtin -- try to improve
  if (strcmp(buffer, CD) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, CLR) == 0){
    printf("\033[H\033[J");
    return;
  }
  else if (strcmp(buffer, DIR) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, ENVIRON) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, PATH) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, ECHO) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, HELP) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, PAUSE) == 0){
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, QUIT) == 0){
    exit(0);
  }
  else{
    cmd->builtin = 0;
  }
  //storing the command in the argument array
  strcpy(cmd->args[0], buffer);

  int i = 1;
  /*
    loop to parse the rest of the input
  */
  while((buffer = strtok(NULL, delims)) != NULL){

    switch (buffer[0]){
      case '>':
        if (buffer[1] == '>'){
          cmd->out1 = i;
        }
        else cmd->out = i;
        break;
      case '<':
        cmd->in = i-1;
        break;
      case '|':
        cmd->pipe = i;
        break;
      case '&':
        cmd->backexec = 1;
        break;
      case '\t':
        break;
      case '\n':
        break;
      default:
        strcpy(cmd->args[i++], buffer);
        cmd->argcount++;
    }
  }
  free(buffer);
}
