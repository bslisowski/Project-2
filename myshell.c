/*
  Brendan Lisowski
  CIS 3207 - Project 2
  02/26/2020
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

//built-in command functions
//other command function

//defining the built-in functions
#define CD "cd"
#define CLR "clr"
#define DR "dir"
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
void handlebuiltin(command);
void cd(command);
void dir(command);
//void environ(command);
//void path(command);
//void echo(command);
//void help(command);
//void pause(command);

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

    //handle parsed command
    if (cmd.builtin){
      handlebuiltin(cmd);
    }

    initstruct(&cmd);
  }

  fclose(fp);
  free(line);
}

//function for errors;
void error(){
  char *error_msg = "an error has occured\n";
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
    cmd->builtin = 1;
  }
  else if (strcmp(buffer, DR) == 0){
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
        else{
          cmd->out = i;
        }
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
/*
  function to handle built-in commands
  contains only if/else statements to handle the different commands
*/
void handlebuiltin(command cmd){

  if (strcmp(cmd.args[0], CD) == 0){
    //cd can have at most one argument
    if (cmd.argcount > 1){
      error();
    }
    else{
        cd(cmd);
    }
  }
  else if (strcmp(cmd.args[0], CLR) == 0){
    //clr is handled here - no seperate function needed
    if (cmd.argcount > 0){
      error();
    }
    else{
      printf("\033[H\033[J");
    }
    return;
  }
  else if (strcmp(cmd.args[0], DR) == 0){
    dir(cmd);
  }
  else if (strcmp(cmd.args[0], ENVIRON) == 0){
    environ(cmd);
  }
  else if (strcmp(cmd.args[0], PATH) == 0){
    cmd.builtin = 1;
  }
  else if (strcmp(cmd.args[0], ECHO) == 0){
    cmd.builtin = 1;
  }
  else if (strcmp(cmd.args[0], HELP) == 0){
    cmd.builtin = 1;
  }
  else if (strcmp(cmd.args[0], PAUSE) == 0){
    cmd.builtin = 1;
  }
  else if (strcmp(cmd.args[0], QUIT) == 0){
    //exit is handled here - no seperate function needed
    if (cmd.argcount > 0){
      error();
    }
    else{
      exit(0);
    }
  }
  else{
    error();
  }
}

/*
  function for the change directory command
  if there are no arguments, the pwd is printed
  else chdir() is called. on success, the new pwd is printed
  otherwise the error() function is called
*/
void cd(command cmd){

  if (cmd.argcount == 0){
    printf("%s\n", pwd);
  }
  else {
    if (chdir(cmd.args[1]) == -1){
      error();
    }
    else {
      getcwd(pwd, 100);
      printf("%s\n", pwd);
    }
  }
}

/*
  function for the directory command
  can be used with output redirection
*/
void dir(command cmd){

  DIR *dir;
  struct dirent *read;
  if (cmd.argcount > 2){
    error();
    return;
  }
  //opening the the current directory if no argument given
  if (cmd.argcount == 0){
    dir = opendir(pwd);
  }
  else if ((cmd.argcount == 1)&&((cmd.out)||(cmd.out1))){
    dir = opendir(pwd);
  }
  else{
  //opening the given directory
    dir = opendir(cmd.args[1]);
    if (dir == NULL){
      error();
      return;
    }
  }

  read = readdir(dir);

  /*
    Checking to see if redirrection is needed and printing the
    contents of the directory appropriately
  */
  if (cmd.out){
    FILE *file = fopen(cmd.args[cmd.out], "w");

    while (read != NULL){
      fprintf(file, read->d_name);
      fprintf(file, "\n");
      read = readdir(dir);
    }
    fclose(file);
  }
  else if (cmd.out1){
    FILE *file = fopen(cmd.args[cmd.out1], "a");

    while (read != NULL){
      fprintf(file, read->d_name);
      fprintf(file, "\n");
      read = readdir(dir);
    }
    fclose(file);
  }
  else {
      while (read != NULL){
        printf("%s\n", read->d_name);
        read = readdir(dir);
      }
  }
  closedir(dir);
}

/*
  function for the environ command
  can be used with output redirection

void environ(command cmd){

  if (cmd.argcount > 1){
    error();
  }
  else if (cmd.argcount == 1){
    //redirection
    if (cmd.out){
      //truncate
    }
    else{
      //append
    }
  }
  else{
    //no redirection
    printf("PATH=%s", path[0]);
    int i = 1;
    while(path[i] != "\0"){
      printf(":%s", path[i++]);
    }
    printf("\nPWD=%s\n", pwd);
    printf("USER=%s\n", getenv("USER"));
    printf("HOME=%s\n", getenv("HOME"));
  }
}
*/

/*
  void path(command cmd)
*/
/*
  void echo(command cmd)
*/
/*
  void help(command cmd)
*/
/*
  void pause(command cmd)
*/

/*
  function to handle other commands

void handleother(command *cmd){

}*/
