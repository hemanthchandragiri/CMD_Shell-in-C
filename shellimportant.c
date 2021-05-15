#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include<readline/readline.h>
#include<readline/history.h>
void init_shell() 
{ 
  
    printf("\n\n\n\n******************"
        "************************"); 
    printf("\n\n\n\t****MY SHELL****"); 

    printf("\n\n\n\n*******************"
        "***********************"); 
    char* username = getenv("USER");    
    printf("\n\n\nUSER is: \e[1;31m%s\e[0m",username); 
    printf("\n"); 

}
//global variables
int pipe_count=0, fd;
static char* args[512];
char *history_file;
char input_buffer[1024];
char *cmd_exec[100];
int flag, len;
char cwd[1024];
int flag_pipe=1;
pid_t pid;
int no_of_lines;
int environmment_flag;
int flag_pipe, flag_without_pipe,  output_redirection, input_redirection;
int cmd_flag;
int pid, status;
char history_data[1000][1000];
char current_directory[1000];
char ret_file[3000];
char his_var[2000];
char *input_redirection_file;
char *output_redirection_file;
extern char** environ;
char* input_buffer1;


static char* skip_space(char* s);

static int split(char *cmd_exec, int, int, int);

static int command(int, int, int, char *cmd_exec);

//for handling interrupt signals
void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fflush(stdout);
}
//sets all variables to 0....
void clear_variables()
{
  fd =0;
  flag=0;
  len=0;
  no_of_lines=0;
  pipe_count=0;
  flag_pipe=0;
  flag_without_pipe=0;
  output_redirection=0;
  input_redirection=0;
  input_buffer[0]='\0';
  cwd[0] = '\0';
  pid=0;
  environmment_flag=0;
  cmd_flag=0;
}
//used for history commands.....
  void hitory()
{

int j=where_history ();
int i;
register HIST_ENTRY **history;
history=history_list ();
if (history)
for (i = 0; history[i]; i++)
printf ("%d: %s\n", i + history_base, history[i]->line);
}
//history with constants
void get_history(char *b)
{
char *ptr = b; 
long valu;
int val = 0;
while (*ptr) { 
    if (isdigit(*ptr)) { 
         valu = strtol(ptr, &ptr, 10); 
        
    } else { 
        ptr++; 
    } 
    }
   val=val+valu;  
  
  int j=where_history ();
  int i;
  register HIST_ENTRY **history;

  history=history_list ();
  j=j+1;
  if(val>=j)
  {
  if (history)
            for (i = 0; history[i]; i++)
              printf ("%d: %s\n", i + history_base, history[i]->line);
              }
              else
              {
              if (history)
              for (i = j-1; val!=0; i--)
              {
              printf ("%d: %s\n", i + history_base, history[i]->line);
              val--;
              }
              }

}
///------------///
//responsible for processing the input
void execute()
{
  char cmd_val[1000];
  char *tokenise_cmd[100], *num_ch[10];
  int i, n=1, num, index=0;
  i=1;

  if(input_buffer[i]=='-')
    {
        n=1;
        num_ch[0]=strtok(input_buffer,"-");
        while ((num_ch[n]=strtok(NULL,"-"))!=NULL)
              n++;
        num_ch[n]=NULL;
        num = atoi(num_ch[1]);

        index = no_of_lines-num;
        strcpy(cmd_val, history_data[index]);
              
    }
  else 
    {
      num_ch[0]=strtok(input_buffer,"!");
      num = atoi(num_ch[0]);
      strcpy(cmd_val, history_data[num-1]);
    }
  tokenise_cmd[0]=strtok(cmd_val," ");
  while ((tokenise_cmd[n]=strtok(NULL,""))!=NULL)
              n++;
  tokenise_cmd[n]=NULL;
  strcpy(cmd_val, tokenise_cmd[1]);
  printf("%s\n", cmd_val );
  strcpy(input_buffer, cmd_val);

    
}
//---------environment variables for builtin commands------------------//
void environmment()
{
  int i =1, index=0;
  char env_val[1000], *value;
  while(args[1][i]!='\0')
              {
                   env_val[index]=args[1][i];
                   index++;
                    i++;
              }
  env_val[index]='\0';
  value=getenv(env_val);

  if(!value)
      printf("\n");
  else printf("%s\n", value);
}

void set_environment_variables()
{  
int n=1;
char *left_right[100];
if(args[1]==NULL)
      {
        char** env;
          for (env = environ; *env != 0; env++)
          {
            char* value = *env;
            printf("declare -x %s\n", value);    
          }  
         return; 
      }
left_right[0]=strtok(args[1],"=");
while ((left_right[n]=strtok(NULL,"="))!=NULL)
      n++;
left_right[n]=NULL;
setenv(left_right[0], left_right[1], 0);
}
//----------------------///////////////////////-----------//

 // a simple function for changing directory
void change_directory()
{
char *h="/home";   
if(args[1]==NULL)
        chdir(h);
else if ((strcmp(args[1], "~")==0) || (strcmp(args[1], "~/")==0))
        chdir(h);
else if(chdir(args[1])<0)
    printf("bash: cd: %s: No such file or directory\n", args[1]);

}

void parent_directory()
{
if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
         printf("%s\n", cwd );
        }
else
       perror("getcwd() error");


}

static char* skip_space(char* s)
{
  while (isspace(*s)) ++s;
  return s;
}
//-------tokenizing commands----------//
void tokenise_commands(char *com_exec)
{
int m=1;
args[0]=strtok(com_exec," ");       
while((args[m]=strtok(NULL," "))!=NULL)
        m++;
}
void tokenise_redirect_input_output(char *cmd_exec)
{
  char *io_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  io_token[0]=strtok(new_cmd_exec1,"<");       
  while((io_token[m]=strtok(NULL,">"))!=NULL)
        m++;
  io_token[1]=skip_space(io_token[1]);
  io_token[2]=skip_space(io_token[2]);
  input_redirection_file=strdup(io_token[1]);
  output_redirection_file=strdup(io_token[2]);
  tokenise_commands(io_token[0]);
  
}
void tokenise_redirect_input(char *cmd_exec)
{
  char *i_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  i_token[0]=strtok(new_cmd_exec1,"<");       
  while((i_token[m]=strtok(NULL,"<"))!=NULL)
        m++;
  i_token[1]=skip_space(i_token[1]);
  input_redirection_file=strdup(i_token[1]);
  tokenise_commands(i_token[0]);
}
void tokenise_redirect_output(char *cmd_exec)
{
  char *o_token[100];
  char *new_cmd_exec1;  
  new_cmd_exec1=strdup(cmd_exec);
  int m=1;
  o_token[0]=strtok(new_cmd_exec1,">");       
  while((o_token[m]=strtok(NULL,">"))!=NULL)
          m++;
  o_token[1]=skip_space(o_token[1]);
  output_redirection_file=strdup(o_token[1]); 
  tokenise_commands(o_token[0]);   
  
}
//----------------/////////////-----------------------///////////
char* skip_comma(char* str)
{
  int i=0, j=0;
  char temp[1000];
  while(str[i++]!='\0')
            {
              if(str[i-1]!='"')
                    temp[j++]=str[i-1];
            }
        temp[j]='\0';
        str = strdup(temp);
  
  return str;
}
//splits the command and sends to the command function
static int split(char *cmd_exec, int input, int first, int last)
{
    char *new_cmd_exec1;  
    new_cmd_exec1=strdup(cmd_exec);
   //else
      {
        int m=1;
        args[0]=strtok(cmd_exec," ");       
        while((args[m]=strtok(NULL," "))!=NULL)
              m++;
        args[m]=NULL;
        if (args[0] != NULL) 
            {

            if (strcmp(args[0], "exit") == 0) 
                    exit(0);
            if (strcmp(args[0], "echo") != 0) 
                    {
                      cmd_exec = skip_comma(new_cmd_exec1);
                      int m=1;
                      args[0]=strtok(cmd_exec," ");       
                      while((args[m]=strtok(NULL," "))!=NULL)
                                m++;
                      args[m]=NULL;

                    }
            if(strcmp("cd",args[0])==0)
                    {
                    change_directory();
                    return 1;
                    }
            else if(strcmp("pwd",args[0])==0)
                    {
                    parent_directory();
                    return 1;
                    }
           
            }
        }
    return command(input, first, last, new_cmd_exec1);
}

//if there is pipe requested this function will be useful
// this function is responsible for checking the command after the initial processing and passes the output to split()

void with_pipe_execute()
{

int i, n=1, input, first;

input=0;
first= 1;

cmd_exec[0]=strtok(input_buffer,"|");

while ((cmd_exec[n]=strtok(NULL,"|"))!=NULL)
      n++;
cmd_exec[n]=NULL;
pipe_count=n-1;
for(i=0; i<n-1; i++)
    {
      input = split(cmd_exec[i], input, first, 0);
      first=0;
    }
input=split(cmd_exec[i], input, first, 1);
input=0;
return;

}
//this is the important one which checks for all possibilities like internal,echo,redirection and indirection etc commands....

static int command(int input, int first, int last, char *cmd_exec)
{
  int mypipefd[2], ret, input_fd, output_fd;
  ret = pipe(mypipefd);
  if(ret == -1)
      {
        perror("pipe");
        return 1;
      }
  pid = fork();
 
  if (pid == 0) 
  {
    if (first==1 && last==0 && input==0) 
    {
      dup2( mypipefd[1], 1 );
    } 
    else if (first==0 && last==0 && input!=0) 
    {
      dup2(input, 0);
      dup2(mypipefd[1], 1);
    } 
    else 
    {
      dup2(input, 0);
    }
    if (strchr(cmd_exec, '<') && strchr(cmd_exec, '>')) 
            {
              input_redirection=1;
              output_redirection=1;
              tokenise_redirect_input_output(cmd_exec);
            }
   else if (strchr(cmd_exec, '<')) 
        {
          input_redirection=1;
          tokenise_redirect_input(cmd_exec);
        }
   else if (strchr(cmd_exec, '>')) 
        {
          output_redirection=1;
          tokenise_redirect_output(cmd_exec);
        }
        //redirection
    if(output_redirection == 1)
                {                    
                        output_fd= creat(output_redirection_file, 0644);
                        if (output_fd < 0)
                          {
                          fprintf(stderr, "failed to open %s for writing\n", output_redirection_file);
                          return(EXIT_FAILURE);
                          }
                        dup2(output_fd, 1);
                        close(output_fd);
                        output_redirection=0;
                }
                //indirection....
    if(input_redirection  == 1)
                  {
                         input_fd=open(input_redirection_file,O_RDONLY, 0);
                         if (input_fd < 0)
                          {
                          fprintf(stderr, "failed to open %s for reading\n", input_redirection_file);
                          return(EXIT_FAILURE);
                          }
                        dup2(input_fd, 0);
                        close(input_fd);
                        input_redirection=0;
                  }
                  //export, an builtin command
     if (strcmp(args[0], "export") == 0)
                  {
                  set_environment_variables();
                  return 1;
                  }
   if (strcmp(args[0], "history") == 0 && args[1]!=NULL)
             {
             get_history(args[1]);
              }
    else if (strcmp(args[0], "history") == 0)
             {
             hitory();
              } 

    else if(execvp(args[0], args)<0) printf("%s: command not found\n", args[0]);//execvp for executing commands
              exit(0);
  }
  else 
  {
     waitpid(pid, 0, 0);  
   }
 
  if (last == 1)
    close(mypipefd[0]);
  if (input != 0) 
    close(input);
  close(mypipefd[1]);
  return mypipefd[0];

}
// a prompt for interface......
void prompt()
{
char cwd[1024];
getcwd(cwd, sizeof(cwd));
printf("\e[1;32m\n@%s~$\e[0m", cwd);

}

int main()
{   
    int status;
    char ch[2]={"\n"};
    getcwd(current_directory, sizeof(current_directory));
    signal(SIGINT, sigintHandler);
    init_shell();//call for the greeting....
    while (1)
    {
      clear_variables();
      prompt();
      using_history ();
     input_buffer1 = readline(" ");//useful for storing the history
     strcpy(input_buffer,input_buffer1);
       len = strlen(input_buffer);
       if (strlen(input_buffer1)!= 0) {
	add_history(input_buffer1);}//for getting the history of commands using up and down arrows
	if (strlen(input_buffer1)== 0){
	  printf("pls give input!!!!\n");
   continue;}
      if(strcmp(input_buffer, ch)==0)
            {
              continue;
            }
      strcpy(his_var, input_buffer);
      if(strcmp(input_buffer, "exit") == 0) 
            {
              flag = 1;
              break;
            }
      with_pipe_execute();
      waitpid(pid,&status,0);
         
    }  
    if(flag==1)
      {
      printf("command shell terminating.....\n");
      exit(0);       
      return 0;
      }
return 0;
}

