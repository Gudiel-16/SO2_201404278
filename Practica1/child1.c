#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define FILENAME "practica1.txt"
#define LINE_LENGTH 8

void escribirEnArchivo(int fd){
    char buffer[LINE_LENGTH + 2]; // +2 para el carácter nulo y el carácter de nueva línea al final

    for (int i = 0; i < LINE_LENGTH; ++i) {
        buffer[i] = 'A' + rand() % 26; // Genera un carácter aleatorio entre 'A' y 'Z'
    }

    buffer[LINE_LENGTH] = '\n'; // Añade un carácter de nueva línea al final
    buffer[LINE_LENGTH + 1] = '\0'; // Añade el carácter nulo al final

    write(fd, buffer, LINE_LENGTH + 1);
}

void leerDesdeArchivo(int fd){
    char buffer[LINE_LENGTH + 1];
    read(fd, buffer, LINE_LENGTH);
    buffer[LINE_LENGTH] = '\0'; // Añade el carácter nulo al final
}

void reposicionarAlPrincipio(int fd){
    lseek(fd, 0, SEEK_SET);
}
 
// int para generar codigos de salida
int main(int argc, char *argv[]){
    printf ("\nSoy el Proceso hijo 1\n");

    int fd = atoi(argv[2]);

    srand(getpid());

    if(strcmp(argv[1], "write") == 0){
        escribirEnArchivo(fd);
    }else if(strcmp(argv[1], "read") == 0){
        leerDesdeArchivo(fd);
    }else {
        reposicionarAlPrincipio(fd);
    }

    /*Se leen los argmentos de argv[]*/
    printf ("Llamada1: %s\n", argv[1]);

}
