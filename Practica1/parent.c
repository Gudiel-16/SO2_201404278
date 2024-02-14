#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

void manejador_sigint(){
    printf("\nPrograma terminado por señal SIGINT (Ctrl + C)\n");
    exit(0);
}

int main(){

    // Manejador SIGINT
    signal(SIGINT, manejador_sigint);

    int fd = open("practica1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);

    pid_t pid1 = fork(); //Se realiza el fork
    pid_t pid2 = fork(); //Se realiza el fork

    while(1){

        // si da negativo es un error
        if(pid1 == -1){
            perror("fork1");
            exit(1);
        }
        
        if(pid2 == -1){
            perror("fork2");
            exit(1);
        }
        
        if(pid1 == 0){ // si da 0 es porque se esta realizando el proceso hijo

            /*Se escriben los argumentos para el proceso hijo*/
            char *arg_Ptr[4];
            arg_Ptr[0] = " child1.c"; // nombre de ejecutable

            int llamada_a_realizar = 1 + rand() % 3;

            if(llamada_a_realizar == 1){
                arg_Ptr[1] = "write"; // solo se pueden pasar cadenas o caracteres
            }else if(llamada_a_realizar == 2){
                arg_Ptr[1] = "read";
            }else {
                arg_Ptr[1] = "seek";
            }

            char file_to_string[2];
            sprintf(file_to_string, "%d", fd);
            
            arg_Ptr[2] = file_to_string;
            arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

            printf("Mi PID es %d.\n", getpid());

            /*Se ejecuta el ejecutable del proceso hijo*/
            execv("/home/gudiel/USAC/SO2/SO2_201404278/Practica1/child1.bin", arg_Ptr);
        
        }else{ // padre
            printf("Soy el proceso padre\n");
            printf("Mi PID es %d.\n", pid1);

            /*Se espera a que el proceso hijo termine*/
            int status;
            wait(&status);
        }

        if(pid2 == 0){ // si da 0 es porque se esta realizando el proceso hijo

            /*Se escriben los argumentos para el proceso hijo*/
            char *arg_Ptr[4];
            arg_Ptr[0] = " child2.c"; // nombre de ejecutable

            int llamada_a_realizar = 1 + rand() % 3;

            if(llamada_a_realizar == 1){
                arg_Ptr[1] = "write"; // solo se pueden pasar cadenas o caracteres
            }else if(llamada_a_realizar == 2){
                arg_Ptr[1] = "read";
            }else {
                arg_Ptr[1] = "seek";
            }

            char file_to_string[2];
            sprintf(file_to_string, "%d", fd);
            
            arg_Ptr[3] = NULL; // El ultimo indice de argv siempre debe de ser NULL

            printf("Mi PID2 es %d.\n", getpid());

            /*Se ejecuta el ejecutable del proceso hijo*/
            execv("/home/gudiel/USAC/SO2/SO2_201404278/Practica1/child2.bin", arg_Ptr);
        
        }else{ // padre
            // printf("Soy el proceso padre\n");

            /*Se espera a que el proceso hijo termine*/
            int status;
            wait(&status);
        }

        // Tiempo aleatorio 1-3 segundos
        int segundos = 1 + rand() % 3;
        printf("%s %d\n", "seg: ", segundos);
        sleep(segundos);

    }
    

    return 0;
}

// gcc parent.c -o parent
// ./parent

// gcc child1.c -o child1.bin
// gcc child1.c -o child2.bin