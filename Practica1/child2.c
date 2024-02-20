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

    // time_t t;
    // srand(20);

    for (int i = 0; i < LINE_LENGTH; ++i) {
        buffer[i] = 'A' + rand() % 26; // Genera un carácter aleatorio entre 'A' y 'Z'
    }

    buffer[LINE_LENGTH] = '\n'; // Añade un carácter de nueva línea al final
    buffer[LINE_LENGTH + 1] = '\0'; // Añade el carácter nulo al final

    write(fd, buffer, LINE_LENGTH + 1);
}

void leerDesdeArchivo(int fd){
    // printf("leer\n");
    char buffer[LINE_LENGTH + 2];
    read(fd, buffer, LINE_LENGTH + 1);
    buffer[LINE_LENGTH] = '\0'; // Añade el carácter nulo al final
}

void reposicionarAlPrincipio(int fd){
    // printf("seek\n");
    lseek(fd, 0, SEEK_SET);
}

void reporte_llamadas(){
    int count_total = 0;
    int count_write = 0;
    int count_lseek = 0;
    int count_read = 0;

    // Abrir el archivo
    FILE *file = fopen("syscalls.log", "r");

    if (file == NULL) {
        perror("Error al abrir el archivo");
    }

    // Leer cada línea del archivo
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Buscar la llamada a la función
        if (strstr(line, "write") != NULL) {
            count_write++;
        } else if (strstr(line, "lseek") != NULL) {
            count_lseek++;
        } else if (strstr(line, "read") != NULL) {
            count_read++;
        }
    }

    count_total = count_write + count_lseek + count_read;

    // Cerrar el archivo
    fclose(file);

    // Imprimir los resultados
    printf("\n-----REPORTE-----\n");
    printf("Total llamadas de procesos hijos: %d\n", count_total);
    printf("Total llamadas a write: %d\n", count_write);
    printf("Total llamadas a lseek: %d\n", count_lseek);
    printf("Total llamadas a read: %d\n", count_read);

}

void manejador_sigint(){
    reporte_llamadas();
    // printf("Llamadas en total al sistema: %d.\n", llamadas_total);
    // printf("Llamadas read: %d.\n", llamadas_read);
    // printf("Llamadas write %d.\n", llamadas_write);
    // printf("Llamadas seek: %d.\n", llamadas_seek);
    exit(0);
}
 
int main(int argc, char *argv[]){

    time_t t;
    int fd = atoi(argv[1]);

    signal(SIGINT, manejador_sigint);

    srand(time(NULL));

    while(1){
                
        // printf("PID hijo2 %d.\n", getpid());

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
