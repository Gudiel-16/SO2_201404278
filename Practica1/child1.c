#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#define LINE_LENGTH 8

void escribirEnArchivo(int fd){
    // printf("escribir\n");
    char buffer[LINE_LENGTH + 2]; // +2 para el carácter nulo y el carácter de nueva línea al final

    // srand(time(NULL));
    for (int i = 0; i < LINE_LENGTH; ++i) {
        buffer[i] = 'A' + rand() % 26; // Genera un carácter aleatorio entre 'A' y 'Z'
    }

    buffer[LINE_LENGTH] = '\n'; // Añade un carácter de nueva línea al final
    buffer[LINE_LENGTH + 1] = '\0'; // Añade el carácter nulo al final

    write(fd, buffer, LINE_LENGTH + 1);
}

void leerDesdeArchivo(int fd){
    // printf("leer\n");
    char buffer[LINE_LENGTH + 1];
    read(fd, buffer, LINE_LENGTH);
    buffer[LINE_LENGTH] = '\0'; // Añade el carácter nulo al final
}

void reposicionarAlPrincipio(int fd){
    // printf("seek\n");
    lseek(fd, 0, SEEK_SET);
}

void manejador_sigint(){
    exit(0);
}
 
int main(int argc, char *argv[]){

    time_t t;
    int fd = atoi(argv[1]);

    // signal(SIGINT, manejador_sigint);

    // srand(time(NULL));
    while(1){
                
        // printf("PID hijo1 %d.\n", getpid());

        // srand(time(NULL));

        int llamada_a_realizar = 1 + rand() % 3;

        if(llamada_a_realizar == 1){
            escribirEnArchivo(fd);
        }else if(llamada_a_realizar == 2){
            leerDesdeArchivo(fd);
        }else {
            reposicionarAlPrincipio(fd);
        }

        // srand(time(NULL));

        // Tiempo aleatorio 1-3 segundos
        int segundos = 1 + rand() % 3;
        sleep(segundos);
    }

    return 0;

}
