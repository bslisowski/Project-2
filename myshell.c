/*
  Brendan Lisowski
  CIS 3207 - Project 2
  03/14/2020
*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

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

int main(int argc, char *argv[]){

  //initializing path and pwd
  strcpy(path[0], "/bin");
  strcpy(path[1], "\0");
  getcwd(pwd, 100);
  getcwd(shellpath, 100);
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
    else {
      fflush(stdin);
      fflush(stdin);
      getline(&line, &size, stdin);
    }

    if (strcmp(line, "\n") != 0){
      parse(line, &cmd);
      //handle parsed command
      if(cmd.builtin == -1){
        error();
      }
      else if (cmd.builtin){
        handlebuiltin(cmd);
      }
      else {
        handleother(cmd);
      }
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
  strncpy(cmd->args[0], buffer, 50);

  int i = 1;
  /*
    loop to parse the rest of the input
  */
  while((buffer = strtok(NULL, delims)) != NULL){

    switch (buffer[0]){
      case '>':
        if (buffer[0] == '>'){
          cmd->out1 = i;
          if (buffer[1] != '\0'){
            cmd->builtin = -1;
          }
        }
        else{
          cmd->out = i;
          if (buffer[2] != '\0'){
            cmd->builtin = -1;
          }
        }
        break;
      case '<':
        cmd->in = i;
        if (buffer[1] != '\0'){
          cmd->builtin = -1;
          return;
        }
        break;
      case '|':
        cmd->pipe++;
        strncpy(cmd->args[i++], buffer, 50);
        cmd->argcount++;
        if (buffer[1] != '\0'){
          cmd->builtin = -1;
          return;
        }
        break;
      case '&':
        cmd->backexec = 1;
        if (buffer[1] != '\0'){
          cmd->builtin = -1;
          return;
        }
        break;
      case '\t':
        break;
      case '\n':
        break;
      default:
        strncpy(cmd->args[i++], buffer, 50);
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
    changepath(cmd);
  }
  else if (strcmp(cmd.args[0], ECHO) == 0){
    echo(cmd);
  }
  else if (strcmp(cmd.args[0], HELP) == 0){
    help(cmd);
  }
  else if (strcmp(cmd.args[0], PAUSE) == 0){
    pauseshell(cmd);
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
      fprintf(file, "%s\n", read->d_name);
      read = readdir(dir);
    }
    fclose(file);
  }
  else if (cmd.out1){
    FILE *file = fopen(cmd.args[cmd.out1], "a");

    while (read != NULL){
      fprintf(file, "%s\n", read->d_name);
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
*/
void environ(command cmd){

  if (cmd.argcount > 1){
    error();
  }
  else if (cmd.argcount == 1){
    FILE *fp;
    //redirection
    if (cmd.out){
      //truncate
      fp = fopen(cmd.args[cmd.out], "w");
    }
    else{
      //append
      fp = fopen(cmd.args[cmd.out1], "a");
    }

    if (fp == NULL){
      error();
    }
    fprintf(fp, "PATH=%s", path[0]);
    int i = 1;
    while(i < pathsize){
      fprintf(fp, ":%s", path[i++]);
    }
    fprintf(fp, "\nshell=%s/myshell\n", shellpath);
    fprintf(fp, "PWD=%s\n", pwd);
    fprintf(fp, "USER=%s\n", getenv("USER"));
    fprintf(fp, "HOME=%s\n", getenv("HOME"));
    fclose(fp);
  }
  else{
    //no redirection
    printf("PATH=%s", path[0]);
    int i = 1;
    while(i < pathsize){
      printf(":%s", path[i++]);
    }
    printf("\nshell=%s/myshell\n", shellpath);
    printf("PWD=%s\n", pwd);
    printf("USER=%s\n", getenv("USER"));
    printf("HOME=%s\n", getenv("HOME"));
  }
}

/*
  function for user to change the path
*/
void changepath(command cmd){
  //if no arguments are passed, the path is set to NULL
  if (cmd.argcount == 0){
    strcpy(path[0], "\0");
    strcpy(path[1], "\0");
    return;
  }
  printf("argcount = %d\n", cmd.argcount);

  int i = 1;
  /*
    this loop uses opendir() to check if the arguments given are
    existing directories
  */
  while (i <= cmd.argcount){
    DIR *dir = opendir(cmd.args[i]);
    if (dir == NULL){
      error();
      return;
    }
    strncpy(path[i-1], cmd.args[i], 50);
    i++;
    pathsize++;
    closedir(dir);
  }
  strcpy(path[i-1], "\0");
}

/*
  function for the echo command
  can be used with output redirection
*/
void echo(command cmd){
  if (cmd.argcount == 0 || cmd.argcount > 2){
    error();
    return;
  }

  if (cmd.out || cmd.out1){
    FILE *fp;
    if (cmd.out){
      fp = fopen(cmd.args[cmd.out], "w");
    }
    else{
      fp = fopen(cmd.args[cmd.out1], "a");
    }

    if (fp == NULL){
      error();
      return;
    }
    fprintf(fp, "%s\n", cmd.args[1]);
    fclose(fp);
  }
  else {
    printf("%s\n", cmd.args[1]);
  }
}

/*
  function for the help command
  prints out the manual
  can be used with output redirection
*/
void help(command cmd){
  //the only argument allowed is redirection
  if (cmd.argcount > 1){
    error();
    return;
  }

  char *line = NULL;
  size_t size = 100;
  FILE *manual = fopen("readme", "r");
  if (manual == NULL){
    error();
    return;
  }
  getline(&line, &size, manual);

  if (cmd.out || cmd.out1){
    FILE *fp;
    if (cmd.out){
      fp = fopen(cmd.args[cmd.out], "w");
    }
    else{
      fp = fopen(cmd.args[cmd.out1], "a");
    }

    if (fp == NULL){
      error();
      return;
    }
    fprintf(fp, "%s", line);
    while(getline(&line, &size, manual) != -1){
      fprintf(fp, "%s", line);
    }
    fclose(fp);
  }
  else{
    printf("%s", line);
    while(getline(&line, &size, manual) != -1){
      printf("%s", line);
    }
  }
  fclose(manual);
}

/*
  function for the pause command
  waits for the user to press enter only
*/
void pauseshell(command cmd){
  if (cmd.argcount){
    error();
    return;
  }
  printf("press enter to continue\n");
  char c;
  int i;
  /*
    This loop checks the user input with getchar.
    If anything other than enter is input, the loop will continue.
    The loop will exit and unpause the shell when only enter is pressed.
  */
  do {
    fflush(stdin);
    i = 0;
    while ((c = getchar()) != '\n'){
      i++;
    }
  } while (i);
}

/*
  function to handle other commands
*/
void handleother(command cmd){

  //if the user erased the path, only builtin commands can be used
  if (strcmp(path[0], "\0") == 0){
    error();
    return;
  }
  if (cmd.pipe > 0){
    handlepipe(cmd.args, cmd.argcount, cmd.pipe);
    return;
  }
  char *checkpath = (char *)malloc(100);
  int i = 0;
  /*
    This loop checks the environment paths for the command the user wants
    to run. The pathfound variable is set to 1 if the command is found in one
    of the path directories.
  */
  int pathfound = 0;
  while(i < pathsize){
    strcat(checkpath, path[i]);
    strcat(checkpath, cmd.args[0]);
    if (access(checkpath, X_OK) == 0){
      pathfound = 1;
      break;
    }
    else{
      strncpy(checkpath, "", 100);
      i++;
    }
  }

  /*
    If the command is not found in the path, this will check
    the current directory.
  */
  if (!pathfound){
    strncpy(checkpath, "", 100);
    strcat(checkpath, cmd.args[0]);

    if(access(checkpath, X_OK) != 0){
      error();
      return;
    }
  }

  //this is for any arguments needed for the new process
  char **arguments = (char **)calloc(100, sizeof(char *));
  i = 0;
  while (i < 100){
    arguments[i] = (char *)calloc(100, sizeof(char));
    i++;
  }

  i = 1;
  int j = 0;
  if (cmd.argcount){
    while(i <= cmd.argcount){
      if (i == cmd.out || i == cmd.out1 || i == cmd.in){
        break;
      }
      strncpy(arguments[j], cmd.args[i], 100);
      i++;
      j++;
    }
    strcpy(arguments[i-1], "\0");
  }
  else{
    strcpy(arguments[0], "\0");
  }


  //creating the new process
  pid_t pid = fork();

  if (pid == -1){
    error();
    free(checkpath);
    free(arguments);
    return;
  }
  else if (pid == 0){
    //Changing the child process' file descriptor for redirection
    if (cmd.in){
      int fd = open(cmd.args[cmd.in], O_RDONLY, S_IRUSR);
      if (fd < 0){
        error();
        exit(1);
      }
      if (dup2(fd, 0) < 0){
        error();
        exit(1);
      }
    }
    if (cmd.out){
      int fd = open(cmd.args[cmd.out], O_WRONLY | O_CREAT, S_IWUSR);
      if (fd < 0){
        error();
        exit(1);
      }
      if (dup2(fd, 1) < 0){
        error();
        exit(1);
      }
    }
    else if (cmd.out1){
      int fd = open(cmd.args[cmd.out1], O_WRONLY | O_APPEND, S_IWUSR);
      if (fd < 0){
        error();
        exit(1);
      }
      if (dup2(fd, 1) < 0){
        error();
        exit(1);
      }
    }
    if (cmd.argcount){
      execv(checkpath, arguments);
      error();
    }
    else{
      execv(checkpath, NULL);
      error();
    }
    free(checkpath);
    free(arguments);
    exit(1);
  }
  else{
    if (cmd.backexec == 0){
      wait(&pid);
    }
  }
  free(checkpath);
  free(arguments);
  i = 0;
  while (i < 100){
    free(arguments[i]);
    i++;
  }
}

/*
  Function for piping commands.
  This function only needs the parsed command and the number
  of pipes (cmd.pipe);
*/
void handlepipe(char args[50][50], int argc, int n){

  if (!n){
    error();
    return;
  }

  //3D char array to hold each commands' arguments
  char arguments[n+1][50][50];

  int i = 0, j = 0, k = 0;
  int index = 0;
  //This loop takes the string of commands and arguments and places them in the proper arrays
  while (index <= argc){
    if (strcmp("|", args[i]) == 0){
      if (strcmp("\0", args[i+1]) == 0){
        error();
        return;
      }
      i++;
      j++;
      k = 0;
    }
    else{
      strncpy(arguments[j][k], args[i], 50);
      i++;
      k++;
    }
    index++;
  }


  i = 0;
  int nextread;         //integer for the
  pid_t pid;
  int fd[2];

  pipe(fd);             //creating the first pipe

  pid = fork();         //creating the first child process

  if (pid == -1){
    error();
    return;
  }

  if (pid == 0){
    //closing the stdout file descriptor
    close(1);
    //redirecting stdout to the write end of the first pipe
    if (dup2(fd[1], 1) == -1){
      error();
      return;
    }
    //closing the other connections to the pipe
    close(fd[0]);
    close(fd[1]);
    //executing the first command
    if (execv(arguments[0][0], NULL) == -1){
      error();
      exit(1);
    }
  }

  while (i < n){
    //Saving the stdin file desctriptor before creating the next pipe
    nextread = fd[0];

    pipe(fd);

    //creating the next child process
    pid = fork();

    if (pid == -1){
      error();
      return;
    }
    //changing the next childs file descriptors and calling execv()
    if (pid == 0){
      close(0);
      if (dup2(nextread, 0) == -1){
        error();
        return;
      }
      close(fd[0]);

      if (dup2(fd[1], 1) == -1){
        error();
        return;
      }
      close(fd[1]);

      if (execv(arguments[i][0], NULL) == -1){
        error();
        exit(1);
      }
    }

    i++;
  }

  //creating the last child process
  pid = fork();
  if (pid == -1){
    error();
    return;
  }

  if (pid == 0){
    close(0);
    if (dup2(fd[0], 0) == -1){
      error();
      return;
    }
    close(fd[0]);
    close(fd[1]);

    if (execv(arguments[i][0], NULL) == -1){
      error();
      exit(1);
    }
  }
  close(fd[0]);
  close(fd[1]);

  //Parent process waits for all of the other processes to finish
  pid_t waitret;
  int status = 0;
  while ((waitret = wait(&status)) > 0);
}
