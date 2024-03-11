#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define NAME_FILE_USERS "usuarios.csv"
#define NAME_FILE_OPERATIONS "operaciones.csv"
#define NAME_FILE_ACCOUNT_STATUS "estado_de_cuenta.csv"

// cantidad de usuarios y errores que se pueden cargar
#define AMOUNT_USERS 100

// cantidad de operaciones y errores que se pueden cargar
#define AMOUNT_OPERATIONS 100

pthread_mutex_t lock; // para usuarios
pthread_mutex_t lock_op;

// para leer el archivo de usuarios
struct args_struct_read_users {
    FILE *file;
    int start_line;
    int end_line;
    char* name_hilo;
    int number_hilo;
};

// para leer el archivo de operaciones
struct args_struct_read_operations {
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

// operaciones cargados por hilo
struct data_operations_loaded{
    int count_retiros;
    int count_depositos;
    int count_transacciones;
    int count_total_operaciones;
    int count_hilo1;
    int count_hilo2;
    int count_hilo3;
    int count_hilo4;
    int total_hilos;
};

// Errores en carga de usuarios
struct data_users_errors{
    int flag_read; // 1 = no esta vacio y poner en reporte, 0 = ignorar
    char error_tipo[100];
};

// Errores en carga de operaciones
struct data_operations_errors{
    int flag_read; // 1 = no esta vacio y poner en reporte, 0 = ignorar
    char error_tipo[100];
};

// donde estaran guardados todos los usuarios
struct data_users all_users[AMOUNT_USERS];

// Almacenar el conteo por hilo
struct data_users_loaded count_users_loaded_per_hilo[1];

// Almacenar el conteo por hilo
struct data_operations_loaded count_operations_loaded_per_hilo[1];

// Almacenar errores carga de usuarios
struct data_users_errors errors_load_users[AMOUNT_USERS];

// Almacenar errores carga de operaciones
struct data_operations_errors errors_load_operations[AMOUNT_OPERATIONS];

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

int count_operations(){

    FILE *fp;
    fp = fopen(NAME_FILE_OPERATIONS, "r"); // permisos de lectura

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

bool validate_only_number(char *token){

    while (*token != '\0') {
        // Verifica si el carácter actual no es un dígito
        if (!isdigit(*token)) {
            return true; // Si hay algún carácter que no es un dígito, no es un número entero
        }
        token++; // Mueve el puntero al siguiente carácter
    }

    return false;
}

bool validate_monto(char *token){

    while (*token != '\0') {
        // Verifica si el carácter actual no es un dígito
        if (!isdigit(*token) && *token != '.') {

            return true; // Si hay algún carácter que no es un dígito, o no es punto

        }
        token++; // Mueve el puntero al siguiente carácter
    }

    double valor = strtod(token, NULL);

    if(valor < 0){
        return true;
    }

    return false;
}

bool validate_account_number(char *token){

    int n = AMOUNT_USERS;
    int valor = atoi(token);

    for(int i = 0; i < n; i++){
        if(all_users[i].flag_read == 1){
            if(all_users[i].no_cuenta == valor){
                return true;
            }
        }        
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

bool validate_integer_positive_whole_type_operations(char *token){

    // validando que solo tenga numeros
    if(validate_only_number(token)){
        return true;
    }

    // validando que sea tipo de operacion 1, 2 o 3
    int valor = strtod(token, NULL);

    if(valor < 1 || valor > 3){
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

    printf("\nREPORTE GENERADO EXITOSAMENTE!\n");

    fclose(fp);

}

void generate_report_operations(){
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Formatear el nombre del archivo
    char nombreArchivo[100];
    char fechaActual[100];
    strftime(nombreArchivo, sizeof(nombreArchivo), "operaciones_%Y_%m_%d-%H_%M_%S.log", timeinfo);
    strftime(fechaActual, sizeof(fechaActual), "%Y-%m-%d %H:%M:%S", timeinfo);

    FILE *fp;
    fp = fopen(nombreArchivo, "w");

    fprintf(fp, "------------- RESUMEN DE OPERACIONES -------------\n\n");

    fprintf(fp, "FECHA: ");
    fprintf(fp, "%s\n\n", fechaActual);

    fprintf(fp, "OPERACIONES REALIZADAS:\n");
    fprintf(fp, "Retiros: %d\n", count_operations_loaded_per_hilo[0].count_retiros);
    fprintf(fp, "Depositos: %d\n", count_operations_loaded_per_hilo[0].count_depositos);
    fprintf(fp, "Transacciones: %d\n", count_operations_loaded_per_hilo[0].count_transacciones);
    fprintf(fp, "Total: %d\n\n", count_operations_loaded_per_hilo[0].count_total_operaciones);

    fprintf(fp, "OPERACIONES POR HILO:\n");
    fprintf(fp, "Hilo #1: %d\n", count_operations_loaded_per_hilo[0].count_hilo1);
    fprintf(fp, "Hilo #2: %d\n", count_operations_loaded_per_hilo[0].count_hilo2);
    fprintf(fp, "Hilo #3: %d\n", count_operations_loaded_per_hilo[0].count_hilo3);
    fprintf(fp, "Hilo #4: %d\n", count_operations_loaded_per_hilo[0].count_hilo4);
    fprintf(fp, "Total: %d\n\n", count_operations_loaded_per_hilo[0].total_hilos);

    fprintf(fp, "ERRORES:\n");

    int n = AMOUNT_OPERATIONS;
    for(int i = 0; i < n; i++){
        if(errors_load_operations[i].flag_read == 1){
            fprintf(fp, "%s", errors_load_operations[i].error_tipo);
        }
    }

    fclose(fp);
}

void make_a_deposit(){
    int n = AMOUNT_USERS;
    int num_cuenta_a_depositar;
    double monto_a_depositar;

    printf("\nIngrese el número de cuenta (a donde se hara el deposito): ");
    if(scanf("%d", &num_cuenta_a_depositar) == 1){

        printf("\nIngrese el monto a depositar: ");
        if(scanf("%lf", &monto_a_depositar) == 1){

            if(monto_a_depositar <= 0){
                printf("\nEL MONTO DEBE SER MAYOR A 0!\n");
                return;
            }

            for(int i = 0; i < n; i++){
                if(all_users[i].flag_read == 1){
                    if(all_users[i].no_cuenta == num_cuenta_a_depositar){
                        all_users[i].saldo = all_users[i].saldo + monto_a_depositar;
                        printf("\nDEPOSITO REALIZADO CON EXITO!\n");
                        return;
                    }
                }        
            }

            printf("\nNO EXISTE EL NUMERO DE CUENTA!\n");

        } else{
            printf("\nINGRESE UN MONTO VALIDO!\n");
            while (getchar() != '\n'); // limpiando buffer de entrada
            return;
        }

    } else{
        printf("\nINGRESE UN NUMERO DE CUENTA VALIDO!\n");
        while (getchar() != '\n'); // limpiando buffer de entrada
        return;
    }
}

void make_a_withdrawal(){
    int n = AMOUNT_USERS;
    int num_cuenta_a_depositar;
    double monto_a_retirar;

    printf("\nIngrese el número de cuenta (de donde se hara el retiro): ");
    if(scanf("%d", &num_cuenta_a_depositar) == 1){

        printf("\nIngrese el monto a retirar: ");
        if(scanf("%lf", &monto_a_retirar) == 1){

            if(monto_a_retirar <= 0){
                printf("\nEL MONTO DEBE SER MAYOR A 0!\n");
                return;
            }

            for(int i = 0; i < n; i++){
                if(all_users[i].flag_read == 1){
                    if(all_users[i].no_cuenta == num_cuenta_a_depositar){

                        if(all_users[i].saldo < monto_a_retirar){
                            printf("\nSALDO INSUFICIENTE PARA HACER EL RETIRO!\n");
                            return;
                        }

                        all_users[i].saldo = all_users[i].saldo - monto_a_retirar;
                        printf("\nRETIRO REALIZADO CON EXITO!\n");
                        return;
                    }
                }        
            }

            printf("\nNO EXISTE EL NUMERO DE CUENTA!\n");

        } else{
            printf("\nINGRESE UN MONTO VALIDO!\n");
            while (getchar() != '\n'); // limpiando buffer de entrada
            return;
        }

    } else{
        printf("\nINGRESE UN NUMERO DE CUENTA VALIDO!\n");
        while (getchar() != '\n'); // limpiando buffer de entrada
        return;
    }
}

void make_a_transaction(){
    int n = AMOUNT_USERS;
    int num_cuenta_a_retirar;
    int num_cuenta_a_depositar;
    double monto_a_depositar;

    printf("\nIngrese el número de cuenta (de donde se va a retirar): ");
    if(scanf("%d", &num_cuenta_a_retirar) == 1){

        printf("\nIngrese el número de cuenta (a donde se hara el deposito): ");
        if(scanf("%d", &num_cuenta_a_depositar) == 1){

            printf("\nIngrese el monto a depositar: ");
            if(scanf("%lf", &monto_a_depositar) == 1){

                if(monto_a_depositar <= 0){
                    printf("\nEL MONTO DEBE SER MAYOR A 0!\n");
                    return;
                }

                if(num_cuenta_a_retirar == num_cuenta_a_depositar){
                    printf("\nLA CUENTA DE RETIRO Y CUENTA DE DEPOSITO ES LA MISMA!\n");
                    return;
                }

                for(int i = 0; i < n; i++){ // buscando cuenta de retiro
                    if(all_users[i].flag_read == 1){
                        if(all_users[i].no_cuenta == num_cuenta_a_retirar){

                            for (int j = 0; j < n; j++){ // buscando cuenta a depositar
                                if(all_users[j].flag_read == 1){
                                    if(all_users[j].no_cuenta == num_cuenta_a_depositar){

                                        if(all_users[i].saldo < monto_a_depositar){
                                            printf("\nSALDO INSUFICIENTE PARA REALIZAR LA TRANSACCION!\n");
                                            return;
                                        }

                                        all_users[i].saldo = all_users[i].saldo - monto_a_depositar;
                                        all_users[j].saldo = all_users[j].saldo + monto_a_depositar;
                                        printf("\nTRANSACCION REALIZADA CON EXITO!\n");
                                        return;

                                    }
                                }
                            }
                        }
                    }        
                }

                printf("\nNO EXISTE NUMERO DE CUENTA DE RETIRO O DE DEPOSITO!\n");

            } else{
                printf("\nINGRESE UN MONTO VALIDO!\n");
                while (getchar() != '\n'); // limpiando buffer de entrada
                return;
            }

        } else{
            printf("\nINGRESE UN NUMERO DE CUENTA VALIDO!\n");
            while (getchar() != '\n'); // limpiando buffer de entrada
            return;
        }

    } else{
        printf("\nINGRESE UN NUMERO DE CUENTA VALIDO!\n");
        while (getchar() != '\n'); // limpiando buffer de entrada
        return;
    }
}

void check_account(){

    int n = AMOUNT_USERS;
    int num_cuenta_a_consultar;

    printf("\nIngrese el número de cuenta: ");

    if(scanf("%d", &num_cuenta_a_consultar) == 1){
        
        for(int i = 0; i < n; i++){
            if(all_users[i].flag_read == 1){
                if(all_users[i].no_cuenta == num_cuenta_a_consultar){
                    printf("\nDATOS DE CUENTA:\n");
                    printf("Numero de Cuenta: %d\nNombre: %s\nSaldo: %f\n", 
                        all_users[i].no_cuenta, all_users[i].nombre, all_users[i].saldo);

                    return;
                }
            }        
        }

    }else{
        printf("\nINGRESE UN NUMERO DE CUENTA VALIDO!\n");
        while (getchar() != '\n'); // limpiando buffer de entrada
        return;
    }

    printf("\nNO EXISTE EL NUMERO DE CUENTA!\n");
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

void* thread_read_operations(void* arg){
    //wait, bloquea todo lo que le sigue (algo como todos los procesos que le siguen), hasta que el mismo proceso lo libere
    pthread_mutex_lock(&lock_op);

    struct args_struct_read_operations *hilo_current = (struct args_struct_read_operations*)arg;
    char row[1000]; // buffer de linea
    char *type_operation; // no se sabe cuantos caracteres, sera puntero de caracteres
    char *cuenta1;
    char *cuenta2;
    char *monto;
    int len_monto;

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

        type_operation = strtok(row, ","); // permite leer una cadena hasta cierto caracter
        cuenta1 = strtok(NULL, ","); // null permite leer desde el punto donde se quedo anterior
        cuenta2 = strtok(NULL, ",");
        monto = strtok(NULL, ",");

        if(validate_integer_positive_whole_type_operations(type_operation)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Operacion no valida -> %s\n", (n+2), type_operation);
            n++;
            continue;
        }

        if(validate_only_number(cuenta1)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Numero de cuenta no es un numero entero positivo   -> %s\n", (n+2), cuenta1);
            n++;
            continue;
        }

        if(validate_only_number(cuenta2)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Numero de cuenta no es un numero entero positivo   -> %s\n", (n+2), cuenta2);
            n++;
            continue;
        }

        if(!validate_account_number(cuenta1)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Numero de cuenta no existe -> %s\n", (n+2), cuenta1);
            n++;
            continue;
        }

        if(!validate_account_number(cuenta2)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Numero de cuenta no existe -> %s\n", (n+2), cuenta2);
            n++;
            continue;
        }

        // para validar el monto, en vez de salto de linea al final se pone un valor null, si no toma todos los valores como incorrectos
        len_monto = strlen(monto);
        if (monto[len_monto - 1] == '\n') {
            monto[len_monto - 1] = '\0'; // Reemplazar el salto de línea por el caracter nulo
        }

        if(validate_monto(monto)){
            errors_load_operations[n].flag_read = 1;
            sprintf(errors_load_operations[n].error_tipo, "    - Linea #%d: Monto no es un numero o es menor a 0 -> %s\n", (n+2), monto);
            n++;
            continue;
        }

        if(atoi(type_operation) == 1){ // deposito

            count_operations_loaded_per_hilo[0].count_depositos++;

        }else if(atoi(type_operation) ==2){ // retiro

            count_operations_loaded_per_hilo[0].count_retiros++;

        }else{ // transaccion

            count_operations_loaded_per_hilo[0].count_transacciones++;

        }

        // aumenta el conteo segun hilo que se esta ejecutando
        if(hilo_current->number_hilo == 1){
            count_operations_loaded_per_hilo[0].count_hilo1++;
        } else if(hilo_current->number_hilo == 2){
            count_operations_loaded_per_hilo[0].count_hilo2++;
        } else if(hilo_current->number_hilo == 3){
            count_operations_loaded_per_hilo[0].count_hilo3++;
        } else{
            count_operations_loaded_per_hilo[0].count_hilo4++;
        }

        count_operations_loaded_per_hilo[0].count_total_operaciones++;
        count_operations_loaded_per_hilo[0].total_hilos++;
        n++;
    }    
  
    // sleep(1); 
      
    //signal 
    // printf("-----------------------Termino - %s\n", hilo_current->name_hilo); 

    pthread_mutex_unlock(&lock_op); // se libere, y se ejecuta el siguiente hilo
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

void read_operations(){
    int count_total = count_operations();
    
    // usuarios por hilo
    int count_hilo_1_2_and_3 = count_total / 4;
    int count_hilo_4 = count_total - (count_hilo_1_2_and_3 * 3);

    // printf("Cantidad: %d\n\n", count_total);
    // printf("Cantidad1-2-3: %d\n\n", count_hilo_1_2_and_3);
    // printf("Cantidad4: %d\n\n", count_hilo_4);

    pthread_mutex_init(&lock_op, NULL);  // Inicializamos nuestro mutex

    FILE *fp;
    fp = fopen(NAME_FILE_OPERATIONS, "r"); // permisos de lectura

    pthread_t t1,t2,t3,t4;

    struct args_struct_read_operations args_hilo1;
    args_hilo1.file = fp;
    args_hilo1.start_line = 0;
    args_hilo1.end_line = count_hilo_1_2_and_3 - 1;
    args_hilo1.name_hilo = "Hilo1";
    args_hilo1.number_hilo = 1;
    pthread_create(&t1, NULL, thread_read_operations, (void *)&args_hilo1); 

    struct args_struct_read_operations args_hilo2;
    args_hilo2.file = fp;
    args_hilo2.start_line = count_hilo_1_2_and_3;
    args_hilo2.end_line = (count_hilo_1_2_and_3 * 2) - 1 ;
    args_hilo2.name_hilo = "Hilo2";
    args_hilo2.number_hilo = 2;
    pthread_create(&t2, NULL, thread_read_operations, (void *)&args_hilo2); 

    struct args_struct_read_operations args_hilo3;
    args_hilo3.file = fp;
    args_hilo3.start_line = (count_hilo_1_2_and_3 * 2);
    args_hilo3.end_line = (count_hilo_1_2_and_3 * 3) - 1 ;
    args_hilo3.name_hilo = "Hilo3";
    args_hilo3.number_hilo = 3;
    pthread_create(&t3, NULL, thread_read_operations, (void *)&args_hilo3); 

    struct args_struct_read_operations args_hilo4;
    args_hilo4.file = fp;
    args_hilo4.start_line = (count_hilo_1_2_and_3 * 3);
    args_hilo4.end_line = count_total - 1;
    args_hilo4.name_hilo = "Hilo4";
    args_hilo4.number_hilo = 4;
    pthread_create(&t4, NULL, thread_read_operations, (void *)&args_hilo4); 

    pthread_join(t1,NULL); 
    pthread_join(t2,NULL); 
    pthread_join(t3,NULL);
    pthread_join(t4,NULL); 

    fclose(fp);
    pthread_mutex_destroy(&lock_op);  // Liberamos los recursos del semaforo

}

void menu(){
    int opcion;

    while (1) {  // Bucle infinito hasta que se elija "Salir"
        // Mostrar el menú
        printf("\n-------- Menú --------\n");
        printf("1. Deposito\n");
        printf("2. Retiro\n");
        printf("3. Transacción\n");
        printf("4. Consultar cuenta\n");
        printf("5. Carga masiva de operaciones\n");
        printf("6. Reporte estado de cuenta\n");
        printf("7. Salir\n");

        // Solicitar la selección de una opción al usuario
        printf("Ingrese el número de la opción deseada: ");
        if(scanf("%d", &opcion) == 1){

            // Manejar la opción seleccionada
            switch (opcion) {
                case 1:
                    make_a_deposit();
                    break;
                case 2:
                    make_a_withdrawal();
                    break;
                case 3:
                    make_a_transaction();
                    break;
                case 4:
                    check_account();
                    break;
                case 5:
                    read_operations();
                    generate_report_operations();
                    break;
                case 6:
                    generate_report_account_status();
                    break;
                case 7:
                    printf("\nSALIENDO DEL PROGRAMA. ¡HASTA LUEGO!\n");
                    return;  // Salir del programa
                default:
                    printf("\nOPCION NO VALIDA.\n");
            }

        }else{
            printf("\nOPCION NO VALIDA!\n");
            while (getchar() != '\n'); // limpiando buffer de entrada   
        }

    }
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
    // generate_report_users();
    // generate_report_account_status();
    menu();

    return 0;
}

// gcc main.c -lpthread -o main