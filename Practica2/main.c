#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>

pthread_mutex_t lock;

struct args_struct_read_users {
    FILE *file;
    int start_line;
    int end_line;
    char* name_hilo;
};

struct data_users{
    int no_cuenta;
    char nombre[50];
    double saldo;
};

struct data_users all_users[100];

int count_users(){

    FILE *fp;
    fp = fopen("usuarios.csv", "r"); // permisos de lectura

    char row[1000]; // buffer de linea

    // mueve el puntero a cada inicio de linea
    fgets(row, 1000, fp); // permite leer linea por linea un archivo

    int n = 0;

    // feof es para saber cuando llegamos al final del archivo
    while (feof(fp) != true)
    {
        fgets(row, 1000, fp); // ya tenemos la primera linea

        n++;
    }

    fclose(fp);

    return n;

}

void* thread_read_users(void* arg) 
{ 
    //wait, bloquea todo lo que le sigue (algo como todos los procesos que le siguen), hasta que el mismo proceso lo libere
    pthread_mutex_lock(&lock);

    struct args_struct_read_users *hilo_current = (struct args_struct_read_users*)arg;
    char row[1000]; // buffer de linea
    char *token; // no se sabe cuantos caracteres, sera puntero de caracteres

    printf("-----------------------Inicio - %s\n", hilo_current->name_hilo); 

    // puntero al inicio
    fseek(hilo_current->file, 0, SEEK_SET);

    // recorremos hasta la linea que le toca
    for (int i = 0; i <= hilo_current->start_line; i++)
    {
        fgets(row, 1000, hilo_current->file); // permite leer linea por linea un archivo
    }

    int n = hilo_current->start_line;
    while (n <= hilo_current->end_line && feof(hilo_current->file) != true){
        fgets(row, 1000, hilo_current->file); // ya tenemos la primera linea

        token = strtok(row, ","); // permite leer una cadena hasta cierto caracter
        all_users[n].no_cuenta = atoi(token);

        token = strtok(NULL, ","); // null permite leer desde el punto donde se quedo anterior
        // printf("Nombre: %s  \n", token);
        sprintf(all_users[n].nombre, "%s", token); // copiar cadena

        token = strtok(NULL, ",");
        all_users[n].saldo = strtod(token, NULL); // convierte a double

        n++;
    }    
  
    sleep(1); 
      
    //signal 
    printf("-----------------------Termino - %s\n", hilo_current->name_hilo); 

    pthread_mutex_unlock(&lock); // se libere, y se ejecuta el siguiente hilo
} 

void read_users(int count_total, int count_hilo1_y_2){

    pthread_mutex_init(&lock, NULL);  // Inicializamos nuestro mutex

    FILE *fp;
    fp = fopen("usuarios.csv", "r"); // permisos de lectura

    pthread_t t1,t2,t3;

    struct args_struct_read_users args_hilo1;
    args_hilo1.file = fp;
    args_hilo1.start_line = 0;
    args_hilo1.end_line = count_hilo1_y_2 - 1;
    args_hilo1.name_hilo = "Hilo1";
    pthread_create(&t1, NULL, thread_read_users, (void *)&args_hilo1); 

    struct args_struct_read_users args_hilo2;
    args_hilo2.file = fp;
    args_hilo2.start_line = count_hilo1_y_2 - 1;
    args_hilo2.end_line = (count_hilo1_y_2 * 2) - 1 ;
    args_hilo2.name_hilo = "Hilo2";
    pthread_create(&t2, NULL, thread_read_users, (void *)&args_hilo2); 

    struct args_struct_read_users args_hilo3;
    args_hilo3.file = fp;
    args_hilo3.start_line = (count_hilo1_y_2 * 2) - 1;
    args_hilo3.end_line = count_total - 1 ;
    args_hilo3.name_hilo = "Hilo3";
    pthread_create(&t3, NULL, thread_read_users, (void *)&args_hilo3); 

    // pthread_create(&t1, NULL, thread_read_users, "Hilo1"); 
    // pthread_create(&t2, NULL, thread_read_users, "Hilo2"); 
    // pthread_create(&t3, NULL, thread_read_users, "Hilo3"); 
    
    pthread_join(t1,NULL); 
    pthread_join(t2,NULL); 
    pthread_join(t3,NULL); 

    fclose(fp);
    pthread_mutex_destroy(&lock);  // Liberamos los recursos del semaforo
    
}



int main(){

    // cantidad de usuarios
    int count_total = count_users();

    // usuarios por hilo
    int count_hilo_one_and_two = count_total / 3;
    int count_hilo_three = count_total - (count_hilo_one_and_two * 2);

    // printf("Cantidad: %d\n\n", count_total);
    // printf("Cantidad1: %d\n\n", count_hilo_one_and_two);
    // printf("Cantidad2: %d\n\n", count_hilo_one_and_two);
    // printf("Cantidad3: %d\n\n", count_hilo_three);

    read_users(count_total, count_hilo_one_and_two);

    int n = 100;
    for(int i = 0; i < n; i++){
        printf("No_cuenta: %d   Nombre: %s   Saldo: %f   \n", 
            all_users[i].no_cuenta, all_users[i].nombre, all_users[i].saldo);
    }

    return 0;
}