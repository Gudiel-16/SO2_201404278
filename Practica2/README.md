# Manual Técnico - Practica 2

* Sistema operativo: Linux Ubuntu 22.04

## Variables globales

* Variables definidas para los nombres de archivos.
    1. Nombre de archivo de carga masiva para usuarios.
    2. Nombre de archivo para carga masiva de operaciones o transacciones.
    3. Nombre de archivo para generar el reporte de estado de cuenta.

```c
#define NAME_FILE_USERS "prueba_usuarios.csv"
#define NAME_FILE_OPERATIONS "prueba_transacciones.csv"
#define NAME_FILE_ACCOUNT_STATUS "estado_de_cuenta.csv"
```

* Variables definidas para cantidad de usuarios y operaciones que se pueden guardar al hacer carga masiva.
* Variables para poder hacer el uso de multihilos con mutex.

```c
// cantidad de usuarios y errores que se pueden cargar
#define AMOUNT_USERS 300

// cantidad de operaciones y errores que se pueden cargar
#define AMOUNT_OPERATIONS 300

pthread_mutex_t lock;
pthread_mutex_t lock_op;
```

## Structs

* Utilizados al momento de leer el CSV.
    1. Archivo CSV el cual se esta leyendo.
    2. Linea en la cual empezara a cargar los datos.
    3. Linea en la cual terminara de cargar los datos.
    4. Un nombre de hilo que se esta ejecutnado.
    5. Numero de hilo que se esta ejecutando.

```c
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
```

* Contiene las propiedades de los usuarios cargados.
    1. Bandera, 1 = hay usuario en esa posicion, 0 = ignorar, por algun error en el momento de cargar, como numero de cuenta invalida.
    2. Numero de cuenta.
    3. Nombre del propietario.
    4. Saldo actual del propietario.

```c
// datos de usuario
struct data_users{
    int flag_read; // 1 = hay usuario en esa pocicion, 0 = ignorar
    int no_cuenta;
    char nombre[50];
    double saldo;
};
```

* Contiene la cantidad de datos cargados del CSV de usuarios por hilo.
* Contiene la cantidad de datos cargados del CSV de operaciones, para saber cuantas de cada tipo se hizo y cuantas cargas hizo cada hilo.

```c
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
```

* Contiene los errores al momento de hacer la carga masiva.
    1. Bandera, para saber si hay error en la posicion actual que se esta leyendo.
    2. Descripcion del error.

```c
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
```

* Array de structs, para poder guardar los datos y poder iterar en ellos.

```c
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
```

## Funciones de conteo

* La funcion `count_users()` y `count_operations` se encargan de hacer un conteo sobre las lineas que tiene el CSV y retorna ese valor, para asi poder repartir el trabajo entre los hilos

* Ambas funciones del mismo tienen la misma logica, cambia el nombre de archivo:

```c
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
```

## Funciones para validaciones

* La funcion `validate_only_number` retorna un booleano, para saber si la cadena que se esta leyendo contiene solo numeros.
    * True = contiene valores que no son numeros.
    * False = contiene solo numeros.

```c
bool validate_only_number(char *token){

    while (*token != '\0') {
        // Verifica si el carÃ¡cter actual no es un dÃ­gito
        if (!isdigit(*token)) {
            return true; // Si hay algÃºn carÃ¡cter que no es un dÃ­gito, no es un nÃºmero entero
        }
        token++; // Mueve el puntero al siguiente carÃ¡cter
    }

    return false;
}
```

* La funcion `validate_monto` retorna un booleano, valida el monto que se esta leyendo, solamente debe contener numeros o numeros y punto decimal, ademas de que sea mayor a 0.
    * True = es invalido.
    * False = es valido.

```c
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
```

* La funcion `validate_account_number` retorna un booleano, valida que el numero de cuenta ya existe o no.
    * True = existe.
    * False = no existe.

```c
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
```

* La funcion `validate_saldo` retorna un booleano, valida el que el saldo sea numerico o double, ademas de que sea mayor a 0.
    * True = existe.
    * False = no existe.

```c
bool validate_saldo(char *token){

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
```

* La funcion `validate_integer_positive_whole_type_operations` retorna un booleano, usado para validar el tipo de operacion, valida que sea solo numeros y que este de 1-3.
    * True = es invalido.
    * False = es valido.

```c
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
```

* La funcion `validate_transaccion` retorna un booleano, esta funcion valida que los parametros para hacer una transaccion sean correctas, como validar numeros de cuentas, monto, etc.
    * True = es invalido.
    * False = es valido.


```c
bool validate_transaccion(char *cuenta1, char *cuenta2, char *monto, int n){
}
```

* La funcion `validate_deposito_o_retiro` retorna un booleano, esta funcion valida que los parametros para hacer un deposito o retiro sean correctas, como validar numeros de cuenta, monto, etc.
    * True = es invalido.
    * False = es valido.

```c
bool validate_deposito_o_retiro(char *cuenta1, char *monto, int n){
}
```

## Metodos para reportes

* El metodo `generate_report_users`, se encarga de generar el reporte de los usuarios cargados, cuantos usuarios fueron cargados por hilo y los errores encontrados.

* El metodo `generate_report_operations`, se encarga de generar el reporte de las operaciones o transacciones cargadas, cuantas operaciones fueron cargadas por hilo, cuantos de cada tipo de operacion y los errores encontrados.

```c
// Para fecha
time_t rawtime;
struct tm *timeinfo;
time(&rawtime);
timeinfo = localtime(&rawtime);

// Formatear el nombre del archivo
char nombreArchivo[100];
char fechaActual[100];
strftime(nombreArchivo, sizeof(nombreArchivo), "carga_%Y_%m_%d-%H_%M_%S.log", timeinfo);
strftime(fechaActual, sizeof(fechaActual), "%Y-%m-%d %H:%M:%S", timeinfo);
```

* El metodo `generate_report_account_status`, se encarga de generar el reporte de estado de cuenta actual de todos los usuarios.

```c
void generate_report_account_status(){
    FILE *fp;
    fp = fopen(NAME_FILE_ACCOUNT_STATUS, "w"); // w para que lo trunque

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
```

## Metodos para operaciones individuales

* El metodo `make_a_deposit`, se encarga de realizar un deposito, con todas sus validaciones correspondientes.

* El metodo `make_a_withdrawal`, se encarga de realizar un retiro, con todas sus validaciones correspondientes.

* El metodo `make_a_transaction`, se encarga de realizar una transaccion, con todas sus validaciones correspondientes.

* El metodo `check_account`, se encarga de revisar el estado de cuenta de un usuario, con todas sus validaciones correspondientes.

```c
// esto es para que pida un valor, pero si no es del tipo que corresponde imprimir un error
if(scanf("%d", &num_cuenta_a_depositar) == 1)

// en dado caso lo anteior sea diferente de 0, limpiamos el buffer para que la consola no se quede bugueada
while (getchar() != '\n'); // limpiando buffer de entrada
```

## Lectura de archivos

* El metodo `thread_read_users` se encarga de ir leyendo linea por linea del CSV de usuarios e ir almacenando los datos, este metodo es ejecutado por cada hilo.

* El metodo `thread_read_operations` se encarga de ir leyendo linea por linea del CSV de operaciones e ir almacenando los datos, este metodo es ejecutado por cada hilo.

```c
//wait, bloquea todo lo que le sigue (algo como todos los procesos que le siguen), hasta que el mismo proceso lo libere
pthread_mutex_lock(&lock_op);

// recibe un struct como parametro, lo convertimos
struct args_struct_read_operations *hilo_current = (struct args_struct_read_operations*)arg;

// recorremos primero el archivo para mover el puntero, hasta la linea en donde tiene que empezar al hilo correspondiente, para ir guardando
for (int i = 0; i <= hilo_current->start_line; i++)
{
    fgets(row, 1000, hilo_current->file); // permite leer linea por linea un archivo
}

// recorrera hasta la linea en donde le toca terminar de guardar datos (end_line)
while (n <= hilo_current->end_line && feof(hilo_current->file) != true){
    // logica y validaciones
}

pthread_mutex_unlock(&lock_op); // se libere, y se ejecuta el siguiente hilo
```

* El metodo `read_users` se encarga de crear los hilos para lectura de archivo de usuarios y ejecutar los mismos.

* El metodo `read_operations` se encarga de crear los hilos para lectura de archivo de operaciones y ejecutar los mismos.

```c
int count_total = count_operations(); // cantidad de lineas que contiene el CSV
    
// operaciones por hilo
int count_hilo_1_2_and_3 = count_total / 4;
int count_hilo_4 = count_total - (count_hilo_1_2_and_3 * 3);

pthread_mutex_init(&lock_op, NULL);  // Inicializamos nuestro mutex

pthread_t t1,t2,t3,t4;

// esto espara cada uno de los hilos
struct args_struct_read_operations args_hilo1;
args_hilo1.file = fp;
args_hilo1.start_line = 0;
args_hilo1.end_line = count_hilo_1_2_and_3 - 1;
args_hilo1.name_hilo = "Hilo1";
args_hilo1.number_hilo = 1;
pthread_create(&t1, NULL, thread_read_operations, (void *)&args_hilo1); 

// al tener todos los hilos
pthread_join(t1,NULL); 
pthread_join(t2,NULL); 
pthread_join(t3,NULL);
pthread_join(t4,NULL); 

pthread_mutex_destroy(&lock_op);  // Liberamos los recursos
```

## Menu y Main

* Se iran llamadando a las funciones y metodos definidos anteriormente.

```c
void menu(){
    int opcion;

    while (1) {  // Bucle infinito hasta que se elija "Salir"
        // Mostrar el menú
        printf("\n------------- Menú --------------\n");
        printf("1. Deposito\n");
        printf("2. Retiro\n");
        printf("3. Transacción\n");
        printf("4. Consultar cuenta\n");
        printf("5. Carga masiva de usuarios\n");
        printf("6. Carga masiva de operaciones\n");
        printf("7. Reporte estado de cuenta\n");
        printf("8. Salir\n");

        // Solicitar la selección de una opción al usuario
        printf("\nIngrese el número de la opción deseada: ");
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
                    read_users();
                    generate_report_users();
                    break;
                case 6:
                    read_operations();
                    generate_report_operations();
                    break;
                case 7:
                    generate_report_account_status();
                    break;
                case 8:
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

    menu();

    return 0;
}
```