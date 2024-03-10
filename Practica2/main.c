#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define NAME_FILE_USERS "usuarios.csv"
#define NAME_FILE_ACCOUNT_STATUS "estado_de_cuenta.csv"

// cantidad de usuarios y errores que se pueden cargar
#define AMOUNT_USERS 100

pthread_mutex_t lock;

// para leer el archivo de usuarios
struct args_struct_read_users {
    FILE *file;
    int start_line;
    int end_line;
    char* name_hilo;
    int number_hilo;
};

// datos de usuario
struct data_users{
    int flag_read; // 1 = hay usuario en esa pocicion, 0 = ignorar
    int no_cuenta;
    char nombre[50];
    double saldo;
};

// Usuarios cargados por hilo
struct data_users_loaded{
    int count_hilo1;
    int count_hilo2;
    int count_hilo3;
    int total_hilos;
};

// Errores en carga de usuarios
struct data_users_errors{
    int flag_read; // 1 = no esta vacio y poner en reporte, 0 = ignorar
    char error_tipo[100];
};

// donde estaran guardados todos los usuarios
struct data_users all_users[AMOUNT_USERS];

// Almacenar el conteo por hilo
struct data_users_loaded count_users_loaded_per_hilo[1];

// Almacenar errores carga de usuarios
struct data_users_errors errors_load_users[AMOUNT_USERS];

int count_users(){

    FILE *fp;
    fp = fopen(NAME_FILE_USERS, "r"); // permisos de lectura

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

bool validate_account_number(char *token){

    int n = AMOUNT_USERS;
    int valor = atoi(token);

    for(int i = 0; i < n; i++){
        // if(all_users[i].flag_read == 1){
            if(all_users[i].no_cuenta == valor){
                return true;
            }
        // }        
    }

    return false;
}

bool validate_integer_positive_whole(char *token){

    double valor = strtod(token, NULL);
    char *punto = strchr(token, '.');

    if(punto != NULL){
        return true; // si contiene punto, es decimal
    }

    if(valor < 0){
        return true;
    }

    return false;
}

void generate_report_users(){
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Formatear el nombre del archivo
    char nombreArchivo[100];
    char fechaActual[100];
    strftime(nombreArchivo, sizeof(nombreArchivo), "carga_%Y_%m_%d-%H_%M_%S.log", timeinfo);
    strftime(fechaActual, sizeof(fechaActual), "%Y-%m-%d %H:%M:%S", timeinfo);

    FILE *fp;
    fp = fopen(nombreArchivo, "w");

    fprintf(fp, "------------- CARGA DE USUARIOS -------------\n\n");

    fprintf(fp, "FECHA: ");
    fprintf(fp, "%s\n\n", fechaActual);

    fprintf(fp, "USUARIOS CARGADOS:\n");
    fprintf(fp, "HIlo #1: %d\n", count_users_loaded_per_hilo[0].count_hilo1);
    fprintf(fp, "HIlo #2: %d\n", count_users_loaded_per_hilo[0].count_hilo2);
    fprintf(fp, "HIlo #3: %d\n", count_users_loaded_per_hilo[0].count_hilo3);
    fprintf(fp, "Total: %d\n\n", count_users_loaded_per_hilo[0].total_hilos);

    fprintf(fp, "ERRORES:\n");

    int n = AMOUNT_USERS;
    for(int i = 0; i < n; i++){
        if(errors_load_users[i].flag_read == 1){
            fprintf(fp, "%s", errors_load_users[i].error_tipo);
        }
    }

    fclose(fp);
}

void generate_report_account_status(){
    FILE *fp;
    fp = fopen(NAME_FILE_ACCOUNT_STATUS, "w");

    fprintf(fp, "no_cuenta,nombre,saldo\n");

    int n = AMOUNT_USERS;
    for(int i = 0; i < n; i++){
        if(all_users[i].flag_read == 1){
            fprintf(fp, "%d,%s,%f\n", all_users[i].no_cuenta, all_users[i].nombre, all_users[i].saldo);
        }
    }

    fclose(fp);

}

void* thread_read_users(void* arg) 
{ 
    //wait, bloquea todo lo que le sigue (algo como todos los procesos que le siguen), hasta que el mismo proceso lo libere
    pthread_mutex_lock(&lock);

    struct args_struct_read_users *hilo_current = (struct args_struct_read_users*)arg;
    char row[1000]; // buffer de linea
    char *token; // no se sabe cuantos caracteres, sera puntero de caracteres

    // printf("-----------------------Inicio - %s\n", hilo_current->name_hilo); 

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

        if(validate_integer_positive_whole(token)){
            errors_load_users[n].flag_read = 1;
            sprintf(errors_load_users[n].error_tipo, "    - Linea #%d: Numero de cuenta no es un numero entero positivo -> %s\n", (n+2), token);
            n++;
            continue;
        }

        if(validate_account_number(token)){
            errors_load_users[n].flag_read = 1;
            sprintf(errors_load_users[n].error_tipo, "    - Linea #%d: Numeros de cuenta duplicados -> %s\n", (n+2), token);
            n++;
            continue;
        }

        all_users[n].no_cuenta = atoi(token);

        token = strtok(NULL, ","); // null permite leer desde el punto donde se quedo anterior
        // printf("Nombre: %s  \n", token);
        sprintf(all_users[n].nombre, "%s", token); // copiar cadena

        token = strtok(NULL, ",");
        all_users[n].saldo = strtod(token, NULL); // convierte a double

        all_users[n].flag_read = 1;

        // aumenta el conteo segun hilo que se esta ejecutando
        if(hilo_current->number_hilo == 1){
            count_users_loaded_per_hilo[0].count_hilo1++;
        } else if(hilo_current->number_hilo == 2){
            count_users_loaded_per_hilo[0].count_hilo2++;
        } else{
            count_users_loaded_per_hilo[0].count_hilo3++;
        }

        count_users_loaded_per_hilo[0].total_hilos++;
        n++;
    }    
  
    // sleep(1); 
      
    //signal 
    // printf("-----------------------Termino - %s\n", hilo_current->name_hilo); 

    pthread_mutex_unlock(&lock); // se libere, y se ejecuta el siguiente hilo
} 

void read_users(int count_total, int count_hilo1_y_2){

    pthread_mutex_init(&lock, NULL);  // Inicializamos nuestro mutex

    FILE *fp;
    fp = fopen(NAME_FILE_USERS, "r"); // permisos de lectura

    pthread_t t1,t2,t3;

    struct args_struct_read_users args_hilo1;
    args_hilo1.file = fp;
    args_hilo1.start_line = 0;
    args_hilo1.end_line = count_hilo1_y_2 - 1;
    args_hilo1.name_hilo = "Hilo1";
    args_hilo1.number_hilo = 1;
    pthread_create(&t1, NULL, thread_read_users, (void *)&args_hilo1); 

    struct args_struct_read_users args_hilo2;
    args_hilo2.file = fp;
    args_hilo2.start_line = count_hilo1_y_2;
    args_hilo2.end_line = (count_hilo1_y_2 * 2) - 1 ;
    args_hilo2.name_hilo = "Hilo2";
    args_hilo2.number_hilo = 2;
    pthread_create(&t2, NULL, thread_read_users, (void *)&args_hilo2); 

    struct args_struct_read_users args_hilo3;
    args_hilo3.file = fp;
    args_hilo3.start_line = (count_hilo1_y_2 * 2);
    args_hilo3.end_line = count_total - 1 ;
    args_hilo3.name_hilo = "Hilo3";
    args_hilo3.number_hilo = 3;
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
    // int count_hilo_three = count_total - (count_hilo_one_and_two * 2);

    // printf("Cantidad: %d\n\n", count_total);
    // printf("Cantidad1: %d\n\n", count_hilo_one_and_two);
    // printf("Cantidad2: %d\n\n", count_hilo_one_and_two);
    // printf("Cantidad3: %d\n\n", count_hilo_three);

    read_users(count_total, count_hilo_one_and_two);
    generate_report_users();
    generate_report_account_status();

    return 0;
}

// gcc main.c -lpthread -o main