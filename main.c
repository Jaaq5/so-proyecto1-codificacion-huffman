#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaraciones de funciones
void EjecutarHuffmanSerial(int argc, char *argv[]);
void EjecutarHuffmanPthread(int argc, char *argv[]);
void EjecutarHuffmanFork(int argc, char *argv[]);

int main() {
    int opcion;
    char dir_txt[1024];          // Para almacenar el directorio de entrada o archivo
    char archivo_salida[1024];   // Para almacenar el archivo de salida o directorio destino
    char *argv[4];               // Arreglo para pasar los parámetros a las funciones

    // Imprimir el mensaje de bienvenida y opciones al usuario
    printf("Proyecto 1 - Codificacion de Huffman\n");

    // Ciclo para asegurar que el usuario elija 1, 2 o 3
    do {
        printf("Seleccione una opción:\n");
        printf("1. Ejecutar Huffman Serial\n");
        printf("2. Ejecutar Huffman Pthread\n");
        printf("3. Ejecutar Huffman Fork\n");
        printf("Ingrese su opción (1 o 3): ");
        scanf("%d", &opcion);

        // Verificar que la opción sea válida
        if (opcion != 1 && opcion != 2 && opcion != 3) {
            printf("\nOpción no válida. Por favor, elija 1, 2 o 3.\n");
        }
    } while (opcion != 1 && opcion != 2 && opcion != 3);

    // Solicitar el directorio o archivo de entrada y el archivo de salida o directorio destino
    if (opcion == 1) {
        // Mensaje para indicar que se ejecutará la versión serial
        printf("\nEjecutando versión Serial...\n");
        printf("Para comprimir ingrese: <directorio_con_txt> (Ej:Top30Gutenberg)\n");
        printf("Para descomprimir ingrese: <archivo_comprimido_s.bin>\n");
        scanf("%s", dir_txt);  // Leer el directorio de archivos
        printf("Para comprimir ingrese: <archivo_comprimido_s.bin>\n");
        printf("Para descomprimir ingrese: <archivos_recuperados_s>\n");
        scanf("%s", archivo_salida);  // Leer el archivo de salida

        // Preparar los argumentos para la función EjecutarHuffmanSerial
        argv[0] = "./huffman";  // El nombre del ejecutable, se ignora cuando se juntaron los 3 en este main
        argv[1] = (char *)malloc(2); // Asignar espacio para el flag (-c o -d)

        // Determinar si se debe comprimir o descomprimir según la extensión del archivo de salida
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }

        argv[2] = dir_txt;         // Directorio de entrada
        argv[3] = archivo_salida;  // Archivo de salida

        // Llamar la función correspondiente para ejecutar Huffman en modo serial
        EjecutarHuffmanSerial(4, argv);

    } else if (opcion == 2) {
        // Mensaje para indicar que se ejecutará la versión con Pthreads
        printf("\nEjecutando versión Pthread...\n");
        printf("Para comprimir ingrese: <directorio_con_txt> (Ej:Top30Gutenberg)\n");
        printf("Para descomprimir ingrese: <archivo_comprimido_p.bin>\n");
        scanf("%s", dir_txt);  // Leer el directorio de archivos
        printf("Para comprimir ingrese: <archivo_comprimido_p.bin>\n");
        printf("Para descomprimir ingrese: archivos_recuperados_p\n");
        scanf("%s", archivo_salida);  // Leer el archivo de salida

        // Preparar los argumentos para la función EjecutarHuffmanPthread
        argv[0] = "./huffman";  // El nombre del ejecutable, se ignora cuando se juntaron los 3 en este main
        argv[1] = (char *)malloc(2); // Asignar espacio para el flag (-c o -d)

        // Determinar si se debe comprimir o descomprimir según la extensión del archivo de salida
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }

        argv[2] = dir_txt;  // Directorio de entrada
        argv[3] = archivo_salida;  // Archivo de salida

        // Llamar la función correspondiente para ejecutar Huffman usando Pthreads
        EjecutarHuffmanPthread(4, argv);

    } else if (opcion == 3) {
        // Mensaje para indicar que se ejecutará la versión con Fork
        printf("\nEjecutando versión Fork...\n");
        printf("Para comprimir ingrese: <directorio_con_txt> (Ej:Top30Gutenberg)\n");
        printf("Para descomprimir ingrese: <archivo_comprimido_f.bin>\n");
        scanf("%s", dir_txt);  // Leer el directorio de archivos
        printf("Para comprimir ingrese: <archivo_comprimido_f.bin>\n");
        printf("Para descomprimir ingrese: <archivos_recuperados_f>\n");
        scanf("%s", archivo_salida);  // Leer el archivo de salida

        // Preparar los argumentos para la función EjecutarHuffmanFork
        argv[0] = "./huffman";  // El nombre del ejecutable, se ignora cuando se juntaron los 3 en este main
        argv[1] = (char *)malloc(2); // Asignar espacio para el flag (-c o -d)

        // Determinar si se debe comprimir o descomprimir según la extensión del archivo de salida
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }

        argv[2] = dir_txt;         // Directorio de entrada
        argv[3] = archivo_salida;  // Archivo de salida

        // Llamar la función correspondiente para ejecutar Huffman usando Fork
        EjecutarHuffmanFork(4, argv);
    }

    return 0;
}
