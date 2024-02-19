# Manual Técnico - Practica 1

* Sistema operativo: Linux Mint 21

## parent.c

* En la linea 12, se abre el archivo practica1.txt el cual van a estar usando los procesos hijos, con los persmisos necesarios, el `O_TRUNC` es para que pueda eliminar todo el contenido que ya tenia el archivo.

```c
int fd = open("practica1.txt", O_RDWR | O_CREAT | O_TRUNC, 0777);
```

* El flujo general para la creacion de los hijos es el siguiente:

```c
int main(){

    // proceso hijo1
    pid_t pid1 = fork(); //Se realiza el fork
        
    if(pid1 == 0){ 

        // hijo 1

    }else{ // padre

        // proceso hijo2
        pid_t pid2 = fork();

        if(pid2 == 0){

            /// hijo 2

        }else{
            
            // padre

        }

    }

    return 0;
    
}
```

* Dentro de ambos hijos se tiene el siguiente codigo:
    * Son los argumentos que se van a pasar a los procesos hijos (.bin).
    * Como hay que enviar el archivo (fd) a los procesos hijos, este es un valor int, entonces se convierte a un string ya que solo deja enviar cadenas o caracteres y del lado del hijo ya se convierte nuevamente a int.
    * Por ultimo se ejecuta el proceso hijo con el comando execv.

```c
/*Se escriben los argumentos para el proceso hijo*/
char *arg_Ptr[3];
arg_Ptr[0] = "child1.c"; // nombre de ejecutable

// pasando el archivo por un argumento
char file_to_string[2];
sprintf(file_to_string, "%d", fd);

arg_Ptr[1] = file_to_string;
arg_Ptr[2] = NULL; // El ultimo indice de argv siempre debe de ser NULL

execv("/home/gudiel/USAC/SO2/SO2_201404278/Practica1/child1.bin", arg_Ptr);
```

* La siguiente porcion de codigo, en el proceso padre:
    * Con `waitpid` se espera que los procesos hijos terminen, pero como estos tienen un ciclo while infinito se utiliza `WNOHANG` que hace que el flujo siga, independientemente si los procesos hijos terminaron o no.
    * Luego se ejecuta el comando para monitoriar los procesos hijos con systemtap, escribimos el comando pasandole pod PID de los procesos hijos a monitorear y luego ejecutamos, todas las salidas seran escritas en el archivo `syscalls.log`.

```c
int status;
waitpid(pid1, &status, WNOHANG);
waitpid(pid2, &status, WNOHANG);

// comando systemtap
char command[100];
sprintf(command, "%s %d %d %s", "sudo stap trace.stp ", pid1, pid2, " > syscalls.log");
system(command);
```

* Por ultimo tenemos un while infinito, esto es para que la consola no termine su ejecucion y poder realizar el signal con ctrl+c correctamente y poder ver el reporte de todas las llamadas realizadas.

```c
 while(1){
        sleep(60);
    }
```

## child1.c y child2.c

* Metodo que recibe como argumento el archivo y se encarga de escribir en el mismo.

```c
void escribirEnArchivo(int fd){
    char buffer[LINE_LENGTH + 2]; // +2 para el carácter nulo y el carácter de nueva línea al final

    for (int i = 0; i < LINE_LENGTH; ++i) {
        srand(i);
        buffer[i] = 'A' + rand() % 26; // Genera un carácter aleatorio entre 'A' y 'Z'
    }

    buffer[LINE_LENGTH] = '\n'; // Añade un carácter de nueva línea al final
    buffer[LINE_LENGTH + 1] = '\0'; // Añade el carácter nulo al final

    write(fd, buffer, LINE_LENGTH + 1);
}
```

* Metodo que recibe como argumento el archivo y se encarga de leer del mismo.

```c
void leerDesdeArchivo(int fd){
    char buffer[LINE_LENGTH + 1];
    read(fd, buffer, LINE_LENGTH);
    buffer[LINE_LENGTH] = '\0'; // Añade el carácter nulo al final
}
```

* Metodo que recibe como argumento el archivo y se encarga de posicionar el puntero al inicio del archivo.

```c
void reposicionarAlPrincipio(int fd){
    lseek(fd, 0, SEEK_SET);
}
```

* En el archivo child2.c se encuentra un metodo para el mostrar el reporte de las llamadas.
    * Este metodo lo que hace es abrir el archivo `syscalls.log` y contar cuandas llamadas de tipo write, read y seek se hicieron, luego con printf() muestra los resultados en consola.

```c
void reporte_llamadas(){ }
```

* En el archivo child2.c tambien se encuentra el metodo de manejar de signal.
    * Este se ejecuta cuando se preciona Ctrl+c en la consola, llama al metodo para mostrar el reporte de las llamadas y luego finaliza su ejecucion.

```c
void manejador_sigint(){
    reporte_llamadas();
    exit(0);
}
```

* Funcion inicial.
    * El `time_t` es para poder usar el srand y rand.
    * Se convierte el archivo a int, ya que viene como una cadena de string.
    * Se ejecuta el while infinito que sera el encargado de que llamadas se van a estar ejecutando.

```c
int main(int argc, char *argv[]){

    time_t t;
    int fd = atoi(argv[1]);

    signal(SIGINT, manejador_sigint);

    while(1){
                
        srand(time(NULL));

        int llamada_a_realizar = 1 + rand() % 3;

        if(llamada_a_realizar == 1){
            escribirEnArchivo(fd);
        }else if(llamada_a_realizar == 2){
            leerDesdeArchivo(fd);
        }else {
            reposicionarAlPrincipio(fd);
        }

        // Tiempo aleatorio 1-3 segundos
        int segundos = 1 + rand() % 3;
        sleep(segundos);
    }

    return 0;

}
```

## trace.stp

* Script de systemtap, que es el encargado de estar monitoreando o interceptanto los procesos hijos.
    * Hay tres metodos: read, write, lseek.
    * Todos reciben como argumento los PID de los procesos hijos a monitorear.
    * Con `execname()` se obtiene el nombre del proceso.
    * Con `pid()` se obtiene el PID del proceso.
    * Con `name` se obtiene que tipo de lectura se esta haciendo, si es read, write o seek.
    * Con `ctime(gettimeofday_s())` se obtiene la fecha y hora actual en que se hace la llamada.

```c
#!/usr/bin/stap

# integer se usa $, si fuera string se usa @
probe syscall.read {
    if(pid() == $1 || pid() == $2){
        printf("%s %d: %s (%s)\n",execname(), pid(), name, ctime(gettimeofday_s())) # nombre proceso, pid, tipo lectura
    }
}

probe syscall.write {
    if(pid() == $1 || pid() == $2){
        printf("%s %d: %s (%s)\n",execname(), pid(), name, ctime(gettimeofday_s()))
    }
}

probe syscall.lseek {
    if(pid() == $1 || pid() == $2){
        printf("%s %d: %s (%s)\n",execname(), pid(), name, ctime(gettimeofday_s()))
    }
}
```

## Compilar

* Estar en la carpeta que contiene los archivos, este caso `Practica1`.
* Para ejecutar los procesos hijos:

```bash
gcc child1.c -o child1.bin
gcc child2.c -o child2.bin
```

* Ejecutar el proceso padre:

```bash
gcc parent.c -o parent
./parent
```

* Para systemtap instalar:

```bash
sudo apt install systemtap linux-headers-$(uname -r)

# para ejecutar con argumentos (PIDs):
sudo stap trace.stp argumento1 argumento2
```