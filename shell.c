#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int ms=1024;

struct Node{
    char *Data;
    int num;
    struct Node *Next;
};

struct Queue{
    struct Node *Head;
    struct Node *Last;
};

int cnt=0;
struct Queue History={NULL,NULL};

void Insertion(char *input){
    if(input[0]!='\0'){
        cnt++;
        struct Node *newNode=(struct Node *)malloc(sizeof(struct Node));
        newNode->Data=strdup(input);
        newNode->Next=NULL;
        newNode->num=cnt;
        if(History.Head==NULL){
            History.Head=History.Last=newNode;
        }
        else{
            History.Last->Next=newNode;
            History.Last=newNode;
        }
    }
}

void free_queue(struct Queue *queue) {
    struct Node *current=queue->Head;
    struct Node *next_node;

    while (current!=NULL) {
        next_node=current->Next;
        free(current->Data);        
        free(current);              
        current=next_node;        
    }
    queue->Head=NULL;
    queue->Last=NULL;
}


void RemoveSpaces(char *input) {
    int ind=0;
    int i=0;
    while(input[ind]==' '){
        ind++;
    }

    while(input[ind]!='\0'){
        input[i++]=input[ind++];
    }
    input[i]='\0';
}

void execution(char *myargs[]){
    int rc=fork();
        if(rc<0){
            fprintf(stderr,"Invalid Command\n");
        }
        else if(rc==0){
            execvp(myargs[0],myargs);
            fprintf(stderr,"Invalid Command\n");
            exit(1);
        }
        else{
            wait(NULL);
            if(strcmp(myargs[0],"cat")==0 && myargs[1]!=NULL){
                printf("\n");
            }
        }
}
void pipe_execution(char *input){
    char *cmd1=strtok(input,"|");
    char *cmd2=strtok(NULL,"|");
    if(cmd1==NULL || cmd2==NULL){
        fprintf(stderr,"Invalid Command\n");
        return;
    }

    char *args1[ms/2 +1];
    char *args2[ms/2 +1];

    char *token=strtok(cmd1," ");
    int i=0;
    while(token!=NULL){
        args1[i]=token;
        i++;
        token=strtok(NULL," ");
    }
    args1[i]=NULL;


    i=0;
    token=strtok(cmd2," ");
    while(token!=NULL){
        args2[i]=token;
        i++;
        token=strtok(NULL," ");
    }
    args2[i]=NULL;
    
    if(strcmp(args1[0],"history")==0 && args1[1]==NULL){
        int pipefd[2];
        if(pipe(pipefd)==-1){
            fprintf(stderr,"Invalid Command\n");
            exit(1);
        }
        int rc1=fork();

        if(rc1<0){
            fprintf(stderr,"Invalid Command\n");
            exit(1);
        }
        else if(rc1==0){
            dup2(pipefd[1],STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);
            struct Node *temp=History.Head;
            while(temp!=NULL){
                printf("%d %s\n",temp->num,temp->Data);
                temp=temp->Next;
            }
            exit(1);
        }
        else{
            wait(NULL);
            int rc2=fork();
            if (rc2<0){
                fprintf(stderr,"Invalid Command\n");
                exit(1);
            }
            else if(rc2==0){
                dup2(pipefd[0],STDIN_FILENO);
                close(pipefd[1]);
                close(pipefd[0]);
                execvp(args2[0],args2);
                fprintf(stderr,"Invalid Command\n");
                exit(1);
            }
            else{
                close(pipefd[0]);
                close(pipefd[1]);
                wait(NULL);
            }
        }
        return;
    }
    int pipefd[2];
    if(pipe(pipefd)==-1){
        fprintf(stderr,"Invalid Command\n");
        exit(1);
    }

    int rc1=fork();
    if (rc1<0) {
        fprintf(stderr,"Invalid Command\n");
        exit(1);
    }
    else if(rc1==0){
        dup2(pipefd[1],STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(args1[0],args1);
        fprintf(stderr,"Invalid Command\n");
        exit(1);
    }
    else{
        wait(NULL);
        int rc2=fork();
        if (rc2<0){
            fprintf(stderr,"Invalid Command\n");
            exit(1);
        }
        else if(rc2==0){
            dup2(pipefd[0],STDIN_FILENO);
            close(pipefd[1]);
            close(pipefd[0]);
            execvp(args2[0],args2);
            fprintf(stderr,"Invalid Command\n");
            exit(1);
        }
        else{
            close(pipefd[0]);
            close(pipefd[1]);
            wait(NULL);
        }

    }

}
int main(int argc,char *argv[]){
    while(1){
        char input[ms];
        printf("MTL458 > ");
        fgets(input,ms,stdin);
        input[strcspn(input, "\n")] = '\0';
        RemoveSpaces(input);
        Insertion(input);
        if(strstr(input,"|")!=NULL){
            pipe_execution(input);
            continue;
        }

        char *myargs[ms/2 + 1];
        char *token = strtok(input, " ");
        int i=0;
        while (token != NULL) {  
            myargs[i] = token;
            i++;
            token = strtok(NULL, " ");
        }
        myargs[i] = NULL;
        if(myargs[0]==NULL){
            continue;
        }
        else if(strcmp(myargs[0],"exit")==0){
            free_queue(&History);
            exit(0);
        }
        else if(strcmp(myargs[0],"history")==0 && myargs[1]==NULL){
            struct Node *temp=History.Head;
            while(temp!=NULL){
                printf("%d %s\n",temp->num,temp->Data);
                temp=temp->Next;
            }
        }
        else if(strcmp(myargs[0],"cd")==0){
            if((myargs[1]==NULL || strcmp(myargs[1],"~")==0 ) && myargs[2]==NULL){
                char *home=getenv("HOME");
                if(home!=NULL){
                    if(chdir(home)!=0){
                        fprintf(stderr,"Invalid Command\n");
                    }
                }
                else{
                    fprintf(stderr,"Invalid Command\n");
                }
            }
            else if(chdir(myargs[1])!=0){
                fprintf(stderr,"Invalid Command\n");
            }
        }
        else{
            execution(myargs);
        }
    }
    return 0;
}
