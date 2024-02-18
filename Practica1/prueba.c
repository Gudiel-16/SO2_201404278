#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

int llamadas_hijo1 = 0;
int llamadas_hijo2 = 0;

void manejador_hijo1(){
    printf("\nPrograma terminado por señal SIGINT (Ctrl + C)\n");
    printf("Llamadas hijo1 %d.\n", llamadas_hijo1);
    exit(0);
}

void manejador_hijo2(){
    printf("\nPrograma terminado por señal SIGINT (Ctrl + C)\n");
    printf("Llamadas hijo2 %d.\n", llamadas_hijo2);
    exit(0);
}

int main(){

    int fd = open("practica1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);

    pid_t pid1 = fork(); //Se realiza el fork
    // pid_t pid2 = fork(); //Se realiza el fork
        
    if(pid1 == 0){ // si da 0 es porque se esta realizando el proceso hijo

        signal(SIGINT, manejador_hijo1);
        while(1){
            printf("PID hijo1 %d.\n", getpid());
            // Tiempo aleatorio 1-3 segundos
            int segundos = 1 + rand() % 3;
            // printf("%s %d\n", "seg: ", segundos);
            sleep(segundos);
            llamadas_hijo1++;
        }

    }else{ // padre
        // printf("PID padre %d.\n", pid1);

        pid_t pid2 = fork();
        if(pid2 == 0){ // si da 0 es porque se esta realizando el proceso hijo

            signal(SIGINT, manejador_hijo2);
            while(1){
                
                printf("PID hijo2 %d.\n", getpid());
                // Tiempo aleatorio 1-3 segundos
                int segundos = 1 + rand() % 3;
                // printf("%s %d\n", "seg: ", segundos);
                sleep(segundos);
                llamadas_hijo2++;
            }

        }

        /*Se espera a que el proceso hijo termine*/
        // int status;
        // wait(&status);
    }

    while(1){
        sleep(60);
        printf("s");
    }

    return 0;
    
}

// gcc parent.c -o parent
// ./parent

// gcc child1.c -o child1.bin
// gcc child1.c -o child2.bin