#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int main(){

    // archivo
    int fd = open("practica1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);

    // proceso hijo1
    pid_t pid1 = fork(); //Se realiza el fork
        
    if(pid1 == 0){ // si da 0 es porque se esta realizando el proceso hijo

        // printf("PID hijo1 %d.\n", getpid());

        /*Se escriben los argumentos para el proceso hijo*/
        char *arg_Ptr[3];
        arg_Ptr[0] = "child1.c"; // nombre de ejecutable

        // pasando el archivo por un argumento
        char file_to_string[2];
        sprintf(file_to_string, "%d", fd);
        
        arg_Ptr[1] = file_to_string;
        arg_Ptr[2] = NULL; // El ultimo indice de argv siempre debe de ser NULL

        execv("/home/gudiel/USAC/SO2/SO2_201404278/Practica1/child1.bin", arg_Ptr);

    }else{ // padre

        // proceso hijo2
        pid_t pid2 = fork();

        if(pid2 == 0){

            // printf("PID hijo2 %d.\n", getpid());

            /*Se escriben los argumentos para el proceso hijo*/
            char *arg_Ptr[3];
            arg_Ptr[0] = "child2.c"; // nombre de ejecutable

            // pasando el archivo por un argumento
            char file_to_string[2];
            sprintf(file_to_string, "%d", fd);
            
            arg_Ptr[1] = file_to_string;
            arg_Ptr[2] = NULL; // El ultimo indice de argv siempre debe de ser NULL

            execv("/home/gudiel/USAC/SO2/SO2_201404278/Practica1/child2.bin", arg_Ptr);


        }else{
            int status;
            waitpid(pid1, &status, WNOHANG);
            waitpid(pid2, &status, WNOHANG);

            // comando systemtap
            char command[100];
            sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > syscalls.log");
            system(command);

            // printf("siuuuu\n");
            // printf("-- %d\n",pid1);
            // printf("-- %d\n",pid2);
            // printf("siuuuu\n");
        }

    }

    while(1){
        sleep(60);
    }

    return 0;
    
}

// gcc parent.c -o parent
// ./parent

// gcc child1.c -o child1.bin
// gcc child1.c -o child2.bin