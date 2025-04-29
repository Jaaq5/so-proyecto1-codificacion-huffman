#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Declaraciones de funciones
void EjecutarHuffmanSerial(int argc, char *argv[]);
void EjecutarHuffmanPthread(int argc, char *argv[]);
void EjecutarHuffmanFork(int argc, char *argv[]);


int main() {
    int opcion;
    char dir_txt[1024];  // Para almacenar el directorio de entrada o archivo
    char archivo_salida[1024];  // Para almacenar el archivo de salida o directorio destino
    char *argv[4];  // Usaremos un arreglo para pasar los parámetros a las funciones

    // Solicitar al usuario que elija entre Huffman Serial o Pthread
    printf("Proyecto 1 - Comparación Huffman Serial vs Paralelo\n");

    // Ciclo para asegurar que el usuario elija 1 o 2
    do {
        printf("Seleccione una opción:\n");
        printf("1. Ejecutar Huffman Serial\n");
        printf("2. Ejecutar Huffman Pthread\n");
        printf("3. Ejecutar Huffman Fork\n");
        printf("Ingrese su opción (1 o 3): ");
        scanf("%d", &opcion);

        if (opcion != 1 && opcion != 2 && opcion != 3) {
            printf("\nOpción no válida. Por favor, elija 1 o 3.\n");
        }
    } while (opcion != 1 && opcion != 2 && opcion != 3);

    // Solicitar el directorio o archivo de entrada y archivo de salida/destino
    if (opcion == 1) {
        printf("\nEjecutando versión Serial...\n");
        printf("Ingrese el directorio de archivos de texto para comprimir/descomprimir: ");
        scanf("%s", dir_txt);
        printf("Ingrese el archivo de salida para comprimir o el directorio destino para descomprimir: ");
        scanf("%s", archivo_salida);

        // Preparar los argumentos para la función EjecutarHuffmanSerial
        argv[0] = "./huffman";  // El nombre del ejecutable
        argv[1] = (char *)malloc(2); // Para el flag (-c o -d)
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }
        argv[2] = dir_txt;
        argv[3] = archivo_salida;

        EjecutarHuffmanSerial(4, argv); // Llamar la función
    } else if (opcion == 2) {
        printf("\nEjecutando versión Pthread...\n");
        printf("Ingrese el directorio de archivos de texto para comprimir/descomprimir: ");
        scanf("%s", dir_txt);
        printf("Ingrese el archivo de salida para comprimir o el directorio destino para descomprimir: ");
        scanf("%s", archivo_salida);

        // Preparar los argumentos para la función EjecutarHuffmanPthread
        argv[0] = "./huffman";  // El nombre del ejecutable
        argv[1] = (char *)malloc(2); // Para el flag (-c o -d)
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }
        argv[2] = dir_txt;
        argv[3] = archivo_salida;

        EjecutarHuffmanPthread(4, argv); // Llamar la función

    } else if (opcion == 3) {
        printf("\nEjecutando versión Fork...\n");
        printf("Ingrese el directorio de archivos de texto para comprimir/descomprimir: ");
        scanf("%s", dir_txt);
        printf("Ingrese el archivo de salida para comprimir o el directorio destino para descomprimir: ");
        scanf("%s", archivo_salida);

        // Preparar los argumentos para la función EjecutarHuffmanPthread
        argv[0] = "./huffman";  // El nombre del ejecutable
        argv[1] = (char *)malloc(2); // Para el flag (-c o -d)
        if (archivo_salida[strlen(archivo_salida)-4] == '.') {
            argv[1] = "-c";  // Comprimir
        } else {
            argv[1] = "-d";  // Descomprimir
        }
        argv[2] = dir_txt;
        argv[3] = archivo_salida;

        EjecutarHuffmanFork(4, argv); // Llamar la función
    }

    return 0;
}
