# Manual Técnico - Practica 1

* Sistema operativo: Linux Ubuntu 22

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

## Funciones

* La funcion `count_users()` y `count_operations` se encargan de hacer un conteo sobre las lineas que tiene el CSV y retorna ese valor, para asi poder repartir el trabajo entre los hilos

* La funcion `validate_only_number` retorna un booleano, para saber si la cadena que se esta leyendo contiene solo numeros.
    * True = contiene valores que no son numeros.
    * False = contiene solo numeros.

* La funcion `validate_monto` retorna un booleano, valida el monto que se esta leyendo, solamente debe contener numeros o numeros y punto decimal, ademas de que sea mayor a 0.
    * True = es invalido.
    * False = es valido.

* La funcion `validate_account_number` retorna un booleano, valida que el numero de cuenta ya existe o no.
    * True = existe.
    * False = no existe.

* La funcion `validate_saldo` retorna un booleano, valida el que el saldo sea numerico o double, ademas de que sea mayor a 0.
    * True = existe.
    * False = no existe.

* La funcion `validate_integer_positive_whole_type_operations` retorna un booleano, usado para validar el tipo de operacion, valida que sea solo numeros y que este de 1-3.
    * True = es invalido.
    * False = es valido.

* La funcion `validate_transaccion` retorna un booleano, esta funcion vaida que los parametros para hacer una transaccion sean correctas, como validar numeros de cuentas, monto etc.
    * True = es invalido.
    * False = es valido.

```c

```

```c

```

```c

```

```c

```

